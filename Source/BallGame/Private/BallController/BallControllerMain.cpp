// Fill out your copyright notice in the Description page of Project Settings.


#include "BallController/BallControllerMain.h"

#include "Camera/CameraActor.h"
#include "Characters/BallPlayer.h"
#include "Kismet/GameplayStatics.h"

void ABallControllerMain::BeginPlay()
{
	Super::BeginPlay();
	bShowMouseCursor = true;

	ControlledBall = Cast<ABallPlayer>(GetPawn());

	ACameraActor* MainCamera = Cast<ACameraActor>(UGameplayStatics::GetActorOfClass(GetWorld(), ACameraActor::StaticClass()));
	if (MainCamera)
	{
		SetViewTarget(MainCamera);
	}
}

void ABallControllerMain::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction(FName("Move"), IE_Pressed, this, &ABallControllerMain::OnMoveKeyPressed);
	InputComponent->BindAction(FName("Move"), IE_Released, this, &ABallControllerMain::OnMoveKeyReleased);
}

void ABallControllerMain::OnMoveKeyPressed() { bIsMoveKeyPressed = true; }
void ABallControllerMain::OnMoveKeyReleased() { bIsMoveKeyPressed = false; }

void ABallControllerMain::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	if (bIsMoveKeyPressed && ControlledBall)
	{
		FVector MouseLocation, MouseDirection;
		DeprojectMousePositionToWorld(MouseLocation, MouseDirection)

		FVector GroundPlaneOrigin(0, 0, 0);
		FVector GroundPlaneNormal(0, 0, 1);
		FVector TargetLocation = FMath::LinePlaneIntersection(MouseLocation, MouseLocation + MouseDirection * 10000.f, GroundPlaneOrigin, GroundPlaneNormal);

		ControlledBall->SetMoveTarget(TargetLocation);
	} 
}
