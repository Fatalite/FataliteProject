// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "FataliteProject.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Math/UnrealMathUtility.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "SwordActor.h"

#include "GameFramework/SpringArmComponent.h"
#include "FataliteCharacter.generated.h"

UCLASS()
class FATALITEPROJECT_API AFataliteCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AFataliteCharacter(const FObjectInitializer& ObjectInitializer);
	UPROPERTY(BlueprintReadOnly)
		UAnimInstance* AnimInstance;
	UPROPERTY(VisibleAnywhere, Category = Camera)
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, Category = Viusal)
	USkeletalMeshComponent* SkeletalMeshComponent;

	UPROPERTY(VisibleAnywhere, Category = Collision)
	UCapsuleComponent* MainCapsuleComponent;

	UStaticMeshComponent* SwordStaticMesh;

	UAnimMontage* SwordAttackAnim;

	UCameraComponent* MainCamera;
	UArrowComponent* MainArrow;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sword")
	TSubclassOf<ASwordActor> SwordClass;
	
	bool isAttacking = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	class UAnimMontage* SwingMontage;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
private:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void UpDownMovement(float NewAxisValue);
	void LeftRightMovement(float NewAxisValue);
	void Turn(float NewAxisValue);
	void LookUp(float NewAxisValue);
	void Jump();
	void Attack();
};
