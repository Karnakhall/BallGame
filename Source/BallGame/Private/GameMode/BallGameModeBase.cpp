// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/BallGameModeBase.h"
#include "Kismet/GameplayStatics.h"

void ABallGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	EdibleEnemiesCount = 0;
	bSpawningFinished = false;
}

void ABallGameModeBase::PlayerLoss()
{
	if (bHasEnded) return;
	bHasEnded = true;
	
	UE_LOG(LogTemp, Warning, TEXT("GAME OVER!"));
	UGameplayStatics::SetGamePaused(GetWorld(), true);
}

void ABallGameModeBase::EnemySpawned(EEnemyType Type)
{
	if (Type == EEnemyType::Red_Strength || Type == EEnemyType::Yellow_Speed)
	{
		++EdibleEnemiesCount;
		++TotalEdibleSpawned;
	}
	UE_LOG(LogTemp, Log, TEXT("Spawned %d edible alive"), EdibleEnemiesCount);
}

void ABallGameModeBase::EnemyEaten(EEnemyType Type)
{
	if (Type == EEnemyType::Red_Strength || Type == EEnemyType::Yellow_Speed)
	{
		EdibleEnemiesCount = FMath::Max(0, EdibleEnemiesCount - 1);
	}
	UE_LOG(LogTemp, Log, TEXT("Eaten. Edible remaining: %d"), EdibleEnemiesCount);
	CheckWinCondition();
}

void ABallGameModeBase::NotifySpawningFinished()
{
	bSpawningFinished = true;
	CheckWinCondition();
}

void ABallGameModeBase::CheckWinCondition()
{
	if (bHasEnded) return;

	// Tryb “zjedz N” – jeśli ustawiono dodatnią liczbę celów
	if (EnemiesToWin > 0)
	{
		const int32 EatenSoFar = GetEatenSoFar(); // TotalEdibleSpawned - EdibleEnemiesCount
		if (EatenSoFar >= EnemiesToWin)
		{
			bHasEnded = true;
			UE_LOG(LogTemp, Warning, TEXT("YOU WIN! (Eaten %d / Target %d)"), EatenSoFar, EnemiesToWin);
			UGameplayStatics::SetGamePaused(GetWorld(), true);
			return;
		}
	}

	// Tryb “zjedz wszystkich” – gdy nie ma celu liczbowego
	if (EnemiesToWin <= 0)
	{
		if (bSpawningFinished && EdibleEnemiesCount <= 0)
		{
			bHasEnded = true;
			UE_LOG(LogTemp, Warning, TEXT("YOU WIN!"));
			UGameplayStatics::SetGamePaused(GetWorld(), true);
		}
	}
	
}

