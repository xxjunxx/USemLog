// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "SLIndividualVisualInfoComponent.generated.h"

// Forward declarations
class USLIndividualComponent;
class UTextRenderComponent;

// Delegate notification when the component is being destroyed
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSLVisualInfoComponentDestroyedSignature, USLIndividualVisualInfoComponent*, DestroyedComponent);

/**
* Component storing the visual information of semantic individuals
*/
UCLASS( ClassGroup=(SL), meta=(BlueprintSpawnableComponent), DisplayName = "SL Individual Visual Info")
class USEMLOG_API USLIndividualVisualInfoComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USLIndividualVisualInfoComponent();

	// Connect to individual component sibling
	bool Init(bool bReset = false);

	// Check if component is visible
	bool IsInit() { return bIsInit; };

	// Load values from individual sibling
	bool Load(bool bReset = false);

	// Check if component is loaded
	bool IsLoaded() const { return bIsLoaded; };

	// Hide/show component
	bool ToggleVisibility();

	// Point text towards the camera
	bool PointToCamera();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// Called after Scene is set, but before CreateRenderState_Concurrent or OnCreatePhysicsState are called
	virtual void OnRegister() override;

	// Called after the C++ constructor and after the properties have been initialized, including those loaded from config.
	virtual void PostInitProperties() override;

	// Called when a component is created (not loaded) (after post init).This can happen in the editor or during gameplay
	virtual void OnComponentCreated() override;

	// Called before destroying the object.
	virtual void BeginDestroy() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	// Called when sibling is being destroyed
	UFUNCTION()
	void OnSiblingIndividualComponentDestroyed(USLIndividualComponent* Component);

	// Called when the individual class value has changed
	UFUNCTION()
	void OnIndividualClassChanged(USLBaseIndividual* BI, const FString& NewVal);

	// Called when the individual id value has changed
	UFUNCTION()
	void OnIndividualIdChanged(USLBaseIndividual* BI, const FString& NewVal);

	// Set the color of the text depending on the sibling state;
	void SetStateColor();

	// Render text subobject creation helper
	UTextRenderComponent* CreateDefaultTextSubobject(float Size,
		float Offset,
		const FString& DefaultName,
		FColor Color,
		class UMaterialInterface* MaterialInterface = nullptr);

public:
	// Called when the component is destroyed
	FSLVisualInfoComponentDestroyedSignature OnDestroyed;

protected:
	// Pointer to the owners individual
	USLIndividualComponent* Sibling;

	// Calcuate tranform to point toward the camera
	class APlayerCameraManager* CameraManager;

	// Individual sibling is set
	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger")
	uint8 bIsInit : 1;

	// Text data is loaded from sibling
	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger")
	uint8 bIsLoaded : 1;

private:
	// Render text lines
	UPROPERTY()
	UTextRenderComponent* FirstLine;

	UPROPERTY()
	UTextRenderComponent* SecondLine;

	UPROPERTY()
	UTextRenderComponent* ThirdLine;

	// Line sizes
	float FirstLineSize;
	float SecondLineSize;
	float ThirdLineSize;
};