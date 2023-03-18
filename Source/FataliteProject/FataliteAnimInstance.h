// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "FataliteAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class FATALITEPROJECT_API UFataliteAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	UFataliteAnimInstance();
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pawn, Meta = (AllowPrivateAccess = true))
		float CurrentPawnSpeed;

};
