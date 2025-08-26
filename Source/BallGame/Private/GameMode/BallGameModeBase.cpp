// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/BallGameModeBase.h"

#include "Kismet/GameplayStatics.h"

void ABallGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	EdibleEnemiesCount = 0;
}

void ABallGameModeBase::PlayerLoss()
{
	UE_LOG(LogTemp, Warning, TEXT("GAME OVER!"));
}

void ABallGameModeBase::EnemyEaten()
{
	EdibleEnemiesCount--;
	UE_LOG(LogTemp, Warning, TEXT("Enemy eaten! Remaining: %d"), EdibleEnemiesCount);

	if (EdibleEnemiesCount <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("YOU WIN!"));
		UGameplayStatics::SetGamePaused(GetWorld(), true);
	}
}

void ABallGameModeBase::EnemySpawned()
{
	EdibleEnemiesCount++;
}

