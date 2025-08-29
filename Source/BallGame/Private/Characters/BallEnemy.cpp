// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/BallEnemy.h"
#include "Characters/BallPlayer.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AttributeSet/BallAttributeSetBase.h"
#include "Components/SphereComponent.h"
#include "GameMode/BallGameModeBase.h"
#include "Kismet/GameplayStatics.h"



ABallEnemy::ABallEnemy()
{
	PrimaryActorTick.bCanEverTick = true;
	
	bUsePhysicsMovement = false;
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	
	FloatingMovement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatingMovement"));
	FloatingMovement->bConstrainToPlane = true;
	FloatingMovement->SetPlaneConstraintNormal(FVector::UpVector);
	FloatingMovement->MaxSpeed = 600.f; // nadpiszemy w Tick
}

void ABallEnemy::BeginPlay()
{
	Super::BeginPlay();
	PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	/*// Ustal płaszczyznę XY na wysokości gracza i skoryguj Z
	if (PlayerPawn.IsValid() && FloatingMovement)
	{
		FloatingMovement->SetPlaneConstraintOrigin(FVector(0,0, PlayerPawn->GetActorLocation().Z));

		FVector Location = GetActorLocation();
		Location.Z = PlayerPawn->GetActorLocation().Z;
		SetActorLocation(Location, false);
	}*/

	if (FloatingMovement)
	{
		// Bardzo ważne – inaczej AddMovementInput nic nie robi
		FloatingMovement->SetUpdatedComponent(SphereComponent);

		// Ruch w XY na stałej wysokości
		FloatingMovement->bConstrainToPlane = true;
		FloatingMovement->SetPlaneConstraintNormal(FVector::UpVector);

		float TargetZ = GetActorLocation().Z;
		if (PlayerPawn.IsValid())
		{
			TargetZ = PlayerPawn->GetActorLocation().Z;
		}
		FloatingMovement->SetPlaneConstraintOrigin(FVector(0, 0, TargetZ));
		FloatingMovement->bSnapToPlaneAtStart = true; // wyrównaj Z do płaszczyzny
	}

	// skoryguj Z od razu po starcie
	if (PlayerPawn.IsValid())
	{
		FVector L = GetActorLocation();
		L.Z = PlayerPawn->GetActorLocation().Z;
		SetActorLocation(L, false);
	}
}

void ABallEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!PlayerPawn.IsValid()) return;

	const float MyStrength = AttributeSet->GetStrength();
	const float MySpeed = AttributeSet->GetSpeed();

	const UAbilitySystemComponent* PlayerASC = PlayerPawn->FindComponentByClass<UAbilitySystemComponent>();
	if (!PlayerASC) return;

	const float PlayerStrength = PlayerASC->GetNumericAttribute(UBallAttributeSetBase::GetStrengthAttribute());

	FVector DirectionToPlayer = PlayerPawn->GetActorLocation() - GetActorLocation();
	DirectionToPlayer.Z = 0;

	const float Distance = DirectionToPlayer.SizeSquared2D();

	if (Distance < KINDA_SMALL_NUMBER) return;
	DirectionToPlayer.Normalize();

	const bool bChase = (MyStrength > PlayerStrength + StrengthHysteresis);
	const FVector MoveDirection = bChase ? DirectionToPlayer : -DirectionToPlayer;

	// Prędkość zależna od atrybutu Speed
	if (FloatingMovement)
	{
		FloatingMovement->MaxSpeed = FMath::Max(100.f, MySpeed * AISpeedScale);
	}

	AddMovementInput(MoveDirection, 1.f);
}

void ABallEnemy::BeEaten(class ABallPlayer* Player)
{
	UAbilitySystemComponent* PlayerASC = Player ? Player->GetAbilitySystemComponent() : nullptr;
	if (PlayerASC && EffectToApply)
	{
		FGameplayEffectContextHandle ContextHandle = PlayerASC->MakeEffectContext();
		ContextHandle.AddSourceObject(this);
		FGameplayEffectSpecHandle SpecHandle = PlayerASC->MakeOutgoingSpec(EffectToApply, 1, ContextHandle);
		if (SpecHandle.IsValid())
		{
			PlayerASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
	if (auto* GameMode = Cast<ABallGameModeBase>(UGameplayStatics::GetGameMode(this))) { GameMode->EnemyEaten(EnemyType); }
	Destroy();
}

void ABallEnemy::CollideWithStrongerPlayer(class ABallPlayer* Player)
{
	if (EnemyType == EEnemyType::Purple_Damage)
	{
		BeEaten(Player);
	}
}


