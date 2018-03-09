// Copyright 2018 Sebastien Rombauts (sebastien.rombauts@gmail.com)

#include "SWeapon.h"

#include "Components/SkeletalMeshComponent.h"

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
