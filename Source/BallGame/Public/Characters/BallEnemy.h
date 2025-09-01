// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/BallPawnBase.h"
//#include "GameFramework/FloatingPawnMovement.h"
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

UENUM()
enum class EEnemyAIState : uint8 { Idle, Chase, Flee };

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

	UPROPERTY(EditDefaultsOnly, Category="GAS")
	TSubclassOf<class UGameplayEffect> EffectOnPlayerWhenStronger;

	UPROPERTY(EditAnywhere, Category="AI")
	float BiteCooldown = 0.35f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	EEnemyType EnemyType = EEnemyType::Red_Strength;

	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<class UGameplayEffect> EffectToApply;
	
	UPROPERTY(EditAnywhere, Category="Movement|Physics")
	float PhysicsAccelBase = 6000.f; 

	UPROPERTY(EditAnywhere, Category="Movement|Physics")
	float PhysicsBrakeAccel = 4000.f;  // siła hamowania gdy brak wejścia

	// Opcjonalnie: moment obrotowy (ładniejsze toczenie), 0 = off
	UPROPERTY(EditAnywhere, Category="Movement|Physics")
	float TorqueScale = 0.f; 
	
	UPROPERTY(EditAnywhere, Category="AI")
	float StrengthHysteresis = 2.0f;

	UPROPERTY(EditAnywhere, Category="AI | Distance")
	float FleeDistance = 900.f;

	UPROPERTY(EditAnywhere, Category="AI | Distance")
	float DistanceHysteresis = 150.f;
	
	UPROPERTY(EditAnywhere, Category="AI | Distance")
	float StopChaseDistance = 120.f;

	EEnemyAIState MoveState = EEnemyAIState::Idle;

private:
	TWeakObjectPtr<APawn> PlayerPawn;

	bool bConsumed = false;
	float ComputeAccelFromAttributes() const;
	void ApplyForce2D(const FVector& DirNorm, float Scale=1.f);
	void ApplyBraking2D();

	double LastBiteTime = -1e9;
};
