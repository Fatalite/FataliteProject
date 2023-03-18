#include "FataliteCharacter.h"

// Sets default values
AFataliteCharacter::AFataliteCharacter(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{	
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	MainCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("MainCamera"));
	MainCamera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

	SpringArm->TargetArmLength = 500.0f;
	//SpringArm->SetRelativeRotation(FRotator::ZeroRotator);
	
	
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->bInheritPitch = true;
	SpringArm->bInheritRoll = true;
	SpringArm->bInheritYaw = true;
	SpringArm->bDoCollisionTest = false;
	bUseControllerRotationYaw = false;
	
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 600.0f;
	GetCharacterMovement()->AirControl = 0.2f;

	//Anim Montage Importing
	static ConstructorHelpers::FObjectFinder<UAnimMontage> SwingMontageFinder(TEXT("AnimMontage'/Game/GreatswordComboPack/Animations/InPlace/SwingMontage.SwingMontage'"));
	if (SwingMontageFinder.Succeeded())
	{
		UE_LOG(LogTemp, Log, TEXT("Swing Montage Imported"));
		SwingMontage = SwingMontageFinder.Object;
	}
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
	

	UE_LOG(LogTemp, Error, TEXT("SwordRightHandSocket Init"));
	//Sword Equipment
	FVector SpawnLocation = FVector::ZeroVector;
	FRotator SpawnRotation = FRotator(0.0f,180.0f,0.0f);
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
	// Actor Component Location
	SpawnedSword->SetPlane(FRotator(0.0f, 90.0f, 0.0f));

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
	
	// Jump Setting
	/*
	enum EInputEvent
	{
    IE_Pressed        =0,
    IE_Released       =1,
    IE_Repeat         =2,
    IE_DoubleClick    =3,
    IE_Axis           =4,
    IE_MAX            =5,
	}
	*/
	PlayerInputComponent->BindAction("Jump", IE_Pressed ,this, &AFataliteCharacter::Jump);
	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &AFataliteCharacter::Attack);
}

void AFataliteCharacter::UpDownMovement(float NewAxisValue) {
	//FRotationMatrix(GetControlRotation()).GetUnitAxis(EAxis::X)
	AddMovementInput(
		FRotationMatrix(Controller->GetControlRotation()).GetUnitAxis(EAxis::X),
		NewAxisValue);
}
void AFataliteCharacter::LeftRightMovement(float NewAxisValue) {
	AddMovementInput(
		FRotationMatrix(Controller->GetControlRotation()).GetUnitAxis(EAxis::Y),
		NewAxisValue);
}
void AFataliteCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void AFataliteCharacter::LookUp(float Value)
{
	AddControllerPitchInput(-Value);
}

void AFataliteCharacter::Jump() {
	//Do Jump Parameter = bReplayingMoves
	GetCharacterMovement()->DoJump(false);
}

void AFataliteCharacter::Attack() {
	//Do Jump Parameter = bReplayingMoves
	UE_LOG(LogTemp, Log, TEXT("Attack"));
	PlayAnimMontage(SwingMontage);
	
		UE_LOG(LogTemp, Log, TEXT("Valid Play"));
	
	
}