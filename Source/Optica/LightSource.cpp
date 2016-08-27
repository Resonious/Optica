// Fill out your copyright notice in the Description page of Project Settings.

#include "Optica.h"
#include "LightSource.h"
#include "LightRay.h"

// Sets default values
ALightSource::ALightSource()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    // This is just the root component, containing our position and orientation -- light rays will be added to this guy.
    USphereComponent* SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("RootComponent"));
    RootComponent = SphereComponent;
    SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SphereComponent->InitSphereRadius(1.0f);
    SphereComponent->SetCollisionProfileName(TEXT("WorldDynamic"));

    // TODO for testing, I will add a LightRay and see wtf I can do with it...
    ULightRay* TestRay = CreateDefaultSubobject<ULightRay>(TEXT("TestLightRay"));
    TestRay->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ALightSource::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ALightSource::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

