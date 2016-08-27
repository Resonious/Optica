// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "Optica.h"
#include "OpticaGameMode.h"
#include "OpticaCharacter.h"

AOpticaGameMode::AOpticaGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/SideScrollerCPP/Blueprints/SideScrollerCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
