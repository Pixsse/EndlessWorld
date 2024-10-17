// Fill out your copyright notice in the Description page of Project Settings.


#include "EndlessWorldComponent.h"
#include <Engine/LevelStreamingDynamic.h>
#include <Kismet/GameplayStatics.h>


// Sets default values for this component's properties
UEndlessWorldComponent::UEndlessWorldComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UEndlessWorldComponent::BeginPlay()
{
	Super::BeginPlay();

	PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	PlayerPawn = PlayerController->GetPawn();

	LastPlayerLocation = FVector(
		PlayerPawn->GetActorLocation().X,
		PlayerPawn->GetActorLocation().Y,
		0
	);

    UpdateChunk(GetPlayerChunkCoords());
	
}


// Called every frame
void UEndlessWorldComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!PlayerController) return;
    if (!PlayerPawn) return;

    PlayerLocation = PlayerPawn->GetActorLocation();

    FVector CurrentChunkCoords = FVector(
        FMath::RoundToInt(PlayerLocation.X / WorldGeneration.ChunkSize),
        FMath::RoundToInt(PlayerLocation.Y / WorldGeneration.ChunkSize),
        0
    );

    FVector LastChunkCoords = FVector(
        FMath::RoundToInt(LastPlayerLocation.X / WorldGeneration.ChunkSize),
        FMath::RoundToInt(LastPlayerLocation.Y / WorldGeneration.ChunkSize),
        0
    );

    if (!CurrentChunkCoords.Equals(LastChunkCoords))
    {
        LastPlayerLocation = PlayerLocation;
        UpdateChunk(CurrentChunkCoords);
    }


}

void UEndlessWorldComponent::UpdateChunk(FVector PlayerCoords)
{
    TSet<FVector> NeededChunks;

    for (int32 x = -MaxChunkViewDistance; x <= MaxChunkViewDistance; ++x)
    {
        for (int32 y = -MaxChunkViewDistance; y <= MaxChunkViewDistance; ++y)
        {
            FVector ChunkCoords = FVector(
                FMath::RoundToInt(PlayerCoords.X) + x,
                FMath::RoundToInt(PlayerCoords.Y) + y,
                0
            );

            NeededChunks.Add(ChunkCoords);

        }
    }

    TArray<ULevelStreamingDynamic*> ChunksToUnload;

    for (auto It = LevelToLocationMap.CreateIterator(); It; ++It)
    {
        ULevelStreamingDynamic* StreamingLevel = It.Key();
        FVector ChunkLocation = It.Value();
        FVector ChunkCoords = ChunkLocation / WorldGeneration.ChunkSize;

        if (!NeededChunks.Contains(ChunkCoords))
        {
            ChunksToUnload.Add(StreamingLevel);
            It.RemoveCurrent();
        }
    }

    for (FVector& ChunkCoords : NeededChunks)
    {
        if (!IsChunkLoaded(ChunkCoords))
        {
            if (ChunkCoords.Equals(FVector::ZeroVector) && !WorldGeneration.StartingLevel.IsNull())
            {
                CreateStartingLevel();
            }
            else
            {
                CreateChunk(ChunkCoords);
            }
        }
    }

    for (ULevelStreamingDynamic* LevelToUnload : ChunksToUnload)
    {
       UnloadChunk(LevelToUnload);
    }
}

void UEndlessWorldComponent::CreateStartingLevel()
{
    bool bOutSuccess = true;
    ULevelStreamingDynamic* StreamingLevel = ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(this, WorldGeneration.StartingLevel, FVector::ZeroVector, FRotator::ZeroRotator, bOutSuccess);

    if (StreamingLevel)
    {
        StreamingLevel->OnLevelLoaded.AddUniqueDynamic(this, &UEndlessWorldComponent::OnLevelLoadedInternal);
        LevelToLocationMap.Add(StreamingLevel, FVector::ZeroVector);
    }

}

