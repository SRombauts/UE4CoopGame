// Copyright 2018 Sebastien Rombauts (sebastien.rombauts@gmail.com)

#include "SWeapon.h"

#include "CoopGame.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"

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

	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComp"));
	WidgetComponent->SetupAttachment(RootComponent, VisorSocketName);

	DefaultDamage = 20.f;
	HighDamage = 80.f;

	TimeBetweenShots = .15f;

	InitialAmmunitions = 30;
	Ammunitions = InitialAmmunitions;
}

// Called when the game starts or when spawned
void ASWeapon::BeginPlay()
{
	Super::BeginPlay();
}

void ASWeapon::StartFire()
{
	const bool bDoLoop = true;
	const float FirstDelay = FMath::Max((LastFireTime - GetWorld()->TimeSeconds + TimeBetweenShots), 0.f);
	GetWorldTimerManager().SetTimer(TimerHandle_AutoFire, this, &ASWeapon::Fire, TimeBetweenShots, bDoLoop, FirstDelay);
}

void ASWeapon::EndFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_AutoFire);
}

void ASWeapon::Fire()
{
	AActor* Owner = GetOwner();
	if (Owner)
	{
		if (Ammunitions > 0)
		{
			--Ammunitions;
			OnAmmunitionsChangedEvent.Broadcast(Ammunitions);

			LastFireTime = GetWorld()->TimeSeconds;

			// Hit-Scan Weapon: Trace the world from our Pawn point of view (camera) toward crosshair direction
			FVector TraceStart;
			FRotator ViewpointOrientation;
			Owner->GetActorEyesViewPoint(TraceStart, ViewpointOrientation);
			const FVector ShotDirection = ViewpointOrientation.Vector();
			const FVector TraceEnd = TraceStart + ShotDirection * 10000.f;

			FVector TracerEndPoint = TraceEnd;
			FColor DrawDebugColor = FColor::White;

			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(Owner);
			QueryParams.AddIgnoredActor(this);
			QueryParams.bTraceComplex = true;
			QueryParams.bReturnPhysicalMaterial = true;

			FHitResult HitResult;
			const bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, COLLISION_WEAPON, QueryParams);
			if (bHit)
			{
				// Blocking hit, process damages
				const EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());

				const float Damage = (SURFACE_FLESH_VULNERABLE == SurfaceType) ? HighDamage : DefaultDamage;

				UGameplayStatics::ApplyPointDamage(HitResult.GetActor(), Damage, ShotDirection, HitResult, Owner->GetInstigatorController(), this, DamageTypeClass);

				UParticleSystem* HitEffect = ImpactEffect;
				if ((SURFACE_FLESH_DEFAULT == SurfaceType) || (SURFACE_FLESH_VULNERABLE == SurfaceType))
				{
					HitEffect = BloodEffect;
					DrawDebugColor = (SURFACE_FLESH_VULNERABLE == SurfaceType) ? FColor::Red : FColor::Orange;
				}
				else
				{
					DrawDebugColor = FColor::Green;
				}
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitEffect, HitResult.ImpactPoint, HitResult.ImpactNormal.Rotation());

				TracerEndPoint = HitResult.ImpactPoint;
			}

			if (DrawDebugWeapon) DrawDebugLine(GetWorld(), TraceStart, TracerEndPoint, DrawDebugColor, false, bHit ? 5.f : 1.f, 0, 1.f);

			PlayFireEffects(TracerEndPoint);

			// TODO: we should add some recoil, with some additional (random) Pitch and a bit of Yaw
		}
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
