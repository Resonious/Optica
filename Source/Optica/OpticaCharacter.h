// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/Character.h"
#include "OpticaCharacter.generated.h"

UCLASS(config=Game)
class AOpticaCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Side view camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* SideViewCameraComponent;

	/** Camera boom positioning the camera beside the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UPlatformerCameraArm* CameraBoom;

protected:

	/** Called for side to side input */
	void MoveRight(float Val);

	/** Handle touch inputs. */
	void TouchStarted(const ETouchIndex::Type FingerIndex, const FVector Location);

	/** Handle touch stop event. */
	void TouchStopped(const ETouchIndex::Type FingerIndex, const FVector Location);

    virtual void Tick(float DeltaTime) override;

    virtual void PickUp();
    virtual void Drop();

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	// End of APawn interface

    virtual void StopJumping() override;

public:
    static const FName PICKUPABLE_TAG;

	AOpticaCharacter();

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float JumpStopVelocity;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PickupRange;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PickupLagSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector PickupOffset;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector DropOffVelocity;

    AActor* PickedUpActor;
    UPrimitiveComponent* PickupPhysics;

    bool IsGrounded() const;

	/** Returns SideViewCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetSideViewCameraComponent() const { return SideViewCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class UPlatformerCameraArm* GetCameraBoom() const { return CameraBoom; }

private:
    // Stuff for picked up things
    FVector PreviousPickupOrigin;
    FVector PreviousDesiredLoc;
};
