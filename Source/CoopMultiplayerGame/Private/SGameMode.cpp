// Fill out your copyright notice in the Description page of Project Settings.

#include "SGameMode.h"
#include "Public/Components/SHealthComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"


ASGameMode::ASGameMode()
	: AGameModeBase()
	, TimeBetweenWaves(2.f)
	, WaveCount(0)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.f;
}

void ASGameMode::StartPlay()
{
	Super::StartPlay();

	PrepareForNextWave();
}

void ASGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	CheckWaveState();
}

void ASGameMode::StartWave()
{
	++WaveCount;

	NrOfBotsToSpawn = 2 * WaveCount;

	GetWorldTimerManager().SetTimer(TimerHandle_BotSpawner, this, &ASGameMode::SpawnTimerElapsed, 0.3f, true, 0.f);
}

void ASGameMode::EndWave()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawner);
}

void ASGameMode::PrepareForNextWave()
{
	GetWorldTimerManager().SetTimer(TimerHandle_NextWaveStart, this, &ASGameMode::StartWave, TimeBetweenWaves, false);
}

void ASGameMode::SpawnTimerElapsed()
{
	SpawnNewBot();

	--NrOfBotsToSpawn;

	if (NrOfBotsToSpawn <= 0)
	{
		EndWave();
	}
}

void ASGameMode::CheckWaveState()
{
	bool bIsPreparingForNextWave = GetWorldTimerManager().IsTimerActive(TimerHandle_NextWaveStart);

	if (NrOfBotsToSpawn > 0 || bIsPreparingForNextWave)
	{
		return;
	}

	bool bIsAnyBotsAlive = false;

	for (FConstPawnIterator Iter = GetWorld()->GetPawnIterator(); Iter; ++Iter)
	{
		APawn* TestPawn = Iter->Get();
		if (TestPawn == nullptr || TestPawn->IsPlayerControlled())
		{
			continue;
		}

		USHealthComponent* HealthComp = Cast<USHealthComponent>(TestPawn->GetComponentByClass(USHealthComponent::StaticClass()));
		if (HealthComp && HealthComp->GetHealth() > 0.f)
		{
			bIsAnyBotsAlive = true;
			break;
		}
	}

	if (!bIsAnyBotsAlive)
	{
		PrepareForNextWave();
	}
}
