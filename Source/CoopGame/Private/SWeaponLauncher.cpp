// Copyright 2018 Sebastien Rombauts (sebastien.rombauts@gmail.com)

#include "SWeaponLauncher.h"

#include "SGrenade.h"

#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
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
		//	FRotator MuzzleRotation = SkeletalMeshComponent->GetSocketRotation(MuzzleSocketName); // NOTE: this has a fixed zero Pitch for now, use ViewpointOrientation instead
			FVector ViewpointLocation;
			FRotator ViewpointOrientation;
			OwningPawn->GetActorEyesViewPoint(ViewpointLocation, ViewpointOrientation);

			// Set Spawn Collision Handling Override
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
			SpawnParams.Instigator = OwningPawn; // required to associate the projectile to the Pawn

			// Spawn a Grenade
			GetWorld()->SpawnActor<ASGrenade>(GrenadeClass, MuzzleLocation, ViewpointOrientation, SpawnParams);

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
