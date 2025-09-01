// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "AbilitySystemInterface.h"
#include "BallPawnBase.generated.h"

class UAbilitySystemComponent;
class UBallAttributeSetBase;
class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class BALLGAME_API ABallPawnBase : public APawn, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	
	ABallPawnBase();

	// Interfejs z IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }

	// Zmiana siły
	virtual void OnStrengthChanged(float NewStrength);
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	USphereComponent* GetCollisionSphere() const { return SphereComponent; }

	// Agregator “inputu” ruchu 2D – wywołuj w klasach pochodnych (Player/Enemy) w każdym ticku
	UFUNCTION(BlueprintCallable, Category="Movement|Physics")
	void AddMoveInput2D(const FVector& Direction, float Scale = 1.f);

	UFUNCTION(BlueprintCallable, Category="Movement|Physics")
	void ClearMoveInput();

	// Twarde przerwanie (np. po puszczeniu LPM)
	UFUNCTION(BlueprintCallable, Category="Movement|Physics")
	void StopMoveInput(bool bReduceVelocity = false);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|Physics", meta=(ClampMin="0", ClampMax="1"))
	float StopVelocityScale2D = 0.6f;     // gdy bReduceVelocity=true: procent prędkości 2D, który zostaje

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|Physics", meta=(ClampMin="0", ClampMax="1"))
	float StopAngularVelocityScale = 0.6f; // gdy bReduceVelocity=true: procent prędkości kątowej, który zostaje

	// Czy ruch fizyką – Player/Enemy = true
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|Physics")
	bool bUsePhysicsMovement = true;

	// Parametry wspólne – stroimy w BP dla Playera i każdego Enemy
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|Physics")
	float PhysicsAccelBase = 6000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|Physics")
	float PhysicsBrakeAccel = 4000.f;

	// Chroni przed “turbo” przy małej Strength: Accel ~ Speed / max(StrengthSoftener, Strength)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|Physics", meta=(ClampMin="0"))
	float StrengthSoftener = 10.f;

	// Docelowa prędkość 2D = Speed * SpeedScale
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|Physics", meta=(ClampMin="0"))
	float SpeedScale = 200.f;

	// Ile pozwolić przekroczyć prędkość docelową zanim zetniemy
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|Physics", meta=(ClampMin="1.0"))
	float MaxSpeedOvershoot = 1.1f;

	// Opcjonalny moment obrotowy dla “toczenia” (0 = off)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|Physics", meta=(ClampMin="0"))
	float TorqueScale = 0.f;

	// Minimalny próg wejścia (żeby nie reagować na bardzo małe wektory)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|Physics", meta=(ClampMin="0"))
	float MinInputThreshold = 0.001f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|Physics", meta=(ClampMin="0"))
	float BrakingDrag = 0.8f;        // “opór powietrza” [1/s] – im większy, tym szybciej gaśnie prędkość

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|Physics", meta=(ClampMin="0"))
	float FreewheelTime = 0.12f;     // ile sekund po puszczeniu inputu nic nie hamujemy

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|Physics", meta=(ClampMin="0"))
	float MinStopSpeed = 25.f;       // poniżej tego prędkości od razu zatrzymaj (bez drżenia)
	
protected:
	
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> SphereComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UBallAttributeSetBase> AttributeSet;

	// Obsługa kolizji
	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent,
	                            AActor* OtherActor,
	                            UPrimitiveComponent* OtherComp,
	                            int32 OtherBodyIndex,
	                            bool bFromSweep,
	                            const FHitResult& SweepResult);

private:
	// Agregator wejścia – klasy pochodne wywołują AddMoveInput2D, a baza przetwarza to w Tick
	FVector PendingMoveInput2D = FVector::ZeroVector;
	bool bHasMoveInput = false;

	double LastInputTime = -1e9;
	
	// Ruch – helpers
	float ComputeAccelFromAttributes() const;
	float GetTargetPlanarSpeed() const;
	virtual void ApplyBraking2D();
	void Seek2D(const FVector& DirNorm);
	void CapPlanarSpeed(float MaxPlanarSpeed);
};
