// Fill out your copyright notice in the Description page of Project Settings.

#include "CoopMultiplayerGame/Public/SProjectileWeapon.h"
#include "Components/SkeletalMeshComponent.h"

ASProjectileWeapon::ASProjectileWeapon()
{
}

void ASProjectileWeapon::StartFire()
{
	AActor* Owner = GetOwner();

	if (Owner && ProjectileClass)
	{
		FVector EyeLocation;
		FRotator EyeRotation;
		Owner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// EyeRotation to fire the direction we looking, not the gun
		GetWorld()->SpawnActor<AActor>(ProjectileClass, MuzzleLocation, EyeRotation, ActorSpawnParams);
	}
}


