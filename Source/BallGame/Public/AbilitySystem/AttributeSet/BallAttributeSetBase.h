// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "BallAttributeSetBase.generated.h"

/**
 * 
 */

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)


UCLASS()
class BALLGAME_API UBallAttributeSetBase : public UAttributeSet
{
	GENERATED_BODY()
public:
	UBallAttributeSetBase();

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
	
	// Wpływa na wielkość
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData Strength;
	ATTRIBUTE_ACCESSORS(UBallAttributeSetBase, Strength);

	UPROPERTY(BlueprintReadOnly, Category="Attributes")
	FGameplayAttributeData MaxStrength;
	ATTRIBUTE_ACCESSORS(UBallAttributeSetBase, MaxStrength)

	// Wpływa na szybkość ruchu
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData Speed;
	ATTRIBUTE_ACCESSORS(UBallAttributeSetBase, Speed);

	UPROPERTY(BlueprintReadOnly, Category="Attributes")
	FGameplayAttributeData MaxSpeed;
	ATTRIBUTE_ACCESSORS(UBallAttributeSetBase, MaxSpeed)

	UPROPERTY(BlueprintReadOnly, Category="Attributes")
	FGameplayAttributeData Level;
	ATTRIBUTE_ACCESSORS(UBallAttributeSetBase, Level)

	// Helpers dla HUD
	UFUNCTION(BlueprintPure, Category="Attributes")
	float GetStrengthPct() const { return (GetMaxStrength() > 0.f) ? GetStrength() / GetMaxStrength() : 0.f; }

	UFUNCTION(BlueprintPure, Category="Attributes")
	float GetSpeedPct() const { return (GetMaxSpeed() > 0.f) ? GetSpeed() / GetMaxSpeed() : 0.f; }

private:
	// Utrzymaj ten sam procent
	void AdjustAttributeForMaxChange(const FGameplayAttribute& AffectedAttribute,
		const FGameplayAttributeData& CurrentValue, const FGameplayAttributeData& MaxValue, float NewMaxValue);

	// Klamruj wartość do [0..Max] dla danego atrybutu
	void ClampAttributeToValidRange(const FGameplayAttribute& Attribute);
	
};
