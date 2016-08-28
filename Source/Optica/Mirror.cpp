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

void AMirror::AcceptLightRay(ULightRay* Ray, FVector& Direction, FHitResult& Hit) {
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
    FVector NewAngleDirection(0.0f, -Cos, -Sin);
    // FQuat NewAngleQuat = FQuat::FindBetweenNormals(FVector(1.0f, 0.0f, 0.0f), NewAngleDirection);

    Ray->CastChild(Hit.ImpactPoint, NewAngleDirection.ToOrientationRotator(), Hit.Actor.Get());
}
