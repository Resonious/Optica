// Fill out your copyright notice in the Description page of Project Settings.

#include "Optica.h"
#include "OpticalDevice.h"


// Sets default values
AOpticalDevice::AOpticalDevice()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

void AOpticalDevice::AcceptLightRay(ULightRay* ray) {
    UE_LOG(LightSource, Warning, TEXT("Undefined optical device accepting a ray??? Or you called super don't do that."));
}
