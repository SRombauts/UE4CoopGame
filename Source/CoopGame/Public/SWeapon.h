// Copyright 2018 Sebastien Rombauts (sebastien.rombauts@gmail.com)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWeapon.generated.h"

// OnAmmunitionsChangedEvent
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAmmunitionsChangedSignature, int32, Ammunitions);

/**
 * Weapon attached to the right-hand socket of our Character's Skeletal Mesh
 */
UCLASS()
class COOPGAME_API ASWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USkeletalMeshComponent* SkeletalMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UWidgetComponent* WidgetComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	class UParticleSystem* MuzzleEffect;

	// Name of the Rifle Muzzle Socket to display exhaust gas on fire
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName MuzzleSocketName;

	// Name of the socket at the visor location, to display current ammunition level
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName VisorSocketName;

	// Name of the FX parameter to set the location of the endpoint
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName TracerTargetName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	class UParticleSystem* TracerEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	class UParticleSystem* ImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	class UParticleSystem* BloodEffect;

	// Time between shots in seconds
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float TimeBetweenShots;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float DefaultDamage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float HighDamage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<UDamageType> DamageTypeClass;

	// Size of a full ammunition magazine
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ammunitions")
	int32 InitialAmmunitions;

	// Count of currently loaded ammunitions
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Ammunitions")
	int32 Ammunitions;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<UCameraShake> FireCameraShake;

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	USoundBase* ShotSound;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void Fire();

private:
	float LastFireTime = 0.f;
	FTimerHandle TimerHandle_AutoFire;

public:
	void StartFire();
	void EndFire();

	void PlayFireEffects(const FVector& EndPoint);

public:
	// Called when the ammunition count changes
	UPROPERTY(BlueprintAssignable, Category="Weapon|Ammunitions")
	FOnAmmunitionsChangedSignature OnAmmunitionsChangedEvent;
};
