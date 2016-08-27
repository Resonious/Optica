// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "OpticalDevice.h"
#include "Mirror.generated.h"

UCLASS()
class OPTICA_API AMirror : public AOpticalDevice
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMirror();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

    virtual void AcceptLightRay(ULightRay* ray) override;
	
};
