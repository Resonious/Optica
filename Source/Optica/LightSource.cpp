// Fill out your copyright notice in the Description page of Project Settings.

#include "Optica.h"
#include "LightSource.h"
#include "LightRay.h"
#include "Components/ArrowComponent.h"

// Sets default values
ALightSource::ALightSource()
    : Ray(nullptr)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    SetActorEnableCollision(false);

    // This is just the root component, containing our position and orientation -- light rays will be added to this guy.
    ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("LightSourceArrow"));
    RootComponent = ArrowComponent;
    ArrowComponent->SetArrowColor_New(LightColor);
    ArrowComponent->SetWorldRotation(FVector(0, 1, 0).ToOrientationRotator());
    ArrowComponent->ArrowSize = 3.0f;
    ArrowComponent->SetSimulatePhysics(false);
}

#ifdef WITH_EDITORONLY_DATA
void ALightSource::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) {
    Super::PostEditChangeProperty(PropertyChangedEvent);

    if (Ray) Ray->SetColor(LightColor);
    ArrowComponent->SetArrowColor_New(LightColor);
}
#endif

// Called when the game starts or when spawned
void ALightSource::BeginPlay()
{
	Super::BeginPlay();
	
    if (Ray) Ray->SetColor(LightColor);
    CastLight();
}

// Called every frame
void ALightSource::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

    CastLight();
}

void ALightSource::InitializeRay() {
    if (Ray) return;
    Ray = NewObject<ULightRay>(this);
    Ray->NestedLevel = 0;
    Ray->Source = this;
    Ray->RegisterComponent();
    bool attached = Ray->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
    ensure(attached);
    Ray->SetVisibility(false, true);
    Ray->SetColor(LightColor);
}

void ALightSource::CastLight() {
    InitializeRay();

    check(Ray);
    Ray->CastLight(GetActorLocation(), GetActorRotation());
}

