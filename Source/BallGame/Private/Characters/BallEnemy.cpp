// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/BallEnemy.h"
#include "Characters/BallPlayer.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AttributeSet/BallAttributeSetBase.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"

static FORCEINLINE FVector Dir2D(const FVector& From, const FVector& To)
{
	FVector D = To - From;
	D.Z = 0.f;
	const float S2 = D.SizeSquared();
	if (S2 <= KINDA_SMALL_NUMBER) return FVector::ZeroVector;
	return D / FMath::Sqrt(S2);
}

ABallEnemy::ABallEnemy()
{
	PrimaryActorTick.bCanEverTick = true;
	bUsePhysicsMovement = true;

	if (SphereComponent)
	{
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		SphereComponent->SetCollisionObjectType(ECC_Pawn);
		SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);   // z innymi kulami – overlap
		SphereComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block); // podłoga – block
		SphereComponent->SetLinearDamping(1.0f);
		SphereComponent->SetAngularDamping(0.6f);
	}
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

float ABallEnemy::ComputeAccelFromAttributes() const
{
	const float Spd = AttributeSet ? AttributeSet->GetSpeed() : 0.f;
	const float Str = AttributeSet ? AttributeSet->GetStrength() : 1.f;
	return PhysicsAccelBase * (Spd / FMath::Max(1.f, Str));
}

void ABallEnemy::ApplyForce2D(const FVector& DirNorm, float Scale)
{
	if (!SphereComponent || DirNorm.IsNearlyZero()) return;

	const float Accel = ComputeAccelFromAttributes() * Scale;
	SphereComponent->WakeAllRigidBodies();
	SphereComponent->AddForce(DirNorm * Accel, NAME_None, true); // bAccelChange = true

	if (TorqueScale > 0.f)
	{
		const FVector Up = FVector::UpVector;
		const FVector TorqueAxis = FVector::CrossProduct(Up, DirNorm).GetSafeNormal();
		SphereComponent->AddTorqueInRadians(TorqueAxis * TorqueScale, NAME_None, true);
	}
}

void ABallEnemy::ApplyBraking2D()
{
	if (!SphereComponent) return;

	const FVector V = SphereComponent->GetPhysicsLinearVelocity();
	const FVector V2D(V.X, V.Y, 0.f);
	if (V2D.SizeSquared() > 10.f)
	{
		SphereComponent->AddForce(-V2D * PhysicsBrakeAccel, NAME_None, true);
	}
}

void ABallEnemy::Tick(float DeltaTime)
{
	if (bConsumed) return;
	Super::Tick(DeltaTime);

	if (!PlayerPawn.IsValid()) { ApplyBraking2D(); return; }

	const float MyStrength = AttributeSet ? AttributeSet->GetStrength() : 0.f;
	const UAbilitySystemComponent* PlayerASC = PlayerPawn->FindComponentByClass<UAbilitySystemComponent>();
	if (!PlayerASC) { ApplyBraking2D(); return; }

	const float PlayerStrength = PlayerASC->GetNumericAttribute(UBallAttributeSetBase::GetStrengthAttribute());

	const FVector ToPlayerDir = Dir2D(GetActorLocation(), PlayerPawn->GetActorLocation());
	if (ToPlayerDir.IsNearlyZero()) { ApplyBraking2D(); return; }

	const float Dist = FVector::Dist2D(GetActorLocation(), PlayerPawn->GetActorLocation());

	// Histereza decyzji
	const bool StrongerEnough = (MyStrength > PlayerStrength + StrengthHysteresis);
	const bool WeakerEnough = (MyStrength < PlayerStrength - StrengthHysteresis);

	switch (MoveState)
	{
	case EEnemyAIState::Idle:
		if (StrongerEnough) MoveState = EEnemyAIState::Chase;
		else if (WeakerEnough) MoveState = EEnemyAIState::Flee;
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
		ApplyForce2D(ToPlayerDir, 1.f);
	}
	else if (MoveState == EEnemyAIState::Flee)
	{
		ApplyForce2D(-ToPlayerDir, 1.f);
	}
	else
	{
		ApplyBraking2D();
	}
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