// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/BallPawnBase.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "BallEnemy.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class EEnemyType : uint8
{
	Red_Strength UMETA(DisplayName="Red_Strength"),
	Yellow_Speed UMETA(DisplayName="Yellow_Speed"),
	Purple_Damage UMETA(DisplayName="Purple_Damage")
};


UCLASS()
class BALLGAME_API ABallEnemy : public ABallPawnBase
{
	GENERATED_BODY()
public:
	ABallEnemy();

	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;

	void BeEaten(class ABallPlayer* Player);
	void CollideWithStrongerPlayer(class ABallPlayer* Player);

	UFUNCTION(BlueprintPure, Category = "Enemy")
	EEnemyType GetEnemyType() const { return EnemyType; }

	bool IsConsumed() const { return bConsumed;}
	
protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	EEnemyType EnemyType = EEnemyType::Red_Strength;

	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<class UGameplayEffect> EffectToApply;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Movement")
	TObjectPtr<UFloatingPawnMovement> FloatingMovement;

	
	UPROPERTY(EditAnywhere, Category="Movement")
	float AISpeedScale = 80.f;

	
	UPROPERTY(EditAnywhere, Category="AI")
	float StrengthHysteresis = 0.1f;

private:
	TWeakObjectPtr<APawn> PlayerPawn;

	bool bConsumed = false;
	
};
