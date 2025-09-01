// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/BallPawnBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AttributeSet/BallAttributeSetBase.h"

// Sets default values
ABallPawnBase::ABallPawnBase()
{
	PrimaryActorTick.bCanEverTick = true;

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	RootComponent = SphereComponent;
	SphereComponent->SetSphereRadius(50.f);
	SphereComponent->SetCollisionProfileName(TEXT("Pawn"));
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SphereComponent->SetCollisionObjectType(ECC_Pawn);
	SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComponent->SetGenerateOverlapEvents(true);
	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &ABallPawnBase::OnSphereOverlap);
	
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionProfileName(TEXT("NoCollision"));
	MeshComponent->SetSimulatePhysics(false);
	

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AttributeSet = CreateDefaultSubobject<UBallAttributeSetBase>(TEXT("AttributeSet"));
}

void ABallPawnBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Konfiguracja fizyki wg flagi
	SphereComponent->SetSimulatePhysics(bUsePhysicsMovement);
	SphereComponent->SetEnableGravity(bUsePhysicsMovement);
	SphereComponent->BodyInstance.bUseCCD = true;
	SphereComponent->BodyInstance.bLockZTranslation = true;
	SphereComponent->SetLinearDamping(1.0f);
	SphereComponent->SetAngularDamping(0.6f);
}

void ABallPawnBase::BeginPlay()
{
	Super::BeginPlay();
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}

void ABallPawnBase::OnStrengthChanged(float NewStrength)
{
	float BaseScale = 1.0f;
	float ScaleMultiplier = 0.1f;
	float FinalScale = FMath::Clamp(BaseScale + (NewStrength * ScaleMultiplier), 0.3f, 5.0f);
	SetActorScale3D(FVector(FinalScale));
}

void ABallPawnBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bUsePhysicsMovement || !SphereComponent) return;

	if (bHasMoveInput && PendingMoveInput2D.SizeSquared() > FMath::Square(MinInputThreshold))
	{
		FVector Dir = PendingMoveInput2D;
		Dir.Z = 0.f;
		Dir.Normalize();
		Seek2D(Dir);
	}
	else
	{
		const UWorld* W = GetWorld();
		const double Now = W ? W->GetTimeSeconds() : 0.0;
		// Krótki poślizg bez hamowania
		if (Now - LastInputTime < FreewheelTime)
		{
			// nic nie robimy – kula “płynie”
		}
		else
		{
			ApplyBraking2D();
		}
	}

	// wyczyść input na następną klatkę
	PendingMoveInput2D = FVector::ZeroVector;
	bHasMoveInput = false;

}

void ABallPawnBase::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Implementacje w BallPlayer i BallEnemy
}

float ABallPawnBase::ComputeAccelFromAttributes() const
{
	const float Spd = AttributeSet ? AttributeSet->GetSpeed() : 0.f;
	const float Str = AttributeSet ? AttributeSet->GetStrength() : 0.f;
	const float Den = FMath::Max(StrengthSoftener, Str);
	return PhysicsAccelBase * (Spd / Den);
}

float ABallPawnBase::GetTargetPlanarSpeed() const
{
	const float Spd = AttributeSet ? AttributeSet->GetSpeed() : 0.f;
	return FMath::Max(120.f, Spd * SpeedScale);
}

void ABallPawnBase::ApplyBraking2D()
{
	if (!SphereComponent) return;

	FVector V = SphereComponent->GetPhysicsLinearVelocity();
	FVector V2D(V.X, V.Y, 0.f);
	const float Speed = V2D.Size();

	// szybkie “docięcie” drobnego resztek
	if (Speed <= MinStopSpeed)
	{
		V.X = 0.f; V.Y = 0.f;
		SphereComponent->SetPhysicsLinearVelocity(V);
		return;
	}

	// hamulec proporcjonalny do prędkości (drag) + mała stała składowa
	// BrakingDrag [1/s] – działa jak eksponenta: większy => szybciej “gaśnie”
	const FVector Drag = -V2D * BrakingDrag;
	const FVector Baseline = -V2D.GetSafeNormal() * PhysicsBrakeAccel; // bardzo małe, np. 100–300

	SphereComponent->AddForce(Drag + Baseline, NAME_None, true);
}

void ABallPawnBase::Seek2D(const FVector& DirNorm)
{
	const float TargetSpeed = GetTargetPlanarSpeed();
	const float Accel = ComputeAccelFromAttributes();

	const FVector V = SphereComponent->GetPhysicsLinearVelocity();
	const FVector V2D(V.X, V.Y, 0.f);
	const float ForwardSpeed = FVector::DotProduct(V2D, DirNorm);

	if (ForwardSpeed < TargetSpeed)
	{
		SphereComponent->WakeAllRigidBodies();
		SphereComponent->AddForce(DirNorm * Accel, NAME_None, true); // bAccelChange = true
	}
	else
	{
		SphereComponent->AddForce(-DirNorm * (PhysicsBrakeAccel * 0.5f), NAME_None, true);
	}
	
	CapPlanarSpeed(TargetSpeed * MaxSpeedOvershoot);

	// opcjonalny moment dla “toczenia”
	if (TorqueScale > 0.f)
	{
		const FVector Up = FVector::UpVector;
		const FVector TorqueAxis = FVector::CrossProduct(Up, DirNorm).GetSafeNormal();
		SphereComponent->AddTorqueInRadians(TorqueAxis * TorqueScale, NAME_None, true);
	}
}

void ABallPawnBase::CapPlanarSpeed(float MaxPlanarSpeed)
{
	FVector V = SphereComponent->GetPhysicsLinearVelocity();
	FVector V2D(V.X, V.Y, 0.f);
	const float S = V2D.Size();
	if (S > MaxPlanarSpeed)
	{
		V2D = V2D.GetSafeNormal() * MaxPlanarSpeed;
		V.X = V2D.X; V.Y = V2D.Y;
		SphereComponent->SetPhysicsLinearVelocity(V);
	}
}

void ABallPawnBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
}

void ABallPawnBase::AddMoveInput2D(const FVector& Direction, float Scale)
{
	FVector D = Direction;
	D.Z = 0.f;
	if (!D.IsNearlyZero())
	{
		PendingMoveInput2D += D.GetSafeNormal() * FMath::Max(0.f, Scale);
		bHasMoveInput = true;

		if (const UWorld* W = GetWorld())
		{
			LastInputTime = W->GetTimeSeconds();
		}
	}
}

void ABallPawnBase::ClearMoveInput()
{
	PendingMoveInput2D = FVector::ZeroVector;
	bHasMoveInput = false;
}

void ABallPawnBase::StopMoveInput(bool bReduceVelocity)
{
	ClearMoveInput();

	// Zainicjuj freewheel tuż po puszczeniu
	if (const UWorld* W = GetWorld())
	{
		LastInputTime = W->GetTimeSeconds();
	}

	if (bReduceVelocity && SphereComponent)
	{
		FVector V = SphereComponent->GetPhysicsLinearVelocity();
		V.Z = 0.f;
		// Zamiast brutalnego x0.2 – łagodna skala (konfigurowalna w BP)
		V.X *= StopVelocityScale2D;
		V.Y *= StopVelocityScale2D;
		SphereComponent->SetPhysicsLinearVelocity(V);

		FVector AV = SphereComponent->GetPhysicsAngularVelocityInDegrees();
		AV *= StopAngularVelocityScale;
		SphereComponent->SetPhysicsAngularVelocityInDegrees(AV);
	}
}

