#include	"cart.h"
#include	"../physics/scene_query.h"
#include	"../math/vec3f.h"
#include	"../physics/physics.h"
#include	"../render/render.h"


/*static float cart_pos[8][3] =
{
	{-CART_WIDTH/2.f, -CART_HEIGHT/2.f, -CART_LENGTH/2.f},
	{-CART_WIDTH/2.f, CART_HEIGHT/2.f, -CART_LENGTH/2.f},
	{CART_WIDTH/2.f, CART_HEIGHT/2.f, -CART_LENGTH/2.f},
	{CART_WIDTH/2.f, -CART_HEIGHT/2.f, -CART_LENGTH/2.f},
	{CART_WIDTH/2.f, CART_HEIGHT/2.f, CART_LENGTH/2.f},
	{-CART_WIDTH/2.f, CART_HEIGHT/2.f, CART_LENGTH/2.f},
	{-CART_WIDTH/2.f, -CART_HEIGHT/2.f, CART_LENGTH/2.f},
	{CART_WIDTH/2.f, -CART_HEIGHT/2.f, CART_LENGTH/2.f}
};*/

//Vehicle_VehicleManager::Vehicle_VehicleManager() 
//:	mNumVehicles(0),
//	mSqWheelRaycastBatchQuery(NULL),
//	mIsIn3WMode(false),
//	mSerializationRegistry(NULL)
//{
//}


static float cart_nor[6][3] =
{
	{-1.f, 0.f, 0.f},
	{1.f, 0.f, 0.f},
	{0.f, -1.f, 0.f},
	{0.f, 1.f, 0.f},
	{0.f, 0.f, -1.f},
	{0.f, 0.f, 1.f}
};

/*static int cart_posindex[36] =
{
	0, 6, 5, 0, 5, 1,
	0, 3, 7, 0, 7, 6,
	0, 1, 2, 0, 2, 3,
	4, 7, 3, 4, 3, 2,
	4, 2, 1, 4, 1, 5,
	4, 5, 6, 4, 6, 7
};*/

static int cart_norindex[36] =
{
	0, 0, 0, 0, 0, 0,
	2, 2, 2, 2, 2, 2,
	4, 4, 4, 4, 4, 4,
	1, 1, 1, 1, 1, 1,
	3, 3, 3, 3, 3, 3,
	5, 5, 5, 5, 5, 5
};

void cart_init(struct cart* c, physicsmanager* pm, vec3f pos)
{
	vec3f dim;

	vec3f_set(dim, CART_WIDTH/2.f, CART_HEIGHT/2.f, CART_LENGTH/2.f);
	c->p_cart = physics_addcart(pm, pos);

	renderable_init(&c->r_cart, RENDER_MODE_TRIANGLES, RENDER_TYPE_SOLID, RENDER_FLAG_NONE);

	vec3f_set(c->r_cart.material.amb, 0.8f, 0.15f, 0.1f);
	vec3f_set(c->r_cart.material.dif, 0.8f, 0.15f, 0.1f);
	vec3f_set(c->r_cart.material.spc, 0.8f, 0.2f, 0.2f);
	c->r_cart.material.shn = 100.f;
}


void cart_generatemesh(struct renderer* r, struct cart* c)
{
	float* ptr;
	int i;

	renderable_allocate(r, &c->r_cart, 36);

	ptr = c->r_cart.buf_verts;

	for (i = 0; i < 36; i++)
	{
		vec3f_copy(ptr, cart_pos[cart_posindex[i]]);
		ptr += RENDER_ATTRIBSIZE_POS;
		vec3f_copy(ptr, cart_nor[cart_norindex[i]]);
		ptr += RENDER_ATTRIBSIZE_NOR;
	}
}

