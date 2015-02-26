#include	"physics.h"

#include	<PxPhysicsAPI.h>
#include	"../math/vec3f.h"
#include	"../mem.h"
#include	"../objects/cart.h"
#include	"../render/render.h"


using namespace physx;


/*	start up the physics manager
	param:	pm				physics manager (modified)
*/
void physicsmanager_startup(struct physicsmanager* pm)
{
	PxTolerancesScale scale;
	int i;

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
	pm->default_material = pm->sdk->createMaterial(0.1f, 0.1f, 0.5f);

	// create the scene for simulation
	PxSceneDesc scenedesc(scale);
	scenedesc.gravity = PxVec3(PHYSICS_DEFAULT_GRAVITY);
	scenedesc.cpuDispatcher = PxDefaultCpuDispatcherCreate(1);
	scenedesc.filterShader = PxDefaultSimulationFilterShader;
	pm->scene = pm->sdk->createScene(scenedesc);

	// initialize vehicle array
	for (i = 0; i < PHYSICS_MAX_VEHICLES; i++)
	{
		pm->vehicles[i].body = NULL;
		pm->vehicles[i].enabled = false;
	}
}

/*	shut down the physics manager
	param:	pm				physics manager (modified)
*/
void physicsmanager_shutdown(struct physicsmanager* pm)
{
	int i;

	for (i = 0; i < PHYSICS_MAX_VEHICLES; i++)
		if (pm->vehicles[i].enabled)
			pm->vehicles[i].body->release();

	pm->scene->release();
	pm->default_material->release();

	PxCloseVehicleSDK();

	pm->cooking->release();
	pm->sdk->release();
	pm->foundation->release();
}


static void updatevehicles(struct physicsmanager* pm, float dt)
{
	vec3f g_origin, g_dir, vel, forward;
	PxRaycastBuffer hit;
	PxHitFlags outflags;
	struct vehicle* v;
	PxVec3 velocity;
	int i, j;

	(void)dt;

	outflags = PxHitFlag::eDISTANCE;
	PxQueryFilterData filterData(PxQueryFlag::eSTATIC);

	for (i = 0; i < PHYSICS_MAX_VEHICLES; i++)
	{
		if (!pm->vehicles[i].enabled)
			continue;

		v = pm->vehicles + i;
		physx::PxMat44 vehicle_world(v->body->getGlobalPose());

		for (j = 0; j < PHYSICS_VEHICLE_RAYCAST_COUNT; j++)
		{
			// transform ray into global coordinates
			mat4f_fulltransformvec3fn(g_origin, v->origins[j], (float*)&vehicle_world);
			mat4f_transformvec3fn(g_dir, v->dir[j], (float*)&vehicle_world);

			// raycast
			v->touching[j] = pm->scene->raycast(PxVec3(g_origin[VX], g_origin[VY], g_origin[VZ]), PxVec3(g_dir[VX], g_dir[VY], g_dir[VZ]), PHYSICS_VEHICLE_RAYCAST_MAXDIST, hit, outflags, filterData);

			if (v->touching[j])
			{
				vec3f force;
				float d;

				d = hit.block.distance;

				// calculate raycast force (quadratic)
				vec3f_scalen(force, v->dir[j], -PHYSICS_VEHICLE_RAYCAST_MAXFORCE * (d*d/(PHYSICS_VEHICLE_RAYCAST_MAXDIST * PHYSICS_VEHICLE_RAYCAST_MAXDIST) - 2.f*d/PHYSICS_VEHICLE_RAYCAST_MAXDIST + 1.f));

				PxRigidBodyExt::addLocalForceAtLocalPos(*v->body, PxVec3(force[VX], force[VY], force[VZ]), PxVec3(v->origins[j][VX], v->origins[j][VY], v->origins[j][VZ]));
			}
		}

		velocity = v->body->getLinearVelocity();
		vec3f_set(vel, velocity.x, velocity.y, velocity.z);

<<<<<<< HEAD
<<<<<<< HEAD

		vec3f_set(forward, CART_FORWARD);
		mat4f_transformvec3f(forward, (float*)&vehicle_world);

		printf("Forward Velocity: %f\n", vec3f_dot(forward, vel));
=======
		//printf("Velocity: %f, %f, %f\n", vel[VX], vel[VY], vel[VZ]);
>>>>>>> e7a03136901237d44e74be367b50c0b98f5c4dc4
=======
		printf("Velocity: %f, %f, %f\n", vel[VX], vel[VY], vel[VZ]);
>>>>>>> 2ec64863d3a143ac75102eef18f1288c5a256508
	}
}


/*	update the physics simulation
	param:	pm				physics manager
	param:	dt				delta time
*/
void physicsmanager_update(struct physicsmanager* pm, float dt)
{
	updatevehicles(pm, dt);

	pm->scene->simulate(dt);
	pm->scene->fetchResults(true);
	
}


/*	add a vehicle to the simulation
	param:	pm				physics manager
	param:	pos				position of the vehicle
	param:	dim				dimensions of the bounding box
	return:	struct vehicle*	pointer to the new vehicle object
*/
struct vehicle* physicsmanager_newvehicle(struct physicsmanager* pm, vec3f pos, vec3f dim){
	struct vehicle* v;
	int i;

	for (i = 0; i < PHYSICS_MAX_VEHICLES; i++)
		if (!pm->vehicles[i].enabled)
			break;

	if (i == PHYSICS_MAX_VEHICLES)
		return NULL;

	v = pm->vehicles + i;

	v->body = PxCreateDynamic(*pm->sdk, PxTransform(pos[VX], pos[VY], pos[VZ]), PxBoxGeometry(dim[VX], dim[VY], dim[VZ]), *pm->default_material, 1.f);
	pm->scene->addActor(*v->body);

	// create the raycast origins in "model" space
	vec3f_set(v->origins[0], -dim[VX] + 0.2f, -dim[VY], -dim[VZ] + 0.2f);
	vec3f_set(v->origins[1], dim[VX] - 0.2f, -dim[VY], -dim[VZ] + 0.2f);
	vec3f_set(v->origins[2], -dim[VX] + 0.2f, -dim[VY], dim[VZ] - 0.2f);
	vec3f_set(v->origins[3], dim[VX] - 0.2f, -dim[VY], dim[VZ] - 0.2f);

	// create the raycast directions in "model" space
	vec3f_set(v->dir[0], 0.f, -1.f, 0.f);
	vec3f_set(v->dir[1], 0.f, -1.f, 0.f);
	vec3f_set(v->dir[2], 0.f, -1.f, 0.f);
	vec3f_set(v->dir[3], 0.f, -1.f, 0.f);

	v->body->setLinearDamping(PHYSICS_VEHICLE_DAMP_LINEAR);
	v->body->setAngularDamping(PHYSICS_VEHICLE_DAMP_ANGULAR);

	v->enabled = true;

	return v;
}

/*	remove a vehicle from the simulation
	param:	pm				physics manager
	param:	v				pointer to vehicle object to remove
*/
void physicsmanager_removevehicle(struct physicsmanager* pm, struct vehicle* v)
{
	int i;

	for (i = 0; i < PHYSICS_MAX_VEHICLES; i++)
		if (v == pm->vehicles + i)
		{
			pm->vehicles[i].body->release();
			pm->vehicles[i].enabled = false;
		}
}


/*	probably temporary	*/
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