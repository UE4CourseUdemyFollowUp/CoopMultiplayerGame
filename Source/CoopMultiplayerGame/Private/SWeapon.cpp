// Fill out your copyright notice in the Description page of Project Settings.

#include "CoopMultiplayerGame/Public/SWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "CollisionQueryParams.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"

static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef DebugWeaponDrawConsoleVar( TEXT("COOP.DebugWeaponsDrawing"), DebugWeaponDrawing, TEXT("Draw Weapon Debug Shapes"), ECVF_Cheat);

// Sets default values
ASWeapon::ASWeapon()
	: MuzzleSocketName("MuzzleSocket")
{
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
}

void ASWeapon::Fire()
{
	AActor* Owner = GetOwner();

	if (Owner)
	{
		FVector EyeLocation;
		FRotator EyeRotation;
		Owner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		FVector ShotDirection = EyeRotation.Vector();

		FVector TraceEnd = EyeLocation + ShotDirection * 10000;

		// A vector to represent an actual end of trace, is a Trace by default and change on hit
		FVector TraceEndPoint = TraceEnd;
		
		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(this);
		CollisionParams.AddIgnoredActor(Owner);
		CollisionParams.bTraceComplex = true;

		FHitResult HitResult;
		if (GetWorld()->LineTraceSingleByChannel(HitResult, EyeLocation, TraceEnd, ECC_Visibility, CollisionParams))
		{
			AActor* HitActor = HitResult.GetActor();

			UGameplayStatics::ApplyPointDamage(HitActor, 20.f, ShotDirection, HitResult, Owner->GetInstigatorController(), this, DamageType);

			TraceEndPoint = HitResult.ImpactPoint;

			if (ImpactEffect)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, HitResult.ImpactPoint, HitResult.ImpactNormal.Rotation());
			}
		}

		PlayFireEffects(TraceEnd);

		if (DebugWeaponDrawing > 0)
		{
			DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::Red, false, 1.f, 0, 1.f);
		}

	}

}

void ASWeapon::PlayFireEffects(const FVector& TraceEnd)
{
	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
	}

	if (TraceEffect)
	{
		FVector MuzzleSocketLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TraceEffect, MuzzleSocketLocation);

		if (TracerComp)
		{
			TracerComp->SetVectorParameter("BeamEnd", TraceEnd);
		}
	}
}
