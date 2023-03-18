// Fill out your copyright notice in the Description page of Project Settings.


#include "FataliteAnimInstance.h"
#include "FataliteCharacter.h"
UFataliteAnimInstance::UFataliteAnimInstance() {
	CurrentPawnSpeed = 0.0f;
}

void UFataliteAnimInstance::NativeUpdateAnimation(float DeltaSeconds) {
	Super::NativeUpdateAnimation(DeltaSeconds);
	auto Pawn = TryGetPawnOwner();
	if (::IsValid(Pawn)) {
		AFataliteCharacter* Character = Cast<AFataliteCharacter>(Pawn);
		CurrentPawnSpeed = Character->GetVelocity().Size();
	}
}