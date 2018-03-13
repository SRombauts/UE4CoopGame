// Copyright 2018 Sebastien Rombauts (sebastien.rombauts@gmail.com)

#include "SWeapon.h"

#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/Pawn.h"

static int32 DrawDebugWeapon = 0;
FAutoConsoleVariableRef CVAR_COOPDebugWeapons(
	TEXT("COOP.DrawDebugWeapon"),
	DrawDebugWeapon,
	TEXT("Draw Debug Line for Weapons"),
	ECVF_Cheat);

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
	AActor* Owner = GetOwner();
	if (Owner)
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
		const bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility, QueryParams);
		if (bHit)
		{
			// Blocking hit, process damages
			AActor* HitActor = HitResult.GetActor();
			UGameplayStatics::ApplyPointDamage(HitActor, 20.f, ShotDirection, HitResult, Owner->GetInstigatorController(), this, DamageTypeClass);

			APawn* HitPawn = Cast<APawn>(HitActor);
			if (HitPawn)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BloodEffect, HitResult.ImpactPoint, HitResult.ImpactNormal.Rotation());
			}
			else
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, HitResult.ImpactPoint, HitResult.ImpactNormal.Rotation());
			}

			TracerEndPoint = HitResult.ImpactPoint;
		}

		if (DrawDebugWeapon) DrawDebugLine(GetWorld(), TraceStart, TracerEndPoint, bHit ? FColor::Red : FColor::White, false, bHit ? 5.f : 1.f, 0, 1.f);

		PlayFireEffects(TracerEndPoint);

		// TODO: we should add some recoil, with some additional (random) Pitch and a bit of Yaw
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ASWeapon::Fire: No Owner set. Please set owning Pawn when equipping this weapon."));
	}
}

void ASWeapon::PlayFireEffects(const FVector& EndPoint)
{
	// Muzzle Particle Effect
	UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, SkeletalMeshComponent, MuzzleSocketName);

	// Fog tracer Particle Effect
	const FVector MuzzleLocation = SkeletalMeshComponent->GetSocketLocation(MuzzleSocketName);
	UParticleSystemComponent* ParticleSystemComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
	if (ParticleSystemComponent)
	{
		ParticleSystemComponent->SetVectorParameter(TracerTargetName, EndPoint);
	}

	// Gunshot sound
	UGameplayStatics::PlaySound2D(GetWorld(), ShotSound);

	// Camera Shake
	APawn* Owner = Cast<APawn>(GetOwner());
	if (Owner)
	{
		APlayerController* PlayerController = Cast<APlayerController>(Owner->GetController());
		if (PlayerController)
		{
			PlayerController->ClientPlayCameraShake(FireCameraShake);
		}
	}
}
