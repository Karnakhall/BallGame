// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/BallPlayerController.h"
#include "Characters/BallPlayer.h"


void ABallPlayerController::BeginPlay()
{
	Super::BeginPlay();
	bShowMouseCursor = true;

	ControlledBall = Cast<ABallPlayer>(GetPawn());

	SetViewTarget(ControlledBall);
}

void ABallPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction(FName("Move"), IE_Pressed, this, &ABallPlayerController::OnMoveKeyPressed);
	InputComponent->BindAction(FName("Move"), IE_Released, this, &ABallPlayerController::OnMoveKeyReleased);
	
}

void ABallPlayerController::OnMoveKeyPressed() { bIsMoveKeyPressed = true; }
void ABallPlayerController::OnMoveKeyReleased()
{
	bIsMoveKeyPressed = false;
	if (ControlledBall)
	{
		ControlledBall->StopMoveInput();
	}
}

void ABallPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	if (!ControlledBall)
	{
		ControlledBall = Cast<ABallPlayer>(GetPawn());
	}
	if (bIsMoveKeyPressed && ControlledBall)
	{
		FVector MouseLocation, MouseDirection;
		DeprojectMousePositionToWorld(MouseLocation, MouseDirection);
		
			const float PlaneZLocation = ControlledBall->GetActorLocation().Z;
			const FVector GroundPlaneOrigin(0, 0, PlaneZLocation);
			const FVector GroundPlaneNormal(0, 0, 1);
			const FVector TargetLocation = FMath::LinePlaneIntersection(MouseLocation, MouseLocation + MouseDirection * 10000.f, GroundPlaneOrigin, GroundPlaneNormal);
			ControlledBall->SetMoveTarget(TargetLocation);
		
	}
}

