// Copyright 2018 Sebastien Rombauts (sebastien.rombauts@gmail.com)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "STrackerBot.generated.h"

UCLASS()
class COOPGAME_API ASTrackerBot : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASTrackerBot();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnHealthChangedEvent(class USHealthComponent* HealthComp, float Health, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	// Play cosmetic effects on clients
	UFUNCTION()
	void OnRep_Exploded();

	FVector GetNextPathPoint();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USHealthComponent* HealthComponent;

	// Next point in navigation point
	FVector NextPathPoint;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
	float Force = 1000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
	float PathPointRadius = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Exploded, VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	bool bExploded = false;
};
