
#include "wheel_query_results.h"
#include "vehicle/PxVehicleSDK.h"
#include "PsFoundation.h"
#include "PsUtilities.h"


//#define CHECK_MSG(exp, msg) (!!(exp) || (physx::shdfnd::getFoundation().error(physx::PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, msg), 0) )


VehicleWheelQueryResults* VehicleWheelQueryResults::allocate(const PxU32 maxNumWheels)
{
	const PxU32 size = sizeof(VehicleWheelQueryResults) + sizeof(PxWheelQueryResult)*maxNumWheels;
	VehicleWheelQueryResults* resData = (VehicleWheelQueryResults*)PX_ALLOC(size, PX_DEBUG_EXP("VehicleWheelQueryResults"));
	resData->init();
	PxU8* ptr = (PxU8*) resData;
	ptr += sizeof(VehicleWheelQueryResults);
	resData->mWheelQueryResults = (PxWheelQueryResult*)ptr;
	ptr +=  sizeof(PxWheelQueryResult)*maxNumWheels;
	resData->mMaxNumWheels=maxNumWheels;
	for(PxU32 i=0;i<maxNumWheels;i++)
	{
		new(&resData->mWheelQueryResults[i]) PxWheelQueryResult();
	}
	return resData;
}

void VehicleWheelQueryResults::free()
{
	PX_FREE(this);
}

PxWheelQueryResult* VehicleWheelQueryResults::addVehicle(const PxU32 numWheels)
{
	PX_ASSERT((mNumWheels + numWheels) <= mMaxNumWheels);
	PxWheelQueryResult* r = &mWheelQueryResults[mNumWheels];
	mNumWheels += numWheels;
	return r;
}


