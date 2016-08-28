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

    ChildRay->SetVisibility(false, true);
    /*
    TArray<USceneComponent*> AllChildren;
    ChildRay->GetChildrenComponents(true, AllChildren);
    for (int i = AllChildren.Num() - 1; i >= 0; --i) {
        AllChildren[i]->DestroyComponent();
    }

    ChildRay = nullptr;
    */
}

void ULightRay::CastChild(FVector Start, FRotator Orientation, AActor* Ignore) {
    CreateChildRay();
    ChildRay->CastLight(Start, Orientation, Ignore);
}

void ULightRay::CastLight(FVector Start, FRotator Orientation, AActor* Ignore) {
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Ignore);

    SetWorldLocationAndRotationNoPhysics(Start, Orientation);

    FHitResult Hit;
    FVector End = Start + Orientation.Vector() * (100.0f * 100.0f);
    bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_WorldDynamic, Params);

    if (bHit) {
        FVector Direction;
        float Length;
        (Hit.ImpactPoint - Start).ToDirectionAndLength(Direction, Length);

        // TODO maybe the 0.2f's here could be thickness??
        if (Length > 0.0f) {
            FVector Scale(Length / 100.0f, 0.2f, 0.2f);
            LightRayMesh->SetRelativeScale3D(Scale);
        }

        LightRayMesh->SetWorldLocationAndRotation(Start, Direction.ToOrientationRotator());
        SetVisibility(true, true);

        // TODO DEBUGGING HERE
        if (Hit.Actor.Get() && Hit.Component.Get())
            LastHitActor = Hit.Actor.Get()->GetName() + TEXT(" :: ") + Hit.Component.Get()->GetName();

        // Don't interact with a device more than 10 times
        if (NestedLevel < 20) {
            auto HitActor = Hit.Actor.Get();
            auto HitComponent = Hit.Component.Get();

            AOpticalDevice* Device = (HitActor && HitComponent) ? Cast<AOpticalDevice>(HitActor) : nullptr;

            auto _Name = HitComponent->GetName();
            auto _Tags = &HitComponent->ComponentTags;

            if (Device && HitComponent->ComponentHasTag(Device->ReceiverTag))
                Device->AcceptLightRay(this, Direction, Hit);
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