void UEndlessWorldComponent::CreateChunk(FVector ChunkCoords)
{
    if (IsChunkLoaded(ChunkCoords)) return;
    if (WorldGeneration.Levels.Num() <= 0) return;

     FVector ChunkLocation = ChunkCoords * WorldGeneration.ChunkSize;
     FRotator RandomRotator = GenerateRandomRotator();

     SetSeedFromChunkCoords(ChunkCoords);

     TArray<int32> LevelIndices;
     for (int32 i = 0; i < WorldGeneration.Levels.Num(); ++i)
     {
         LevelIndices.Add(i);
     }

     ShuffleArray(LevelIndices);

     float TotalProbability = 0.f;
     for (FSubLevel& Level : WorldGeneration.Levels)
     {
         TotalProbability += Level.SpawnProbability;
     }

     float RandomPoint = RandomStream.FRandRange(0.f, TotalProbability);
     float AccumulatedProbability = 0.f;

     for (int32 Index : LevelIndices)
     {
         FSubLevel& Level = WorldGeneration.Levels[Index];
         AccumulatedProbability += Level.SpawnProbability;
         if (RandomPoint <= AccumulatedProbability)
         {
             GenerateLevel(Level, ChunkLocation, RandomRotator);
             return;
         }
     }
}

void UEndlessWorldComponent::UnloadChunk(ULevelStreamingDynamic* LevelToUnload)
{
    if (!LevelToUnload) return;
    
    LevelToUnload->SetShouldBeLoaded(false);
    LevelToUnload->SetShouldBeVisible(false);
    LevelToLocationMap.Remove(LevelToUnload);
    
}

void UEndlessWorldComponent::OnLevelLoadedInternal()
{

}

void UEndlessWorldComponent::GenerateLevel(FSubLevel LevelInfo, FVector Location, FRotator Rotation)
{

    for (auto& Pair : LevelToLocationMap)
    {
        if (Pair.Value.Equals(Location)) return;
    }

    bool bOutSuccess = true;
    ULevelStreamingDynamic* StreamingLevel = ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(this, LevelInfo.Level, Location, Rotation, bOutSuccess);

    if (StreamingLevel)
    {

        if (!StreamingLevel->IsLevelLoaded())
        {
            StreamingLevel->OnLevelLoaded.AddUniqueDynamic(this, &UEndlessWorldComponent::OnLevelLoadedInternal);
            LevelToLocationMap.Add(StreamingLevel, Location);

            StreamingLevel->ShouldBeLoaded();
            StreamingLevel->ShouldBeVisible();
        }
    }

}

void UEndlessWorldComponent::ShuffleArray(TArray<int32>& Array)
{
    int32 LastIndex = Array.Num() - 1;

    for (int32 i = 0; i < LastIndex; i++)
    {
        int32 RandomIndex = RandomStream.RandRange(i, LastIndex);
        Array.Swap(i, RandomIndex);
    }
}

void UEndlessWorldComponent::SetSeedFromChunkCoords(FVector ChunkCoords)
{
    int32 SeedX = FMath::RoundToInt(ChunkCoords.X);
    int32 SeedY = FMath::RoundToInt(ChunkCoords.Y);

    int32 OffsetSeed = SeedX * 73856093 ^ SeedY * 19349663 ^ WorldSeed;

    OffsetSeed = OffsetSeed % MAX_int32;

    RandomStream.Initialize(OffsetSeed);
}

FVector UEndlessWorldComponent::GetPlayerChunkCoords()
{
	return FVector(
		FMath::RoundToInt(PlayerLocation.X / WorldGeneration.ChunkSize),
		FMath::RoundToInt(PlayerLocation.Y / WorldGeneration.ChunkSize),
		0
	);
}

FRotator UEndlessWorldComponent::GenerateRandomRotator()
{
    int32 RandomRotation = RandomStream.RandRange(0, 3) * 90;
    return FRotator(0, RandomRotation, 0);
}

bool UEndlessWorldComponent::IsChunkLoaded(FVector ChunkCoords)
{
    if (LevelToLocationMap.IsEmpty()) return false;

    FVector ChunkLocation = ChunkCoords * WorldGeneration.ChunkSize;
    for (auto& Pair : LevelToLocationMap)
    {
        if (Pair.Value.Equals(ChunkLocation))
        {
            return true;
        }
    }
    return false;
}

