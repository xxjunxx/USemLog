// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Animation/SkeletalMeshActor.h"

/**
 * Class exporting raw data during gameplay
 */
class SEMLOG_API FSLRawDataExporter
{
public:
	// Constructor
	FSLRawDataExporter(const float DistThreshSqr,
		const FString Path,
		TMap<ASkeletalMeshActor*, FString> SkelActPtrToUniqNameMap,
		TMap<AStaticMeshActor*, FString> DynamicActPtrToUniqNameMap,
		TMap<AStaticMeshActor*, FString> StaticActPtrToUniqNameMap,
		TPair<USceneComponent*, FString> CamToUniqName);

	// Destructor
	~FSLRawDataExporter();

	// Log step
	void Update(const float Timestamp);

	// Structure of skeletal mesh comp with its previous pose
	struct SLSkelLogRawStruct
	{
		SLSkelLogRawStruct(ASkeletalMeshActor* SkMComp, const FString UniqName) :
			SkelMeshComp(SkMComp),
			UniqueName(UniqName),
			PrevLoc(FVector(0.0f)),
			PrevRot(FRotator(0.0f)) {};
		ASkeletalMeshActor* SkelMeshComp;
		FString UniqueName;
		FVector PrevLoc;
		FRotator PrevRot;
	};

	// Structure of dyamic actors with prev pose and unique name
	struct SLDynActLogRawStruct
	{
		SLDynActLogRawStruct(AStaticMeshActor* StMAct, const FString UniqName) : 
			StaticMeshAct(StMAct),
			UniqueName(UniqName),
			PrevLoc(FVector(0.0f)),
			PrevRot(FRotator(0.0f)) {};
		AStaticMeshActor* StaticMeshAct;
		FString UniqueName;
		FVector PrevLoc;
		FRotator PrevRot;
	};
	
private:
	// Create Json object with a 3d location
	TSharedPtr<FJsonObject> CreateLocationJsonObject(const FVector Location);

	// Create Json object with a 3d rotation as quaternion 
	TSharedPtr<FJsonObject> CreateRotationJsonObject(const FQuat Rotation);

	// Create Json object with name location and rotation
	TSharedPtr<FJsonObject> CreateNameLocRotJsonObject(
		const FString Name, const FVector Location, const FQuat Rotation);

	// Init items to log from the level
	void InitItemsToLog(
		const TMap<ASkeletalMeshActor*, FString>& SkelActPtrToUniqNameMap,
		const TMap<AStaticMeshActor*, FString>& DynamicActPtrToUniqNameMap,
		const TMap<AStaticMeshActor*, FString>& StaticActPtrToUniqNameMap);

	// Distance threshold (squared) for raw data logging
	float DistanceThresholdSquared;

	// File handle to append raw data
	TSharedPtr<IFileHandle> RawFileHandle;

	// Array of skeletal meshes with prev position and orientation
	TArray<SLSkelLogRawStruct> SkelActStructArr;

	// Array of static meshes with prev position and orientation
	TArray<SLDynActLogRawStruct> DynamicActStructArr;

	// Map of static map actors  to unique name
	TMap<AStaticMeshActor*, FString> StaticActToUniqName;

	// User camera to unique name
	TPair<USceneComponent*, FString> CameraToUniqueName;
	
	// Camera previous location
	FVector CameraPrevLoc;
};

