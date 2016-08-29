// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "OpticalDevice.h"
#include "OpticalReceiver.generated.h"

UCLASS()
class OPTICA_API AOpticalReceiver : public AOpticalDevice
{
	GENERATED_BODY()
	
public:	
    struct FRequiredColor {
        FLinearColor Color;
        UStaticMeshComponent* Comp;
        int Satisfied;
        bool bSatisfied;
    };

	// Sets default values for this actor's properties
	AOpticalReceiver();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

    virtual void AcceptLightRay(ULightRay* Ray, FVector& Direction, FHitResult& Hit);

    UFUNCTION(BlueprintCallable, Category = "OpticalReceiver")
    void SetRequiredColors(TArray<FLinearColor> RequiredColors);

private:
    static const FVector IndicatorSpots[14];
    static const FVector IndicatorScale;
    static const FName ColorParam;
    static const FName ReceiverTagName;

    TArray<FRequiredColor> Colors;

    UStaticMesh* IndicatorMesh;
    UMaterial* IndicatorMat;
};
