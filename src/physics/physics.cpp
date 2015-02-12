#include	"physics.h"

#include	<PxPhysicsAPI.h>
#include	"../math/vec3f.h"
#include	"../mem.h"
#include	"../objects/cart.h"
#include	"../render/render.h"



//Collision types and flags describing collision interactions of each collision type.
enum
{
	COLLISION_FLAG_GROUND			=	1 << 0,
	COLLISION_FLAG_WHEEL			=	1 << 1,
	COLLISION_FLAG_CHASSIS			=	1 << 2,
	COLLISION_FLAG_OBSTACLE			=	1 << 3,
	COLLISION_FLAG_DRIVABLE_OBSTACLE=	1 << 4,

	COLLISION_FLAG_GROUND_AGAINST	=															COLLISION_FLAG_CHASSIS | COLLISION_FLAG_OBSTACLE | COLLISION_FLAG_DRIVABLE_OBSTACLE,
	COLLISION_FLAG_WHEEL_AGAINST	=									COLLISION_FLAG_WHEEL |	COLLISION_FLAG_CHASSIS | COLLISION_FLAG_OBSTACLE,
	COLLISION_FLAG_CHASSIS_AGAINST	=			COLLISION_FLAG_GROUND | COLLISION_FLAG_WHEEL |	COLLISION_FLAG_CHASSIS | COLLISION_FLAG_OBSTACLE | COLLISION_FLAG_DRIVABLE_OBSTACLE,
	COLLISION_FLAG_OBSTACLE_AGAINST	=			COLLISION_FLAG_GROUND | COLLISION_FLAG_WHEEL |	COLLISION_FLAG_CHASSIS | COLLISION_FLAG_OBSTACLE | COLLISION_FLAG_DRIVABLE_OBSTACLE,
	COLLISION_FLAG_DRIVABLE_OBSTACLE_AGAINST=	COLLISION_FLAG_GROUND 						 |	COLLISION_FLAG_CHASSIS | COLLISION_FLAG_OBSTACLE | COLLISION_FLAG_DRIVABLE_OBSTACLE,
};

using namespace physx;

PxVehicleDrive4W*		gVehicle4W		= NULL;

PxMaterial*				gMaterial	= NULL;

PxVehicleDrivableSurfaceType	mVehicleDrivableSurfaceTypes[MAX_NUM_INDEX_BUFFERS];
const PxMaterial*						mStandardMaterials[MAX_NUM_INDEX_BUFFERS];


//Tire model friction for each combination of drivable surface type and tire type.
static PxF32 gTireFrictionMultipliers[MAX_NUM_SURFACE_TYPES][MAX_NUM_TIRE_TYPES]=
{
        //WETS  SLICKS  ICE             MUD
        {0.95f, 0.95f,  0.95f,  0.95f},         //MUD
        {1.10f, 1.15f,  1.10f,  1.10f},         //TARMAC
        {0.70f, 0.70f,  0.70f,  0.70f},         //ICE
        {0.80f, 0.80f,  0.80f,  0.80f}          //GRASS
};

static PxSceneQueryHitType::Enum VehicleWheelRaycastPreFilter(	
	PxFilterData filterData0, 
	PxFilterData filterData1,
	const void* constantBlock, PxU32 constantBlockSize,
	PxSceneQueryFlags& queryFlags)
{
	//filterData0 is the vehicle suspension raycast.
	//filterData1 is the shape potentially hit by the raycast.
	PX_UNUSED(queryFlags);
	PX_UNUSED(constantBlockSize);
	PX_UNUSED(constantBlock);
	PX_UNUSED(filterData0);
	return ((0 == (filterData1.word3 & SAMPLEVEHICLE_DRIVABLE_SURFACE)) ? PxSceneQueryHitType::eNONE : PxSceneQueryHitType::eBLOCK);
}
/*	start up the physics manager
	param:	pm				physics manager (modified)
*/

struct VehicleDesc
{
	PxF32 chassisMass;
	PxVec3 chassisDims;
	PxVec3 chassisMOI;
	PxVec3 chassisCMOffset;
	PxMaterial* chassisMaterial;
	PxF32 wheelMass;
	PxF32 wheelWidth;
	PxF32 wheelRadius;
	PxF32 wheelMOI;
	PxMaterial* wheelMaterial;
	PxU32 numWheels;
};

