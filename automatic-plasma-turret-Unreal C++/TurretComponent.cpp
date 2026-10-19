
#include "TurretComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"	// used to access the components of the actor that owns this turret component.
#include "Kismet/GameplayStatics.h" // Provides helper functions, including access to the player pawn.
#include "GameFramework/Pawn.h"		// Provides APawn, the base class for player-controlled characters.
#include "Engine/World.h"			// Provides UWorld and its collision trace functions. LineTraceSingleByChannel
#include "DrawDebugHelpers.h"		// Provides functions for drawing debug shapes and lines
#include "TimerManager.h"			// Provides functions for starting and stopping timers.
#include "NiagaraComponent.h"		// Provides UNiagaraComponent, which is used for the aiming laser effect.
#include "NiagaraFunctionLibrary.h" // Provides functions for spawning and controlling Niagara particle systems. (flash)

// Set to 1 to draw debug traces, or 0 to disable them.
#define TURRET_DEBUG_TRACES 0

UTurretComponent::UTurretComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UTurretComponent::BeginPlay()
{
	Super::BeginPlay();

	// Collect all scene components belonging to the turret actor - Base, Head, and Muzzle.
	TArray<USceneComponent*> Components;
	GetOwner()->GetComponents<USceneComponent>(Components);

	// Find the turret head and muzzle by their component tags.
	for (USceneComponent* Component : Components)
	{
		if (Component->ComponentHasTag(FName(TEXT("TurretHead"))))
		{
			HeadComponent = Component;

			UE_LOG(LogTemp, Display,
				TEXT("TurretComponent: TurretHead head found."));
		}

		if (Component->ComponentHasTag(FName(TEXT("TurretMuzzle"))))
		{
			MuzzleComponent = Component;

			UE_LOG(LogTemp, Display,
				TEXT("TurretComponent: TurretMuzzle found."));
		}

		// Find the Niagara component tagged as TurretLaser.
		if (Component->ComponentHasTag(FName(TEXT("TurretLaser"))))
		{
			AimLaserComponent = Cast<UNiagaraComponent>(Component);
		}
	}

	if (HeadComponent == nullptr)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("TurretComponent: NO TurretHead found."));
	}

	if (MuzzleComponent == nullptr)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("TurretComponent: NO TurretMuzzle found."));
	}
}

void UTurretComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Keep the laser active and pointing straight ahead from the muzzle.
	if (AimLaserComponent != nullptr && MuzzleComponent != nullptr)
	{
		const FVector LaserStart =
			MuzzleComponent->GetComponentLocation();

		FVector LaserEnd =
			LaserStart + MuzzleComponent->GetForwardVector() * DetectionRange;


		// Shorten the laser to the first blocking surface.
		FHitResult LaserHit;
		FCollisionQueryParams LaserTraceParams;
		LaserTraceParams.AddIgnoredActor(GetOwner());

		if (GetWorld()->LineTraceSingleByChannel(
			LaserHit,
			LaserStart,
			LaserEnd,
			ECC_Visibility,
			LaserTraceParams))
		{
			LaserEnd = LaserHit.ImpactPoint;
		}

		AimLaserComponent->SetVariablePosition(
			FName(TEXT("User.LaserStart")),
			LaserStart
		);

		AimLaserComponent->SetVariablePosition(
			FName(TEXT("User.LaserEnd")),
			LaserEnd
		);

		AimLaserComponent->SetVisibility(true);

		if (!AimLaserComponent->IsActive())
		{
			AimLaserComponent->Activate();
		}
	}

		/*
		to aim laser at middle of player, the following code instead of the above line:
		AimLaserComponent->SetVariablePosition(
			FName(TEXT("User.LaserEnd")),
			PlayerLocation
		);
		*/


	if (HeadComponent == nullptr || MuzzleComponent == nullptr)
	{
		StopFiring();
		return;
	}

	// Get the pawn controlled by the first local player.
	APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	if (Player == nullptr)
	{
		StopFiring();
		return;
	}

	// get locations and calculate the distance between the turret head and the player. If the player is within detection range, rotate the turret head to face the player.
	const FVector HeadLocation = HeadComponent->GetComponentLocation();
	const FVector PlayerLocation = Player->GetActorLocation();
	const float Distance = (PlayerLocation - HeadLocation).Size();

	if (Distance <= DetectionRange)
	{
		// Perform a visibility trace from the turret's muzzle to the player to determine if the player is visible.
		const FVector TraceStart = MuzzleComponent->GetComponentLocation();
		const FVector TraceEnd = PlayerLocation;

		// Store information about the object hit by the visibility trace.
		FHitResult HitResult;

		// Configure the trace to ignore the turret actor.
		FCollisionQueryParams TraceParams; // settings of checking collision, like ignoring the owner of the turret component.
		TraceParams.AddIgnoredActor(GetOwner());

		// Check for the first blocking hit between the muzzle and the player.
		const bool bHit = GetWorld()->LineTraceSingleByChannel(
			HitResult,
			TraceStart,
			TraceEnd,
			ECC_Visibility,
			TraceParams
		);

		// The player is visible if the first blocking hit is the player.
		const bool bCanSeePlayer = bHit && HitResult.GetActor() == Player;

#if TURRET_DEBUG_TRACES
		// Draw a green line when the player is visible, otherwise draw red.
		DrawDebugLine(
			GetWorld(),
			TraceStart,
			TraceEnd,
			bCanSeePlayer ? FColor::Green : FColor::Red,
			false,	// Persistent lines
			-1.f,	// line is temporary and will be removed after a single frame.
			0,		// Depth priority
			2.f		// Line thickness
		);
#endif

		// Rotate towards the player only while they are visible.
		if (bCanSeePlayer)
		{
			const FVector Direction = (PlayerLocation - HeadLocation).GetSafeNormal();

			const FRotator CurrentRotation = HeadComponent->GetComponentRotation();

			// Preserve pitch and roll, changing only yaw.
			FRotator DesiredRotation = CurrentRotation;
			DesiredRotation.Yaw = Direction.Rotation().Yaw;

			const FRotator NewRotation = FMath::RInterpConstantTo(
				CurrentRotation,
				DesiredRotation,
				DeltaTime,
				RotationSpeed
			);

			HeadComponent->SetWorldRotation(NewRotation);

			// Start automatic firing while the player is visible.
			StartFiring();
		}
		else
		{
			// Stop firing when an obstacle hides the player.
			StopFiring();
		}
	}
	else
	{
		// Stop firing when the player leaves detection range.
		StopFiring();
	}
}

