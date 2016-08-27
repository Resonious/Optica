// Fill out your copyright notice in the Description page of Project Settings.

#include "Optica.h"
#include "LightSource.h"
#include "LightRay.h"
#include "Components/ArrowComponent.h"

// Sets default values
ALightSource::ALightSource() : Timer(0)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    SetActorEnableCollision(false);

    // This is just the root component, containing our position and orientation -- light rays will be added to this guy.
    ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("LightSourceArrow"));
    RootComponent = ArrowComponent;
    ArrowComponent->SetArrowColor_New(FColor::Silver);
    ArrowComponent->SetRelativeRotation(FVector(0, 1, 0).ToOrientationRotator());
    ArrowComponent->ArrowSize = 3.0f;

    // TODO for testing, I will add a LightRay and see wtf I can do with it...
    TestRay = CreateDefaultSubobject<ULightRay>(TEXT("TestLightRay"));
    TestRay->SetRelativeScale3D(FVector(0,0,0));
    TestRay->SetVisibility(false, true);
    TestRay->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ALightSource::BeginPlay()
{
	Super::BeginPlay();
	
    CastLight();
}

// Called every frame
void ALightSource::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

    Timer += DeltaTime;

    CastLight();
}

void ALightSource::CastLight() {
    // TODO here's a raytrace oh god
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    FHitResult Hit;
    FVector Start = GetActorLocation();
    FVector End = Start + GetActorRotation().Vector() * (100.0f * 100.0f);
    bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_WorldStatic, Params);

    if (bHit) {
        FVector Direction;
        float Length;
        (Hit.ImpactPoint - Start).ToDirectionAndLength(Direction, Length);

        // TODO maybe the 0.2f's here could be thickness??
        if (Length > 0.0f) {
            FVector Scale(0.2f, Length / 100.0f, 0.2f);
            TestRay->SetRelativeScale3D(Scale);
        }

        TestRay->SetRelativeRotation(Direction.ToOrientationRotator());
        TestRay->SetVisibility(true, true);
    }
    else {
        UE_LOG(LightSource, Warning, TEXT("YOOOOOOOOOOOOO WE DID NOT HIT"));
        TestRay->SetVisibility(false, true);
        // TODO or maybe extend a long amount in this case?
    }
}
