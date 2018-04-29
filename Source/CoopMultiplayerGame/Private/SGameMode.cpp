// Fill out your copyright notice in the Description page of Project Settings.

#include "SGameMode.h"
#include "Public/Components/SHealthComponent.h"
#include "Public/SGameState.h"
#include "TimerManager.h"
#include "Engine/World.h"


ASGameMode::ASGameMode()
	: AGameModeBase()
	, TimeBetweenWaves(2.f)
	, WaveCount(0)
{
	GameStateClass = ASGameState::StaticClass();

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
	CheckAnyPlayerAlive();
}

void ASGameMode::StartWave()
{
	++WaveCount;

	NrOfBotsToSpawn = 2 * WaveCount;

	GetWorldTimerManager().SetTimer(TimerHandle_BotSpawner, this, &ASGameMode::SpawnTimerElapsed, 0.3f, true, 0.f);

	SetWaveState(EWaveState::WaveInProgress);
}

void ASGameMode::EndWave()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawner);

	SetWaveState(EWaveState::WaitingToComplete);
}

void ASGameMode::PrepareForNextWave()
{
	GetWorldTimerManager().SetTimer(TimerHandle_NextWaveStart, this, &ASGameMode::StartWave, TimeBetweenWaves, false);

	SetWaveState(EWaveState::WaitingToStart);
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
		SetWaveState(EWaveState::WaveComplete);

		PrepareForNextWave();
	}
}

void ASGameMode::CheckAnyPlayerAlive()
{
	for (FConstPlayerControllerIterator Iter = GetWorld()->GetPlayerControllerIterator(); Iter; ++Iter)
	{
		APlayerController* PC = Iter->Get();

		if (PC && PC->GetPawn() != nullptr)
		{
			USHealthComponent* HealthComp = Cast<USHealthComponent>(PC->GetPawn()->GetComponentByClass(USHealthComponent::StaticClass()));
			if (HealthComp && HealthComp->GetHealth() > 0.f)
			{
				// There is atleast one alive Player Pawn
				return;
			}
		}
	}

	GameOver();
}

void ASGameMode::GameOver()
{
	EndWave();

	SetWaveState(EWaveState::GameOver);

	UE_LOG(LogTemp, Error, TEXT("GAME OVER! Players died!"));
}

void ASGameMode::SetWaveState(EWaveState NewWaveState)
{
	ASGameState* GS = GetGameState<ASGameState>();
	if (ensureAlways(GS))
	{
		GS->SetWaveState(NewWaveState);
	}
}
