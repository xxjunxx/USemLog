// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLGraspOverlapShape.h"
#include "Animation/SkeletalMeshActor.h"

// Ctor
USLGraspOverlapShape::USLGraspOverlapShape()
{
	// Default sphere radius
	InitSphereRadius(1.0f);

	//bMultiBodyOverlap = true;

	// Default group of the finger
	Group = ESLGraspOverlapGroup::A;
	bVisualDebug = true;
	bSnapToBone = true;

#if WITH_EDITOR
	// Mimic a button to attach to the bone	
	bAttachButton = false;
#endif // WITH_EDITOR
}

// Dtor
USLGraspOverlapShape::~USLGraspOverlapShape()
{
}

// Attach to bone 
bool USLGraspOverlapShape::Init()
{
	if (!bIsInit)
	{
		// Make sure the shape is attached to it parent
		if (AttachToBone())
		{
			if (bVisualDebug)
			{
				SetHiddenInGame(false);
				SetColor(FColor::Yellow);
			}
			bIsInit = true;
			return true;
		}
	}
	return true;
}

// Start listening to overlaps 
void USLGraspOverlapShape::Start()
{
	if (!bIsStarted && bIsInit)
	{
		if (bVisualDebug)
		{
			SetColor(FColor::Red);
		}

		// Enable overlap events
		SetGenerateOverlapEvents(true);

		// Broadcast currently overlapping components
		TriggerInitialOverlaps();

		// Bind future overlapping event delegates
		OnComponentBeginOverlap.AddDynamic(this, &USLGraspOverlapShape::OnOverlapBegin);
		OnComponentEndOverlap.AddDynamic(this, &USLGraspOverlapShape::OnOverlapEnd);

		// Mark as started
		bIsStarted = true;
	}
}

// Pause/continue listening to overlaps 
void USLGraspOverlapShape::Idle(bool bInIdle)
{
	if (bInIdle != bIsIdle)
	{
		// Pause / start overlap events
		SetGenerateOverlapEvents(!bInIdle);

		if (bInIdle)
		{
			// Broadcast ending of any active grasp related contacts
			for (auto& AC : ActiveContacts)
			{
				OnEndSLGraspOverlap.Broadcast(AC);
			}
			ActiveContacts.Empty();

			if (bVisualDebug)
			{
				SetColor(FColor::Yellow);
			}
		}
		else
		{
			// Broadcast currently overlapping components
			TriggerInitialOverlaps();

			if (bVisualDebug)
			{
				SetColor(FColor::Blue);
			}
		}

		bIsIdle = bInIdle;
	}
}


// Stop publishing overlap events
void USLGraspOverlapShape::Finish(bool bForced)
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		// Mark as finished
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}

#if WITH_EDITOR
// Called when a property is changed in the editor
void USLGraspOverlapShape::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(USLGraspOverlapShape, bAttachButton))
	{
		if (AttachToBone())
		{
			if (Rename(*BoneName.ToString()))
			{
				// TODO find the 'refresh' to see the renaming
				//GetOwner()->MarkPackageDirty();
				//MarkPackageDirty();
			}
			SetColor(FColor::Green);
		}
		else
		{
			SetColor(FColor::Red);
		}
		bAttachButton = false;
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLGraspOverlapShape, BoneName))
	{
		if (AttachToBone())
		{
			if (Rename(*BoneName.ToString()))
			{
			}
			SetColor(FColor::Green);
		}
		else
		{
			SetColor(FColor::Red);
		}
		bAttachButton = false;
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLGraspOverlapShape, bVisualDebug))
	{
		SetHiddenInGame(bVisualDebug);
	}
}
#endif // WITH_EDITOR

// Attach component to bone
bool USLGraspOverlapShape::AttachToBone()
{
	// Check if owner is a skeletal actor
	if (ASkeletalMeshActor* SkelAct = Cast<ASkeletalMeshActor>(GetOwner()))
	{
		// Get the skeletal mesh component
		if (USkeletalMeshComponent* SMC = SkelAct->GetSkeletalMeshComponent())
		{
			if (SMC->GetBoneIndex(BoneName) != INDEX_NONE)
			{
				FAttachmentTransformRules AttachmentRule = bSnapToBone ? FAttachmentTransformRules::SnapToTargetIncludingScale
					: FAttachmentTransformRules::KeepRelativeTransform;

				if (AttachToComponent(SMC, AttachmentRule, BoneName))
				{
					UE_LOG(LogTemp, Warning, TEXT("%s::%d Attached component %s to the bone %s"),
						TEXT(__func__), __LINE__, *GetName(), *BoneName.ToString());
					return true;
				}
			}
		}
	}
	UE_LOG(LogTemp, Error, TEXT("%s::%d Could not attach component %s to the bone %s"),
		TEXT(__func__), __LINE__, *GetName(), *BoneName.ToString());
	return false;
}

// Set debug color
void USLGraspOverlapShape::SetColor(FColor Color)
{
	if (ShapeColor != Color)
	{
		ShapeColor = Color;
		MarkRenderStateDirty();
	}
}

// Publish currently overlapping components
void USLGraspOverlapShape::TriggerInitialOverlaps()
{
	// If objects are already overlapping at begin play, they will not be triggered
	// Here we do a manual overlap check and forward them to OnOverlapBegin
	TSet<UPrimitiveComponent*> CurrOverlappingComponents;
	GetOverlappingComponents(CurrOverlappingComponents);
	FHitResult Dummy;
	for (const auto& CompItr : CurrOverlappingComponents)
	{
		OnOverlapBegin(this, CompItr->GetOwner(), CompItr, 0, false, Dummy);
	}
}

// Called on overlap begin events
void USLGraspOverlapShape::OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	// Ignore self overlaps 
	if (OtherActor == GetOwner())
	{
		//UE_LOG(LogTemp, Error, TEXT("%s::%d %s Self Overlap"), TEXT(__func__), __LINE__, *GetName());
		if (bVisualDebug)
		{
			if (ActiveContacts.Num() == 0)
			{
				SetColor(FColor::Red);
			}
		}
		return;
	}

	if (OtherActor->IsA(AStaticMeshActor::StaticClass()))
	{
		ActiveContacts.Emplace(OtherActor);

		if (bVisualDebug)
		{
			SetColor(FColor::Green);
		}

		OnBeginSLGraspOverlap.Broadcast(OtherActor);
	}
}

// Called on overlap end events
void USLGraspOverlapShape::OnOverlapEnd(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	// Ignore self overlaps 
	if (OtherActor == GetOwner())
	{
		//UE_LOG(LogTemp, Error, TEXT("%s::%d *END* %s Self Overlap"), TEXT(__func__), __LINE__, *GetName());
		if (bVisualDebug)
		{
			if (ActiveContacts.Num() == 0)
			{
				SetColor(FColor::Blue);
			}
		}
		return;
	}

	if (OtherActor->IsA(AStaticMeshActor::StaticClass()))
	{
		ActiveContacts.Remove(OtherActor);
		
		if (bVisualDebug)
		{
			if (ActiveContacts.Num() == 0)
			{
				SetColor(FColor::Blue);
			}
		}
		
		OnEndSLGraspOverlap.Broadcast(OtherActor);
	}
}
