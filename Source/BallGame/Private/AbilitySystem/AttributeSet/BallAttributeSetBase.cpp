// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AttributeSet/BallAttributeSetBase.h"
#include "GameplayEffectExtension.h"
#include "Characters/BallPawnBase.h"
#include "GameMode/BallGameModeBase.h"
#include "Kismet/GameplayStatics.h"

UBallAttributeSetBase::UBallAttributeSetBase()
{
}

void UBallAttributeSetBase::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	AActor* TargetActor = Data.Target.GetOwnerActor();
	if (!TargetActor) return;

	ABallPawnBase* TargetBall = Cast<ABallPawnBase>(TargetActor);

	// Zaktualizujemy skalę kuli, jeśli siła została zmieniona
	if (Data.EvaluatedData.Attribute == GetStrengthAttribute())
	{
		if (TargetBall)
		{
			TargetBall->OnStrengthChanged(GetStrength());
		}
	}

	if (TargetBall && TargetBall->IsPlayerControlled())
	{
		if (GetStrength() <= 0.f || GetSpeed() <= 0.f)
		{
			AGameModeBase* GameMode = Cast<AGameModeBase>(UGameplayStatics::GetGameMode(TargetActor->GetWorld()));
			if (GameMode)
			{
				//GameMode->HandlePlayerLoss();
			}
		}
	}
}
