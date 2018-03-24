// Copyright 2018 Sebastien Rombauts (sebastien.rombauts@gmail.com)

#include "SExplosiveBarrel.h"

#include "Components/SHealthComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "PhysicsEngine/RadialForceComponent.h"

extern int32 DrawDebugWeapon; // Defined in SWeapon.cpp

// Sets default values
ASExplosiveBarrel::ASExplosiveBarrel()
{
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	MeshComponent->SetSimulatePhysics(true);
	MeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody); // Let RadialForceComponent affect us (ie another barrel explode neaaby)
	RootComponent = MeshComponent;

	// Use a sphere as a simple explosion area effect representation
	RadialForceComponent = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComponent"));
	RadialForceComponent->Radius = 400;
	RadialForceComponent->Falloff = ERadialImpulseFalloff::RIF_Linear;
	RadialForceComponent->ImpulseStrength = 50000.f;
//	RadialForceComponent->ForceStrength;
//	RadialForceComponent->DestructibleDamage;
	RadialForceComponent->bAutoActivate = false; // Prevent radial force to apply on every Tick, use FireImpulse() instead
	RadialForceComponent->bIgnoreOwningActor = true;
	RadialForceComponent->SetupAttachment(RootComponent);

	HealthComponent = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComponent"));

	VerticalImpulse = 50000.f;
	ExplosionDamage = 150.f;
	ExplosionRadius = 400.f;

	SetReplicates(true);
	SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void ASExplosiveBarrel::BeginPlay()
{
	Super::BeginPlay();

	HealthComponent->OnHealthChangedEvent.AddDynamic(this, &ASExplosiveBarrel::OnHealthChangedEvent);
}

void ASExplosiveBarrel::OnHealthChangedEvent(USHealthComponent* HealthComp, float Health, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	// The following only runs on the Server, since damage & health event binding are only if (Role == ROLE_Authority)
	if (Health <= 0.f)
	{
		// Explode! Replicated to play cosmetic effects on clients
		bExploded = true;
		UE_LOG(LogTemp, Log, TEXT("Explode!"));

		// Launch the barrel upward
		const FVector ImpulseVector = FVector::UpVector * VerticalImpulse;
		MeshComponent->AddImpulse(ImpulseVector);

		// Blasts away physics enabled actors
		RadialForceComponent->FireImpulse();

		// Apply Explosion damage
		AController* Controller = Instigator ? Instigator->GetInstigatorController() : nullptr;
		const bool bDamageApplied = UGameplayStatics::ApplyRadialDamage(GetWorld(), ExplosionDamage, GetActorLocation(), ExplosionRadius, DamageTypeClass, TArray<AActor*>(), this, Controller);
		UE_LOG(LogTemp, Log, TEXT("ASExplosiveBarrel::OnExplosion: bDamageApplied=%d"), bDamageApplied);
		if (DrawDebugWeapon) DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 16, bDamageApplied ? FColor::Red : FColor::Green, false, 1.f, 0, 1.f);
	}
}

void ASExplosiveBarrel::OnRep_Exploded()
{
	// Switch material from red to black
	MeshComponent->SetMaterial(0, ExplodedMaterial);

	// Explosion Particle Effect and Sound
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
	UGameplayStatics::PlaySound2D(GetWorld(), ExplosionSound);
}

void ASExplosiveBarrel::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASExplosiveBarrel, bExploded);
}
