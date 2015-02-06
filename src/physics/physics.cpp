#include	"physics.h"

#include	<PxPhysicsAPI.h>
#include	"../math/vec3f.h"
#include	"../mem.h"
#include	"../objects/cart.h"
#include	"../render/render.h"


using namespace physx;

#define eFRONT_LEFT_WHEEL	0
#define eFRONT_RIGHT_WHEEL	1
#define eREAR_LEFT_WHEEL	2
#define eREAR_RIGHT_WHEEL	3

#define TIRE_TYPE_WETS		0
#define	TIRE_TYPE_SLICKS	1

void physics_startup(struct physicsmanager* pm)
{
	PxTolerancesScale scale;

	// initialize foundation object
	pm->foundation = PxCreateFoundation(PX_PHYSICS_VERSION, pm->default_alloc, pm->default_error);
	
	// initialize the top-level physics object
	pm->sdk = PxCreateBasePhysics(PX_PHYSICS_VERSION, *pm->foundation, scale, false);

	// initialize mesh cooking object
	PxCookingParams params(scale);
	params.meshWeldTolerance = 0.01f;
	params.meshPreprocessParams = PxMeshPreprocessingFlags(PxMeshPreprocessingFlag::eWELD_VERTICES | PxMeshPreprocessingFlag::eREMOVE_UNREFERENCED_VERTICES | PxMeshPreprocessingFlag::eREMOVE_DUPLICATED_TRIANGLES);
	pm->cooking = PxCreateCooking(PX_PHYSICS_VERSION, *pm->foundation, params);

	// setup vehicle sdk
	PxInitVehicleSDK(*pm->sdk);
	PxVehicleSetBasisVectors(PxVec3(CART_UP), PxVec3(CART_FORWARD));
	PxVehicleSetUpdateMode(PxVehicleUpdateMode::eVELOCITY_CHANGE);

	// create default material
	pm->default_material = pm->sdk->createMaterial(0.5f, 0.5f, 0.1f);

	// create the scene for simulation
	PxSceneDesc scenedesc(scale);
	scenedesc.gravity = PxVec3(PHYSICS_DEFAULT_GRAVITY);
	scenedesc.cpuDispatcher = PxDefaultCpuDispatcherCreate(1);
	scenedesc.filterShader = PxDefaultSimulationFilterShader;
	pm->scene = pm->sdk->createScene(scenedesc);
}

void physics_update(struct physicsmanager* pm, float dt)
{
	pm->scene->simulate(dt);
	pm->scene->fetchResults(true);
}

void physics_shutdown(struct physicsmanager* pm)
{
	pm->scene->release();
	pm->default_material->release();

	PxCloseVehicleSDK();

	pm->cooking->release();

	pm->sdk->release();
	pm->foundation->release();
}