//
//void Vehicle_VehicleManager::init(PxPhysics& physics, const PxMaterial** drivableSurfaceMaterials, const PxVehicleDrivableSurfaceType* drivableSurfaceTypes)
//{
//#if defined(SERIALIZE_VEHICLE_RPEX) || defined(SERIALIZE_VEHICLE_BINARY)
//	mSerializationRegistry = PxSerialization::createSerializationRegistry(physics);
//#endif
//
//	//Initialise the sdk.
//	PxInitVehicleSDK(physics, mSerializationRegistry);
//
//	//Set the basis vectors.
//	PxVec3 up(0,1,0);
//	PxVec3 forward(0,0,1);
//	PxVehicleSetBasisVectors(up,forward);
//
//	//Set the vehicle update mode to be immediate velocity changes.
//	PxVehicleSetUpdateMode(PxVehicleUpdateMode::eVELOCITY_CHANGE);
//	
//	//Initialise all vehicle ptrs to null.
//	for(PxU32 i=0;i<MAX_NUM_4W_VEHICLES;i++)
//	{
//		mVehicles[i]=NULL;
//	}
//
//	//Allocate simulation data so we can switch from 3-wheeled to 4-wheeled cars by switching simulation data.
//	mWheelsSimData4W = PxVehicleWheelsSimData::allocate(4);
//
//	//Scene query data for to allow raycasts for all suspensions of all vehicles.
//	mSqData = VehicleSceneQueryData::allocate(MAX_NUM_4W_VEHICLES*4);
//
//	//Data to store reports for each wheel.
//	mWheelQueryResults = VehicleWheelQueryResults::allocate(MAX_NUM_4W_VEHICLES*4);
//
//	//Set up the friction values arising from combinations of tire type and surface type.
//	mSurfaceTirePairs=PxVehicleDrivableSurfaceToTireFrictionPairs::allocate(MAX_NUM_TIRE_TYPES,MAX_NUM_SURFACE_TYPES);
//	mSurfaceTirePairs->setup(MAX_NUM_TIRE_TYPES,MAX_NUM_SURFACE_TYPES,drivableSurfaceMaterials,drivableSurfaceTypes);
//	for(PxU32 i=0;i<MAX_NUM_SURFACE_TYPES;i++)
//	{
//		for(PxU32 j=0;j<MAX_NUM_TIRE_TYPES;j++)
//		{
//			mSurfaceTirePairs->setTypePairFriction(i,j,TireFrictionMultipliers::getValue(i, j));
//		}
//	}
//
//#ifdef PX_PS3 
//	setSpuRaycastShader(mSqData);
//#endif
//}
//
//void Vehicle_VehicleManager::shutdown()
//{
//	//Remove the N-wheeled vehicles.
//	for(PxU32 i=0;i<mNumVehicles;i++)
//	{
//		switch(mVehicles[i]->getVehicleType())
//		{
//		case PxVehicleTypes::eDRIVE4W:
//			{
//				PxVehicleDrive4W* veh=(PxVehicleDrive4W*)mVehicles[i];
//				veh->free();
//			}
//			break;
//		case PxVehicleTypes::eDRIVENW:
//			{
//				PxVehicleDriveNW* veh=(PxVehicleDriveNW*)mVehicles[i];
//				veh->free();
//			}
//			break;
//		case PxVehicleTypes::eDRIVETANK:
//			{
//				PxVehicleDriveTank* veh=(PxVehicleDriveTank*)mVehicles[i];
//				veh->free();
//			}
//			break;
//		default:
//			PX_ASSERT(false);
//			break;
//		}
//	}
//
//	//Deallocate simulation data that was used to switch from 3-wheeled to 4-wheeled cars by switching simulation data.
//	mWheelsSimData4W->free();
//
//	//Deallocate scene query data that was used for suspension raycasts.
//	mSqData->free();
//
//	//Deallocate buffers that store wheel reports.
//	mWheelQueryResults->free();
//
//	//Release the  friction values used for combinations of tire type and surface type.
//	mSurfaceTirePairs->release();
//
//	//Scene query.
//	if(mSqWheelRaycastBatchQuery)
//	{
//		mSqWheelRaycastBatchQuery=NULL;
//	}
//
//	PxCloseVehicleSDK(mSerializationRegistry);
//
//	if(mSerializationRegistry)
//		mSerializationRegistry->release();
//}
