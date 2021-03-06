// Fill out your copyright notice in the Description page of Project Settings.

#include "Optica.h"
#include "PlatformerCameraArm.h"
#include "GameFramework/Character.h"
#include "OpticaCharacter.h"

//////////////////////////////////////////////////////////////////////////
// UPlatformerCameraArm

static void SetDeprecatedControllerViewRotation(UPlatformerCameraArm& Component, bool bValue);
const FName UPlatformerCameraArm::SocketName(TEXT("PlatformerCameraEndpoint"));

UPlatformerCameraArm::UPlatformerCameraArm() :
    CharacterToWatch(nullptr)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
	
	bAutoActivate = true;
	bTickInEditor = true;
	bUsePawnControlRotation = false;
	bDoCollisionTest = true;

	bInheritPitch = true;
	bInheritYaw = true;
	bInheritRoll = true;

	TargetArmLength = 300.0f;
	ProbeSize = 12.0f;
	ProbeChannel = ECC_Camera;

	RelativeSocketRotation = FQuat::Identity;

	bUseCameraLagSubstepping = true;
	CameraLagSpeed = 10.f;
	CameraRotationLagSpeed = 10.f;
	CameraLagMaxTimeStep = 1.f / 60.f;
	CameraLagMaxDistance = 0.f;
	
	// Init deprecated var, for old code that may refer to it.
	SetDeprecatedControllerViewRotation(*this, bUsePawnControlRotation);
}

void UPlatformerCameraArm::UpdateDesiredArmLocation(bool bDoTrace, bool bDoLocationLag, bool bDoRotationLag, float DeltaTime)
{
	FRotator DesiredRot = GetComponentRotation();

	// If inheriting rotation, check options for which components to inherit
	if(!bAbsoluteRotation)
	{
		if(!bInheritPitch)
		{
			DesiredRot.Pitch = RelativeRotation.Pitch;
		}

		if (!bInheritYaw)
		{
			DesiredRot.Yaw = RelativeRotation.Yaw;
		}

		if (!bInheritRoll)
		{
			DesiredRot.Roll = RelativeRotation.Roll;
		}
	}

	const float InverseCameraLagMaxTimeStep = (1.f / CameraLagMaxTimeStep);

	// Apply 'lag' to rotation if desired
	if(bDoRotationLag)
	{
		if (bUseCameraLagSubstepping && DeltaTime > CameraLagMaxTimeStep && CameraRotationLagSpeed > 0.f)
		{
			const FRotator ArmRotStep = (DesiredRot - PreviousDesiredRot).GetNormalized() * (CameraLagMaxTimeStep / DeltaTime);
			FRotator LerpTarget = PreviousDesiredRot;
			float RemainingTime = DeltaTime;
			while (RemainingTime > KINDA_SMALL_NUMBER)
			{
				const float LerpAmount = FMath::Min(CameraLagMaxTimeStep, RemainingTime);
				LerpTarget += ArmRotStep * (LerpAmount * InverseCameraLagMaxTimeStep);
				RemainingTime -= LerpAmount;

				DesiredRot = FMath::RInterpTo(PreviousDesiredRot, LerpTarget, LerpAmount, CameraRotationLagSpeed);
				PreviousDesiredRot = DesiredRot;
			}
		}
		else
		{
			DesiredRot = FMath::RInterpTo(PreviousDesiredRot, DesiredRot, DeltaTime, CameraRotationLagSpeed);
		}
	}
	PreviousDesiredRot = DesiredRot;

	// Get the spring arm 'origin', the target we want to look at
	FVector ArmOrigin = GetComponentLocation() + TargetOffset;
	// We lag the target, not the actual camera position, so rotating the camera around does not have lag
	FVector DesiredLoc = ArmOrigin;

    float CharacterDistanceDown = PreviousDesiredLoc.Z - ArmOrigin.Z;
    bool bDontTrackZAxis = CharacterToWatch && !CharacterToWatch->IsGrounded() && CharacterDistanceDown < 0.0f;

    if (bDontTrackZAxis)
        DesiredLoc.Z = PreviousDesiredLoc.Z;

	if (bDoLocationLag)
	{
		if (bUseCameraLagSubstepping && DeltaTime > CameraLagMaxTimeStep && CameraLagSpeed > 0.f)
		{
			const FVector ArmMovementStep = (ArmOrigin - PreviousArmOrigin) * (CameraLagMaxTimeStep / DeltaTime);			
			FVector LerpTarget = PreviousArmOrigin;
			float RemainingTime = DeltaTime;
			while (RemainingTime > KINDA_SMALL_NUMBER)
			{
				const float LerpAmount = FMath::Min(CameraLagMaxTimeStep, RemainingTime);
				LerpTarget += ArmMovementStep * (LerpAmount * InverseCameraLagMaxTimeStep);
				RemainingTime -= LerpAmount;

				DesiredLoc = FMath::VInterpTo(PreviousDesiredLoc, LerpTarget, LerpAmount, CameraLagSpeed);
				PreviousDesiredLoc = DesiredLoc;
			}
		}
		else
		{
			DesiredLoc = FMath::VInterpTo(PreviousDesiredLoc, DesiredLoc, DeltaTime, CameraLagSpeed);
		}

		// Clamp distance if requested
		bool bClampedDist = false;
		if (CameraLagMaxDistance > 0.f)
		{
			const FVector FromOrigin = DesiredLoc - ArmOrigin;
			if (FromOrigin.SizeSquared() > FMath::Square(CameraLagMaxDistance))
			{
				DesiredLoc = ArmOrigin + FromOrigin.GetClampedToMaxSize(CameraLagMaxDistance);
				bClampedDist = true;
			}
		}		
	}

	PreviousArmOrigin = ArmOrigin;
	PreviousDesiredLoc = DesiredLoc;

	// Now offset camera position back along our rotation
	DesiredLoc -= DesiredRot.Vector() * TargetArmLength;
	// Add socket offset in local space
	DesiredLoc += FRotationMatrix(DesiredRot).TransformVector(SocketOffset);

	// Do a sweep to ensure we are not penetrating the world
	FVector ResultLoc(DesiredLoc);

	// Form a transform for new world transform for camera
	FTransform WorldCamTM(DesiredRot, ResultLoc);
	// Convert to relative to component
	FTransform RelCamTM = WorldCamTM.GetRelativeTransform(ComponentToWorld);

	// Update socket location/rotation
	RelativeSocketLocation = RelCamTM.GetLocation();
	RelativeSocketRotation = RelCamTM.GetRotation();

	UpdateChildTransforms();
}

