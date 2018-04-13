// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SPickupActor.generated.h"

class USphereComponent;
class ASPowerupActor;

UCLASS()
class COOPMULTIPLAYERGAME_API ASPickupActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASPickupActor();

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	USphereComponent* SphereComp;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	UDecalComponent* DecalComp;

	UPROPERTY(EditInstanceOnly, Category = "Components")
	TSubclassOf<ASPowerupActor> PowerupActorClass;

	ASPowerupActor* PowerupActorInstance;

	UPROPERTY(EditDefaultsOnly, Category = "Components")
	float CooldownDuration;

	FTimerHandle TimerHandle_RespawnTimer;

	void Respawn();
	
};
