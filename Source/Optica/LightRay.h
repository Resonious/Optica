// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/SceneComponent.h"
#include "LightRay.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class OPTICA_API ULightRay : public USceneComponent
{
    GENERATED_BODY()

public:    
    // Sets default values for this component's properties
    ULightRay();

    // Called when the game starts
    virtual void BeginPlay() override;
    
    // Called every frame
    virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

    void SetColor(FLinearColor color);
    void CastLight(FVector Start, FRotator Orientation, AActor* Ignore = nullptr);
    void CastChild(FVector Start, FRotator Orientation, AActor* Ignore = nullptr);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor LightColor;
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
    int NestedLevel;

    UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
    FString LastHitActor;

private:
    void CreateChildRay();
    void DestroyChildRay();

    class UStaticMeshComponent* LightRayMesh;
    FName ColorParam;
    ULightRay* ChildRay;

    UStaticMesh* MeshAsset;
    UMaterial* MatAsset;
    
};
