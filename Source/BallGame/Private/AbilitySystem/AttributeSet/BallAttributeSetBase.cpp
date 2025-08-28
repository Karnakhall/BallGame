// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AttributeSet/BallAttributeSetBase.h"
#include "GameplayEffectExtension.h"
#include "Characters/BallPawnBase.h"
#include "GameMode/BallGameModeBase.h"
#include "Characters/BallPlayer.h"
#include "Kismet/GameplayStatics.h"

UBallAttributeSetBase::UBallAttributeSetBase()
{
}

/*void UBallAttributeSetBase::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetStrengthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, 1000.f);
	}
	else if (Attribute == GetSpeedAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, 1000.f);
	}
}*/

void UBallAttributeSetBase::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	AActor* TargetActor = Data.Target.GetOwnerActor();
	if (!TargetActor) return;

	ABallPlayer* TargetBall = Cast<ABallPlayer>(TargetActor);

	// Zaktualizujemy skalę kuli, jeśli siła została zmieniona
	if (Data.EvaluatedData.Attribute == GetStrengthAttribute())
	{
		TargetBall->UpdateScale(GetStrength());
		/*if (TargetBall)
		{
			TargetBall->OnStrengthChanged(GetStrength());
		}*/
	}

	if (TargetBall && TargetBall->IsPlayerControlled())
	{
		if (GetStrength() <= 0.f || GetSpeed() <= 0.f)
		{
			ABallGameModeBase* GameMode = Cast<ABallGameModeBase>(UGameplayStatics::GetGameMode(TargetActor->GetWorld()));
			if (GameMode)
			{
				GameMode->PlayerLoss();
			}
		}
	}
}