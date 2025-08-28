// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/BallPawnBase.h"
#include "BallPlayer.generated.h"

/**
 * 
 */
UCLASS()
class BALLGAME_API ABallPlayer : public ABallPawnBase
{
	GENERATED_BODY()

public:
	ABallPlayer();
	
	virtual void Tick(float DeltaTime) override;

	// Docelowa pozycja
	void SetMoveTarget(const FVector& TargetLocation);
	void UpdateScale(float NewStrength);

protected:
	virtual void BeginPlay() override;

	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult) override;

	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<class UGameplayEffect> InitialEffect;

private:
	FVector MoveTargetLocation;
	bool bIsMoving = false;
};
