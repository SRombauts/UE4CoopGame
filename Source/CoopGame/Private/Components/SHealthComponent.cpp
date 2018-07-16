// Copyright 2018 Sebastien Rombauts (sebastien.rombauts@gmail.com)

#include "Components/SHealthComponent.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
USHealthComponent::USHealthComponent()
{
	InitialHealth = 100.f;

	// TODO NOCOMMIT TickComponent() to test ticking actors with ticking components
	PrimaryComponentTick.bCanEverTick = true;
	// TODO: this is required so that Deactive() do something
	bAutoActivate = true; 

	SetIsReplicated(true);
}

// Called when the game starts
void USHealthComponent::BeginPlay()
{
	Super::BeginPlay();	

	Health = InitialHealth;

	// Only hook damage events on the server
	if (GetOwnerRole() == ROLE_Authority)
	{
		AActor* Owner = GetOwner();
		if (Owner)
		{
			Owner->OnTakeAnyDamage.AddDynamic(this, &USHealthComponent::OnTakeAnyDamage);
		}
	}
}

void USHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// TODO NOCOMMIT
	if (GetOwner()->GetName() == TEXT("BP_TrackerBot_2"))
	{
		UE_LOG(LogTemp, Log, TEXT("%s %s TickComponent(%f)"), *GetOwner()->GetName(), *GetName(), DeltaTime);
	}
}

void USHealthComponent::OnTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if ((Health > 0.f) && (Damage > 0.f))
	{
		// Update Health clamped
	//	Health = FMath::Clamp(Health - Damage, 0.f, InitialHealth);
		Health = FMath::Max(Health - Damage, 0.f);

		UE_LOG(LogTemp, Log, TEXT("Health changed: %s"), *FString::SanitizeFloat(Health));

		OnHealthChangedEvent.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);
	}
}

void USHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USHealthComponent, Health);
}