static void createVehicle4WSimulationData
(const PxF32 chassisMass, const PxF32 wheelMass, float wheelWidth, float wheelRadius, const PxVec3* wheelCentreOffsets,
 PxVehicleWheelsSimData& wheelsData, PxVehicleDriveSimData4W& driveData, PxVehicleChassisData& chassisData)
{
        //Extract the chassis AABB dimensions from the chassis convex mesh.
	const PxVec3 chassisDims (CART_WIDTH, CART_HEIGHT, CART_LENGTH);

        //The origin is at the center of the chassis mesh.
        //Set the center of mass to be below this point and a little towards the front.
        //const PxVec3 chassisCMOffset=PxVec3(0.0f,-chassisDims.y*0.5f+0.65f,0.25f);
		const PxVec3 chassisCMOffset=PxVec3(0.0f,0.0f,0.0f);

        //Now compute the chassis mass and moment of inertia.
        //Use the moment of inertia of a cuboid as an approximate value for the chassis moi.
        PxVec3 chassisMOI
                ((chassisDims.y*chassisDims.y + chassisDims.z*chassisDims.z)*chassisMass/12.0f,
                 (chassisDims.x*chassisDims.x + chassisDims.z*chassisDims.z)*chassisMass/12.0f,
                 (chassisDims.x*chassisDims.x + chassisDims.y*chassisDims.y)*chassisMass/12.0f);
        //A bit of tweaking here.  The car will have more responsive turning if we reduce the
        //y-component of the chassis moment of inertia.
        chassisMOI.y*=0.8f;

        //Let's set up the chassis data structure now.
        chassisData.mMass=chassisMass;
        chassisData.mMOI=chassisMOI;
        chassisData.mCMOffset=chassisCMOffset;

        //Compute the sprung masses of each suspension spring using a helper function.
        PxF32 suspSprungMasses[4];
        PxVehicleComputeSprungMasses(4,wheelCentreOffsets,chassisCMOffset,chassisMass,1,suspSprungMasses);

        //Extract the wheel radius and width from the wheel convex meshes.
        PxF32 wheelWidths[4];
		PxF32 wheelRadii[4];
		for (int i = 0; i<4; i++) {
			wheelWidths[i] = wheelWidth;
			wheelRadii[i] = wheelRadius;
		}

        //Now compute the wheel masses and inertias components around the axle's axis.
        //http://en.wikipedia.org/wiki/List_of_moments_of_inertia
        PxF32 wheelMOIs[4];
        for(PxU32 i=0;i<4;i++)
        {
                wheelMOIs[i]=0.5f*wheelMass*wheelRadii[i]*wheelRadii[i];
        }
        //Let's set up the wheel data structures now with radius, mass, and moi.
        PxVehicleWheelData wheels[4];
        for(PxU32 i=0;i<4;i++)
        {
                wheels[i].mRadius=wheelRadii[i];
                wheels[i].mMass=wheelMass;
                wheels[i].mMOI=wheelMOIs[i];
                wheels[i].mWidth=wheelWidths[i];
        }
        //Disable the handbrake from the front wheels and enable for the rear wheels
        wheels[eFRONT_LEFT_WHEEL].mMaxHandBrakeTorque=0.0f;
        wheels[eFRONT_RIGHT_WHEEL].mMaxHandBrakeTorque=0.0f;
        wheels[eREAR_LEFT_WHEEL].mMaxHandBrakeTorque=4000.0f;
        wheels[eREAR_RIGHT_WHEEL].mMaxHandBrakeTorque=4000.0f;
        //Enable steering for the front wheels and disable for the front wheels.
        wheels[eFRONT_LEFT_WHEEL].mMaxSteer=PxPi*0.3333f;
        wheels[eFRONT_RIGHT_WHEEL].mMaxSteer=PxPi*0.3333f;
        wheels[eREAR_LEFT_WHEEL].mMaxSteer=0.0f;
        wheels[eREAR_RIGHT_WHEEL].mMaxSteer=0.0f;

        //Let's set up the tire data structures now.
        //Put slicks on the front tires and wets on the rear tires.
        PxVehicleTireData tires[4];
        tires[eFRONT_LEFT_WHEEL].mType=TIRE_TYPE_SLICKS;
        tires[eFRONT_RIGHT_WHEEL].mType=TIRE_TYPE_SLICKS;
        tires[eREAR_LEFT_WHEEL].mType=TIRE_TYPE_WETS;
        tires[eREAR_RIGHT_WHEEL].mType=TIRE_TYPE_WETS;

        //Let's set up the suspension data structures now.
        PxVehicleSuspensionData susps[4];
        for(PxU32 i=0;i<4;i++)
        {
                susps[i].mMaxCompression=0.3f;
                susps[i].mMaxDroop=0.1f;
                susps[i].mSpringStrength=35000.0f;
                susps[i].mSpringDamperRate=4500.0f;
        }
        susps[eFRONT_LEFT_WHEEL].mSprungMass=suspSprungMasses[eFRONT_LEFT_WHEEL];
        susps[eFRONT_RIGHT_WHEEL].mSprungMass=suspSprungMasses[eFRONT_RIGHT_WHEEL];
        susps[eREAR_LEFT_WHEEL].mSprungMass=suspSprungMasses[eREAR_LEFT_WHEEL];
        susps[eREAR_RIGHT_WHEEL].mSprungMass=suspSprungMasses[eREAR_RIGHT_WHEEL];

        //Set up the camber.
        //Remember that the left and right wheels need opposite camber so that the car preserves symmetry about the forward direction.
        //Set the camber to 0.0f when the spring is neither compressed or elongated.
        const PxF32 camberAngleAtRest=0.0;
        susps[eFRONT_LEFT_WHEEL].mCamberAtRest=camberAngleAtRest;
        susps[eFRONT_RIGHT_WHEEL].mCamberAtRest=-camberAngleAtRest;
        susps[eREAR_LEFT_WHEEL].mCamberAtRest=camberAngleAtRest;
        susps[eREAR_RIGHT_WHEEL].mCamberAtRest=-camberAngleAtRest;
        //Set the wheels to camber inwards at maximum droop (the left and right wheels almost form a V shape)
        const PxF32 camberAngleAtMaxDroop=0.1f;
        susps[eFRONT_LEFT_WHEEL].mCamberAtMaxDroop=camberAngleAtMaxDroop;
        susps[eFRONT_RIGHT_WHEEL].mCamberAtMaxDroop=-camberAngleAtMaxDroop;
        susps[eREAR_LEFT_WHEEL].mCamberAtMaxDroop=camberAngleAtMaxDroop;
        susps[eREAR_RIGHT_WHEEL].mCamberAtMaxDroop=-camberAngleAtMaxDroop;
        //Set the wheels to camber outwards at maximum compression (the left and right wheels almost form a A shape).
        const PxF32 camberAngleAtMaxCompression=-0.1f;
        susps[eFRONT_LEFT_WHEEL].mCamberAtMaxCompression=camberAngleAtMaxCompression;
        susps[eFRONT_RIGHT_WHEEL].mCamberAtMaxCompression=-camberAngleAtMaxCompression;
        susps[eREAR_LEFT_WHEEL].mCamberAtMaxCompression=camberAngleAtMaxCompression;
        susps[eREAR_RIGHT_WHEEL].mCamberAtMaxCompression=-camberAngleAtMaxCompression;

        //We need to set up geometry data for the suspension, wheels, and tires.
        //We already know the wheel centers described as offsets from the actor center and the center of mass offset from actor center.
        //From here we can approximate application points for the tire and suspension forces.
        //Lets assume that the suspension travel directions are absolutely vertical.
        //Also assume that we apply the tire and suspension forces 30cm below the center of mass.
        PxVec3 suspTravelDirections[4]={PxVec3(0,-1,0),PxVec3(0,-1,0),PxVec3(0,-1,0),PxVec3(0,-1,0)};
        PxVec3 wheelCentreCMOffsets[4];
        PxVec3 suspForceAppCMOffsets[4];
        PxVec3 tireForceAppCMOffsets[4];
        for(PxU32 i=0;i<4;i++)
        {
                wheelCentreCMOffsets[i]=wheelCentreOffsets[i]-chassisCMOffset;
                suspForceAppCMOffsets[i]=PxVec3(wheelCentreCMOffsets[i].x,-0.3f,wheelCentreCMOffsets[i].z);
                tireForceAppCMOffsets[i]=PxVec3(wheelCentreCMOffsets[i].x,-0.3f,wheelCentreCMOffsets[i].z);
        }

        //Now add the wheel, tire and suspension data.
        for(PxU32 i=0;i<4;i++)
        {
                wheelsData.setWheelData(i,wheels[i]);
                wheelsData.setTireData(i,tires[i]);
                wheelsData.setSuspensionData(i,susps[i]);
                wheelsData.setSuspTravelDirection(i,suspTravelDirections[i]);
                wheelsData.setWheelCentreOffset(i,wheelCentreCMOffsets[i]);
                wheelsData.setSuspForceAppPointOffset(i,suspForceAppCMOffsets[i]);
                wheelsData.setTireForceAppPointOffset(i,tireForceAppCMOffsets[i]);
        }

        //Set the car to perform 3 sub-steps when it moves with a forwards speed of less than 5.0
        //and with a single step when it moves at speed greater than or equal to 5.0.
        wheelsData.setSubStepCount(5.0f, 3, 1);


        //Now set up the differential, engine, gears, clutch, and ackermann steering.

        //Diff
        PxVehicleDifferential4WData diff;
        diff.mType=PxVehicleDifferential4WData::eDIFF_TYPE_LS_4WD;
        driveData.setDiffData(diff);

        //Engine
        PxVehicleEngineData engine;
        engine.mPeakTorque=500.0f;
        engine.mMaxOmega=600.0f;//approx 6000 rpm
        driveData.setEngineData(engine);

        //Gears
        PxVehicleGearsData gears;
        gears.mSwitchTime=0.5f;
        driveData.setGearsData(gears);

        //Clutch
        PxVehicleClutchData clutch;
        clutch.mStrength=10.0f;
        driveData.setClutchData(clutch);

        //Ackermann steer accuracy
        PxVehicleAckermannGeometryData ackermann;
        ackermann.mAccuracy=1.0f;
        ackermann.mAxleSeparation=wheelCentreOffsets[eFRONT_LEFT_WHEEL].z-wheelCentreOffsets[eREAR_LEFT_WHEEL].z;
        ackermann.mFrontWidth=wheelCentreOffsets[eFRONT_RIGHT_WHEEL].x-wheelCentreOffsets[eFRONT_LEFT_WHEEL].x;
        ackermann.mRearWidth=wheelCentreOffsets[eREAR_RIGHT_WHEEL].x-wheelCentreOffsets[eREAR_LEFT_WHEEL].x;
        driveData.setAckermannGeometryData(ackermann);
}

