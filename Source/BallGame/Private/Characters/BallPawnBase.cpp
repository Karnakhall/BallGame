// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawns/BallPawnBase.h"

// Sets default values
ABallPawnBase::ABallPawnBase()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void ABallPawnBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABallPawnBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called to bind functionality to input
void ABallPawnBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

