// Copyright 2018 Sebastien Rombauts (sebastien.rombauts@gmail.com)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SCharacter.generated.h"

/**
 * Third Person player Character with an appropriate Skeletal Mesh, Animations, Intpu Bindings...
 */
UCLASS()
class COOPGAME_API ASCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float Value);
	void MoveRight(float Value);

	void BeginCrouch();
	void EndCrouch();
	void BeginAimDownSight();
	void EndAimDownSight();

	void StartFire();
	void EndFire();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UCameraComponent* CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USpringArmComponent* SpringArmComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Player", Meta = (ClampMin = 10, ClampMax = 90))
	float AimDownSightFOV;

	UPROPERTY(EditDefaultsOnly, Category = "Player", Meta = (ClampMin = 1.f, ClampMax = 100.f))
	float AimDownSightSpeed;

	float DefaultFOV;

	bool bAimDownSight = false;

	class ASWeapon* CurrentWeapon;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<ASWeapon> WeaponClass;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	FName WeaponSocketName;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** @return	Pawn's eye location */
	virtual FVector GetPawnViewLocation() const override;
};
