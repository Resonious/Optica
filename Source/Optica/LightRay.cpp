// Fill out your copyright notice in the Description page of Project Settings.

#include "Optica.h"
#include "LightRay.h"


// Sets default values for this component's properties
ULightRay::ULightRay()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = true;

    // Add mesh...
    UStaticMeshComponent* LightRayMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LightRayMesh"));
    LightRayMesh->SetupAttachment(this);
    LightRayMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh(TEXT("StaticMesh'/Game/Optica/lightbeam'"));
    if (Mesh.Object)
        LightRayMesh->SetStaticMesh(Mesh.Object);

    static ConstructorHelpers::FObjectFinder<UMaterial> Mat(TEXT("Material'/Game/Optica/LightRay'"));
    if (Mat.Object) {
        auto DynamicMat = UMaterialInstanceDynamic::Create(Mat.Object, LightRayMesh);
        LightRayMesh->SetMaterial(0, DynamicMat);
    }
}


// Called when the game starts
void ULightRay::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void ULightRay::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	// ...
}