VehicleDesc initVehicleDesc()
{
	//Set up the chassis mass, dimensions, moment of inertia, and center of mass offset.
	//The moment of inertia is just the moment of inertia of a cuboid but modified for easier steering.
	//Center of mass offset is 0.65m above the base of the chassis and 0.25m towards the front.
	const PxF32 chassisMass = 1500.0f;
	const PxVec3 chassisDims(2.5f,2.0f,5.0f);
	const PxVec3 chassisMOI
		((chassisDims.y*chassisDims.y + chassisDims.z*chassisDims.z)*chassisMass/12.0f,
		 (chassisDims.x*chassisDims.x + chassisDims.z*chassisDims.z)*0.8f*chassisMass/12.0f,
		 (chassisDims.x*chassisDims.x + chassisDims.y*chassisDims.y)*chassisMass/12.0f);
	const PxVec3 chassisCMOffset(0.0f, -chassisDims.y*0.5f + 0.65f, 0.25f);

	//Set up the wheel mass, radius, width, moment of inertia, and number of wheels.
	//Moment of inertia is just the moment of inertia of a cylinder.
	const PxF32 wheelMass = 20.0f;
	const PxF32 wheelRadius = 0.5f;
	const PxF32 wheelWidth = 0.4f;
	const PxF32 wheelMOI = 0.5f*wheelMass*wheelRadius*wheelRadius;
	const PxU32 nbWheels = 6;

	VehicleDesc vehicleDesc;
	vehicleDesc.chassisMass = chassisMass;
	vehicleDesc.chassisDims = chassisDims;
	vehicleDesc.chassisMOI = chassisMOI;
	vehicleDesc.chassisCMOffset = chassisCMOffset;
	vehicleDesc.chassisMaterial = gMaterial;
	vehicleDesc.wheelMass = wheelMass;
	vehicleDesc.wheelRadius = wheelRadius;
	vehicleDesc.wheelWidth = wheelWidth;
	vehicleDesc.wheelMOI = wheelMOI;
	vehicleDesc.numWheels = nbWheels;
	vehicleDesc.wheelMaterial = gMaterial;
	return vehicleDesc;
}

