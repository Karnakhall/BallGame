// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/BallPawnBase.h"
#include "BallEnemy.generated.h"


UENUM(BlueprintType)
enum class EEnemyType : uint8
{
    Red_Strength   UMETA(DisplayName="Red_Strength"),
    Yellow_Speed   UMETA(DisplayName="Yellow_Speed"),
    Purple_Damage  UMETA(DisplayName="Purple_Damage")
};

UENUM()
enum class EEnemyAIState : uint8 { Idle, Chase, Flee };

UCLASS()
class BALLGAME_API ABallEnemy : public ABallPawnBase
{
    GENERATED_BODY()

public:
    ABallEnemy();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    void BeEaten(class ABallPlayer* Player);
    void CollideWithStrongerPlayer(class ABallPlayer* Player);

    UFUNCTION(BlueprintPure, Category="Enemy")
    EEnemyType GetEnemyType() const { return EnemyType; }

    bool IsConsumed() const { return bConsumed; }

protected:
    // GE nakładany na GRACZA, gdy ten wróg jest silniejszy i “gryzie”
    UPROPERTY(EditDefaultsOnly, Category="GAS")
    TSubclassOf<class UGameplayEffect> EffectOnPlayerWhenStronger;

    UPROPERTY(EditAnywhere, Category="AI")
    float BiteCooldown = 0.35f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Enemy")
    EEnemyType EnemyType = EEnemyType::Red_Strength;

    // GE, który gracz dostaje po zjedzeniu TEGO wroga (np. +Strength, +Speed, -Staty dla Purple)
    UPROPERTY(EditDefaultsOnly, Category="GAS")
    TSubclassOf<class UGameplayEffect> EffectToApply;

    // AI parametry
    UPROPERTY(EditAnywhere, Category="AI")
    float StrengthHysteresis = 2.0f;

    UPROPERTY(EditAnywhere, Category="AI|Distance")
    float FleeDistance = 900.f;

    UPROPERTY(EditAnywhere, Category="AI|Distance")
    float DistanceHysteresis = 150.f;

    UPROPERTY(EditAnywhere, Category="AI|Distance")
    float StopChaseDistance = 120.f;

    EEnemyAIState MoveState = EEnemyAIState::Idle;

private:
    TWeakObjectPtr<APawn> PlayerPawn;
    bool bConsumed = false;
    double LastBiteTime = -1e9;

    // wyliczenia kierunku 2D
    static FORCEINLINE FVector Dir2D(const FVector& From, const FVector& To)
    {
        FVector D = To - From; D.Z = 0.f;
        const float S2 = D.SizeSquared();
        return (S2 <= KINDA_SMALL_NUMBER) ? FVector::ZeroVector : D / FMath::Sqrt(S2);
    }
};
