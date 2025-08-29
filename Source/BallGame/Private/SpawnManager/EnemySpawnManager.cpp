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
#include "AbilitySystem/AttributeSet/BallAttributeSetBase.h"
#include "Components/SphereComponent.h"

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
	if (TotalEnemiesToSpawn >= 0 && SpawnedSoFar >= TotalEnemiesToSpawn)
	{
		GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
		if (GameModeRef.IsValid())
		{
			GameModeRef->NotifySpawningFinished();
		}
		return;
	}
	
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
		if (Info.EnemyClass) { TotalChance += Info.SpawnChance; }
	}
	if (TotalChance <= 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnManager: Brak prawidłowych klas w EnemySpawnPool!"))
		return;
	}
	

	float RandomValue = FMath::FRandRange(0.f, TotalChance);
	TSubclassOf<ABallEnemy> SelectedEnemyClass = nullptr;

	for (const FEnemySpawnInfo& Info : EnemySpawnPool)
	{
		if (!Info.EnemyClass) continue;
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
	const FVector PlayerLocation = PlayerPawn->GetActorLocation();
	const float RandomDistance = FMath::FRandRange(SpawnRadiusMin, SpawnRadiusMax);
	FVector RandomDirection = FMath::VRand();
	RandomDirection.Z = 0;
	RandomDirection.Normalize();

	FVector SpawnLocation = PlayerLocation + RandomDirection * RandomDistance;
	SpawnLocation.Z = PlayerLocation.Z;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Spawnowanie
	ABallEnemy* NewEnemy = GetWorld()->SpawnActor<ABallEnemy>(SelectedEnemyClass, SpawnLocation, FRotator::ZeroRotator, Params);
	if (!NewEnemy)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnManager: Przeciwnik się nie zespawnował!"));
		return;
	}

	//Nadanie statystyk przez GAS
	float DistanceFromOrigin = RandomDistance;
	float CalculatedStrength = BaseStrength + (DistanceFromOrigin * StatsScaling);
	float CalculatedSpeed = BaseSpeed + (DistanceFromOrigin * StatsScaling);

	if (UAbilitySystemComponent* ASC = NewEnemy->GetAbilitySystemComponent())
	{
		if ( EnemyStatsEffect)
		{
			FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
			ContextHandle.AddSourceObject(this);
			FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(EnemyStatsEffect, 1, ContextHandle);

			if (SpecHandle.IsValid())
			{
				const FGameplayTag StrengthTag = FGameplayTag::RequestGameplayTag(FName("Data.Attribute.Strength"), false);
				const FGameplayTag SpeedTag = FGameplayTag::RequestGameplayTag(FName("Data.Attribute.Speed"),    false);

				//bool bAppliedSetByCaller = false;

				if (StrengthTag.IsValid() && SpeedTag.IsValid())
				{
					SpecHandle.Data->SetSetByCallerMagnitude(StrengthTag, CalculatedStrength);
					SpecHandle.Data->SetSetByCallerMagnitude(SpeedTag, CalculatedSpeed);

					UE_LOG(LogTemp, Log, TEXT("Init enemy S=%.1f, V=%.1f"), CalculatedStrength, CalculatedSpeed);
					ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
					//bAppliedSetByCaller = true;
				}
				else
				{
					ASC->SetNumericAttributeBase(UBallAttributeSetBase::GetStrengthAttribute(), CalculatedStrength);
					ASC->SetNumericAttributeBase(UBallAttributeSetBase::GetSpeedAttribute(), CalculatedSpeed);
					UE_LOG(LogTemp, Warning, TEXT("SetByCaller TAGS missing. Using fallback set base attributes."));
				}
			}
		}
	}
	

	if (NewEnemy && PlayerPawn)
	{
		const USphereComponent* PlayerSphere = Cast<USphereComponent>(Cast<ABallPawnBase>(PlayerPawn)->GetCollisionSphere());
		const USphereComponent* EnemySphere  = NewEnemy->GetCollisionSphere();

		const float PlayerRadius = PlayerSphere ? PlayerSphere->GetScaledSphereRadius() : 50.f;
		const float EnemyRadius  = EnemySphere  ? EnemySphere->GetScaledSphereRadius()  : 50.f;

		const float FloorZ = PlayerPawn->GetActorLocation().Z - PlayerRadius;
		FVector L = NewEnemy->GetActorLocation();
		L.Z = FloorZ + EnemyRadius;
		NewEnemy->SetActorLocation(L, false);
	}
	
	// Nowy przeciwnik informacja dla GameMode
	if (GameModeRef.IsValid())
	{
		GameModeRef->EnemySpawned(NewEnemy->GetEnemyType());
	}

	++SpawnedSoFar;
	if (TotalEnemiesToSpawn >= 0 && SpawnedSoFar >= TotalEnemiesToSpawn)
	{
		GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
		if (GameModeRef.IsValid())
		{
			GameModeRef->NotifySpawningFinished();
		}
	}
	
}