//static void create_cart_data(float chassis_mass, PxConvexMesh* chassis_mesh

PxRigidDynamic* physics_addcart(struct physicsmanager* pm, vec3f pos)
{
	PxVehicleWheelsSimData* wheelsSimData=PxVehicleWheelsSimData::allocate(4);
	PxVehicleDriveSimData4W driveSimData;
	PxVehicleChassisData chassisData;

	PxVec3 wheelOffsets[4];
	wheelOffsets[0] = PxVec3(-CART_WIDTH / 2, -CART_HEIGHT / 2, - CART_LENGTH / 2);
	wheelOffsets[1] = PxVec3(CART_WIDTH / 2, -CART_HEIGHT / 2, - CART_LENGTH / 2);
	wheelOffsets[2] = PxVec3(-CART_WIDTH / 2, -CART_HEIGHT / 2, CART_LENGTH / 2);
	wheelOffsets[3] = PxVec3(CART_WIDTH / 2, -CART_HEIGHT / 2,  CART_LENGTH / 2);

	PxRigidDynamic* cart;

	createVehicle4WSimulationData (1500.0, 20.0, .4, .5, wheelOffsets, *wheelsSimData, driveSimData, chassisData);

	/*static void createVehicle4WSimulationData
(const PxF32 chassisMass, const PxF32 wheelMass, float wheelWidth, float wheelRadius, const PxVec3* wheelCentreOffsets,
 PxVehicleWheelsSimData& wheelsData, PxVehicleDriveSimData4W& driveData, PxVehicleChassisData& chassisData)*/

	//cart = 

	(void)pm;
	(void)pos;

	return cart;
}

