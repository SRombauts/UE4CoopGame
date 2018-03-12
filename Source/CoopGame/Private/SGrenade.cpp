// Copyright 2018 Sebastien Rombauts (sebastien.rombauts@gmail.com)

#include "SGrenade.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// Sets default values
ASGrenade::ASGrenade()
{
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	CollisionComponent->InitSphereRadius(10.f);
	CollisionComponent->SetSimulatePhysics(true);
//	CollisionComponent->SetCollisionProfileName("Projectile");
	RootComponent = CollisionComponent;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(CollisionComponent);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->UpdatedComponent = RootComponent;
	ProjectileMovementComponent->InitialSpeed = 2000.f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->bShouldBounce = true;
	ProjectileMovementComponent->bForceSubStepping = true;

	FuzeTime = 0.5f;
	ExplosionDamage = 100.f;
	ExplosionRadius = 500.f;

	SetReplicates(true);
	SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void ASGrenade::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &ASGrenade::OnExplosion, FuzeTime);
}

void ASGrenade::OnExplosion()
{
	// Effects
	UGameplayStatics::PlaySound2D(GetWorld(), ExplosionSound);
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());

	// Apply Explosion damage
	AController* Controller = Instigator ? Instigator->GetInstigatorController() : nullptr;
	const bool bDamageApplied = UGameplayStatics::ApplyRadialDamage(GetWorld(), ExplosionDamage, GetActorLocation(), ExplosionRadius, DamageTypeClass, TArray<AActor*>(), this, Controller);
	UE_LOG(LogTemp, Log, TEXT("ASGrenade::OnExplosion: bDamageApplied=%d"), bDamageApplied);
	DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 16, bDamageApplied?FColor::Red:FColor::Green, false, 1.f, 0, 1.f);

	if (Role == ENetRole::ROLE_Authority)
	{
		MakeNoise(1.0f, Instigator); // Needs investigator to be set appropriately when Spawn

		Destroy();
	}
}
