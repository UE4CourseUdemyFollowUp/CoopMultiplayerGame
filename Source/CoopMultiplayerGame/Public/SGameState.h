// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SGameState.generated.h"


UENUM()
enum class EWaveState : uint8
{
	WaitingToStart,

	WaveInProgress,

	WaitingToComplete,

	WaveComplete,

	GameOver
};

/**
 * 
 */
UCLASS()
class COOPMULTIPLAYERGAME_API ASGameState : public AGameStateBase
{
	GENERATED_BODY()
	

public:

	void SetWaveState(const EWaveState& NewState);

protected:

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_WaveState, Category = "GameState")
	EWaveState WaveState;

	UFUNCTION(BlueprintImplementableEvent, Category = "GameState")
	void WaveStateChanged(EWaveState NewState, EWaveState OldState);

	UFUNCTION()
	void OnRep_WaveState(EWaveState OldState);
	
};
