#include "TurretProjectile.h"
#include "Components/SphereComponent.h" // Provides sphere collision.
#include "GameFramework/ProjectileMovementComponent.h" // Provides projectile movement.
#include "Components/StaticMeshComponent.h" // Provides the visual mesh component.

ATurretProjectile::ATurretProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create the projectile's sphere collision component.
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	RootComponent = CollisionComponent;

	// Set a 5 cm collision radius and block all channels.
	CollisionComponent->InitSphereRadius(5.f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Block);

	// Create the projectile movement component.
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));

	// Move the collision sphere and check for collisions along its path. wskazanie, ktory obiekt ma byc aktualizowany przez komponent ruchu pocisku 
	ProjectileMovement->SetUpdatedComponent(CollisionComponent);

	// Set the initial and maximum speed in centimeters per second.
	ProjectileMovement->InitialSpeed = 2000.f;
	ProjectileMovement->MaxSpeed = 2000.f;

	// Launch along the projectile's local forward axis (+X).
	ProjectileMovement->bInitialVelocityInLocalSpace = true;
	ProjectileMovement->Velocity = FVector(1.f, 0.f, 0.f);
	ProjectileMovement->ProjectileGravityScale = 0.f;

	// Automatically destroy the projectile after 5 seconds.
	InitialLifeSpan = 5.f;

	// Create the projectile's visual component, attach it to the collision sphere (parent/child), and disable collision for it (the sphere handles collisions).
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(CollisionComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	// Bind the OnProjectileStopped function to the projectile movement component's OnProjectileStop event.
	ProjectileMovement->OnProjectileStop.AddDynamic(
		this,
		&ATurretProjectile::OnProjectileStopped
	);
}

void ATurretProjectile::BeginPlay()
{
	Super::BeginPlay();	
}

// Called every frame
void ATurretProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Ignore the firing turret when moving the projectile.
	if (AActor* FiringActor = GetOwner())
	{
		CollisionComponent->IgnoreActorWhenMoving(FiringActor, true);
	}

}

void ATurretProjectile::OnProjectileStopped(const FHitResult& Hit)
{
	// Remove the projectile after a blocking impact.
	Destroy();
}