PxRigidDynamic* physics_adddynamic_box(struct physicsmanager* pm, vec3f pos, vec3f dim)
{
	PxRigidDynamic* actor;

	actor = PxCreateDynamic(*pm->sdk, PxTransform(pos[VX], pos[VY], pos[VZ]), PxBoxGeometry(dim[VX], dim[VY], dim[VZ]), *pm->default_material, 1.f);
	pm->scene->addActor(*actor);

	return actor;
}

void physics_addstatic_trianglestrip(struct physicsmanager* pm, unsigned num_verts, unsigned stride, float* buf_verts)
{
	PxDefaultMemoryOutputStream writebuf;
	PxTriangleMeshDesc meshdesc;
	PxTriangleMesh* mesh;
	PxRigidStatic* obj;
	unsigned* indices, * ptr;
	unsigned i;

	// allocate index buffer
	indices = (unsigned*)mem_alloc(sizeof(int) * 3 * (num_verts - 2));

	// populate index buffer
	ptr = indices;
	for (i = 0; i < num_verts-2; i++)
	{
		*(ptr++) = i;
		if (i&1)
		{
			*(ptr++) = i+2;
			*(ptr++) = i+1;
		} else
		{
			*(ptr++) = i+1;
			*(ptr++) = i+2;
		}
	}

	// initialize mesh description
	meshdesc.points.count = num_verts;
	meshdesc.points.stride = stride;
	meshdesc.points.data = buf_verts;
	meshdesc.triangles.count = num_verts - 2;
	meshdesc.triangles.stride = 3 * sizeof(int);
	meshdesc.triangles.data = indices;

	// cook mesh
	pm->cooking->cookTriangleMesh(meshdesc, writebuf);

	PxDefaultMemoryInputData readbuf(writebuf.getData(), writebuf.getSize());
	mesh = pm->sdk->createTriangleMesh(readbuf);

	obj = PxCreateStatic(*pm->sdk, PxTransform(0.f, 0.f, 0.f), PxTriangleMeshGeometry(mesh), *pm->default_material);
	pm->scene->addActor(*obj);

	mem_free(indices);
}