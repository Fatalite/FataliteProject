// Fill out your copyright notice in the Description page of Project Settings.


#include "FataliteCharacter.h"

// Sets default values
AFataliteCharacter::AFataliteCharacter(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{	

 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	MainCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("MainCamera"));
	MainCamera->SetupAttachment(SpringArm);
	MainArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("MainArrow"));
	MainArrow->SetupAttachment(RootComponent);
	SetActorLocation(FVector(0.0f, 0.0f, 0.0f));
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SkeletalMeshAsset(TEXT("SkeletalMesh'/Game/Characters/Mannequin/Character/Mesh/SK_Mannequin.SK_Mannequin'"));

	if (SkeletalMeshAsset.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(SkeletalMeshAsset.Object);
	}

}

// Called when the game starts or when spawned
void AFataliteCharacter::BeginPlay()
{
	Super::BeginPlay();
	SpringArm->TargetArmLength = 400.0f;
	MainCamera->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, 50.0f), FRotator(-60.0f, 0.0f, 0.0f));
	MainArrow->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, 50.0f), FRotator(-60.0f, 0.0f, 0.0f));
}

// Called every frame
void AFataliteCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

// Called to bind functionality to input
void AFataliteCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("UpDown"), this, &AFataliteCharacter::UpDownMovement);
	PlayerInputComponent->BindAxis(TEXT("RightLeft"), this, &AFataliteCharacter::LeftRightMovement);
}

void AFataliteCharacter::UpDownMovement(float NewAxisValue) {
	AddMovementInput(GetActorForwardVector(), NewAxisValue);
}
void AFataliteCharacter::LeftRightMovement(float NewAxisValue) {
	AddMovementInput(GetActorRightVector(), NewAxisValue);
}
