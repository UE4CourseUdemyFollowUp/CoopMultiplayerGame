// Fill out your copyright notice in the Description page of Project Settings.

#include "CoopMultiplayerGame/Public/SCharacter.h"
#include "CoopMultiplayerGame/Public/SWeapon.h"
#include "CoopMultiplayerGame/Public/Components/SHealthComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Components/CapsuleComponent.h"

#include "CoopMultiplayerGame.h"

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

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));

	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;
	//GetMovementComponent()->GetNavAgentPropertiesRef().bCanJump = true;
	GetMovementComponent()->GetNavAgentPropertiesRef().bCanWalk = true;

	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECollisionResponse::ECR_Ignore);

	ZoomFOV = 60.f;
	ZoomInterpSpeed = 20.f;

	WeaponAttachSocket = "WeaponSocket";

	bDied = false;
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
	
	if (CameraComp)
	{
		DefaultFOV = CameraComp->FieldOfView;
		UE_LOG(LogTemp, Warning, TEXT("DefaultFOV set"));
	}

	DefaultStartWeapon;

	FActorSpawnParameters WeaponSpawnParams;
	WeaponSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	CurrentWeapon = GetWorld()->SpawnActor<ASWeapon>(DefaultStartWeapon, FVector::ZeroVector, FRotator::ZeroRotator, WeaponSpawnParams);
	if (CurrentWeapon)
	{
		CurrentWeapon->SetOwner(this);
		CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocket);
	}

	HealthComp->OnHealthChanged.AddDynamic(this, &ASCharacter::OnHealthChanged);
}

void ASCharacter::StartFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StartFire();
	}
}

void ASCharacter::StopFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StopFire();
	}
}

// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float CurrentZoom = bWantsToZoom ? ZoomFOV : DefaultFOV;

	float NewZoom = FMath::FInterpTo(CameraComp->FieldOfView, CurrentZoom, DeltaTime, ZoomInterpSpeed);

	CameraComp->SetFieldOfView(NewZoom);
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

void ASCharacter::StartZoom()
{
	bWantsToZoom = true;
	UE_LOG(LogTemp, Warning, TEXT("StartZoom"));
}

void ASCharacter::StopZoom()
{
	bWantsToZoom = false;
	UE_LOG(LogTemp, Warning, TEXT("StopZoom"));
}

void ASCharacter::OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (Health <= 0.f && !bDied)
	{
		// Death

		bDied = true;

		GetMovementComponent()->StopMovementImmediately();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		DetachFromControllerPendingDestroy();
		SetLifeSpan(10.f);
	}
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

	PlayerInputComponent->BindAction("Zoom", IE_Pressed, this, &ASCharacter::StartZoom);
	PlayerInputComponent->BindAction("Zoom", IE_Released, this, &ASCharacter::StopZoom);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ASCharacter::StopFire);
}