void createStandardMaterials(struct physicsmanager* pm)
{
	const PxF32 restitutions[MAX_NUM_SURFACE_TYPES] = {0.2f, 0.2f, 0.2f, 0.2f};
	const PxF32 staticFrictions[MAX_NUM_SURFACE_TYPES] = {0.5f, 0.5f, 0.5f, 0.5f};
	const PxF32 dynamicFrictions[MAX_NUM_SURFACE_TYPES] = {0.5f, 0.5f, 0.5f, 0.5f};

	for(PxU32 i=0;i<MAX_NUM_SURFACE_TYPES;i++) 
	{
		////Create a new material.
		mStandardMaterials[i] = pm->sdk->createMaterial(staticFrictions[i], dynamicFrictions[i], restitutions[i]);
		//if(!mStandardMaterials[i])
		//{
		//	getSampleErrorCallback().reportError(PxErrorCode::eINTERNAL_ERROR, "createMaterial failed", __FILE__, __LINE__);
		//}

		//Set up the drivable surface type that will be used for the new material.
		mVehicleDrivableSurfaceTypes[i].mType = i;
	}

	/*mChassisMaterialDrivable = getPhysics().createMaterial(0.0f, 0.0f, 0.0f);
	if(!mChassisMaterialDrivable)
	{
		getSampleErrorCallback().reportError(PxErrorCode::eINTERNAL_ERROR, "createMaterial failed", __FILE__, __LINE__);
	}

	mChassisMaterialNonDrivable = getPhysics().createMaterial(1.0f, 1.0f, 0.0f);
	if(!mChassisMaterialNonDrivable)
	{
		getSampleErrorCallback().reportError(PxErrorCode::eINTERNAL_ERROR, "createMaterial failed", __FILE__, __LINE__);
	}*/
}
PxFilterFlags VehicleFilterShader(	
	PxFilterObjectAttributes attributes0, PxFilterData filterData0, 
	PxFilterObjectAttributes attributes1, PxFilterData filterData1,
	PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
{
	PX_UNUSED(constantBlock);
	PX_UNUSED(constantBlockSize);

	// let triggers through
	if(PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1))
	{
		pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
		return PxFilterFlags();
	}



	// use a group-based mechanism for all other pairs:
	// - Objects within the default group (mask 0) always collide
	// - By default, objects of the default group do not collide
	//   with any other group. If they should collide with another
	//   group then this can only be specified through the filter
	//   data of the default group objects (objects of a different
	//   group can not choose to do so)
	// - For objects that are not in the default group, a bitmask
	//   is used to define the groups they should collide with
	if ((filterData0.word0 != 0 || filterData1.word0 != 0) &&
		!(filterData0.word0&filterData1.word1 || filterData1.word0&filterData0.word1))
		return PxFilterFlag::eSUPPRESS;

	pairFlags = PxPairFlag::eCONTACT_DEFAULT;

	// The pairFlags for each object are stored in word2 of the filter data. Combine them.
	pairFlags |= PxPairFlags(PxU16(filterData0.word2 | filterData1.word2));
	return PxFilterFlags();
}
void customizeSceneDesc(PxSceneDesc& sceneDesc)
{
	sceneDesc.filterShader	= VehicleFilterShader;
	//sceneDesc.flags			|= PxSceneFlag::eREQUIRE_RW_LOCK;
}
void physicsmanager_startup(struct physicsmanager* pm)
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
	customizeSceneDesc(scenedesc);
	pm->scene = pm->sdk->createScene(scenedesc);

	gMaterial = pm->sdk->createMaterial(0.5f, 0.5f, 0.6f);

	pm->mSurfaceTirePairs=PxVehicleDrivableSurfaceToTireFrictionPairs::allocate(MAX_NUM_TIRE_TYPES,MAX_NUM_SURFACE_TYPES);
	pm->mSurfaceTirePairs->setup(MAX_NUM_TIRE_TYPES,MAX_NUM_SURFACE_TYPES,mStandardMaterials,mVehicleDrivableSurfaceTypes);

	for(PxU32 i=0;i<MAX_NUM_SURFACE_TYPES;i++)
	{
        for(PxU32 j=0;j<MAX_NUM_TIRE_TYPES;j++)
        {
                pm->mSurfaceTirePairs->setTypePairFriction(i,j,gTireFrictionMultipliers[i][j]);
        }
	}


}
void VehicleSetupDrivableShapeQueryFilterData(PxFilterData* qryFilterData)
{
	qryFilterData->word3 = (PxU32)SAMPLEVEHICLE_DRIVABLE_SURFACE;
}
void VehicleSetupNonDrivableShapeQueryFilterData(PxFilterData* qryFilterData)
{
	qryFilterData->word3 = (PxU32)SAMPLEVEHICLE_UNDRIVABLE_SURFACE;
}

/*	shut down the physics manager
	param:	pm				physics manager (modified)
*/
void physicsmanager_shutdown(struct physicsmanager* pm)
{
	pm->scene->release();
	pm->default_material->release();

	PxCloseVehicleSDK();

	gMaterial->release();
	pm->cooking->release();
	pm->sdk->release();
	pm->foundation->release();
}





/*	update the physics simulation
	param:	pm				physics manager
	param:	dt				delta time
*/
void physicsmanager_update(struct physicsmanager* pm, float dt)
{
	pm->scene->simulate(dt);
	pm->scene->fetchResults(true);
}

static PxConvexMesh* createConvexMesh(const PxVec3* verts, const PxU32 numVerts, PxPhysics& physics, PxCooking& cooking)
{
	// Create descriptor for convex mesh
	PxConvexMeshDesc convexDesc;
	convexDesc.points.count			= numVerts;
	convexDesc.points.stride		= sizeof(PxVec3);
	convexDesc.points.data			= verts;
	convexDesc.flags				= PxConvexFlag::eCOMPUTE_CONVEX | PxConvexFlag::eINFLATE_CONVEX;

	PxConvexMesh* convexMesh = NULL;
	PxDefaultMemoryOutputStream buf;
	if(cooking.cookConvexMesh(convexDesc, buf))
	{
		PxDefaultMemoryInputData id(buf.getData(), buf.getSize());
		convexMesh = physics.createConvexMesh(id);
	}

	return convexMesh;
}

