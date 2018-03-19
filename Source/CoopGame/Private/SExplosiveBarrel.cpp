// Copyright 2018 Sebastien Rombauts (sebastien.rombauts@gmail.com)

#include "SExplosiveBarrel.h"

#include "Components/SphereComponent.h"
#include "Components/SHealthComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ASExplosiveBarrel::ASExplosiveBarrel()
{
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	MeshComponent->SetSimulatePhysics(true);
	RootComponent = MeshComponent;

	// Use a sphere as a simple explosion area effect representation
	RepulseComponent = CreateDefaultSubobject<USphereComponent>(TEXT("RadialForceComponent"));
	RepulseComponent->SetSphereRadius(600);
	RepulseComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	RepulseComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	RepulseComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_PhysicsBody, ECollisionResponse::ECR_Overlap); // collision filters : only IsSimulatingPhysics() components
	RepulseComponent->SetupAttachment(RootComponent);

	HealthComponent = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComponent"));

	VerticalImpulse = 50000.f;
	RepulseForce = 50000.f;
}

// Called when the game starts or when spawned
void ASExplosiveBarrel::BeginPlay()
{
	Super::BeginPlay();

	HealthComponent->OnHealthChangedEvent.AddDynamic(this, &ASExplosiveBarrel::OnHealthChangedEvent);
}

void ASExplosiveBarrel::OnHealthChangedEvent(USHealthComponent* HealthComp, float Health, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (Health <= 0.f)
	{
		// Die!
		UE_LOG(LogTemp, Log, TEXT("Die!"));

		// Switch material from red to black
		MeshComponent->SetMaterial(0, ExplodedMaterial);

		// Explosion Particle Effect and Sound
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
		UGameplayStatics::PlaySound2D(GetWorld(), ExplosionSound);

		// Launch the barrel upward
		const FVector ImpulseVector = FVector::UpVector * VerticalImpulse;
		MeshComponent->AddImpulse(ImpulseVector);

		// Add Radial force to repulse physics enabled actors
		TArray<UPrimitiveComponent*> ComponentsToRepulse;
		RepulseComponent->GetOverlappingComponents(ComponentsToRepulse);
		for (auto& ComponentToRepulse : ComponentsToRepulse)
		{
			if (ComponentToRepulse && ComponentToRepulse->IsSimulatingPhysics()) // NOTE: this is redundant with the collision filters we have applied
			{
				const bool bAccelChange = true; // If true, Force is taken as a change in acceleration instead of a physical force (i.e. mass will have no affect).
				ComponentToRepulse->AddRadialForce(GetActorLocation(), RepulseComponent->GetScaledSphereRadius(), RepulseForce, ERadialImpulseFalloff::RIF_Linear, bAccelChange);
			}
		}
	}
}
