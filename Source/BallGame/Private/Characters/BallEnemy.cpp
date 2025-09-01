// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/BallEnemy.h"
#include "Characters/BallPlayer.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AttributeSet/BallAttributeSetBase.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"

ABallEnemy::ABallEnemy()
{
	PrimaryActorTick.bCanEverTick = true;
	bUsePhysicsMovement = true;
	
}

void ABallEnemy::BeginPlay()
{
	Super::BeginPlay();
	PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	if (PlayerPawn.IsValid())
	{
		FVector L = GetActorLocation();
		L.Z = PlayerPawn->GetActorLocation().Z;
		SetActorLocation(L, false);
	}
}

void ABallEnemy::Tick(float DeltaTime)
{
	// Uwaga: podajemy TYLKO kierunek, a bazowa klasa wykona AddForce/hamowanie/cap
    if (bConsumed) { Super::Tick(DeltaTime); return; }

    FVector MoveDir = FVector::ZeroVector;

    if (!PlayerPawn.IsValid())
    {
        // brak celu → brak inputu; baza wyhamuje
        Super::Tick(DeltaTime);
        return;
    }

    // Atrybuty
    const float MyStrength = AttributeSet ? AttributeSet->GetStrength() : 0.f;

    const UAbilitySystemComponent* PlayerASC = PlayerPawn->FindComponentByClass<UAbilitySystemComponent>();
    if (!PlayerASC)
    {
        Super::Tick(DeltaTime);
        return;
    }

    const float PlayerStrength = PlayerASC->GetNumericAttribute(UBallAttributeSetBase::GetStrengthAttribute());

    const FVector ToPlayerDir = Dir2D(GetActorLocation(), PlayerPawn->GetActorLocation());
    if (ToPlayerDir.IsNearlyZero())
    {
        Super::Tick(DeltaTime);
        return;
    }

    const float Dist = FVector::Dist2D(GetActorLocation(), PlayerPawn->GetActorLocation());

    // Histereza 
    const bool StrongerEnough = (MyStrength > PlayerStrength + StrengthHysteresis);
    const bool WeakerEnough   = (MyStrength < PlayerStrength - StrengthHysteresis);

    switch (MoveState)
    {
    case EEnemyAIState::Idle:
        if (StrongerEnough)        MoveState = EEnemyAIState::Chase;
        else if (WeakerEnough)     MoveState = EEnemyAIState::Flee;
        break;

    case EEnemyAIState::Chase:
        if (!StrongerEnough || Dist < StopChaseDistance)
            MoveState = EEnemyAIState::Idle;
        break;

    case EEnemyAIState::Flee:
        if (!WeakerEnough || Dist > (FleeDistance + DistanceHysteresis))
            MoveState = EEnemyAIState::Idle;
        break;
    }

    if (MoveState == EEnemyAIState::Chase)
    {
        MoveDir = ToPlayerDir;
    }
    else if (MoveState == EEnemyAIState::Flee)
    {
        MoveDir = -ToPlayerDir;
    }
    // Idle → MoveDir = Zero → baza zahamuje

    if (!MoveDir.IsNearlyZero())
    {
        AddMoveInput2D(MoveDir, 1.f);  // przekazujemy kierunek do bazy
    }
    // UWAGA: nie wywołujemy tu AddForce – zrobi to baza

    Super::Tick(DeltaTime); // baza przetworzy PendingMoveInput2D (AddForce/Brake/Cap)
}

void ABallEnemy::BeEaten(class ABallPlayer* Player)
{
	if (bConsumed) return;
	bConsumed = true;

	SetActorTickEnabled(false);
	
	if (SphereComponent)
	{
		SphereComponent->SetSimulatePhysics(false);
		SphereComponent->SetEnableGravity(false);
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SphereComponent->SetGenerateOverlapEvents(false);
		SphereComponent->SetNotifyRigidBodyCollision(false);
	}	

	if (UAbilitySystemComponent* PlayerASC = Player ? Player->GetAbilitySystemComponent() : nullptr)
	{
		if (EffectToApply)
		{
			FGameplayEffectContextHandle ContextHandle = PlayerASC->MakeEffectContext();
			ContextHandle.AddSourceObject(this);
			FGameplayEffectSpecHandle SpecHandle = PlayerASC->MakeOutgoingSpec(EffectToApply, 1, ContextHandle);
			if (SpecHandle.IsValid())
			{
				PlayerASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}

	SetLifeSpan(0.01f);
}

void ABallEnemy::CollideWithStrongerPlayer(class ABallPlayer* Player)
{
	if (!Player) return;
	
	const double Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;
	if (Now - LastBiteTime < BiteCooldown)
	{
		return;
	}

	if (EnemyType == EEnemyType::Purple_Damage)
	{
		BeEaten(Player);
		LastBiteTime = Now;
		return;
	}

	if (EffectOnPlayerWhenStronger)
	{
		if (UAbilitySystemComponent* PlayerASC = Player->GetAbilitySystemComponent())
		{
			FGameplayEffectContextHandle Ctx = PlayerASC->MakeEffectContext();
			Ctx.AddSourceObject(this);
			if (FGameplayEffectSpecHandle Spec = PlayerASC->MakeOutgoingSpec(EffectOnPlayerWhenStronger, 1.f, Ctx);
				Spec.IsValid())
			{
				PlayerASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());

				// debug (na chwilę): zobacz nową Siłę/Szybkość
				const float NewStr = PlayerASC->GetNumericAttribute(UBallAttributeSetBase::GetStrengthAttribute());
				UE_LOG(LogTemp, Log, TEXT("Red bite -> Player Strength = %.1f"), NewStr);
				const float NewSpeed = PlayerASC->GetNumericAttribute(UBallAttributeSetBase::GetSpeedAttribute());
				UE_LOG(LogTemp, Log, TEXT("Red bite -> Player Speed = %.1f"), NewSpeed);
			}
		}
	}
	LastBiteTime = Now;
}