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
#include "Net/UnrealNetwork.h"

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

	SetReplicates(true);

	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
}

void ASWeapon::BeginPlay()
{
	Super::BeginPlay();

	TimeBetweenShots = 60.f / RateOfFire;
}

void ASWeapon::Fire()
{
	if (Role < ROLE_Authority)
	{
		ServerFire();
		return;
	}

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
		EPhysicalSurface PhysicalSurface = EPhysicalSurface::SurfaceType_Default;
		
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

			PhysicalSurface = UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());

			float ActualDamage = BaseDamage;
			if (PhysicalSurface == SURFACE_FLASHVULNERABLE)
			{
				ActualDamage *= 4.f;
			}

			UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, HitResult, Owner->GetInstigatorController(), this, DamageType);

			PlayImpactEffects(PhysicalSurface, TraceEndPoint);
		}

		PlayFireEffects(HitScanTrace.TraceTo);

		if (Role == ROLE_Authority)
		{
			HitScanTrace.TraceTo = TraceEndPoint;
			HitScanTrace.SurfaceType = PhysicalSurface;
		}

		if (DebugWeaponDrawing > 0)
		{
			DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::Red, false, 1.f, 0, 1.f);
		}

	}

}

void ASWeapon::ServerFire_Implementation()
{
	Fire();
}

bool ASWeapon::ServerFire_Validate()
{
	return true;
}

void ASWeapon::OnRep_HitTraceScan()
{
	PlayFireEffects(HitScanTrace.TraceTo);
	PlayImpactEffects(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);
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

void ASWeapon::PlayFireEffects(const FVector& TraceEnd) const
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

void ASWeapon::PlayImpactEffects(const EPhysicalSurface & SurfaceType, const FVector& ImpactPoint) const
{
	UParticleSystem* SelectedEffect = nullptr;

	switch (SurfaceType)
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
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

		FVector ShotDirection = ImpactPoint - MuzzleLocation;
		ShotDirection.Normalize();

		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, ImpactPoint, ShotDirection.Rotation());
	}
}

void ASWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASWeapon, HitScanTrace, COND_SkipOwner);
}