// Fire() spawns a projectile at the muzzle if the player is within detection range and visible.
void UTurretComponent::Fire()
{

	UE_LOG(LogTemp, Display, TEXT("TurretComponent: Fire called."));

	// Stop firing if the projectile class or muzzle is missing.
	if (ProjectileClass == nullptr || MuzzleComponent == nullptr)
	{
		StopFiring();
		return;
	}

	// Get the current player pawn before firing.
	APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	// Stop firing if the player or turret head is unavailable.
	if (Player == nullptr || HeadComponent == nullptr)
	{
		StopFiring();
		return;
	}

	// Check the current distance before firing.
	const float Distance = FVector::Distance(
		HeadComponent->GetComponentLocation(),
		Player->GetActorLocation()
	);

	if (Distance > DetectionRange)
	{
		StopFiring();
		return;
	}

	// Check visibility again immediately before spawning a projectile.
	FHitResult HitResult;

	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(GetOwner());

	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		MuzzleComponent->GetComponentLocation(),
		Player->GetActorLocation(),
		ECC_Visibility,
		TraceParams
	);

	// Cancel this shot and stop the timer if the player is hidden.
	if (!bHit || HitResult.GetActor() != Player)
	{
		StopFiring();
		return;
	}

	// Get the muzzle's world position and rotation.
	const FVector SpawnLocation = MuzzleComponent->GetComponentLocation();
	const FRotator SpawnRotation = MuzzleComponent->GetComponentRotation();

	// Use the same aiming origin as the head rotation.
	FVector DirectionToPlayer =
		Player->GetActorLocation() - HeadComponent->GetComponentLocation();

	DirectionToPlayer.Z = 0.f;

	if (DirectionToPlayer.IsNearlyZero())
	{
		return;
	}

	// Compare the muzzle's current yaw with the yaw needed to face the player.
	const float TargetYaw = DirectionToPlayer.Rotation().Yaw;
	const float HeadYaw = HeadComponent->GetComponentRotation().Yaw;

	const float AimError = FMath::Abs(
		FMath::FindDeltaAngleDegrees(HeadYaw, TargetYaw)
	);

	// Set the turret actor as the owner of the spawned projectile.
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();

	// Store the spawned projectile to check whether spawning succeeded.
	AActor* SpawnedProjectile = GetWorld()->SpawnActor<AActor>(
		ProjectileClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);

	// Spawn a muzzle flash after a successful projectile spawn.
	if (SpawnedProjectile != nullptr && MuzzleFlashEffect != nullptr)
	{
		UE_LOG(LogTemp, Display,
			TEXT("TurretComponent: spawning muzzle flash."));

		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			MuzzleFlashEffect,
			SpawnLocation,
			SpawnRotation
		);
	}

	// Play the firing sound at the muzzle after a successful shot.
	if (SpawnedProjectile != nullptr && FireSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			FireSound,
			SpawnLocation
		);
	}

	UE_LOG(LogTemp, Display,
		TEXT("[%.2f s] TurretComponent: spawn result: %s"),
		GetWorld()->GetTimeSeconds(),
		SpawnedProjectile ? TEXT("SUCCESS") : TEXT("FAILED"));

}



//	StopFiring() stops automatic firing.
void UTurretComponent::StopFiring()
{
	// Stop the automatic fire timer.
	GetWorld()->GetTimerManager().ClearTimer(FireTimerHandle);

	/*
	// Hide the aiming laser immediately when tracking stops.
	if (AimLaserComponent != nullptr)
	{
		AimLaserComponent->SetVisibility(false);
		AimLaserComponent->Deactivate();
	}
	*/

}

// StartFiring() starts automatic firing if it is not already active.
void UTurretComponent::StartFiring()
{
	// Do not start another timer if automatic firing is already active.
	if (GetWorld()->GetTimerManager().IsTimerActive(FireTimerHandle))
	{
		return;
	}

	// Do not fire with an invalid fire rate.
	if (ShotsPerSecond <= 0.f)
	{
		return;
	}

	// Convert shots per second into the interval between shots.
	const float ShotInterval = 1.f / ShotsPerSecond;

	// Repeatedly call Fire() at the specified interval.
	GetWorld()->GetTimerManager().SetTimer(
		FireTimerHandle,
		this,
		&UTurretComponent::Fire,
		ShotInterval,
		true
	);
}