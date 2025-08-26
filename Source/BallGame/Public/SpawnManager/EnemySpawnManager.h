// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
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
	
private:
	void SpawnEnemy();

	FTimerHandle SpawnTimerHandle;

	TWeakObjectPtr<ABallGameModeBase> GameModeRef;
};
