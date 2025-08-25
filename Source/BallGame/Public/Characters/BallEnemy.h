// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/BallPawnBase.h"
#include "BallEnemy.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class EEnemyType : uint8
{
	Red_Strength,
	Yellow_Speed,
	Purple_Damage
};


UCLASS()
class BALLGAME_API ABallEnemy : public ABallPawnBase
{
	GENERATED_BODY()
public:
	ABallEnemy();

	virtual void Tick(float DeltaTime) override;

	void BeEaten(class ABallPlayer* Player);

	void CollideWithStrongerPlayer(class ABallPlayer* Player);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	EEnemyType EnemyType;

	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<class UGameplayEffect> EffectToApply;

private:
	TWeakObjectPtr<APawn> PlayerPawn;
	
};
