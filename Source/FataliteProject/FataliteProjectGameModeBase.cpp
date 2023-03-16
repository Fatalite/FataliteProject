// Copyright Epic Games, Inc. All Rights Reserved.


#include "FataliteProjectGameModeBase.h"

// 
AFataliteProjectGameModeBase::AFataliteProjectGameModeBase()
{	
	//Assign Fatalite Character as a Default Pawn Class;
	this->DefaultPawnClass = AFataliteCharacter::StaticClass();
	this->PlayerControllerClass = AFatalitePlayerController::StaticClass();
}