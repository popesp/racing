
#ifndef WHEELQUERYRESULTS_H
#define WHEELQUERYRESULTS_H

#include "vehicle/PxVehicleSDK.h"
#include "vehicle/PxVehicleUpdate.h"

using namespace physx;

//Data structure to store reports for each wheel. 
class VehicleWheelQueryResults
{
public:

	//Allocate a buffer of wheel query results for up to maxNumWheels.
	static VehicleWheelQueryResults* allocate(const PxU32 maxNumWheels);

	//Free allocated buffer.
	void free();

	PxWheelQueryResult* addVehicle(const PxU32 numWheels);

private:

	//One result for each wheel.
	PxWheelQueryResult* mWheelQueryResults;

	//Maximum number of wheels.
	PxU32 mMaxNumWheels;

	//Number of wheels 
	PxU32 mNumWheels;


	VehicleWheelQueryResults()
		: mWheelQueryResults(NULL),mMaxNumWheels(0), mNumWheels(0)
	{
		init();
	}

	~VehicleWheelQueryResults()
	{
	}

	void init()
	{
		mWheelQueryResults=NULL;
		mMaxNumWheels=0;
		mNumWheels=0;
	}
};


#endif //WHEELQUERYRESULTS_H
