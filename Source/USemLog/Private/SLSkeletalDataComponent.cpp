// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLSkeletalDataComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Tags.h"

// Sets default values for this component's properties
USLSkeletalDataComponent::USLSkeletalDataComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	bInit = false;
	bReloadData = false;
	bClearAllData = false;
}


// Called when the game starts
void USLSkeletalDataComponent::BeginPlay()
{
	Super::BeginPlay();
}

#if WITH_EDITOR
// Called when a property is changed in the editor
void USLSkeletalDataComponent::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(USLSkeletalDataComponent, LoadFromDataAsset))
	{
		USLSkeletalDataComponent::LoadData();
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLSkeletalDataComponent, bReloadData))
	{
		USLSkeletalDataComponent::LoadData();
		bReloadData = false;
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLSkeletalDataComponent, bClearAllData))
	{
		USLSkeletalDataComponent::ClearData();
		bClearAllData = false;
	}
}
#endif // WITH_EDITOR

// Init the component for runtime
bool USLSkeletalDataComponent::Init()
{
	return bInit = bInit ? true : USLSkeletalDataComponent::SetOwnerSemanticData() && USLSkeletalDataComponent::SetSkeletalParent();

	//if (bInit)
	//{
	//	return true;
	//}
	//else
	//{
	//	bInit = USLSkeletalDataComponent::SetOwnerSemanticData() && USLSkeletalDataComponent::SetSkeletalParent();
	//	return bInit;
	//}
}

#if WITH_EDITOR
// Update the data
void USLSkeletalDataComponent::LoadData()
{
	// Continue if attachment parent is a skeletal mesh component
	if (USkeletalMeshComponent* SkMC = Cast<USkeletalMeshComponent>(GetAttachParent()))
	{
		// Set the parent
		SkeletalMeshParent = SkMC;

		// Update from data asset
		if (LoadFromDataAsset)
		{
			// Iterate bone classes, ignore empty classes
			for (const auto& BoneClassPair : LoadFromDataAsset->BoneClasses)
			{
				const FString BoneClass = BoneClassPair.Value;
				const FName BoneName = BoneClassPair.Key;
				if (!BoneClass.IsEmpty())
				{
					// Override any previous data for now
					if (BonesData.Contains(BoneName))
					{
						// Find the material slot with the bone class name
						if (FSkeletalMaterial* SkelMat = SkMC->SkeletalMesh->Materials.FindByPredicate(
							[&BoneClass](const FSkeletalMaterial& InMat)
							{return InMat.MaterialSlotName.ToString().Equals(BoneClass); }))
						{
							BonesData[BoneName].MaskMaterial = Cast<UMaterialInterface>(SkelMat->MaterialInterface);
						}
					}
					else
					{
						FSLBoneData BoneData;
						BoneData.Class = BoneClass;
						// Find the material slot with the bone class name
						if (FSkeletalMaterial* SkelMat = SkMC->SkeletalMesh->Materials.FindByPredicate(
							[&BoneClass](const FSkeletalMaterial& InMat)
							{return InMat.MaterialSlotName.ToString().Equals(BoneClass); }))
						{
							BoneData.MaskMaterial = Cast<UMaterialInterface>(SkelMat->MaterialInterface);
						}
						BonesData.Add(BoneName, BoneData);
					}
				}
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Parent is not a USkeletalMeshComponent.."), TEXT(__FUNCTION__), __LINE__);
	}
}

// Clear all data
void USLSkeletalDataComponent::ClearData(bool bIncludeSkeletal)
{
	if (bIncludeSkeletal)
	{
		BonesData.Empty();
	}
	SkeletalMeshParent = nullptr;
	SemanticOwner = nullptr;
	if (OwnerSemanticData.IsValid())
	{
		OwnerSemanticData.Reset();
	}
	bInit = false;
}
#endif // WITH_EDITOR

// Set the skeletal parent, returns true if already set
bool USLSkeletalDataComponent::SetSkeletalParent()
{
	if (SkeletalMeshParent)
	{
		// Mesh parent already set
		return true; 
	}
	else
	{
		if (USkeletalMeshComponent* SkMC = Cast<USkeletalMeshComponent>(GetAttachParent()))
		{
			SkeletalMeshParent = SkMC;
			return true;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d GetAttachParent() is not of type USkeletalMeshComponent.."),
				TEXT(__FUNCTION__), __LINE__);
			return false;
		}
	}
}

// Set the semantic parent, returns true if already set
bool USLSkeletalDataComponent::SetOwnerSemanticData()
{
	if (OwnerSemanticData.IsValid() && OwnerSemanticData->IsValid())
	{
		return true; // Semantic data already set
	}
	else
	{
		// Clear ptr
		OwnerSemanticData.Reset();

		// Check if the attachment is the semantic owner
		FString Id = FTags::GetValue(GetAttachParent(), "SemLog", "Id");
		FString Class = FTags::GetValue(GetAttachParent(), "SemLog", "Class");
		if (!Id.IsEmpty() && !Class.IsEmpty())
		{
			OwnerSemanticData = MakeShareable(new FSLEntity(GetAttachParent(), Id, Class));
			return true;
		}
		else
		{
			// Check if the owner is the semantic parent
			Id = FTags::GetValue(GetOwner(), "SemLog", "Id");
			Class = FTags::GetValue(GetOwner(), "SemLog", "Class");
			if (!Id.IsEmpty() && !Class.IsEmpty())
			{
				OwnerSemanticData = MakeShareable(new FSLEntity(GetOwner(), Id, Class));
				return true;
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d None of GetAttachParent() or GetOwner() is a semantic owner.."),
					TEXT(__FUNCTION__), __LINE__);
				return false;
			}
		}
	}
}