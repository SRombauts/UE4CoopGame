// Copyright 2018 Sebastien Rombauts (sebastien.rombauts@gmail.com)

#include "SWeapon.h"

#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/SkeletalMeshSocket.h"

// Sets default values
ASWeapon::ASWeapon()
{
	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
	RootComponent = SkeletalMeshComponent;
}

// Called when the game starts or when spawned
void ASWeapon::BeginPlay()
{
	Super::BeginPlay();

}

void ASWeapon::Fire()
{
	UWorld* World = GetWorld();
	AActor* Owner = GetOwner();
	if (World && Owner)
	{
		// Hit-Scan Weapon: Trace the world from our Pawn point of view (camera) toward crosshair direction
		FVector TraceStart;
		FRotator ViewpointOrientation;
		Owner->GetActorEyesViewPoint(TraceStart, ViewpointOrientation);
		const FVector ShotDirection = ViewpointOrientation.Vector();
		const FVector TraceEnd = TraceStart + ShotDirection * 10000.f;
		
		FVector TracerEndPoint = TraceEnd;

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(Owner);
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true;

		FHitResult HitResult;
		const bool bHit = World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility, QueryParams);
		if (bHit)
		{
			// Blocking hit, process damages
			AActor* HitActor = HitResult.GetActor();
			UGameplayStatics::ApplyPointDamage(HitActor, 20.f, ShotDirection, HitResult, Owner->GetInstigatorController(), this, DamageTypeClass);

			APawn* HitPawn = Cast<APawn>(HitActor);
			if (HitPawn && BloodEffect)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BloodEffect, HitResult.ImpactPoint, HitResult.ImpactNormal.Rotation());
			}
			else if (ImpactEffect)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, HitResult.ImpactPoint, HitResult.ImpactNormal.Rotation());
			}

			TracerEndPoint = HitResult.ImpactPoint;
		}

		if (MuzzleEffect)
		{
			UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, SkeletalMeshComponent, MuzzleSocketName);
		}

		if (TracerEffect)
		{
			const FVector MuzzleLocation = SkeletalMeshComponent->GetSocketLocation(MuzzleSocketName);

			UParticleSystemComponent* ParticleSystemComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
			ParticleSystemComponent->SetVectorParameter(TracerTargetName, TracerEndPoint);
		}
	}
}
