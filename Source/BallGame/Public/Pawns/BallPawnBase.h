// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "BallPawnBase.generated.h"

UCLASS()
class BALLGAME_API ABallPawnBase : public APawn
{
	GENERATED_BODY()

public:
	
	ABallPawnBase();
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	
	virtual void BeginPlay() override;

};
