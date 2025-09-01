// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawnManager/EnemySpawnManager.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "GameMode/BallGameModeBase.h"
#include "Characters/BallEnemy.h"
#include "Characters/BallPawnBase.h"           
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayTagsManager.h"
#include "AbilitySystem/AttributeSet/BallAttributeSetBase.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"    

AEnemySpawnManager::AEnemySpawnManager()
{
 	
	PrimaryActorTick.bCanEverTick = false;

}


void AEnemySpawnManager::BeginPlay()
{
	Super::BeginPlay();

	CacheFloorBounds();

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

	// Wyznacz legalną pozycję spawnu (z granicami)
	FVector SpawnLocation;
	float DistanceFromPlayer = 0.f;
	if (!GetRandomSpawnLocation(SpawnLocation, DistanceFromPlayer))
	{
		// Brak legalnego punktu w tej turze (np. przy rogu) – po prostu pomiń spawn
		return;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Spawnowanie
	ABallEnemy* NewEnemy = GetWorld()->SpawnActor<ABallEnemy>(SelectedEnemyClass, SpawnLocation, FRotator::ZeroRotator, Params);
	if (!NewEnemy)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnManager: Przeciwnik się nie zespawnował!"));
		return;
	}

	if (bDrawFloorBounds)
	{
		DrawDebugPoint(GetWorld(), SpawnLocation, 120.f, FColor::Green, false, 2.f);
	}

	//Nadanie statystyk przez GAS
	float DistanceFromOrigin = DistanceFromPlayer;
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

				if (StrengthTag.IsValid() && SpeedTag.IsValid())
				{
					SpecHandle.Data->SetSetByCallerMagnitude(StrengthTag, CalculatedStrength);
					SpecHandle.Data->SetSetByCallerMagnitude(SpeedTag, CalculatedSpeed);

					UE_LOG(LogTemp, Log, TEXT("Init enemy S=%.1f, V=%.1f"), CalculatedStrength, CalculatedSpeed);
					ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
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

void AEnemySpawnManager::CacheFloorBounds()
{
	CachedFloorBounds = FBox(EForceInit::ForceInitToZero);
	FloorTopZ = 0.f;

	if (FloorActor.IsValid())
	{
		// AABB wszystkich komponentów aktora (world-space)
		CachedFloorBounds = FloorActor->GetComponentsBoundingBox(true);
		FloorTopZ = CachedFloorBounds.Max.Z;

		if (bDrawFloorBounds)
		{
			// narysuj pudełko dla podglądu
			const FVector Center = CachedFloorBounds.GetCenter();
			const FVector Extent = CachedFloorBounds.GetExtent();
			DrawDebugBox(GetWorld(), Center, Extent, FColor::Cyan, true, 10.f, 0, 2.f);
		}
	}
}

bool AEnemySpawnManager::IsInsideFloor2D(const FVector& Loc) const
{
	if (!CachedFloorBounds.IsValid) return true; // brak podłogi = brak ograniczeń

	return  Loc.X >= (CachedFloorBounds.Min.X + EdgePadding) &&
			Loc.X <= (CachedFloorBounds.Max.X - EdgePadding) &&
			Loc.Y >= (CachedFloorBounds.Min.Y + EdgePadding) &&
			Loc.Y <= (CachedFloorBounds.Max.Y - EdgePadding);
}

FVector AEnemySpawnManager::ClampToFloor2D(FVector Loc) const
{
	if (!CachedFloorBounds.IsValid) return Loc;

	Loc.X = FMath::Clamp(Loc.X, CachedFloorBounds.Min.X + EdgePadding, CachedFloorBounds.Max.X - EdgePadding);
	Loc.Y = FMath::Clamp(Loc.Y, CachedFloorBounds.Min.Y + EdgePadding, CachedFloorBounds.Max.Y - EdgePadding);
	return Loc;
}

bool AEnemySpawnManager::IsInsideBounds2D(const FVector& Loc) const
{
	if (!SpawnBounds.IsValid()) return true; // brak granic = zawsze OK

	const FBox Bounds = SpawnBounds->GetComponentsBoundingBox(true);
	return  Loc.X >= Bounds.Min.X + EdgePadding &&
			Loc.X <= Bounds.Max.X - EdgePadding &&
			Loc.Y >= Bounds.Min.Y + EdgePadding &&
			Loc.Y <= Bounds.Max.Y - EdgePadding;
}

FVector AEnemySpawnManager::ClampToBounds2D(FVector Loc) const
{
	if (!SpawnBounds.IsValid()) return Loc;

	const FBox Bounds = SpawnBounds->GetComponentsBoundingBox(true);
	Loc.X = FMath::Clamp(Loc.X, Bounds.Min.X + EdgePadding, Bounds.Max.X - EdgePadding);
	Loc.Y = FMath::Clamp(Loc.Y, Bounds.Min.Y + EdgePadding, Bounds.Max.Y - EdgePadding);
	return Loc;
}

bool AEnemySpawnManager::GetRandomSpawnLocation(FVector& OutLocation, float& OutDistanceFromPlayer)
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn) return false;

	const FVector PlayerLoc = PlayerPawn->GetActorLocation();
	const float PlaneZ = PlayerLoc.Z; // lub FloorTopZ, jeśli wolisz podłogę

	// Funkcje lokalne do sprawdzania granic (najpierw TriggerBox, potem Floor)
	auto InBounds2D = [this](const FVector& P)->bool
	{
		if (SpawnBounds.IsValid())   return IsInsideBounds2D(P);
		if (FloorActor.IsValid())    return IsInsideFloor2D(P);
		return true; // brak granic
	};
	auto Clamp2D = [this](const FVector& P)->FVector
	{
		if (SpawnBounds.IsValid())   return ClampToBounds2D(P);
		if (FloorActor.IsValid())    return ClampToFloor2D(P);
		return P;
	};

	int32 Attempts = 0;
	while (Attempts++ < MaxSpawnAttempts)
	{
		const float Angle   = FMath::FRandRange(0.f, 2.f * PI);
		const float Radius  = FMath::FRandRange(SpawnRadiusMin, SpawnRadiusMax);
		const FVector2D Dir(FMath::Cos(Angle), FMath::Sin(Angle));

		FVector Candidate(PlayerLoc.X + Dir.X * Radius,
						  PlayerLoc.Y + Dir.Y * Radius,
						  PlaneZ);

		if (InBounds2D(Candidate))
		{
			OutLocation = Candidate;
			OutDistanceFromPlayer = FVector::Dist2D(OutLocation, PlayerLoc);
			return true;
		}

		if (bClampOutsideToFloor)
		{
			Candidate = Clamp2D(Candidate);
			Candidate.Z = PlaneZ;
			OutLocation = Candidate;
			OutDistanceFromPlayer = FVector::Dist2D(OutLocation, PlayerLoc);
			return true;
		}

		// w przeciwnym razie – próbuj dalej
	}

	return false; // brak legalnego punktu (np. stoisz w samym rogu)
}


