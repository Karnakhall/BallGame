// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/BallPawnBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AttributeSet/BallAttributeSetBase.h"
#include "Kismet/GameplayStatics.h"

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
	
	/*SphereComponent->BodyInstance.bLockZTranslation = true;
	SphereComponent->BodyInstance.bLockXRotation = false;
	SphereComponent->BodyInstance.bLockYRotation = false;
	*/

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

	if (bUsePhysicsMovement)
	{
		// Player – pełna fizyka (toczenie)
		SphereComponent->SetSimulatePhysics(true);
		SphereComponent->SetEnableGravity(true);
		SphereComponent->BodyInstance.bLockZTranslation = false; // grawitacja działa
		SphereComponent->BodyInstance.bLockXRotation = false;
		SphereComponent->BodyInstance.bLockYRotation = false;
		SphereComponent->BodyInstance.bLockZRotation = false;
	}
	else
	{
		// Enemy – bez fizyki, płaszczyzna XY (ruch zapewni MovementComponent w klasie pochodnej)
		SphereComponent->SetSimulatePhysics(false);
		SphereComponent->SetEnableGravity(false);
		SphereComponent->BodyInstance.bLockZTranslation = true;
	}
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
	float FinaleScale = FMath::Clamp(BaseScale + (NewStrength * ScaleMultiplier), 0.3f, 5.0f);
	SetActorScale3D(FVector(FinaleScale));
}

void ABallPawnBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABallPawnBase::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}

// Called to bind functionality to input
void ABallPawnBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
}

