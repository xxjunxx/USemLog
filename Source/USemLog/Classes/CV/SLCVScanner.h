// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "SLCVScanner.generated.h"

// Forward declarations
class ASLIndividualManager;
class USLVisibleIndividual;
class AStaticMeshActor;
class UGameViewportClient;
class UMaterialInstanceDynamic;
class ADirectionalLight;

/**
* View modes
*/
UENUM()
enum class ESLCVViewMode : uint8
{
	NONE					UMETA(DisplayName = "None"),
	Lit						UMETA(DisplayName = "Lit"),
	Unlit					UMETA(DisplayName = "Unlit"),
	Mask					UMETA(DisplayName = "Mask"),
	Depth					UMETA(DisplayName = "Depth"),
	Normal					UMETA(DisplayName = "Normal"),
};

/**
 * 
 */
UCLASS(ClassGroup = (SL), DisplayName = "SL CV Scanner")
class ASLCVScanner : public AInfo
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASLCVScanner();

	// Dtor
	~ASLCVScanner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called when actor removed from game or game ended
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Set up any required references and connect to server
	void Init();

	// Start processing any incomming messages
	void Start();

	// Stop processing the messages, and disconnect from server
	void Finish(bool bForced = false);

	// Get init state
	bool IsInit() const { return bIsInit; };

	// Get started state
	bool IsStarted() const { return bIsStarted; };

	// Get finished state
	bool IsFinished() const { return bIsFinished; };

protected:
	// Request a high res screenshot
	void RequestScreenshotAsync();

	// Called when the screenshot is captured
	void ScreenshotCapturedCallback(int32 SizeX, int32 SizeY, const TArray<FColor>& InBitmap);
	
	// Set next view mode (return false if the last view mode was reached)
	bool SetNextViewMode();

	// Set next camera pose (return false if the last pose was reached)
	bool SetNextCameraPose();

	// Set next individual (return false if the last individual was reached)
	bool SetNextIndividual();

	//  Quit the editor when finished
	void QuitEditor();

private:
	// Apply the selected view mode
	void SetViewMode(ESLCVViewMode Mode);

	// Show original individual
	void ShowOriginalIndividual();

	// Show mask individual
	void ShowMaskIndividual();

	// Remove detachments and hide all actors in the world
	void SetWorldState();

	// Set screenshot image resolution
	void SetScreenshotResolution(FIntPoint InResolution);

	// Set the rendering parameters
	void SetRenderParams();

	// Get the individual manager from the world (or spawn a new one)
	bool SetIndividualManager();

	// Set the individuals to be scanned
	bool SetScanIndividuals();

	// Spawn a light actor which will also be used to move the camera around
	bool SetCameraPoseAndLightActor();

	// Create clones of the individuals with mask material
	bool SetMaskClones();

	// Generate sphere camera scan poses
	bool SetScanPoses(uint32 MaxNumPoints/*, float Radius = 1.f*/);

protected:
	// Skip auto init and start
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	uint8 bIgnore : 1;

	// Save images to file
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	uint8 bSaveToFile : 1;

	// True when all references are set and it is connected to the server
	uint8 bIsInit : 1;

	// True when active
	uint8 bIsStarted : 1;

	// True when done 
	uint8 bIsFinished : 1;
	
	// Rendering modes to include
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	TArray<ESLCVViewMode> ViewModes;

	// Maximal number of scan points on the sphere
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	uint32 MaxNumScanPoints = 32;

	// Image resolution
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FIntPoint Resolution = FIntPoint(640, 480);

	// Maximal item size
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	float MaxBoundsSphereRadius = 500.f;

	// Directional camera light intensity
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	float CameraLightIntensity = 1.6f;

	// Folder to store the images in
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FString TaskId;

	// Mongo server ip addres
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FString MongoServerIP = TEXT("127.0.0.1");

	// Knowrob server port
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	int32 MongoServerPort = 27017;

	// Keeps access to all the individuals in the world
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	ASLIndividualManager* IndividualManager;

	// Convenience actor for setting the camera pose (SetViewTarget(InActor))
	UPROPERTY()
	ADirectionalLight* CameraPoseAndLightActor;

private:
	// Camera poses on the sphere
	TArray<FTransform> CameraScanPoses;

	// Individuals to calibrate
	TArray<USLVisibleIndividual*> Individuals;

	// Clones with dynamic mask materials on
	TMap<USLVisibleIndividual*, AStaticMeshActor*> IndividualsMaskClones;

	// Current active view mode
	ESLCVViewMode CurrViewmode = ESLCVViewMode::NONE;

	// Current radius of the camera sphere poses
	float CurrCameraPoseSphereRadius;

	// Current individual index in the array
	int32 IndividualIdx = INDEX_NONE;

	// Current camera pose index in the array
	int32 CameraPoseIdx = INDEX_NONE;

	// Current view mode index in the array
	int32 ViewModeIdx = INDEX_NONE;

	// Used for triggering the screenshot request
	UGameViewportClient* ViewportClient;

	// The name of the current image
	FString CurrImageName;

	// The view mode post fix to be applied to the image name
	FString ViewModePostfix;

	// The camera pose post fix to be applied to the image name
	FString CameraPosePostfix;

	// The camera pose post fix to be applied to the image name
	FString IndividualPostfix;

	/* Constants */
	static constexpr auto DynMaskMatAssetPath = TEXT("/USemLog/Vision/M_SLDefaultMask.M_SLDefaultMask");
};