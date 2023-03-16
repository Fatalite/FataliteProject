#include "FataliteCharacter.h"

// Sets default values
AFataliteCharacter::AFataliteCharacter(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{	

	PrimaryActorTick.bCanEverTick = true;
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	MainCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("MainCamera"));
	MainCamera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SkeletalMeshAsset(TEXT("SkeletalMesh'/Game/Characters/Mannequin/Character/Mesh/SK_Mannequin.SK_Mannequin'"));

	if (SkeletalMeshAsset.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(SkeletalMeshAsset.Object);
		GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -88.0f), FRotator(0.0f, -90.0f, 0.0f));
		static ConstructorHelpers::FObjectFinder<UAnimBlueprintGeneratedClass> 
			AnimBlueprint(TEXT("AnimBlueprint'/Game/Characters/Mannequin/Animations/ThirdPerson_AnimBP.ThirdPerson_AnimBP_C'"));

		if (AnimBlueprint.Succeeded())
		{
			GetMesh()->SetAnimInstanceClass(AnimBlueprint.Object);
		}
		else {
			UE_LOG(LogTemp, Log, TEXT("Can not Import Anim Blueprint"));
		}
	}
	//Sword Static Mesh importing 
	//static ConstructorHelpers::FObjectFinder<UStaticMesh>()
}

// Called when the game starts or when spawned
void AFataliteCharacter::BeginPlay()
{
	Super::BeginPlay();
	SpringArm->TargetArmLength = 500.0f;
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->bInheritPitch = true;
	SpringArm->bInheritYaw = true;
	SpringArm->bInheritRoll = true;
	bUseControllerRotationYaw = false;
	SpringArm->bDoCollisionTest = true;
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
	// WASD Input Setting
	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AFataliteCharacter::UpDownMovement);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AFataliteCharacter::LeftRightMovement);

	// Mouse LookUp & Turn Setting
	PlayerInputComponent->BindAxis("Turn", this, &AFataliteCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AFataliteCharacter::LookUp);


}

void AFataliteCharacter::UpDownMovement(float NewAxisValue) {
	AddMovementInput(FRotationMatrix(GetControlRotation()).GetUnitAxis(EAxis::X), NewAxisValue);
}
void AFataliteCharacter::LeftRightMovement(float NewAxisValue) {
	AddMovementInput(FRotationMatrix(GetControlRotation()).GetUnitAxis(EAxis::Y), NewAxisValue);
}
void AFataliteCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void AFataliteCharacter::LookUp(float Value)
{
	AddControllerPitchInput(-Value);
}