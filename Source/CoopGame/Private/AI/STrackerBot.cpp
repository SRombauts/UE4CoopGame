// Copyright 2018 Sebastien Rombauts (sebastien.rombauts@gmail.com)

#include "STrackerBot.h"

#include "AI/Navigation/NavigationPath.h"
#include "AI/Navigation/NavigationSystem.h"
#include "Components/SHealthComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

int32 DrawDebugNav = 0;
FAutoConsoleVariableRef CVAR_COOPDebugNav(
	TEXT("COOP.DrawDebugNav"),
	DrawDebugNav,
	TEXT("Draw Debug Sphere for Navigation"),
	ECVF_Cheat);

// Sets default values
ASTrackerBot::ASTrackerBot()
{
 	// Set this pawn to call Tick() every frame, as a way to navigate toward the player.
	PrimaryActorTick.bCanEverTick = true;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	MeshComponent->SetSimulatePhysics(true);
	MeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody); // Let other physics objects affect us (ie another bot)
	MeshComponent->SetCanEverAffectNavigation(false);
	RootComponent = MeshComponent;

	HealthComponent = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComponent"));

	SetReplicates(true);
	SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void ASTrackerBot::BeginPlay()
{
	Super::BeginPlay();

	HealthComponent->OnHealthChangedEvent.AddDynamic(this, &ASTrackerBot::OnHealthChangedEvent);

	NextPathPoint = GetNextPathPoint();
}

void ASTrackerBot::OnHealthChangedEvent(class USHealthComponent* HealthComp, float Health, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	// The following only runs on the Server, since damage & health event binding are only if (Role == ROLE_Authority)
	if (Health <= 0.f)
	{
		// Explode! Replicated to play cosmetic effects on clients
		bExploded = true;
		UE_LOG(LogTemp, Log, TEXT("Explode!"));
	}
}

void ASTrackerBot::OnRep_Exploded()
{
	// TODO Play cosmetic effects on clients
}

FVector ASTrackerBot::GetNextPathPoint()
{
	APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (Player)
	{
		UNavigationPath* Path = UNavigationSystem::FindPathToActorSynchronously(this, GetActorLocation(), Player);
		if (Path && Path->PathPoints.Num() > 1)
		{
			if (DrawDebugNav)
			{
				for (size_t Idx = 1; Idx < Path->PathPoints.Num(); Idx++)
				{
					DrawDebugSphere(GetWorld(), Path->PathPoints[Idx], PathPointRadius, 16, FColor::Yellow, false, 3.f, 0, 1.f);
				}
			}

			// Return the next point in the path
			return Path->PathPoints[1];
		}
	}
	return GetActorLocation();
}

// Called every frame
void ASTrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetActorLocation().Equals(NextPathPoint, PathPointRadius))
	{
		// When near to the next Path Point, find and target the one after it
		NextPathPoint = GetNextPathPoint();
	}
	else
	{
		// Keep moving toward the next point in navigation path
		FVector ForceDirection = NextPathPoint - GetActorLocation();
		ForceDirection.Normalize();
		ForceDirection *= Force;

		const bool bAccelChange = true;
		MeshComponent->AddForce(ForceDirection, NAME_None, bAccelChange);
	}
}

void ASTrackerBot::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASTrackerBot, bExploded);
}