FVector UPlatformerCameraArm::BlendLocations(const FVector& DesiredArmLocation, const FVector& TraceHitLocation, bool bHitSomething, float DeltaTime)
{
	return bHitSomething ? TraceHitLocation : DesiredArmLocation;
}

void UPlatformerCameraArm::OnRegister()
{
	Super::OnRegister();

	// enforce reasonable limits to avoid potential div-by-zero
	CameraLagMaxTimeStep = FMath::Max(CameraLagMaxTimeStep, 1.f / 200.f);
	CameraLagSpeed = FMath::Max(CameraLagSpeed, 0.f);

	// Set initial location (without lag).
	UpdateDesiredArmLocation(false, false, false, 0.f);

	// Init deprecated var, for old code that may refer to it.
	SetDeprecatedControllerViewRotation(*this, bUsePawnControlRotation);

	// 4.11.2 Hack (UE-24725): Set up tick dependency on simulated objects in the parent hierarchy, so we use post-physics transforms.
	// Probably only relevant for simulated SkeletalMeshComponents.
	if (!IsTemplate())
	{
		USceneComponent* Parent = GetAttachParent();
		while (Parent != nullptr)
		{
			if (UPrimitiveComponent* PrimitiveParent = Cast<UPrimitiveComponent>(Parent))
			{
				if (PrimitiveParent->IsSimulatingPhysics())
				{
					PrimitiveParent->PostPhysicsComponentTick.bCanEverTick = true;
					PrimitiveParent->PostPhysicsComponentTick.SetTickFunctionEnable(true);
					PrimaryComponentTick.AddPrerequisite(PrimitiveParent, PrimitiveParent->PostPhysicsComponentTick);
					break;
				}
			}
			Parent = Parent->GetAttachParent();
		}
	}
}

void UPlatformerCameraArm::PostLoad()
{
	Super::PostLoad();

	// Init deprecated var, for old code that may refer to it.
	SetDeprecatedControllerViewRotation(*this, bUsePawnControlRotation);
}

void UPlatformerCameraArm::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bUsePawnControlRotation)
	{
		if (APawn* OwningPawn = Cast<APawn>(GetOwner()))
		{
			const FRotator PawnViewRotation = OwningPawn->GetViewRotation();
			if (PawnViewRotation != GetComponentRotation())
			{
				SetWorldRotation(PawnViewRotation);
			}
		}
	}

	UpdateDesiredArmLocation(bDoCollisionTest, bEnableCameraLag, bEnableCameraRotationLag, DeltaTime);
}

FTransform UPlatformerCameraArm::GetSocketTransform(FName InSocketName, ERelativeTransformSpace TransformSpace) const
{
	FTransform RelativeTransform(RelativeSocketRotation, RelativeSocketLocation);

	switch(TransformSpace)
	{
		case RTS_World:
		{
			return RelativeTransform * ComponentToWorld;
			break;
		}
		case RTS_Actor:
		{
			if( const AActor* Actor = GetOwner() )
			{
				FTransform SocketTransform = RelativeTransform * ComponentToWorld;
				return SocketTransform.GetRelativeTransform(Actor->GetTransform());
			}
			break;
		}
		case RTS_Component:
		{
			return RelativeTransform;
		}
	}
	return RelativeTransform;
}

bool UPlatformerCameraArm::HasAnySockets() const
{
	return true;
}

void UPlatformerCameraArm::QuerySupportedSockets(TArray<FComponentSocketDescription>& OutSockets) const
{
	new (OutSockets) FComponentSocketDescription(SocketName, EComponentSocketType::Socket);
}


void SetDeprecatedControllerViewRotation(UPlatformerCameraArm& Component, bool bValue)
{
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	Component.bUseControllerViewRotation = bValue;
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
}
