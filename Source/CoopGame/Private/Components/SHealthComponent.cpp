// Copyright 2018 Sebastien Rombauts (sebastien.rombauts@gmail.com)

#include "Components/SHealthComponent.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
USHealthComponent::USHealthComponent()
{
	InitialHealth = 100.f;

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
