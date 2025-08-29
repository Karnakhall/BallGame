// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Characters/BallEnemy.h"
#include "BallGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class BALLGAME_API ABallGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void PlayerLoss();

	UFUNCTION(BlueprintCallable)
	void EnemySpawned(EEnemyType Type);

	UFUNCTION(BlueprintCallable)
	void EnemyEaten(EEnemyType Type);
	
	UFUNCTION(BlueprintCallable)
	void NotifySpawningFinished();
	
protected:
	virtual void BeginPlay() override;

private:
	
	int32 EdibleEnemiesCount = 0;
	bool bSpawningFinished = false;
	bool bHasEnded = false;

	UPROPERTY(EditDefaultsOnly, Category = "Rules")
	int32 EnemiesToWin = 20;

	void CheckWinCondition();
};
