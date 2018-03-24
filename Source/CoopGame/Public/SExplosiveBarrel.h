// Copyright 2018 Sebastien Rombauts (sebastien.rombauts@gmail.com)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SExplosiveBarrel.generated.h"

UCLASS()
class COOPGAME_API ASExplosiveBarrel : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASExplosiveBarrel();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Barrel Mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	// Sphere collision component to apply radial force to physical actors
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= "Components")
	class URadialForceComponent* RadialForceComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USHealthComponent* HealthComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	class UParticleSystem* ExplosionEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	UMaterialInterface* ExplodedMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	USoundBase* ExplosionSound;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
	float VerticalImpulse;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
	float ExplosionDamage;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
	float ExplosionRadius;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
	TSubclassOf<UDamageType> DamageTypeClass;

	UFUNCTION()
	void OnHealthChangedEvent(class USHealthComponent* HealthComp, float Health, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UPROPERTY(ReplicatedUsing = OnRep_Exploded, VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	bool bExploded = false;

	// Play cosmetic effects on clients
	UFUNCTION()
	void OnRep_Exploded();
};
