// Copyright 2018 Sebastien Rombauts (sebastien.rombauts@gmail.com)

#include "SWeapon.h"

#include "CoopGame.h"

#include "DrawDebugHelpers.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "TimerManager.h"

int32 DrawDebugWeapon = 0;
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
	ReloadTime = 2.5f;

	InitialAmmunitions = 30;
	Ammunitions = InitialAmmunitions;

	MuzzleSocketName = TEXT("MuzzleFlashSocket");
	VisorSocketName = TEXT("HoloVisorSocket");
	MagazineSocketName = TEXT("MagazineSocket");
	TracerTargetName = TEXT("BeamEnd");

	SetReplicates(true);
	MinNetUpdateFrequency = 33.f;
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
	if (Role != ROLE_Authority)
	{
		// Fire, replicate and apply damage on the server
		ServerFire();
	}

	// Do the following on the client owner as well so that there is a minimal amount of latency when firing
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
				DrawDebugColor = FColor::Red;
				TracerEndPoint = HitResult.ImpactPoint;

				PlayImpactEffects(SurfaceType, HitResult.ImpactPoint);

				if (Role == ROLE_Authority)
				{
					const float Damage = (SURFACE_FLESH_VULNERABLE == SurfaceType) ? HighDamage : DefaultDamage;
					UGameplayStatics::ApplyPointDamage(HitResult.GetActor(), Damage, ShotDirection, HitResult, Owner->GetInstigatorController(), this, DamageTypeClass);

					HitScanTrace.TraceTo = HitResult.ImpactPoint;
					HitScanTrace.SurfaceType = SurfaceType;
				}
			}
			else
			{
				if (Role == ROLE_Authority)
				{
					HitScanTrace.TraceTo = TracerEndPoint;
					HitScanTrace.SurfaceType = SurfaceType_Default;
				}
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

void ASWeapon::OnRep_HitScanTrace()
{
	// Play cosmetic effects on other clients
	PlayFireEffects(HitScanTrace.TraceTo);
	PlayImpactEffects(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);
}

void ASWeapon::ServerFire_Implementation() // Replicated
{
	Fire();
}

bool ASWeapon::ServerFire_Validate() // WithValidation
{
	return true;
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

	// Bullet's shell ejection
	const FVector MagazineLocation = SkeletalMeshComponent->GetSocketLocation(MagazineSocketName);
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ShellEjectEffect, MagazineLocation);

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

void ASWeapon::PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint)
{
	UParticleSystem* HitEffect = ImpactEffect;
	if ((SURFACE_FLESH_DEFAULT == SurfaceType) || (SURFACE_FLESH_VULNERABLE == SurfaceType))
	{
		HitEffect = BloodEffect;
	}
	const FVector MuzzleLocation = SkeletalMeshComponent->GetSocketLocation(MuzzleSocketName);
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitEffect, ImpactPoint, (ImpactPoint - MuzzleLocation).Rotation());
}

void ASWeapon::Reload()
{
	FTimerHandle TimerHandle_Reload;
	GetWorldTimerManager().SetTimer(TimerHandle_Reload, this, &ASWeapon::ReloadDone, ReloadTime);

	Ammunitions = 0;
	OnAmmunitionsChangedEvent.Broadcast(Ammunitions);
}

void ASWeapon::ReloadDone()
{
	Ammunitions = InitialAmmunitions;
	OnAmmunitionsChangedEvent.Broadcast(Ammunitions);

	LastFireTime = GetWorld()->TimeSeconds - TimeBetweenShots;
}

void ASWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASWeapon, Ammunitions, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ASWeapon, HitScanTrace, COND_SkipOwner);
}
