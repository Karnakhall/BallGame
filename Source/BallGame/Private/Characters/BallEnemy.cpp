// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/BallEnemy.h"
#include "Characters/BallPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AttributeSet/BallAttributeSetBase.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"


ABallEnemy::ABallEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void ABallEnemy::BeginPlay()
{
	Super::BeginPlay();
	PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
}

void ABallEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!PlayerPawn.IsValid()) return;

	const float MyStrength = AttributeSet->GetStrength();

	const UAbilitySystemComponent* PlayerASC = PlayerPawn->FindComponentByClass<UAbilitySystemComponent>();
	if (!PlayerASC) return;

	const float PlayerStrength = PlayerASC->GetNumericAttribute(UBallAttributeSetBase::GetStrengthAttribute());

	FVector DirectionToPlayer = PlayerPawn->GetActorLocation() - GetActorLocation();

	DirectionToPlayer.Z = 0;

	AAIController* AIController = Cast<AAIController>(GetController());
	if (AIController)
	{
		if (MyStrength > PlayerStrength)  //Gonitwa
		{
			AIController->MoveToActor(PlayerPawn.Get(), 100.f);
		}
		else
		{
			FVector FleeLocation = GetActorLocation() - DirectionToPlayer.GetSafeNormal() * 1000.f;
			UAIBlueprintHelperLibrary::SimpleMoveToLocation(AIController, FleeLocation);
		}
	}
}

void ABallEnemy::BeEaten(class ABallPlayer* Player)
{
	UAbilitySystemComponent* PlayerASC = Player->GetAbilitySystemComponent();
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
	Destroy();
}

void ABallEnemy::CollideWithStrongerPlayer(class ABallPlayer* Player)
{
	if (EnemyType == EEnemyType::Purple_Damage)
	{
		BeEaten(Player);
	}
}


