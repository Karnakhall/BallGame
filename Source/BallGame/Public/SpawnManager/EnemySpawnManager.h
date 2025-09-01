// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TriggerBox.h"
#include "EnemySpawnManager.generated.h"

class ABallEnemy;
class UGameplayEffect;
class ABallGameModeBase;

USTRUCT(BlueprintType)
struct FEnemySpawnInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<ABallEnemy> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.1"))
	float SpawnChance = 1.0f;
};

UCLASS()
class BALLGAME_API AEnemySpawnManager : public AActor
{
	GENERATED_BODY()
	
public:	
	AEnemySpawnManager();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning | Timing")
	float SpawnInterval = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning | Location")
	float SpawnRadiusMin = 1000.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning | Location")
	float SpawnRadiusMax = 3000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning | Pool")
	TArray<FEnemySpawnInfo> EnemySpawnPool;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning | Scaling")
	float StatsScaling = 0.005f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning | Scaling")
	float BaseStrength = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning | Scaling")
	float BaseSpeed = 20.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning | GAS")
	TSubclassOf<UGameplayEffect> EnemyStatsEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning | Timing")
	int32 TotalEnemiesToSpawn = -1;

	UPROPERTY(EditAnywhere, Category="Spawning | Bounds")
	TWeakObjectPtr<ATriggerBox> SpawnBounds;

	UPROPERTY(EditAnywhere, Category="Spawning | Bounds")
	float EdgePadding = 150.f;
	
	// Referencja do podłogi (wskazujesz w edytorze aktora Floor/Plane/Landscape)
	UPROPERTY(EditAnywhere, Category="Spawning | Bounds")
	TWeakObjectPtr<AActor> FloorActor;
	
	// ile prób losowania zanim zrezygnujemy/zaklamrujemy
	UPROPERTY(EditAnywhere, Category="Spawning | Bounds")
	int32 MaxSpawnAttempts = 16;

	// jeśli true – zamiast odrzucać, klamruj do pudełka
	UPROPERTY(EditAnywhere, Category="Spawning | Bounds")
	bool bClampOutsideToFloor = true;

	// Debug
	UPROPERTY(EditAnywhere, Category="Spawning | Debug")
	bool bDrawFloorBounds = false;

	
private:
	void SpawnEnemy();

	FTimerHandle SpawnTimerHandle;

	TWeakObjectPtr<ABallGameModeBase> GameModeRef;
	int32 SpawnedSoFar = 0;

	FBox CachedFloorBounds;
	bool bFloorBoundsValid = false;
	float FloorTopZ = 0.f; 

	void CacheFloorBounds();
	bool IsInsideFloor2D(const FVector& Loc) const;
	FVector ClampToFloor2D(FVector Loc) const;
	
	bool IsInsideBounds2D(const FVector& Loc) const;
	FVector ClampToBounds2D(FVector Loc) const;
	bool GetRandomSpawnLocation(FVector& OutLocation, float& OutDistanceFromPlayer); 
};
