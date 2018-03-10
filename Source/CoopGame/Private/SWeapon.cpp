// Copyright 2018 Sebastien Rombauts (sebastien.rombauts@gmail.com)

#include "SWeapon.h"

#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"

// Sets default values
ASWeapon::ASWeapon()
{
	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
	RootComponent = SkeletalMeshComponent;
}

void ASWeapon::Fire()
{
	UWorld* World = GetWorld();
	AActor* Owner = GetOwner();
	if (World && Owner)
	{
		// Trace the world from pawn point of view (camera) toward crosshair direction
		FVector TraceStart;
		FRotator ViewpointOrientation;
		Owner->GetActorEyesViewPoint(TraceStart, ViewpointOrientation);
		const FVector TraceEnd = TraceStart + ViewpointOrientation.Vector() * 10000.f;

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(Owner);
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true;

		FHitResult HitResult;
		const bool bHit = World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility, QueryParams);
		if (bHit)
		{
			DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Green, false, 1.f, 0, 1.f);

			// TODO: blocking hit, process damages
		}
	}
}

// Called when the game starts or when spawned
void ASWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}
