// Copyright 2018 Sebastien Rombauts (sebastien.rombauts@gmail.com)

#include "STrackerBot.h"

#include "Components/StaticMeshComponent.h"

// Sets default values
ASTrackerBot::ASTrackerBot()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	MeshComponent->SetSimulatePhysics(true);
	MeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody); // Let other physics objects affect us (ie another bot)
	MeshComponent->SetCanEverAffectNavigation(false);
	RootComponent = MeshComponent;
}

// Called when the game starts or when spawned
void ASTrackerBot::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASTrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
