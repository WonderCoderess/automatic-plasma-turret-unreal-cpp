#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TurretComponent.generated.h"
class USceneComponent; 
class UNiagaraComponent;
class UNiagaraSystem;
class USoundBase;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ARENASHOOTER_API UTurretComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UTurretComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// The range within which the turret can detect targets and speed rotate towards them.
	UPROPERTY(EditAnywhere, Category = "Turret settings")
	float DetectionRange = 1500.f;

	UPROPERTY(EditAnywhere, Category = "Turret settings")
	float RotationSpeed = 90.f;

	// Projectile class to spawn when the turret fires.
	UPROPERTY(EditAnywhere, Category = "Turret")
	TSubclassOf<AActor> ProjectileClass;

	// Niagara effect spawned at the muzzle after a successful shot.
	UPROPERTY(EditAnywhere, Category = "Turret")
	UNiagaraSystem* MuzzleFlashEffect = nullptr;

private:
	//Reference to the head component used for rotation and muzzle direction. 
	UPROPERTY()
	USceneComponent* HeadComponent = nullptr;

	UPROPERTY()
	USceneComponent* MuzzleComponent = nullptr;

	// Reference to the Niagara component used for the aiming laser.
	UPROPERTY()
	UNiagaraComponent* AimLaserComponent = nullptr;

	// Number of shots fired per second.
	UPROPERTY(EditAnywhere, Category = "Turret", meta = (ClampMin = "0.1"))
	float ShotsPerSecond = 2.f;

	// Maximum horizontal aiming error allowed before firing, in degrees.
	UPROPERTY(EditAnywhere, Category = "Turret", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float AimTolerance = 5.f;

	// Sound played at the muzzle after a successful shot.
	UPROPERTY(EditAnywhere, Category = "Turret")
	USoundBase* FireSound = nullptr;

	// Spawn one projectile at the muzzle.
	void Fire();

	// Handle used to control the automatic fire timer.
	FTimerHandle FireTimerHandle;

	// Start automatic firing if it is not already active.
	void StartFiring();

	// Stop automatic firing.
	void StopFiring();
};
