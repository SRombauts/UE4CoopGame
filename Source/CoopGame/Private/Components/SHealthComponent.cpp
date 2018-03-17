// Copyright 2018 Sebastien Rombauts (sebastien.rombauts@gmail.com)

#include "Components/SHealthComponent.h"
#include "GameFramework/Actor.h"

// Sets default values for this component's properties
USHealthComponent::USHealthComponent()
{
	InitialHealth = 100.f;
}

// Called when the game starts
void USHealthComponent::BeginPlay()
{
	Super::BeginPlay();	

	Health = InitialHealth;

	AActor* Owner = GetOwner();
	if (Owner)
	{
		Owner->OnTakeAnyDamage.AddDynamic(this, &USHealthComponent::OnTakeAnyDamage);
	}
}

void USHealthComponent::OnTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage > 0.f)
	{
		// Update Health clamped
	//	Health = FMath::Clamp(Health - Damage, 0.f, InitialHealth);
		Health = FMath::Max(Health - Damage, 0.f);

		UE_LOG(LogTemp, Log, TEXT("Health changed: %f (%s)"), Health, *FString::SanitizeFloat(Health));

		OnHealthChangedEvent.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);
	}
}
