#include	"physics.h"

#include	<PxPhysicsAPI.h>
#include	"../mem.h"
#include	"../render/render.h"


using namespace physx;


void physics_startup(struct physicsmanager* pm)
{
	// initialize foundation object
	pm->foundation = PxCreateFoundation(PX_PHYSICS_VERSION, pm->default_alloc, pm->default_error);
	
	// initialize the top-level physics object
	pm->sdk = PxCreateBasePhysics(PX_PHYSICS_VERSION, *pm->foundation, PxTolerancesScale(), false);

	// initialize mesh cooking object
	pm->cooking = PxCreateCooking(PX_PHYSICS_VERSION, *pm->foundation, PxCookingParams(pm->sdk->getTolerancesScale()));

	// for particle effects
	//PxRegisterParticles(*pm->sdk);

	// create default material
	pm->default_material = pm->sdk->createMaterial(0.5f, 0.5f, 0.1f);

	// create the scene for simulation
	PxSceneDesc scenedesc(pm->sdk->getTolerancesScale());
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
	pm->sdk->release();
	pm->foundation->release();
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
		*(ptr++) = i+1;
		*(ptr++) = i+2;
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