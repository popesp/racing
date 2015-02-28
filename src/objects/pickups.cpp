#include	"pickups.h"

void pickup_init(struct pickup*, struct physicsmanager*, vec3f){
}

void pickup_delete(struct pickupmanager* pum){
	renderable_deallocate(&pum->r_pickup);
}