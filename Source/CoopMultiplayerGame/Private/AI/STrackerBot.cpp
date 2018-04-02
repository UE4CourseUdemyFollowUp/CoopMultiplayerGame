// Fill out your copyright notice in the Description page of Project Settings.

#include "Public/AI/STrackerBot.h"
#include "Public/SCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Public/Components/SHealthComponent.h"
#include "Runtime/Engine/Classes/AI/Navigation/NavigationSystem.h"
#include "Runtime/Engine/Classes/AI/Navigation/NavigationPath.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "DrawDebugHelpers.h"

// Sets default values
ASTrackerBot::ASTrackerBot()
	: MovementForce(1000.f)
	, bUseVelocityChange(false)
	, RequiredDistanceToTarget(100.f)
	, MatInstance(nullptr)
	, ExplosionDamage(100.f)
	, ExplosionRadius(200.f)
	, bIsExploded(false)
	, bStartedSelfDestruction(false)
	, SelfDestructionInterval(0.5f)
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCanEverAffectNavigation(false);
	MeshComp->SetSimulatePhysics(true);
	RootComponent = MeshComp;

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealtComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &ASTrackerBot::HandleTakeDamage);

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetSphereRadius(ExplosionRadius);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComp->SetupAttachment(RootComponent);
}

void ASTrackerBot::NotifyActorBeginOverlap(AActor * OtherActor)
{
	if (!bStartedSelfDestruction && !bIsExploded)
	{
		ASCharacter* Character = Cast<ASCharacter>(OtherActor);

		if (Character)
		{
			GetWorldTimerManager().SetTimer(TimerHandle_DamageSelf, this, &ASTrackerBot::DamageSelf, SelfDestructionInterval, true, 0.f);

			UGameplayStatics::SpawnSoundAttached(SelfDestructionSound, RootComponent);

			bStartedSelfDestruction = true;
		}

		
	}

}

// Called when the game starts or when spawned
void ASTrackerBot::BeginPlay()
{
	Super::BeginPlay();

	if (Role == ROLE_Authority)
	{
		NextPathPoint = GetNextPathPoint();
	}
}

FVector ASTrackerBot::GetNextPathPoint()
{
	FVector RetVal;

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);

	UNavigationPath* NavPath = UNavigationSystem::FindPathToActorSynchronously(this, GetActorLocation(), PlayerPawn);

	if (NavPath && NavPath->PathPoints.Num() > 1)
	{
		RetVal = NavPath->PathPoints[1];

	}

	return RetVal;
}

void ASTrackerBot::HandleTakeDamage(USHealthComponent* OwningHealthComp, float Health, float HealthDelta,
	const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{

	if (MatInstance == nullptr)
	{
		MatInstance = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}

	if (MatInstance)
	{
		MatInstance->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->TimeSeconds);
	}

	if (Health <= 0.f)
	{
		SelfDestruct();
	}


	UE_LOG(LogTemp, Warning, TEXT("Health %s of %s"), *FString::SanitizeFloat(Health), *GetName());
}

void ASTrackerBot::SelfDestruct()
{
	if (false == bIsExploded)
	{
		bIsExploded = true;

		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
		UGameplayStatics::PlaySoundAtLocation(this, ExplosionSound, GetActorLocation());

		MeshComp->SetVisibility(false, true);
		MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		if (Role == ROLE_Authority)
		{
			TArray<AActor*> IgnoredActors;
			IgnoredActors.Add(this);
			IgnoredActors.Shrink();

			UGameplayStatics::ApplyRadialDamage(this, ExplosionDamage, GetActorLocation(), ExplosionRadius, nullptr, IgnoredActors, this, GetInstigatorController(), true);

			DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Red, false, 2.f, 0, 2.f);

			SetLifeSpan(2.f);
		}
	}

}

void ASTrackerBot::DamageSelf()
{
	UGameplayStatics::ApplyDamage(this, 20.f, GetInstigatorController(), this, nullptr);
}

// Called every frame
void ASTrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Role == ROLE_Authority && !bIsExploded)
	{
		float DistanceToTargetLocation = (GetActorLocation() - NextPathPoint).Size();

		if (DistanceToTargetLocation <= RequiredDistanceToTarget)
		{
			NextPathPoint = GetNextPathPoint();

			DrawDebugString(GetWorld(), GetActorLocation(), "Target Reached");
		}
		else
		{
			FVector ForceDirection = NextPathPoint - GetActorLocation();
			ForceDirection.Normalize();

			ForceDirection *= MovementForce;

			MeshComp->AddForce(ForceDirection, NAME_None, bUseVelocityChange);

			DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + NextPathPoint, 32.f, FColor::Yellow, false, 0.f, 0, 1.f);
		}

		DrawDebugSphere(GetWorld(), NextPathPoint, 20.f, 12, FColor::Yellow, false, 10.f, 0, 1.f);
	}
}