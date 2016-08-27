// Fill out your copyright notice in the Description page of Project Settings.

#include "Optica.h"
#include "Mirror.h"


// Sets default values
AMirror::AMirror()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMirror::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMirror::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

void AMirror::AcceptLightRay(ULightRay* ray) {
}
