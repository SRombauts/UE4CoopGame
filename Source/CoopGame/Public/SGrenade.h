// Copyright 2018 Sebastien Rombauts (sebastien.rombauts@gmail.com)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SGrenade.generated.h"

UCLASS()
class COOPGAME_API ASGrenade : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASGrenade();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Sphere collision component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= "Components")
	class USphereComponent* CollisionComponent;

	/** Static Mesh component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= "Components")
	class UStaticMeshComponent* StaticMeshComponent;

	/** Projectile movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	UParticleSystem* ExplosionEffect;

	// Sound to play on explosion
	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	USoundBase* ExplosionSound;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
	float FuzeTime = 0.5f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
	float ExplosionDamage = 100.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
	float ExplosionRadius = 500.f;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
	TSubclassOf<UDamageType> DamageTypeClass;

	void OnExplosion();
};
