// Fill out your copyright notice in the Description page of Project Settings.


#include "SwordActor.h"

// Sets default values
ASwordActor::ASwordActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SwordMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SwordMesh"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> StaticSwordMeshAsset(TEXT("StaticMesh'/Game/GreatswordComboPack/StaticMeshes/ExampleSword/GreatSwordExample.GreatSwordExample'"));

	if (StaticSwordMeshAsset.Succeeded())
	{
		UE_LOG(LogTemp, Log, TEXT("Succeeded Static Sword Mesh "));
		SwordMesh->SetStaticMesh(StaticSwordMeshAsset.Object);
		SwordMesh->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, 0.0f), FRotator(0.0f, 0.0f, 0.0f));
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("Failed Static Sword Mesh "));
	}
	RootComponent = SwordMesh;
}

// Called when the game starts or when spawned
void ASwordActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASwordActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASwordActor::AttachObjectToSwordSocket(AActor* ObjectToAttach, FName SocketName)
{
	if (ObjectToAttach && SwordMesh->DoesSocketExist(SocketName))
	{
		ObjectToAttach->AttachToComponent(SwordMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, SocketName);
	}
}