// Fill out your copyright notice in the Description page of Project Settings.

#include "Optica.h"
#include "LightRay.h"
#include "OpticalDevice.h"


// Sets default values for this component's properties
ULightRay::ULightRay() :
    ColorParam(TEXT("Color")),
    ChildRay(nullptr),
    LightRayMesh(nullptr),
    NestedLevel(0)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = true;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh(TEXT("StaticMesh'/Game/Optica/lightbeam'"));
    MeshAsset = Mesh.Object;
    static ConstructorHelpers::FObjectFinder<UMaterial> Mat(TEXT("Material'/Game/Optica/LightRay'"));
    MatAsset = Mat.Object;
}

void ULightRay::SetColor(FLinearColor color) {
    if (LightRayMesh)
        LightRayMesh->SetVectorParameterValueOnMaterials(ColorParam, FVector(color.R, color.G, color.B));
    LightColor = color;
}


// Called when the game starts
void ULightRay::BeginPlay()
{
	Super::BeginPlay();

    if (LightRayMesh) return;

    // Add the mesh and material
    LightRayMesh = NewObject<UStaticMeshComponent>(this);
    LightRayMesh->RegisterComponent();
    bool attached = LightRayMesh->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetIncludingScale);
    ensure(attached);

    if (MeshAsset)
        LightRayMesh->SetStaticMesh(MeshAsset);

    if (MatAsset) {
        auto DynamicMat = UMaterialInstanceDynamic::Create(MatAsset, LightRayMesh);
        LightRayMesh->SetMaterial(0, DynamicMat);
    }

    LightRayMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}


// Called every frame
void ULightRay::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	// ...
}

void ULightRay::CreateChildRay() {
    if (ChildRay) return;
    ChildRay = NewObject<ULightRay>(this);
    ChildRay->NestedLevel = NestedLevel + 1;
    ChildRay->RegisterComponent();
    bool attached = ChildRay->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetIncludingScale);
    ensure(attached);
    ChildRay->SetVisibility(false, true);
    ChildRay->SetColor(LightColor);
}
void ULightRay::DestroyChildRay() {
    if (!ChildRay) return;

    ChildRay->LightRayMesh->DestroyComponent();
    ChildRay->DestroyComponent();
    ChildRay = nullptr;
}

void ULightRay::CastLight(FVector Start, FRotator Orientation, AActor* Ignore) {
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Ignore);

    SetWorldLocationAndRotationNoPhysics(Start, Orientation);

    FHitResult Hit;
    FVector End = Start + Orientation.Vector() * (100.0f * 100.0f);
    bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_WorldStatic, Params);

    if (bHit) {
        FVector Direction;
        float Length;
        (Hit.ImpactPoint - Start).ToDirectionAndLength(Direction, Length);
        FRotator Orientation = Direction.ToOrientationRotator();

        // TODO maybe the 0.2f's here could be thickness??
        if (Length > 0.0f) {
            FVector Scale(0.2f, Length / 100.0f, 0.2f);
            LightRayMesh->SetRelativeScale3D(Scale);
        }

        LightRayMesh->SetRelativeRotation(Orientation);
        SetVisibility(true, true);

        // TODO reflect uhhhhhh yeah we will check type of hit actor at some point too
        if (NestedLevel < 10) {
            auto HitActor = Hit.Actor.Get();
            AOpticalDevice* Device = HitActor ? Cast<AOpticalDevice>(HitActor) : nullptr;

            if (Device) {
                // TODO the following logic should be within the Device (currently just trying to get it to work)

                FVector2D Norm2D(Hit.ImpactNormal.Y, Hit.ImpactNormal.Z);
                Norm2D.Normalize();
                FVector2D Direction2D(Direction.Y, Direction.Z);
                Direction2D.Normalize();

                float RayAngle  = FMath::Atan2(Direction2D.Y, Direction2D.X) * 180.0f / PI;
                float NormAngle = FMath::Atan2(Norm2D.Y, Norm2D.X) * 180.0f / PI + 180.0f;
                float NewRayAngle = NormAngle + (NormAngle - RayAngle);
                if (NewRayAngle >= 360.0f)
                    NewRayAngle -= FMath::FloorToFloat(NewRayAngle / 360.0f) * 360.0f;

                float Sin, Cos;
                FMath::SinCos(&Sin, &Cos, NewRayAngle * PI / 180.0f);
                FVector NewAngleDirection(0.0f, Cos, Sin);
                FQuat NewAngleQuat = FQuat::FindBetweenNormals(FVector(0.0f, 1.0f, 0.0f), NewAngleDirection);

                CreateChildRay();
                ChildRay->SetColor(LightColor);
                ChildRay->CastLight(Hit.ImpactPoint, NewAngleQuat.Rotator(), HitActor);
            }
            else
                DestroyChildRay();
        }
    }
    else {
        DestroyChildRay();
        UE_LOG(LightSource, Warning, TEXT("YOOOOOOOOOOOOO WE DID NOT HIT"));
        SetVisibility(false, true);
        // TODO or maybe extend a long amount in this case?
    }
}
