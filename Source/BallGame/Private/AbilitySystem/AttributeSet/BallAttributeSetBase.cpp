// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AttributeSet/BallAttributeSetBase.h"
#include "GameplayEffectExtension.h"
#include "Characters/BallPawnBase.h"
#include "GameMode/BallGameModeBase.h"
#include "Kismet/GameplayStatics.h"

UBallAttributeSetBase::UBallAttributeSetBase()
{
	InitStrength(10.f);
	InitMaxStrength(50.f);
	InitSpeed(10.f);
	InitMaxSpeed(50.f);
	InitLevel(1.f);
}

void UBallAttributeSetBase::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	
	if (Attribute == GetMaxStrengthAttribute())
	{
		NewValue = FMath::Max(0.f, NewValue);
		AdjustAttributeForMaxChange(GetStrengthAttribute(), Strength, MaxStrength, NewValue);
	}
	else if (Attribute == GetMaxSpeedAttribute())
	{
		NewValue = FMath::Max(0.f, NewValue);
		AdjustAttributeForMaxChange(GetSpeedAttribute(), Speed, MaxSpeed, NewValue);
	}
	
	else if (Attribute == GetStrengthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxStrength());
	}
	else if (Attribute == GetSpeedAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxSpeed());
	}
}

void UBallAttributeSetBase::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	AActor* TargetActor = Data.Target.GetOwnerActor();
	if (!TargetActor) return;

	// Klamrowanie wartości po modyfikacjach GE
	if (Data.EvaluatedData.Attribute == GetStrengthAttribute()
	 || Data.EvaluatedData.Attribute == GetMaxStrengthAttribute()
	 || Data.EvaluatedData.Attribute == GetSpeedAttribute()
	 || Data.EvaluatedData.Attribute == GetMaxSpeedAttribute())
	{
		ClampAttributeToValidRange(GetStrengthAttribute());
		ClampAttributeToValidRange(GetSpeedAttribute());
	}

	// Zaktualizujemy skalę kuli, jeśli siła została zmieniona
	if (Data.EvaluatedData.Attribute == GetStrengthAttribute())
	{
		if (ABallPawnBase* TargetBall = Cast<ABallPawnBase>(TargetActor))
		{
			TargetBall->OnStrengthChanged(GetStrength());
		}
	}

	
	if (ABallPawnBase* TargetBall = Cast<ABallPawnBase>(TargetActor))
	{
		if (TargetBall->IsPlayerControlled())
		{
			if (GetStrength() <= 0.f || GetSpeed() <= 0.f)
			{
				if (ABallGameModeBase* GameMode = Cast<ABallGameModeBase>(UGameplayStatics::GetGameMode(TargetActor->GetWorld())))
				{
					GameMode->PlayerLoss();
				}
			}
		}
	}
}

void UBallAttributeSetBase::AdjustAttributeForMaxChange(const FGameplayAttribute& AffectedAttribute,
	const FGameplayAttributeData& CurrentValue, const FGameplayAttributeData& MaxValue, float NewMaxValue)
{
	const float OldMax = MaxValue.GetCurrentValue();
	const float Current = CurrentValue.GetCurrentValue();

	float NewCurrent = Current;

	if (OldMax > 0.f)
	{
		const float CurrentPct = Current / OldMax;
		NewCurrent = CurrentPct * NewMaxValue;
	}
	else
	{
		NewCurrent = NewMaxValue;
	}

	// Ustaw bazę przez ASC (jeśli mamy) lub bezpośrednio
	if (UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent())
	{
		ASC->SetNumericAttributeBase(AffectedAttribute, FMath::Clamp(NewCurrent, 0.f, NewMaxValue));
	}
	else
	{
		// fallback – bez ASC
		if (AffectedAttribute == GetStrengthAttribute())
		{
			InitStrength(FMath::Clamp(NewCurrent, 0.f, NewMaxValue));
		}
		else if (AffectedAttribute == GetSpeedAttribute())
		{
			InitSpeed(FMath::Clamp(NewCurrent, 0.f, NewMaxValue));
		}
	}
}

void UBallAttributeSetBase::ClampAttributeToValidRange(const FGameplayAttribute& Attribute)
{
	if (AActor* Owner = GetOwningActor())
	{
		if (ABallPawnBase* Ball = Cast<ABallPawnBase>(Owner))
		{
			Ball->OnStrengthChanged(GetStrength());
		}
	}
}

