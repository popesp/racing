
#ifndef VEHICLE_UTILSCENEQUERY_H
#define VEHICLE_UTILSCENEQUERY_H

#include "common/PxPhysXCommonConfig.h"
#include "vehicle/PxVehicleSDK.h"
#include "foundation/PxPreprocessor.h"
#include "PxScene.h"
#include "PxBatchQueryDesc.h"

using namespace physx;




//Make sure that suspension raycasts only consider shapes flagged as drivable that don't belong to the owner vehicle.
enum
{
	Vehicle_DRIVABLE_SURFACE = 0xffff0000,
	Vehicle_UNDRIVABLE_SURFACE = 0x0000ffff
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
	return ((0 == (filterData1.word3 & Vehicle_DRIVABLE_SURFACE)) ? PxSceneQueryHitType::eNONE : PxSceneQueryHitType::eBLOCK);
}


//Data structure for quick setup of scene queries for suspension raycasts.
class VehicleSceneQueryData
{
public:

	//Allocate scene query data for up to maxNumWheels suspension raycasts.
	static VehicleSceneQueryData* allocate(const PxU32 maxNumWheels);

	//Free allocated buffer for scene queries of suspension raycasts.
	void free();

	//Create a PxBatchQuery instance that will be used as a single batched raycast of multiple suspension lines of multiple vehicles
	PxBatchQuery* setUpBatchedSceneQuery(PxScene* scene);

	//Get the buffer of scene query results that will be used by PxVehicleNWSuspensionRaycasts
	PxRaycastQueryResult* getRaycastQueryResultBuffer() {return mSqResults;}

	//Get the number of scene query results that have been allocated for use by PxVehicleNWSuspensionRaycasts
	PxU32 getRaycastQueryResultBufferSize() const {return mNumQueries;}

	//Set the pre-filter shader 
	void setPreFilterShader(PxBatchQueryPreFilterShader preFilterShader) {mPreFilterShader=preFilterShader;}

	//Set the spu pre-filter shader (essential to run filtering on spu)
	void setSpuPreFilterShader(void* spuPreFilterShader, const PxU32 spuPreFilterShaderSize) {mSpuPreFilterShader=spuPreFilterShader; mSpuPreFilterShaderSize=spuPreFilterShaderSize;}

private:

	//One result for each wheel.
	PxRaycastQueryResult* mSqResults;
	PxU32 mNbSqResults;

	//One hit for each wheel.
	PxRaycastHit* mSqHitBuffer;

	//Filter shader used to filter drivable and non-drivable surfaces
	PxBatchQueryPreFilterShader mPreFilterShader;

	//Ptr to compiled spu filter shader 
	//Set this on ps3 for spu raycasts
	void* mSpuPreFilterShader;

	//Size of compiled spu filter shader 
	//Set this on ps3 for spu raycasts.
	PxU32 mSpuPreFilterShaderSize;

	//Maximum number of suspension raycasts that can be supported by the allocated buffers 
	//assuming a single query and hit per suspension line.
	PxU32 mNumQueries;

	void init()
	{
		mPreFilterShader=VehicleWheelRaycastPreFilter;
		mSpuPreFilterShader=NULL;
		mSpuPreFilterShaderSize=0;
	}

	VehicleSceneQueryData()
	{
		init();
	}

	~VehicleSceneQueryData()
	{
	}
};

#endif //VEHICLE_UTILSCENEQUERY_H