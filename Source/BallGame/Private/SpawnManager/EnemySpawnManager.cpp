// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawnManager/EnemySpawnManager.h"

#include "GameMode/BallGameModeBase.h"
#include "Kismet/GameplayStatics.h"


AEnemySpawnManager::AEnemySpawnManager()
{
 	
	PrimaryActorTick.bCanEverTick = false;

}


void AEnemySpawnManager::BeginPlay()
{
	Super::BeginPlay();

	GameModeRef = Cast<ABallGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));

	GetWorld()->GetTimerManager().SetTimer(
		SpawnTimerHandle,
		this,
		&AEnemySpawnManager::SpawnEnemy,
		SpawnInterval,
		true,
		2.0f);
}

void AEnemySpawnManager::SpawnEnemy()
{
	
}


