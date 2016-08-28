// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "Optica.h"
#include "OpticaCharacter.h"
#include "PlatformerCameraArm.h"

AOpticaCharacter::AOpticaCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Create a camera boom attached to the root (capsule)
	CameraBoom = CreateDefaultSubobject<UPlatformerCameraArm>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->bAbsoluteRotation = true; // Rotation of the character should not affect rotation of boom
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->TargetArmLength = 432.f;
	CameraBoom->SocketOffset = FVector(0.f,0.f,160.f);
	CameraBoom->RelativeRotation = FRotator(0.f,0.f,0.f);
    CameraBoom->CharacterToWatch = this;

	// Create a camera and attach to boom
	SideViewCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("SideViewCamera"));
	SideViewCameraComponent->SetupAttachment(CameraBoom, UPlatformerCameraArm::SocketName);
	SideViewCameraComponent->bUsePawnControlRotation = false; // We don't want the controller rotating the camera
    SideViewCameraComponent->SetProjectionMode(ECameraProjectionMode::Orthographic);
    SideViewCameraComponent->SetOrthoWidth(1950.0f);

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Face in the direction we are moving..
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 5000.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->GravityScale = 2.f;
	GetCharacterMovement()->AirControl = 0.80f;
	GetCharacterMovement()->JumpZVelocity = 1000.f;
	GetCharacterMovement()->GroundFriction = 3.f;
	GetCharacterMovement()->MaxWalkSpeed = 600.f;
	GetCharacterMovement()->MaxFlySpeed = 600.f;
    JumpStopVelocity = 500.0f;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

bool AOpticaCharacter::IsGrounded() const {
    return FMath::Abs(GetCharacterMovement()->Velocity.Z) < KINDA_SMALL_NUMBER;
}

//////////////////////////////////////////////////////////////////////////
// Input

void AOpticaCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	// set up gameplay key bindings
	InputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	InputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	InputComponent->BindAxis("MoveRight", this, &AOpticaCharacter::MoveRight);

	InputComponent->BindTouch(IE_Pressed, this, &AOpticaCharacter::TouchStarted);
	InputComponent->BindTouch(IE_Released, this, &AOpticaCharacter::TouchStopped);
}

void AOpticaCharacter::MoveRight(float Value)
{
	// add movement in that direction
	AddMovementInput(FVector(0.f,1.f,0.f), Value);
}

void AOpticaCharacter::TouchStarted(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	// jump on any touch
	Jump();
}

void AOpticaCharacter::TouchStopped(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	StopJumping();
}

void AOpticaCharacter::StopJumping() {
    Super::StopJumping();
    if (GetCharacterMovement()->Velocity.Z > JumpStopVelocity)
        GetCharacterMovement()->Velocity.Z = JumpStopVelocity;
}

