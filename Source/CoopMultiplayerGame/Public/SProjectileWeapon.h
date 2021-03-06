// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SWeapon.h"
#include "SProjectileWeapon.generated.h"

/**
 * 
 */
UCLASS()
class COOPMULTIPLAYERGAME_API ASProjectileWeapon : public ASWeapon
{
	GENERATED_BODY()

public:
	ASProjectileWeapon();
	
protected:

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void StartFire() override;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<AActor> ProjectileClass;	
};
