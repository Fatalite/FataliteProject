#pragma once


#include "CoreMinimal.h"
#include "Async/AsyncWork.h"

class ACylinderActor; // Forward declaration
class FATALITEPROJECT_API FMyAsyncSimulationTask : public FNonAbandonableTask
{
public:
    FMyAsyncSimulationTask(ACylinderActor* CylinderActor, float DeltaTime)
        : CylinderActor(CylinderActor), DeltaTime(DeltaTime)
    {
    }

    // Required for FAsyncTask
    FORCEINLINE TStatId GetStatId() const
    {
        RETURN_QUICK_DECLARE_CYCLE_STAT(FMyAsyncSimulationTask, STATGROUP_ThreadPoolAsyncTasks);
    }

    // The actual work function
    void DoWork();

private:
    ACylinderActor* CylinderActor;
    float DeltaTime;
};