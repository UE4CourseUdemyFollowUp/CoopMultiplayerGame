// Fill out your copyright notice in the Description page of Project Settings.

#include "CoopMultiplayerGame/Public/SWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "CollisionQueryParams.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

#include "CoopMultiplayerGame/CoopMultiplayerGame.h"

static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef DebugWeaponDrawConsoleVar( TEXT("COOP.DebugWeaponsDrawing"), DebugWeaponDrawing, TEXT("Draw Weapon Debug Shapes"), ECVF_Cheat);

// Sets default values
ASWeapon::ASWeapon()
	: MuzzleSocketName("MuzzleSocket")
	, BaseDamage(20.f)
	, RateOfFire(600.f)
{
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
}

void ASWeapon::BeginPlay()
{
	Super::BeginPlay();

	TimeBetweenShots = 60.f / RateOfFire;
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
		CollisionParams.bReturnPhysicalMaterial = true;

		FHitResult HitResult;
		if (GetWorld()->LineTraceSingleByChannel(HitResult, EyeLocation, TraceEnd, COLLISION_WEAPON, CollisionParams))
		{
			AActor* HitActor = HitResult.GetActor();

			TraceEndPoint = HitResult.ImpactPoint;

			EPhysicalSurface PhysicalSurface = UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());

			float ActualDamage = BaseDamage;
			if (PhysicalSurface == SURFACE_FLASHVULNERABLE)
			{
				ActualDamage *= 4.f;
			}

			UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, HitResult, Owner->GetInstigatorController(), this, DamageType);

			UParticleSystem* SelectedEffect = nullptr;

			switch (PhysicalSurface)
			{
			case SURFACE_FLESHDEFAULT:
			case SURFACE_FLASHVULNERABLE:
				SelectedEffect = FleshImpactEffect;
				break;
			default:
				SelectedEffect = DefaultImpactEffect;
				break;
			}

			if (SelectedEffect)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, HitResult.ImpactPoint, HitResult.ImpactNormal.Rotation());
			}
		}

		PlayFireEffects(TraceEnd);

		if (DebugWeaponDrawing > 0)
		{
			DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::Red, false, 1.f, 0, 1.f);
		}

	}

}

void ASWeapon::StartFire()
{
	float FirstDelay = FMath::Max(LastTimeFired + TimeBetweenShots - GetWorld()->TimeSeconds, 0.f);

	GetWorldTimerManager().SetTimer(HandleTimer_TimeBetweenShots, this, &ASWeapon::Fire, TimeBetweenShots, true, FirstDelay);
}

void ASWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(HandleTimer_TimeBetweenShots);
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

	APawn* Owner = Cast<APawn>(GetOwner());
	if(Owner)
	{
		APlayerController* PlayerController = Cast<APlayerController>(Owner->GetController());
		if (PlayerController)
		{
			PlayerController->ClientPlayCameraShake(CameraShake);
		}
	}
}