PxConvexMesh* createChassisMesh(const PxVec3 dims, PxPhysics& physics, PxCooking& cooking)
{
	const PxF32 x = dims.x*0.5f;
	const PxF32 y = dims.y*0.5f;
	const PxF32 z = dims.z*0.5f;
	PxVec3 verts[8] =
	{
		PxVec3(x,y,-z), 
		PxVec3(x,y,z),
		PxVec3(x,-y,z),
		PxVec3(x,-y,-z),
		PxVec3(-x,y,-z), 
		PxVec3(-x,y,z),
		PxVec3(-x,-y,z),
		PxVec3(-x,-y,-z)
	};

	return createConvexMesh(verts,8,physics,cooking);
}

PxConvexMesh* createWheelMesh(const PxF32 width, const PxF32 radius, PxPhysics& physics, PxCooking& cooking)
{
	PxVec3 points[2*16];
	for(PxU32 i = 0; i < 16; i++)
	{
		const PxF32 cosTheta = PxCos(i*PxPi*2.0f/16.0f);
		const PxF32 sinTheta = PxSin(i*PxPi*2.0f/16.0f);
		const PxF32 y = radius*cosTheta;
		const PxF32 z = radius*sinTheta;
		points[2*i+0] = PxVec3(-width/2.0f, y, z);
		points[2*i+1] = PxVec3(+width/2.0f, y, z);
	}

	return createConvexMesh(points,32,physics,cooking);
}

