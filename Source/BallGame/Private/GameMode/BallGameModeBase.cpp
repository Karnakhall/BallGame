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
	UE_LOG(LogTemp, Warning, TEXT("GAME OVER!"));
	UGameplayStatics::SetGamePaused(GetWorld(), true);
}

void ABallGameModeBase::EnemySpawned(EEnemyType Type)
{
	if (Type == EEnemyType::Red_Strength || Type == EEnemyType::Yellow_Speed)
	{
		++EdibleEnemiesCount;
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
	if (bSpawningFinished && EdibleEnemiesCount <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("YOU WIN!"));
		UGameplayStatics::SetGamePaused(GetWorld(), true);
	}
}

