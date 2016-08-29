// Fill out your copyright notice in the Description page of Project Settings.

#include "Optica.h"
#include "OpticalReceiver.h"

const FVector AOpticalReceiver::IndicatorSpots[14] = {
    FVector(-55.f,   0.f, 25.f),
    FVector(-55.f, -15.f, 25.f),
    FVector(-55.f,  15.f, 25.f),
    FVector(-55.f, -30.f, 25.f),
    FVector(-55.f,  30.f, 25.f),
    FVector(-55.f, -45.f, 25.f),
    FVector(-55.f,  45.f, 25.f),

    FVector(-55.f,   0.f, 10.f),
    FVector(-55.f, -15.f, 10.f),
    FVector(-55.f,  15.f, 10.f),
    FVector(-55.f, -30.f, 10.f),
    FVector(-55.f,  30.f, 10.f),
    FVector(-55.f, -45.f, 10.f),
    FVector(-55.f,  45.f, 10.f),
};
const FVector AOpticalReceiver::IndicatorScale(0.1f, 0.1f, 0.1f);
const FName AOpticalReceiver::ColorParam(TEXT("Color"));
const FName AOpticalReceiver::ReceiverTagName(TEXT("ORR"));

// Sets default values
AOpticalReceiver::AOpticalReceiver()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = ETickingGroup::TG_PrePhysics;

    ReceiverTag = ReceiverTagName;

    UStaticMeshComponent* Base = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ReceiverBase"));;
    RootComponent = Base;
    Base->SetSimulatePhysics(false);

    UStaticMeshComponent* Bulb = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ReceiverBulb"));;
    Bulb->SetupAttachment(Base);
    Bulb->SetSimulatePhysics(false);
    Bulb->ComponentTags.Add(ReceiverTagName);

    // ======================= BASE ======================
    // Mesh
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh(TEXT("StaticMesh'/Game/Optica/Props/receiver_Cube'"));
    if (Mesh.Object)
        Base->SetStaticMesh(Mesh.Object);

    // Material
    static ConstructorHelpers::FObjectFinder<UMaterial> Mat(TEXT("Material'/Game/Optica/Platform'"));
    if (Mat.Object) {
        auto DynamicMat = UMaterialInstanceDynamic::Create(Mat.Object, Base);
        Base->SetMaterial(0, DynamicMat);
    }

    // ======================= RECEIVER BULB ======================
    // Mesh
    static ConstructorHelpers::FObjectFinder<UStaticMesh> RMesh(TEXT("StaticMesh'/Game/Optica/Props/receiver_Sphere'"));
    if (RMesh.Object)
        Bulb->SetStaticMesh(RMesh.Object);

    // Material
    static ConstructorHelpers::FObjectFinder<UMaterial> RMat(TEXT("Material'/Game/Optica/Mirror'"));
    if (RMat.Object) {
        auto DynamicMat = UMaterialInstanceDynamic::Create(RMat.Object, Bulb);
        Bulb->SetMaterial(0, DynamicMat);
    }

    // ======================= INDICATORS ======================
    // Mesh
    static ConstructorHelpers::FObjectFinder<UStaticMesh> IMesh(TEXT("StaticMesh'/Engine/BasicShapes/Sphere'"));
    IndicatorMesh = IMesh.Object;
    ensure(IndicatorMesh);

    // Material
    static ConstructorHelpers::FObjectFinder<UMaterial> IMat(TEXT("Material'/Game/Optica/ReceiverBulb'"));
    IndicatorMat = IMat.Object;
    ensure(IndicatorMat);
}

// Called when the game starts or when spawned
void AOpticalReceiver::BeginPlay()
{
    ReceiverTag = ReceiverTagName;
	PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = ETickingGroup::TG_PrePhysics;
	Super::BeginPlay();
}

// Called every frame
void AOpticalReceiver::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

    for (auto& Color : Colors) {
        Color.bSatisfied = false;
        if (Color.Satisfied <= 0)
            continue;

        if (--Color.Satisfied <= 0)
            Color.Comp->SetVectorParameterValueOnMaterials(
                ColorParam,
                FVector(Color.Color.R, Color.Color.G, Color.Color.B)
            );
    }
}

void AOpticalReceiver::AcceptLightRay(ULightRay* Ray, FVector& Direction, FHitResult& Hit) {
    for (auto& Color : Colors) {
        if (Color.bSatisfied) continue;

        if (Color.Color.Equals(Ray->LightColor, 0.001)) {
            Color.Comp->SetVectorParameterValueOnMaterials(ColorParam, FVector::ZeroVector);
            Color.Satisfied = 2;
            Color.bSatisfied = true;
            break;
        }
    }
}

void AOpticalReceiver::SetRequiredColors(TArray<FLinearColor> RequiredColors) {
    // For now going to assume this only ever gets called once on one instance of AOpticalReceiver.
    // (The mesh components never get destroyed)

    Colors.Reserve(RequiredColors.Num());

    int IndicatorSpot = 0;
    for (auto& Color : RequiredColors) {
        UStaticMeshComponent* Indicator = NewObject<UStaticMeshComponent>(this);
        Indicator->RegisterComponent();
        bool attached = Indicator->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
        ensure(attached);

        Indicator->SetStaticMesh(IndicatorMesh);

        auto DynamicMat = UMaterialInstanceDynamic::Create(IndicatorMat, Indicator);
        Indicator->SetMaterial(0, DynamicMat);
        Indicator->SetVectorParameterValueOnMaterials(ColorParam, FVector(Color.R, Color.G, Color.B));

        Indicator->SetRelativeScale3D(IndicatorScale);
        if (IndicatorSpot < 14)
            Indicator->SetRelativeLocation(IndicatorSpots[IndicatorSpot++]);

        Colors.Add({ Color, Indicator, 0, false });
    }
}
