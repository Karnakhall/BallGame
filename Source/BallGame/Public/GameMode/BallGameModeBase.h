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

	UFUNCTION(BlueprintPure, Category="Rules")
	int32 GetEdibleRemaining() const { return EdibleEnemiesCount; }

	UFUNCTION(BlueprintPure, Category="Rules")
	int32 GetEdibleTotal() const { return TotalEdibleSpawned; }

	UFUNCTION(BlueprintPure, Category="Rules")
	int32 GetEnemiesToWin() const { return EnemiesToWin;}

	// Ile do tej pory zjedzono jadalnych (Red/Yellow)
	UFUNCTION(BlueprintPure, Category="Rules")
	int32 GetEatenSoFar() const { return FMath::Max(0, TotalEdibleSpawned - EdibleEnemiesCount); }

	// Ile jeszcze trzeba zjeść, żeby wygrać (uwzględnia EnemiesToWin; dla 0 => tryb “zjedz wszystkich”)
	UFUNCTION(BlueprintPure, Category="Rules")
	int32 GetRemainingToWin() const
	{
		if (EnemiesToWin <= 0) return EdibleEnemiesCount; // tryb “zjedz wszystkich”
		const int32 Eaten = GetEatenSoFar();
		return FMath::Max(0, EnemiesToWin - Eaten);
	}
	
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
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Rules", meta=(AllowPrivateAccess="true"))
	int32 EdibleEnemiesCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Rules", meta=(AllowPrivateAccess="true"))
	int32 TotalEdibleSpawned = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Rules", meta=(AllowPrivateAccess="true"))
	bool bSpawningFinished = false;
	
	bool bHasEnded = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rules", meta=(AllowPrivateAccess="true"))
	int32 EnemiesToWin = 20;

	void CheckWinCondition();
};
