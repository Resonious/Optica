// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "Optica.h"
#include "OpticaCharacter.h"
#include "PlatformerCameraArm.h"

const FName AOpticaCharacter::PICKUPABLE_TAG(TEXT("PickupAble"));

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
    CameraBoom->bEnableCameraLag = true;
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
    PickupRange = 50.0f;
    PickedUpActor = nullptr;
    PickupPhysics = nullptr;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

bool AOpticaCharacter::IsGrounded() const {
    return FMath::Abs(GetCharacterMovement()->Velocity.Z) < KINDA_SMALL_NUMBER * 2.0f && !IsJumpProvidingForce();
}

//////////////////////////////////////////////////////////////////////////
// Input

void AOpticaCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	// set up gameplay key bindings
	InputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	InputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
    InputComponent->BindAction("PickUp", IE_Pressed, this, &AOpticaCharacter::PickUp);
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

void AOpticaCharacter::Drop() {
    if (PickupPhysics) {
        PickupPhysics->SetSimulatePhysics(true);
        PickupPhysics = nullptr;
    }

    PickedUpActor = nullptr;
}

void AOpticaCharacter::PickUp() {
    if (PickedUpActor) {
        Drop();
        return;
    }

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    FVector Start(GetActorLocation() + GetActorRotation().Vector() * 50.0f);
    FVector End(Start + GetActorRotation().Vector() * PickupRange);

    TArray<FHitResult> Hits;
    bool bHit = GetWorld()->LineTraceMultiByChannel(Hits, Start, End, ECollisionChannel::ECC_WorldDynamic);

    if (!bHit) {
        // TODO perhaps play a "didn't work" sound
        UE_LOG(LightSource, Warning, TEXT("No hit objects at all"));
        return;
    }

    AActor* Pickup = nullptr;
    for (auto& Hit : Hits) {
        auto HitActor = Hit.Actor.Get();
        if (!HitActor) continue;
        if (!HitActor->ActorHasTag(PICKUPABLE_TAG)) continue;

        Pickup = HitActor;
        break;
    }

    if (!Pickup) {
        // TODO play sound
        UE_LOG(LightSource, Warning, TEXT("No hit objects were pickupable"));
        return;
    }

    UE_LOG(LightSource, Warning, TEXT("PICKED UP"));
    PickedUpActor = Pickup;

    for (auto& ActorComp : Pickup->GetComponentsByClass(UPrimitiveComponent::StaticClass())) {
        auto Comp = Cast<UPrimitiveComponent>(ActorComp);
        if (!Comp) continue;

        if (Comp->IsSimulatingPhysics()) {
            PickupPhysics = Comp;
            Comp->SetSimulatePhysics(false);
            break;
        }
    }
}

void AOpticaCharacter::StopJumping() {
    Super::StopJumping();
    if (GetCharacterMovement()->Velocity.Z > JumpStopVelocity)
        GetCharacterMovement()->Velocity.Z = JumpStopVelocity;
}

void AOpticaCharacter::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);
    if (!PickedUpActor) return;

    // PickedUpActor->SetActorLocation(GetActorLocation() + PickupOffset);
    FVector PickupOrigin = GetActorLocation() + GetActorRotation().RotateVector(PickupOffset);
	FVector DesiredLoc = PickupOrigin;
	const float PickupLagMaxTimeStep = 1.f / 60.f;
	const float InversePickupLagMaxTimeStep = (1.f / PickupLagMaxTimeStep);
	const float PickupLagSpeed = 10.f;

    const FVector PickupMovementStep = (PickupOrigin - PreviousPickupOrigin) * (PickupLagMaxTimeStep / DeltaTime);			
    FVector LerpTarget = PreviousPickupOrigin;
    float RemainingTime = DeltaTime;
    while (RemainingTime > KINDA_SMALL_NUMBER)
    {
        const float LerpAmount = FMath::Min(PickupLagMaxTimeStep, RemainingTime);
        LerpTarget += PickupMovementStep * (LerpAmount * InversePickupLagMaxTimeStep);
        RemainingTime -= LerpAmount;

        DesiredLoc = FMath::VInterpTo(PreviousDesiredLoc, LerpTarget, LerpAmount, PickupLagSpeed);
        PreviousDesiredLoc = DesiredLoc;
    }

    PickedUpActor->SetActorLocation(DesiredLoc);
}

