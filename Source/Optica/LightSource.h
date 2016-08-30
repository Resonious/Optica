// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "LightSource.generated.h"

UCLASS()
class OPTICA_API ALightSource : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALightSource();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

    virtual void CastLight();

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor LightColor = FLinearColor(0,0,0);

    UFUNCTION(BlueprintImplementableEvent, Category = "LightSource")
    void HitCharacter(class ACharacter* character, FRotator orientation);

protected:
    class UArrowComponent* ArrowComponent;

private:
    void InitializeRay();
    class ULightRay* Ray;

};
