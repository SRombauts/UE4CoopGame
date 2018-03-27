// Copyright 2018 Sebastien Rombauts (sebastien.rombauts@gmail.com)

#include "SCharacter.h"

#include "SWeapon.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SHealthComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ASCharacter::ASCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->bUsePawnControlRotation = true;
//	SpringArmComponent->SocketOffset = FVector(0.f, 40.f, 98.f); // This would offset the socket, that is the camera, not the attachment point on the character (which is not nice when the camera get closer)
	SpringArmComponent->AddLocalOffset(FVector(0.f, 40.f, 98.f)); // This offset the attachment point into the character, so that we see the head when the camera get closer
	SpringArmComponent->TargetArmLength = 160.0f; // Default is 300.0f;
	SpringArmComponent->bEnableCameraLag = true;
//	SpringArmComponent->CameraLagSpeed = 3.0f; // Default is 10.0f
	SpringArmComponent->SetupAttachment(RootComponent);

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName); // attaching to the socket does not seems to be needed anymore
	CameraComponent->RelativeRotation = FRotator(-10.f, 0.f, 0.f);

	HealthComponent = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComponent"));

	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;

	AimDownSightFOV = 60.f;
	AimDownSightSpeed = 20.f;
}

// Called when an instance of this class is placed (in editor) or spawned, after all Blueprint Construction Scripts.
void ASCharacter::OnConstruction(const FTransform & Transform)
{
	Super::OnConstruction(Transform);

	DefaultFOV = CameraComponent->FieldOfView;

	if (Role == ROLE_Authority)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;
		CurrentWeapon = GetWorld()->SpawnActor<ASWeapon>(WeaponClass, SpawnParams);
		if (CurrentWeapon)
		{
			CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocketName);
		}
	}
}

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();

	HealthComponent->OnHealthChangedEvent.AddDynamic(this, &ASCharacter::OnHealthChangedEvent);
}

void ASCharacter::OnHealthChangedEvent(USHealthComponent* HealthComp, float Health, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	// The following only runs on the Server, since damage & health event binding are only if (Role == ROLE_Authority)
	if (Health <= 0.f)
	{
		// Die!
		bDied = true;
		UE_LOG(LogTemp, Log, TEXT("Die!"));

		GetMovementComponent()->StopMovementImmediately();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		DetachFromControllerPendingDestroy();

		SetLifeSpan(10.f);
	}
}

// Called to bind functionality to input
void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASCharacter::MoveRight);

	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);

	PlayerInputComponent->BindAction("Crouch", EInputEvent::IE_Pressed, this, &ASCharacter::BeginCrouch);
	PlayerInputComponent->BindAction("Crouch", EInputEvent::IE_Released, this, &ASCharacter::EndCrouch);

	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("AimDownSight", EInputEvent::IE_Pressed, this, &ASCharacter::BeginAimDownSight);
	PlayerInputComponent->BindAction("AimDownSight", EInputEvent::IE_Released, this, &ASCharacter::EndAimDownSight);

	PlayerInputComponent->BindAction("Fire", EInputEvent::IE_Pressed, this, &ASCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", EInputEvent::IE_Released, this, &ASCharacter::EndFire);

	PlayerInputComponent->BindAction("Reload", EInputEvent::IE_Pressed, this, &ASCharacter::Reload);
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

void ASCharacter::StartFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StartFire();
	}
}

void ASCharacter::EndFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->EndFire();
	}
}

void ASCharacter::Reload()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->Reload();
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

FVector ASCharacter::GetPawnViewLocation() const
{
	return CameraComponent ? CameraComponent->GetComponentLocation() : Super::GetPawnViewLocation();
}

void ASCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASCharacter, CurrentWeapon);
	DOREPLIFETIME(ASCharacter, bDied);
}
