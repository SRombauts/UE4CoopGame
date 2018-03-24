// Copyright 2018 Sebastien Rombauts (sebastien.rombauts@gmail.com)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWeapon.generated.h"

// OnAmmunitionsChangedEvent
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAmmunitionsChangedSignature, int32, Ammunitions);

// Contains information about a single hitscan weapon line trace
USTRUCT()
struct FHitScanTrace
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TEnumAsByte<EPhysicalSurface> SurfaceType;

	UPROPERTY()
	FVector_NetQuantize TraceTo;
};

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

	// UMG BP UI to display the ammunitions count next to the holographic visor
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UWidgetComponent* WidgetComponent;

	// Name of the Rifle Muzzle Socket to display exhaust gas on fire
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName MuzzleSocketName;

	// Name of the socket at the visor location, to display current ammunition level
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName VisorSocketName;

	// Name of the socket at the ammunition magazine, for shell ejection
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName MagazineSocketName;

	// Name of the Fog Tracer FX parameter "BeamEnd" to set the location of the endpoint
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName TracerTargetName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	class UParticleSystem* MuzzleEffect;

	// Fog effect on the bullet's trajectory
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	class UParticleSystem* TracerEffect;

	// Default particle effect of the bullet's impact on hard surfaces
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	class UParticleSystem* ImpactEffect;

	// Blood-splat on bullet's penetration on flesh
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	class UParticleSystem* BloodEffect;

	// Effect to display on ejection of bullet's shell on each fire
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	class UParticleSystem* ShellEjectEffect;

	// Time between shots in seconds
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float TimeBetweenShots;

	// Reload time in seconds
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float ReloadTime;

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
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Ammunitions")
	int32 Ammunitions;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<UCameraShake> FireCameraShake;

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	USoundBase* ShotSound;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void Fire();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire();

private:
	float LastFireTime = 0.f;
	FTimerHandle TimerHandle_AutoFire;

public:
	void StartFire();
	void EndFire();

	void Reload();
	void ReloadDone();

	void PlayFireEffects(const FVector& EndPoint);
	void PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint);

	UPROPERTY(ReplicatedUsing=OnRep_HitScanTrace)
	FHitScanTrace HitScanTrace;

	UFUNCTION()
	void OnRep_HitScanTrace();

public:
	// Called when the ammunition count changes
	UPROPERTY(BlueprintAssignable, Category="Weapon|Ammunitions")
	FOnAmmunitionsChangedSignature OnAmmunitionsChangedEvent;
};
