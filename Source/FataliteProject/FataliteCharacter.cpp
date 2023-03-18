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
}

// Called when the game starts or when spawned
void AFataliteCharacter::BeginPlay()
{
	Super::BeginPlay();
	SpringArm->TargetArmLength = 500.0f;
	SpringArm->bUsePawnControlRotation = true;
	bUseControllerRotationYaw = false;
	MainCamera->bUsePawnControlRotation = false;

	UE_LOG(LogTemp, Error, TEXT("SwordRightHandSocket Init"));
	//Sword Equipment
	FVector SpawnLocation = FVector::ZeroVector;
	FRotator SpawnRotation = FRotator::ZeroRotator;
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ASwordActor* SpawnedSword = GetWorld()->SpawnActor<ASwordActor>(ASwordActor::StaticClass(), SpawnLocation, SpawnRotation, SpawnParameters);
	if (SpawnedSword)
	{
		UE_LOG(LogTemp, Log, TEXT("SwordRightHandSocket Spawned"));
		FName SwordSocketName = TEXT("SwordRightHandSocket");
		USkeletalMeshComponent* CharacterMesh = GetMesh();
		if (CharacterMesh && CharacterMesh->DoesSocketExist(SwordSocketName))
		{
			SpawnedSword->AttachToComponent(CharacterMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, SwordSocketName);
			UE_LOG(LogTemp, Log, TEXT("SwordRightHandSocket Attached"));
		}
		else {
			UE_LOG(LogTemp, Error, TEXT("SwordRightHandSocket Not Attached"));
		}
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("SwordRightHandSocket Not Spawned"));
	}
	

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
	//FRotationMatrix(GetControlRotation()).GetUnitAxis(EAxis::X)
	AddMovementInput(GetActorForwardVector(), NewAxisValue);
}
void AFataliteCharacter::LeftRightMovement(float NewAxisValue) {
	AddMovementInput(GetActorRightVector(), NewAxisValue);
}
void AFataliteCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void AFataliteCharacter::LookUp(float Value)
{
	AddControllerPitchInput(-Value);
}