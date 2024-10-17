// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include <Engine/LevelStreamingDynamic.h>
#include "EndlessWorldComponent.generated.h"

USTRUCT(BlueprintType)
struct FSubLevel
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level")
	float SpawnProbability = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level")
	TSoftObjectPtr<UWorld> Level;

};

USTRUCT(BlueprintType)
struct FWorldGeneration
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Generation")
	float ChunkSize = 5000.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Generation")
	TSoftObjectPtr<UWorld> StartingLevel;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Generation")
	TArray<FSubLevel> Levels;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ENDLESSWORLD_API UEndlessWorldComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UEndlessWorldComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
	int32 WorldSeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
	int32 MaxChunkViewDistance = 5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
	FWorldGeneration WorldGeneration;


private:
	void UpdateChunk(FVector PlayerCoords);
	void CreateStartingLevel();
	void CreateChunk(FVector ChunkCoords);
	void UnloadChunk(ULevelStreamingDynamic* LevelToUnload);
	void OnLevelLoadedInternal();
	void GenerateLevel(FSubLevel LevelInfo, FVector Location, FRotator Rotation);
	void ShuffleArray(TArray<int32>& Array);
	void SetSeedFromChunkCoords(FVector ChunkCoords);

	FVector GetPlayerChunkCoords();
	FRotator GenerateRandomRotator();

	bool IsChunkLoaded(FVector ChunkCoords);
	
private:
	APlayerController* PlayerController;
	APawn* PlayerPawn;

	FRandomStream RandomStream;

	FVector PlayerLocation;
	FVector LastPlayerLocation;
	TMap<ULevelStreamingDynamic*, FVector> LevelToLocationMap;
};
