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

	// Wpływa na wielkość
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	FGameplayAttributeData Strength;
	//ATTRIBUTE_ACCESSORS(UBallAttributeSetBase, Strength);
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(UBallAttributeSetBase, Strength) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(Strength) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(Strength) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(Strength)

	// Wpływa na szybkość ruchu
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData Speed;
	//ATTRIBUTE_ACCESSORS(UBallAttributeSetBase, Speed);
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(UBallAttributeSetBase, Speed) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(Speed) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(Speed) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(Speed)

	//float GetStrength() const { return Strength.GetCurrentValue(); }
	//float GetSpeed() const { return Speed.GetCurrentValue(); }

	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
	//virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
};
