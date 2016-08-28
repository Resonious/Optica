// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "LightRay.h"
#include "OpticalDevice.generated.h"

UCLASS()
class OPTICA_API AOpticalDevice : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AOpticalDevice();

    // Set this to the tag of the components that you want to receive light from
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ReceiverTag;

    virtual void AcceptLightRay(ULightRay* Ray, FVector& Direction, FHitResult& Hit);
};
