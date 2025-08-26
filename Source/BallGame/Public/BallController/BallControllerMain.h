// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BallControllerMain.generated.h"

/**
 * 
 */
UCLASS()
class BALLGAME_API ABallControllerMain : public APlayerController
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;

private:
	UPROPERTY()
	TObjectPtr<class ABallPlayer> ControlledBall;

	void OnMoveKeyPressed();
	void OnMoveKeyReleased();

	bool bIsMoveKeyPressed = false;
};