void createVehicle4WSimulationData
(const PxF32 chassisMass, VehicleDesc vehicleDesc,
 const PxF32 wheelMass, PxConvexMesh** wheelConvexMeshes, const PxVec3* wheelCentreOffsets,
 PxVehicleWheelsSimData& wheelsData, PxVehicleDriveSimData4W& driveData, PxVehicleChassisData& chassisData)
{
        //Extract the chassis AABB dimensions from the chassis convex mesh.
	const PxVec3 chassisDims=vehicleDesc.chassisDims;

        //The origin is at the center of the chassis mesh.
        //Set the center of mass to be below this point and a little towards the front.
        const PxVec3 chassisCMOffset=PxVec3(0.0f,-chassisDims.y*0.5f+0.65f,0.25f);

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
        for (int i = 0; i < 4; i++) {
			wheelWidths[i] = vehicleDesc.wheelWidth;
			wheelRadii[i] = vehicleDesc.wheelRadius;
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
        ackermann.mAxleSeparation=CART_LENGTH;
        //ackermann.mAxleSeparation=wheelCentreOffsets[eFRONT_LEFT_WHEEL].z-wheelCentreOffsets[eREAR_LEFT_WHEEL].z;
        ackermann.mFrontWidth=wheelCentreOffsets[eFRONT_RIGHT_WHEEL].x-wheelCentreOffsets[eFRONT_LEFT_WHEEL].x;
        ackermann.mRearWidth=wheelCentreOffsets[eREAR_RIGHT_WHEEL].x-wheelCentreOffsets[eREAR_LEFT_WHEEL].x;
        driveData.setAckermannGeometryData(ackermann);
}

void setupActor
(PxRigidDynamic* vehActor, 
 const PxFilterData& vehQryFilterData,
 const PxGeometry** wheelGeometries, const PxTransform* wheelLocalPoses, const PxU32 numWheelGeometries, const PxMaterial* wheelMaterial, const PxFilterData& wheelCollFilterData,
 const PxGeometry** chassisGeometries, const PxTransform* chassisLocalPoses, const PxU32 numChassisGeometries, const PxMaterial* chassisMaterial, const PxFilterData& chassisCollFilterData,
 const PxVehicleChassisData& chassisData,
 PxPhysics* physics,vec3f pos)
{
	//Add all the wheel shapes to the actor.
	for(PxU32 i=0;i<numWheelGeometries;i++)
	{
		PxShape* wheelShape=vehActor->createShape(*wheelGeometries[i],*wheelMaterial);
		wheelShape->setQueryFilterData(vehQryFilterData);
		wheelShape->setSimulationFilterData(wheelCollFilterData);
		wheelShape->setLocalPose(wheelLocalPoses[i]);
	}

	//Add the chassis shapes to the actor.
	for(PxU32 i=0;i<numChassisGeometries;i++)
	{
		PxShape* chassisShape=vehActor->createShape(*chassisGeometries[i],*chassisMaterial);
		chassisShape->setQueryFilterData(vehQryFilterData);
		chassisShape->setSimulationFilterData(chassisCollFilterData);
		chassisShape->setLocalPose(chassisLocalPoses[i]);
	}

	vehActor->setMass(chassisData.mMass);
	vehActor->setMassSpaceInertiaTensor(chassisData.mMOI);
	vehActor->setCMassLocalPose(PxTransform(chassisData.mCMOffset, PxQuat::createIdentity()));
	//vehActor->setCMassLocalPose(PxTransform(pos[VX], pos[VY], pos[VZ]));
}

PxRigidDynamic* createVehicleActor4W
(const PxVehicleChassisData& chassisData,
 PxConvexMesh** wheelConvexMeshes, PxConvexMesh* chassisConvexMesh, 
 PxScene& scene, PxPhysics& physics, const PxMaterial& material, vec3f pos, PxVec3 * wheeloffsets)
{
	//We need a rigid body actor for the vehicle.
	//Don't forget to add the actor the scene after setting up the associated vehicle.
	PxRigidDynamic* vehActor=physics.createRigidDynamic(PxTransform(pos[VX], pos[VY], pos[VZ]));

	//We need to add wheel collision shapes, their local poses, a material for the wheels, and a simulation filter for the wheels.
	PxConvexMeshGeometry frontLeftWheelGeom(wheelConvexMeshes[0]);
	PxConvexMeshGeometry frontRightWheelGeom(wheelConvexMeshes[1]);
	PxConvexMeshGeometry rearLeftWheelGeom(wheelConvexMeshes[2]);
	PxConvexMeshGeometry rearRightWheelGeom(wheelConvexMeshes[3]);
	const PxGeometry* wheelGeometries[4]={&frontLeftWheelGeom,&frontRightWheelGeom,&rearLeftWheelGeom,&rearRightWheelGeom};
	//const PxTransform wheelLocalPoses[4]={PxTransform(pos[VX] - CART_WIDTH* 0.5, pos[VY], pos[VZ]+ CART_LENGTH*0.5),PxTransform(pos[VX] + CART_WIDTH*0.5, pos[VY], pos[VZ] + CART_LENGTH*0.5),PxTransform(pos[VX] - CART_WIDTH *0.5, pos[VY], pos[VZ]- CART_LENGTH *0.5),PxTransform(pos[VX]+ CART_WIDTH *0.5, pos[VY], pos[VZ] - CART_LENGTH*0.5)};
	const PxTransform wheelLocalPoses[4]={PxTransform(wheeloffsets[0]),PxTransform(wheeloffsets[1]),PxTransform(wheeloffsets[2]), PxTransform(wheeloffsets[3])};
	const PxMaterial& wheelMaterial=material;
	PxFilterData wheelCollFilterData;
	wheelCollFilterData.word0=COLLISION_FLAG_WHEEL;
	wheelCollFilterData.word1=COLLISION_FLAG_WHEEL_AGAINST;

	//We need to add chassis collision shapes, their local poses, a material for the chassis, and a simulation filter for the chassis.
	PxConvexMeshGeometry chassisConvexGeom(chassisConvexMesh);
	const PxGeometry* chassisGeoms[1]={&chassisConvexGeom};
	const PxTransform chassisLocalPoses[1]={PxTransform::createIdentity()};
	const PxMaterial& chassisMaterial=material;
	PxFilterData chassisCollFilterData;
	chassisCollFilterData.word0=COLLISION_FLAG_CHASSIS;
	chassisCollFilterData.word1=COLLISION_FLAG_CHASSIS_AGAINST;

	//Create a query filter data for the car to ensure that cars
	//do not attempt to drive on themselves.
	PxFilterData vehQryFilterData;
	VehicleSetupNonDrivableShapeQueryFilterData(&vehQryFilterData);

	//Set up the physx rigid body actor with shapes, local poses, and filters.
	setupActor
		(vehActor,
		vehQryFilterData,
		wheelGeometries,wheelLocalPoses,4,&wheelMaterial,wheelCollFilterData,
		chassisGeoms,chassisLocalPoses,1,&chassisMaterial,chassisCollFilterData,
		chassisData,
		&physics, pos);

	return vehActor;
}

PxRigidDynamic* physics_addcart(struct physicsmanager* pm, vec3f pos)
{
	PxVehicleWheelsSimData* wheelsSimData=PxVehicleWheelsSimData::allocate(4);
	PxVehicleDriveSimData4W driveSimData;
	PxVehicleChassisData chassisData;

	VehicleDesc vehicleDesc = initVehicleDesc();
	//Create the wheel meshes
	PxConvexMesh* wheelConvexMeshes4[4];
	for (int i = 0; i < 4; i++) {
		wheelConvexMeshes4[i] = createWheelMesh(vehicleDesc.wheelWidth, vehicleDesc.wheelRadius, *pm->sdk, *pm->cooking);
	}
	//Create the chassis mesh
	PxU32 numVerts = 36;
	PxVec3 verts[36];
	for (int i = 0; i < 36; i++)
	{
		PxVec3 tempVert( cart_pos[cart_posindex[i]][0], cart_pos[cart_posindex[i]][1], cart_pos[cart_posindex[i]][2]);
		verts[i] = tempVert;
	}
	PxConvexMesh* chassisConvexMesh = createConvexMesh(verts, numVerts, *pm->sdk, *pm->cooking);

	//Create wheel offsets
	PxVec3 wheelCentreOffsets4[4];
	wheelCentreOffsets4[0] = PxVec3(-CART_WIDTH / 2, -CART_HEIGHT / 2, - CART_LENGTH / 2);
	wheelCentreOffsets4[1] = PxVec3(CART_WIDTH / 2, -CART_HEIGHT / 2, - CART_LENGTH / 2);
	wheelCentreOffsets4[2] = PxVec3(-CART_WIDTH / 2, -CART_HEIGHT / 2, CART_LENGTH / 2);
	wheelCentreOffsets4[3] = PxVec3(CART_WIDTH / 2, -CART_HEIGHT / 2,  CART_LENGTH / 2);

	createVehicle4WSimulationData(vehicleDesc.chassisMass, vehicleDesc, vehicleDesc.wheelMass, wheelConvexMeshes4, wheelCentreOffsets4, *wheelsSimData, driveSimData, chassisData);

	PxRigidDynamic* vehActor=createVehicleActor4W(chassisData,wheelConvexMeshes4,chassisConvexMesh,*pm->scene,*pm->sdk,*gMaterial, pos, wheelCentreOffsets4);

	pm->scene->addActor(*vehActor);

	return vehActor;

}


PxRigidDynamic* physics_adddynamic_box(struct physicsmanager* pm, vec3f pos, vec3f dim)
{
	PxRigidDynamic* actor;

	actor = PxCreateDynamic(*pm->sdk, PxTransform(pos[VX], pos[VY], pos[VZ]), PxBoxGeometry(dim[VX], dim[VY], dim[VZ]), *pm->default_material, 1.f);
	pm->scene->addActor(*actor);

	return actor;
}

/*	add a static triangle mesh stored in a triangle strip vertex buffer
	param:	pm				physics manager
	param:	num_verts		number of vertices in the buffer
	param:	stride			stride between vertices in the buffer
	param:	buf_verts		vertex buffer
*/
void physicsmanager_addstatic_trianglestrip(struct physicsmanager* pm, unsigned num_verts, unsigned stride, float* buf_verts)
{
	PxDefaultMemoryOutputStream writebuf;
	PxTriangleMeshDesc meshdesc;
	PxTriangleMesh* mesh;
	PxRigidStatic* obj;
	PxFilterData simFilterData;
	PxFilterData qryFilterData;

	simFilterData.word0=COLLISION_FLAG_GROUND;
	simFilterData.word1=COLLISION_FLAG_GROUND_AGAINST;
	
	VehicleSetupDrivableShapeQueryFilterData(&qryFilterData);

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
	
	obj=pm->sdk->createRigidStatic(PxTransform(0.f, 0.f, 0.f));
	//obj = PxCreateStatic(*pm->sdk, PxTransform(0.f, 0.f, 0.f), PxTriangleMeshGeometry(mesh), *pm->default_material);
	PxShape* shape=obj->createShape(PxTriangleMeshGeometry(mesh), *pm->default_material);
	
	shape->setQueryFilterData(qryFilterData);
	shape->setSimulationFilterData(simFilterData);
	pm->scene->addActor(*obj);

	mem_free(indices);
}