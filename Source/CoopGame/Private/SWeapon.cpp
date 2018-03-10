// Copyright 2018 Sebastien Rombauts (sebastien.rombauts@gmail.com)

#include "SWeapon.h"

#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

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

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(Owner);
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true;

		FHitResult HitResult;
		const bool bHit = World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility, QueryParams);
		if (bHit)
		{
			DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Green, false, 1.f, 0, 1.f);

			// Blocking hit, process damages
			AActor* HitActor = HitResult.GetActor();
			if (HitActor)
			{
				UGameplayStatics::ApplyPointDamage(HitActor, 20.f, ShotDirection, HitResult, Owner->GetInstigatorController(), this, DamageTypeClass);
			}
		}
	}
}
