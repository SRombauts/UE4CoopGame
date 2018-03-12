// Copyright 2018 Sebastien Rombauts (sebastien.rombauts@gmail.com)

#include "SCharacter.h"

#include "SWeapon.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PawnMovementComponent.h"

// Sets default values
ASCharacter::ASCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->bUsePawnControlRotation = true;
	SpringArmComponent->SocketOffset = FVector(0.f, 40.f, 78.f); // At the height of Character's eyes, just a bit to the right to better see
	SpringArmComponent->TargetArmLength = 160.0f; // Default is 300.0f;
	SpringArmComponent->bEnableCameraLag = true;
//	SpringArmComponent->CameraLagSpeed = 3.0f; // Default is 10.0f
	SpringArmComponent->SetupAttachment(RootComponent);

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName); // attaching to the socket does not seems to be needed anymore

	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;

	AimDownSightFOV = 60.f;
	AimDownSightSpeed = 20.f;
}

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	DefaultFOV = CameraComponent->FieldOfView;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	CurrentWeapon = GetWorld()->SpawnActor<ASWeapon>(WeaponClass, SpawnParams);
	if (CurrentWeapon)
	{
		CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocketName);
	}
}

void ASCharacter::MoveForward(float Value)
{
	AddMovementInput(GetActorForwardVector() * Value);
}

void ASCharacter::MoveRight(float Value)
{
	AddMovementInput(GetActorRightVector() * Value);
}

void ASCharacter::BeginCrouch()
{
	Crouch();
}

void ASCharacter::EndCrouch()
{
	UnCrouch();
}

void ASCharacter::BeginAimDownSight()
{
	bAimDownSight = true;
}

void ASCharacter::EndAimDownSight()
{
	bAimDownSight = false;
}

void ASCharacter::Fire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->Fire();
	}
}

// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const float CurrentFOV = CameraComponent->FieldOfView;
	const float TargetFOV = bAimDownSight ? AimDownSightFOV : DefaultFOV;

	const float NewFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, AimDownSightSpeed);

	CameraComponent->SetFieldOfView(NewFOV);
}

// Called to bind functionality to input
void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASCharacter::MoveRight);

	PlayerInputComponent->BindAxis("LookUp", this, &ASCharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &ASCharacter::AddControllerYawInput);

	PlayerInputComponent->BindAction("Crouch", EInputEvent::IE_Pressed, this, &ASCharacter::BeginCrouch);
	PlayerInputComponent->BindAction("Crouch", EInputEvent::IE_Released, this, &ASCharacter::EndCrouch);

	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &ASCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Released, this, &ASCharacter::StopJumping);

	PlayerInputComponent->BindAction("AimDownSight", EInputEvent::IE_Pressed, this, &ASCharacter::BeginAimDownSight);
	PlayerInputComponent->BindAction("AimDownSight", EInputEvent::IE_Released, this, &ASCharacter::EndAimDownSight);

	PlayerInputComponent->BindAction("Fire", EInputEvent::IE_Pressed, this, &ASCharacter::Fire);
}

FVector ASCharacter::GetPawnViewLocation() const
{
	return CameraComponent ? CameraComponent->GetComponentLocation() : Super::GetPawnViewLocation();
}
