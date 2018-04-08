// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "STrackerBot.generated.h"

class USHealthComponent;
class USphereComponent;
class USoundCue;

UCLASS()
class COOPMULTIPLAYERGAME_API ASTrackerBot : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASTrackerBot();

	void OnCheckNearbyBots();

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	USHealthComponent* HealthComp;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	USphereComponent* SphereComp;

	FVector GetNextPathPoint();

	FVector NextPathPoint;

	UFUNCTION()
	void HandleTakeDamage(USHealthComponent* OwningHealthComp, float Health, float HealthDelta,
		const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float MovementForce;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	bool bUseVelocityChange;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float RequiredDistanceToTarget;

	UMaterialInstanceDynamic* MatInstance;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	UParticleSystem* ExplosionEffect;

	void SelfDestruct();

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float ExplosionDamage;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float ExplosionRadius;

	bool bIsExploded;

	bool bStartedSelfDestruction;

	FTimerHandle TimerHandle_DamageSelf;

	void DamageSelf();

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float SelfDestructionInterval;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	USoundCue* SelfDestructionSound;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	USoundCue* ExplosionSound;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float NearbyBotsCheckInterval;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	int BotsBundleRadius;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	int MaxPowerLevel;

	int PowerLevel;

};
