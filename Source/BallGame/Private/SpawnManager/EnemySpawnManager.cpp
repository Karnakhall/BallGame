// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawnManager/EnemySpawnManager.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "GameMode/BallGameModeBase.h"
#include "Characters/BallEnemy.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayTagsManager.h"





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
	// Warunki początkowe
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnManager: Nie można znaleźć gracza!"));
		return;
	}

	if (EnemySpawnPool.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnManager: Pula przeciwników (EnemySpawnPool) jest pusta!"))
		return;
	}

	// Przeciwnik z puli
	float TotalChance = 0.f;
	for (const FEnemySpawnInfo& Info : EnemySpawnPool)
	{
		TotalChance += Info.SpawnChance;
	}

	float RandomValue = FMath::FRandRange(0.f, TotalChance);
	TSubclassOf<ABallEnemy> SelectedEnemyClass;

	for (const FEnemySpawnInfo& Info : EnemySpawnPool)
	{
		if (RandomValue <= Info.SpawnChance)
		{
			SelectedEnemyClass = Info.EnemyClass;
			break;
		}
		RandomValue -= Info.SpawnChance;
	}

	if (!SelectedEnemyClass)
	{
		SelectedEnemyClass = EnemySpawnPool[0].EnemyClass;
	}

	// Obliczenia miejsca Spawnu
	FVector PlayerLocation = PlayerPawn->GetActorLocation();
	float RandomDistance = FMath::FRandRange(SpawnRadiusMin, SpawnRadiusMax);
	FVector RandomDirection = FMath::VRand();
	RandomDirection.Z = 0;
	RandomDirection.Normalize();

	FVector SpawnLocation = PlayerLocation + RandomDirection * RandomDistance;

	// Spawnowanie
	ABallEnemy* NewEnemy = GetWorld()->SpawnActor<ABallEnemy>(SelectedEnemyClass, SpawnLocation, FRotator::ZeroRotator);
	if (!NewEnemy)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnManager: Przeciwnik się nie zespawnował!"));
		return;
	}

	//Nadanie statystyk przez GAS
	float DistanceFromOrigin = SpawnLocation.Size();
	float CalculatedStrength = BaseStrength + (DistanceFromOrigin * StatsScaling);
	float CalculatedSpeed = BaseSpeed + (DistanceFromOrigin * StatsScaling);

	UAbilitySystemComponent* ASC = NewEnemy->GetAbilitySystemComponent();
	if (ASC && EnemyStatsEffect)
	{
		FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
		ContextHandle.AddSourceObject(this);
		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(EnemyStatsEffect, 1, ContextHandle);

		if (SpecHandle.IsValid())
		{
			SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Attribute.Strength")), CalculatedStrength);
			SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Attribute.Speed")), CalculatedSpeed);
			ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}

	// Nowy przeciwnik informacja dla GameMode
	if (GameModeRef.IsValid())
	{
		GameModeRef->EnemySpawned();
	}
	
}


