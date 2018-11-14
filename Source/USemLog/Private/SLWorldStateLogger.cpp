// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLWorldStateLogger.h"
//#include "Engine/GameInstance.h"

// Constructor
USLWorldStateLogger::USLWorldStateLogger()
{
	// State flags
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;
}

// Destructor
USLWorldStateLogger::~USLWorldStateLogger()
{
	if (!bIsFinished)
	{
		USLWorldStateLogger::Finish();
	}
}

// Init logger
void USLWorldStateLogger::Init(bool bLogVisionData,
	ESLWorldStateWriterType WriterType,
	float DistanceStepSize,
	float RotationStepSize,
	const FString& EpisodeId,
	const FString& Location,
	const FString& HostIP,
	const uint16 HostPort)
{
	if (!bIsInit)
	{
		// Create async worker to do the writing on a separate thread
		AsyncWorker = new FAsyncTask<FSLWorldStateAsyncWorker>();

		// Init async worker (create the writer and set logging parameters)
		AsyncWorker->GetTask().Init(GetWorld(), WriterType, DistanceStepSize, RotationStepSize,
			EpisodeId, Location, HostIP, HostPort);

		UE_LOG(LogSL, Warning, TEXT("%s::%d"), TEXT(__FUNCTION__), __LINE__);
#if WITH_SL_VIS
		if (bLogVisionData)
		{
			//VisionDataLogger->Init(GetWorld(), WriterType, EpisodeId, Location, HostIP, HostPort)
			UE_LOG(LogSL, Warning, TEXT("\t\t%s::%d"), TEXT(__FUNCTION__), __LINE__);
				RecordingName = EpisodeId + "_RP";
		}
#endif // WITH_SL_VIS

		// Flag as init
		bIsInit = true;
	}
}

// Start logger
void USLWorldStateLogger::Start(const float UpdateRate)
{
	if (!bIsStarted && bIsInit)
	{
		// Call before binding the recurrent Update function
		// this ensures the first world state is logged (static and movable semantic items)
		USLWorldStateLogger::PreUpdate();

		// Start updating
		if (UpdateRate > 0.0f)
		{
			// Update logger on custom timer tick (does not guarantees the UpdateRate value,
			// since it will be eventually triggered from the game thread tick
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, this,
				&USLWorldStateLogger::Update, UpdateRate, true);
		}
		else
		{
			// Update logger on tick (updates every game thread tick, update rate can vary)
			bIsTickable = true;
		}

#if WITH_SL_VIS
		if (!RecordingName.IsEmpty())
		{
			//VisionDataLogger->Init(GetWorld(), WriterType, EpisodeId, Location, HostIP, HostPort)
			UE_LOG(LogSL, Warning, TEXT("%s::%d"), TEXT(__FUNCTION__), __LINE__);
			if (UGameInstance* GI = GetWorld()->GetGameInstance())
			{
				UE_LOG(LogSL, Warning, TEXT(" \t\t %s::%d"), TEXT(__FUNCTION__), __LINE__);
				GI->StartRecordingReplay(RecordingName, RecordingName);
			}
		}
#endif // WITH_SL_VIS

		// Set flags
		bIsStarted = true;
	}
}

// Finish logger
void USLWorldStateLogger::Finish()
{
	if (bIsStarted || bIsInit)
	{
		if (AsyncWorker)
		{
			// Wait for worker to complete before deleting it
			AsyncWorker->EnsureCompletion();
			delete AsyncWorker;
			AsyncWorker = nullptr;
		}
		
		// Stop update timer;
		if (TimerHandle.IsValid())
		{
			GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
		}
		//  Disable tick
		if (bIsTickable)
		{
			bIsTickable = false;
		}

#if WITH_SL_VIS
		if (!RecordingName.IsEmpty())
		{
			UE_LOG(LogSL, Warning, TEXT("%s::%d"), TEXT(__FUNCTION__), __LINE__);
			if (GetWorld())
			{
				if (GetWorld()->IsPendingKill())
				{
					UE_LOG(LogSL, Warning, TEXT(" \t\t %s::%d  GetWorld IsPendingKill"), TEXT(__FUNCTION__), __LINE__);
				}
			}
			else
			{
				UE_LOG(LogSL, Warning, TEXT(" \t\t %s::%d  GetWorld nullptr"), TEXT(__FUNCTION__), __LINE__);
			}

			if (GetWorld())
			{
				if (UGameInstance* GI = GetWorld()->GetGameInstance())
				{
					if (GI->IsPendingKill())
					{
						UE_LOG(LogSL, Warning, TEXT(" \t\t %s::%d GI->IsPendingKill"), TEXT(__FUNCTION__), __LINE__);
					}

					GI->StopRecordingReplay();

					GI->PlayReplay(RecordingName);
				}
			}
		}
#endif // WITH_SL_VIS

		// Mark logger as finished
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
		UE_LOG(LogSL, Warning, TEXT(" \t\t %s::%d  MARKD DONE"), TEXT(__FUNCTION__), __LINE__);
	}
}

/** Begin FTickableGameObject interface */
// Called after ticking all actors, DeltaTime is the time passed since the last call.
void USLWorldStateLogger::Tick(float DeltaTime)
{
	// Call update on tick
	USLWorldStateLogger::Update();
}

// Return if object is ready to be ticked
bool USLWorldStateLogger::IsTickable() const
{
	return bIsTickable;
}

// Return the stat id to use for this tickable
TStatId USLWorldStateLogger::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(USLWorldStateLogger, STATGROUP_Tickables);
}
/** End FTickableGameObject interface */

// Log initial state of the world (static and dynamic entities)
void USLWorldStateLogger::PreUpdate()
{
	// Start async worker
	AsyncWorker->StartBackgroundTask();
	
	// Wait for worker to complete (we only use the blocking wait for the initial state log)
	AsyncWorker->EnsureCompletion();

	// Remove all non-dynamic objects from worker
	AsyncWorker->GetTask().RemoveStaticItems();
}

// Log current state of the world (dynamic objects that moved more than the distance threshold)
void USLWorldStateLogger::Update()
{
	// Start task if worker is done with its previous work
	if (AsyncWorker->IsDone())
	{
		AsyncWorker->StartBackgroundTask();
	}
	else
	{
		UE_LOG(LogSL, Error, TEXT("[%s][%d] Previous task not finished, SKIPPING new task.."), TEXT(__FUNCTION__), __LINE__);
	}
}
