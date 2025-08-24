// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/BallPlayer.h"
#include "AbilitySystem/AttributeSet/BallAttributeSetBase.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "Characters/BallEnemy.h"
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

		if (Direction.SizeSquared() > 100.f)
		{
			Direction.Normalize();

			const float CurrentSpeed = AttributeSet->GetSpeed();
			const float CurrentStrength = FMath::Max(1.f, AttributeSet->GetStrength());

			float ForceMagnitude = 1000.f * (CurrentSpeed / CurrentStrength);

			MeshComponent->AddForce(Direction * ForceMagnitude, NAME_None, true);
		}
	}
}

void ABallPlayer::SetMoveTarget(const FVector& TargetLocation)
{
	MoveTargetLocation = TargetLocation;
	bIsMoving = true;
}

void ABallPlayer::BeginPlay()
{
	Super::BeginPlay();

	if (InitialEffect)
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

	if (PlayerStrength > EnemyStrength)
	{
		//Enemy->BeEatenBy(this);
	}

	ABallGameModeBase* GameMode = Cast<ABallGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GameMode)
	{
		//GameMode->HandleEnemyEaten();
	}
	else
	{
		//Enemy->CollideWithStrongerPlayer(this);
	}
	
}
