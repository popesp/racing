#include	"physics.h"

#include	<PxPhysicsAPI.h>
#include	"../math/vec3f.h"
#include	"../mem.h"
#include	"../objects/cart.h"
#include	"../render/render.h"


using namespace physx;


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

	// for particle effects
	//PxRegisterParticles(*pm->sdk);

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


//static void create_cart_data(float chassis_mass, PxConvexMesh* chassis_mesh

PxRigidDynamic* physics_addcart(struct physicsmanager* pm, vec3f pos)
{
	PxRigidDynamic* cart;

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