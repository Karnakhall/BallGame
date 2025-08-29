// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/BallPlayer.h"
#include "AbilitySystem/AttributeSet/BallAttributeSetBase.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "Characters/BallEnemy.h"
#include "Components/SphereComponent.h"
#include "GameMode/BallGameModeBase.h"
#include "Kismet/GameplayStatics.h"


ABallPlayer::ABallPlayer()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABallPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsMoving)
	{
		FVector CurrentLocation = GetActorLocation();
		FVector Direction = MoveTargetLocation - CurrentLocation;
		Direction.Z = 0;

		if (Direction.SizeSquared() <= 300.f)
		{
			StopMoveInput();
			return;
		}
		Direction.Normalize();

		const float CurrentSpeed = AttributeSet->GetSpeed();
		const float CurrentStrength = FMath::Max(1.f, AttributeSet->GetStrength());

		float ForceMagnitude = 10000.f * (CurrentSpeed / CurrentStrength);

		SphereComponent->WakeAllRigidBodies();
		SphereComponent->AddForce(Direction * ForceMagnitude, NAME_None, true);
	}
	else
	{
		
	}
}

void ABallPlayer::SetMoveTarget(const FVector& TargetLocation)
{
	MoveTargetLocation = TargetLocation;
	bIsMoving = true;
}

void ABallPlayer::StopMoveInput()
{
	bIsMoving = false;

	// lekkie “zduszenie” prędkości i obrotów
	if (SphereComponent)
	{
		FVector V = SphereComponent->GetPhysicsLinearVelocity();
		V.Z = 0; // 2D
		SphereComponent->SetPhysicsLinearVelocity(V * 0.8f); // natychmiastowe ścięcie 80%
		FVector AV = SphereComponent->GetPhysicsAngularVelocityInDegrees();
		SphereComponent->SetPhysicsAngularVelocityInDegrees(AV * 0.8f);
	}
}

void ABallPlayer::BeginPlay()
{
	Super::BeginPlay();

	if (InitialEffect && AbilitySystemComponent)
	{
		FGameplayEffectContextHandle ContextHandle = AbilitySystemComponent->MakeEffectContext();
		ContextHandle.AddSourceObject(this);
		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(InitialEffect, 1, ContextHandle);
		if (SpecHandle.IsValid())
		{
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
}

void ABallPlayer::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	ABallEnemy* Enemy = Cast<ABallEnemy>(OtherActor);
	if (!Enemy) return;

	const float PlayerStrength = AttributeSet->GetStrength();
	const UBallAttributeSetBase* EnemyAttributeSet = Cast<UBallAttributeSetBase>(Enemy->GetAbilitySystemComponent()->GetAttributeSet(UBallAttributeSetBase::StaticClass()));
	const float EnemyStrength = EnemyAttributeSet ? EnemyAttributeSet->GetStrength() : 0.f;

	ABallGameModeBase* GameMode = Cast<ABallGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	
	if (PlayerStrength > EnemyStrength)
	{
		const EEnemyType Type = Enemy->GetEnemyType();
		Enemy->BeEaten(this);
		if (GameMode)
		{
			GameMode->EnemyEaten(Type);
		}
	}
	else
	{
		Enemy->CollideWithStrongerPlayer(this);
	}
	UE_LOG(LogTemp, Log, TEXT("Player overlap with %s"), *OtherActor->GetName());
}
