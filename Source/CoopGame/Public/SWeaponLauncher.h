// Copyright 2018 Sebastien Rombauts (sebastien.rombauts@gmail.com)

#pragma once

#include "CoreMinimal.h"
#include "SWeapon.h"
#include "SWeaponLauncher.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAME_API ASWeaponLauncher : public ASWeapon
{
	GENERATED_BODY()
	
protected:
	/** Grenade BP class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = "Grenade")
	TSubclassOf<class ASGrenade> GrenadeClass;

	// Interface ASWeapon
//	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void Fire();
};
