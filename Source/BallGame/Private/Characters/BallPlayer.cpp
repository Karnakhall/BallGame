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

	Camera = CreateDefaultSubobject<USpringArmComponent>("Camera");
	Camera->SetupAttachment(RootComponent);
	Camera->TargetArmLength = 1500.f; // wysokość “od góry” (dobierz)
	Camera->bDoCollisionTest = false; // w 2D niepotrzebne
	Camera->bUsePawnControlRotation = false;
	Camera->bInheritPitch = false;
	Camera->bInheritYaw = false;
	Camera->bInheritRoll = false;
	Camera->SetUsingAbsoluteRotation(true); // nie obracaj wraz z graczem
	Camera->SetRelativeRotation(FRotator(-90,0,0));// patrz w dół
	// Nie dziedzicz skali od gracza (bo rośnie):
	Camera->SetAbsolute(false, true, true);

	Camera->bEnableCameraLag = true;
	Camera->CameraLagSpeed = 12.f; // 8–20 zwykle ok

	OrthoCamera = CreateDefaultSubobject<UCameraComponent>("OrthoCamera");
	OrthoCamera->SetupAttachment(Camera, USpringArmComponent::SocketName);
	OrthoCamera->SetProjectionMode(ECameraProjectionMode::Orthographic);
	OrthoCamera->OrthoWidth = 4000.f; // dobierz pod scenę
	OrthoCamera->bUsePawnControlRotation = false;
}

void ABallPlayer::Tick(float DeltaTime)
{
	// Najpierw policz kierunek i przekaż input do bazy…
	if (bIsMoving)
	{
		FVector Dir = MoveTargetLocation - GetActorLocation();
		Dir.Z = 0.f;
		if (Dir.SizeSquared() <= FMath::Square(StopDistance))
		{
			StopMoveInput(); // soft stop
		}
		else
		{
			Dir.Normalize();
			AddMoveInput2D(Dir, 1.f);
		}
	}

	// …a dopiero potem Super::Tick, żeby baza przetworzyła PendingMoveInput2D
	Super::Tick(DeltaTime);
}

void ABallPlayer::SetMoveTarget(const FVector& TargetLocation)
{
	MoveTargetLocation = TargetLocation;
	bIsMoving = true;
}

void ABallPlayer::StopMoveInput()
{
	bIsMoving = false;
	ABallPawnBase::StopMoveInput(false); // czyści input + przydusza prędkość
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
	if (!Enemy || Enemy->IsActorBeingDestroyed() || Enemy->IsConsumed()) return;

	const float PlayerStrength = AttributeSet->GetStrength();
	const UBallAttributeSetBase* EnemyAttributeSet = Cast<UBallAttributeSetBase>(Enemy->GetAbilitySystemComponent()->GetAttributeSet(UBallAttributeSetBase::StaticClass()));
	const float EnemyStrength = EnemyAttributeSet ? EnemyAttributeSet->GetStrength() : 0.f;

	ABallGameModeBase* GameMode = Cast<ABallGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	
	if (PlayerStrength > EnemyStrength)
	{
		const EEnemyType Type = Enemy->GetEnemyType();
		Enemy->BeEaten(this);

		if (GameMode) { GameMode->EnemyEaten(Type); }
	}
	else
	{
		Enemy->CollideWithStrongerPlayer(this);
	}
	UE_LOG(LogTemp, Log, TEXT("Player overlap with %s"), *OtherActor->GetName());
}
