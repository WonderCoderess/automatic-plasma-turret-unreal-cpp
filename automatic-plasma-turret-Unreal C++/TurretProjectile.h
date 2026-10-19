// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TurretProjectile.generated.h"
class USphereComponent;
class UProjectileMovementComponent;
class UStaticMeshComponent;

UCLASS()
class ARENASHOOTER_API ATurretProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATurretProjectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	// Sphere used to detect projectile collisions.
	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	USphereComponent* CollisionComponent;

	// Handles projectile movement and collision sweeps (this way no manual position updates needed in Tick)
	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	UProjectileMovementComponent* ProjectileMovement;

	// Visual representation of the projectile.
	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	UStaticMeshComponent* ProjectileMesh;

	// Destroy the projectile when its movement stops on impact.
	UFUNCTION()
	void OnProjectileStopped(const FHitResult& Hit);



};
