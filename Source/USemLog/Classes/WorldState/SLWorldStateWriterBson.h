// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "ISLWorldStateWriter.h"
#if WITH_LIBMONGO
#include "bson.h"
#endif //WITH_LIBMONGO

// Forward declaration
class FSLWorldStateAsyncWorker;

/**
 * Raw data logger to bson format
 */
class FSLWorldStateWriterBson : public ISLWorldStateWriter
{
public:
	// Default constr
	FSLWorldStateWriterBson();

	// Constructor with init
	FSLWorldStateWriterBson(FSLWorldStateAsyncWorker* InWorkerParent, const FString& LogDirectory, const FString& EpisodeId);

	// Destr
	virtual ~FSLWorldStateWriterBson();

	// Init
	void Init(FSLWorldStateAsyncWorker* InWorkerParent, const FString& LogDirectory, const FString& EpisodeId);

	// Called to write the data
	virtual void WriteData() override;

private:
	// Set the file handle for the logger
	void SetFileHandle(const FString& LogDirectory, const FString& InEpisodeId);
#if WITH_LIBMONGO
	// Add actors
	void AddActors(bson_t& OutBsonEntitiesArr);

	// Add components
	void AddComponents(bson_t& OutBsonEntitiesArr);

	// Get entry as Bson object
	bson_t GetAsBsonEntry(const FString& InId,
		const FString& InClass,
		const FVector& InLoc,
		const FQuat& InQuat);

	// Write entry to file
	void WriteData(uint8* memorybuffer, int64 bufferlen);
#endif //WITH_LIBMONGO
	// Pointer to worker parent (access to raw data structure)
	FSLWorldStateAsyncWorker* WorkerParent;

	// File handle to write the raw data to file
	IFileHandle* FileHandle;
};