// Copyright 2018 Sebastien Rombauts (sebastien.rombauts@gmail.com)

#include "SWeaponLauncher.h"

#include "SGrenade.h"

#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/SkeletalMeshSocket.h"

void ASWeaponLauncher::Fire()
{
	APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (OwningPawn)
	{
		if (GrenadeClass)
		{
			FVector MuzzleLocation = SkeletalMeshComponent->GetSocketLocation(MuzzleSocketName);
			FRotator MuzzleRotation = SkeletalMeshComponent->GetSocketRotation(MuzzleSocketName);

			// Set Spawn Collision Handling Override
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
			ActorSpawnParams.Instigator = OwningPawn; // required to associate the projectile to the Pawn

			// Spawn a Grenade
			GetWorld()->SpawnActor<ASGrenade>(GrenadeClass, MuzzleLocation, MuzzleRotation);

			// Muzzle Particle Effect
			UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, SkeletalMeshComponent, MuzzleSocketName);

			// Gunshot sound
			UGameplayStatics::PlaySound2D(GetWorld(), ShotSound);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ASWeaponLauncher::Fire: No GrenadeClass set."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ASWeaponLauncher::Fire: No Owner set. Please set owning Pawn when equipping this weapon."));
	}
}
