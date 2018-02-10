// Fill out your copyright notice in the Description page of Project Settings.

#include "CoopMultiplayerGame/Public/SCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PawnMovementComponent.h"

// Sets default values
ASCharacter::ASCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//GetMovementComponent()->GetNavAgent

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->SetupAttachment(RootComponent);

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);	

	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;
	//GetMovementComponent()->GetNavAgentPropertiesRef().bCanJump = true;
	GetMovementComponent()->GetNavAgentPropertiesRef().bCanWalk = true;
}

FVector ASCharacter::GetPawnViewLocation() const
{
	FVector ViewLocation;

	if (CameraComp)
	{
		ViewLocation = CameraComp->GetComponentLocation();
	}
	else
	{
		ViewLocation = Super::GetPawnViewLocation();
	}

	return ViewLocation;
}

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASCharacter::MoveForward(float Value)
{
	AddMovementInput(GetActorForwardVector() * Value);

}

void ASCharacter::MoveRight(float Value)
{
	AddMovementInput(GetActorRightVector() * Value);
}

void ASCharacter::StartCrouch()
{
	Crouch();

	UE_LOG(LogTemp, Warning, TEXT("StartCrouch"));
}

void ASCharacter::StopCrouch()
{
	UnCrouch();

	UE_LOG(LogTemp, Warning, TEXT("StopCrouch"));
}

void ASCharacter::StartJump()
{
	Jump();
	UE_LOG(LogTemp, Warning, TEXT("StartJump"));
}

// Called to bind functionality to input
void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASCharacter::MoveRight);

	PlayerInputComponent->BindAxis("LookUp", this, &ASCharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &ASCharacter::AddControllerYawInput);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASCharacter::StartCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASCharacter::StopCrouch);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ASCharacter::StartJump);
}

