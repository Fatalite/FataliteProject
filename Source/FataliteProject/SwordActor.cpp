// Fill out your copyright notice in the Description page of Project Settings.


#include "SwordActor.h"
#include "FataliteCharacter.h"
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

	SwordMesh->SetCollisionProfileName(TEXT("OverlapAll"));
	//IngoreOnlyPawn
}

// Called when the game starts or when spawned
void ASwordActor::BeginPlay()
{
	Super::BeginPlay();
	//PlaneNormalVector = FVector(0,0,0);
	//Register the events
	SwordMesh->OnComponentBeginOverlap.AddDynamic(this, &ASwordActor::OnSwordBeginOverlap);
	SwordMesh->OnComponentEndOverlap.AddDynamic(this, &ASwordActor::OnSwordEndOverlap);

}

// Called every frame
void ASwordActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	PreviousStartLocation = CurrentStartLocation;
	PreviousEndLocation = CurrentEndLocation;
	CurrentStartLocation = SwordMesh->GetSocketLocation("StartPoint");
	CurrentEndLocation = SwordMesh->GetSocketLocation("EndPoint");


	PreviousStartToEnd = CurrentStartToEnd;
	PreviousLeftToRight = CurrentLeftToRight;
	CurrentStartToEnd = SwordMesh->GetSocketLocation("EndPoint") - SwordMesh->GetSocketLocation("StartPoint");
	CurrentLeftToRight = SwordMesh->GetSocketLocation("RightPoint") - SwordMesh->GetSocketLocation("LeftPoint");
	//UE_LOG(LogTemp, Error, TEXT("%s"), *CurrentLeftToRight.ToString());
}

void ASwordActor::AttachObjectToSwordSocket(AActor* ObjectToAttach, FName SocketName)
{
	if (ObjectToAttach && SwordMesh->DoesSocketExist(SocketName))
	{
		ObjectToAttach->AttachToComponent(SwordMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, SocketName);
	}
}


void ASwordActor::SetPlane(FRotator Rotation)
{
	PlaneComponent->SetRelativeRotation(Rotation);
}


void ASwordActor::OnSwordBeginOverlap(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AFataliteCharacter* ParentCharacter = Cast<AFataliteCharacter>(GetAttachParentActor());
	if (!ParentCharacter || OtherActor == Cast<AActor>(ParentCharacter) || OtherActor == this) {
		return;
	}
	if (OtherActor != nullptr && OtherActor != this && OtherComp != nullptr)
	{
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("Cant get Start Vector"));
	}
}

void ASwordActor::OnSwordEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AFataliteCharacter* ParentCharacter = Cast<AFataliteCharacter>(GetAttachParentActor());
	UE_LOG(LogTemp, Log, TEXT("End Collide"));
	if (!ParentCharacter) {
		return;
	}
	else {
		if (OtherActor != nullptr && OtherActor != this && OtherComp != nullptr && OtherActor != Cast<AActor>(ParentCharacter))
		{
			
		}
		else {
			UE_LOG(LogTemp, Error, TEXT("Parent"));
		}

	}
	

}
void ASwordActor::SaveStaticMeshTriangles(UStaticMeshComponent* StaticMeshComponent) {
	if (!StaticMeshComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid StaticMeshComponent pointer."));
		return;
	}
	UStaticMesh* StaticMesh = StaticMeshComponent->GetStaticMesh();
	if (!StaticMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid StaticMesh pointer."));
		return;
	}

	FTransform ComponentTransform = StaticMeshComponent->GetComponentTransform();
	FStaticMeshLODResources& LODModel = StaticMesh->GetRenderData()->LODResources[0];
	FIndexArrayView Indices = LODModel.IndexBuffer.GetArrayView();

	const uint32 NumSections = LODModel.Sections.Num();
	for (uint32 SectionIndex = 0; SectionIndex < NumSections; ++SectionIndex)
	{
		FStaticMeshSection& Section = LODModel.Sections[SectionIndex];

		TArray<Triangle> triangleArray;

		for (uint32 i = Section.FirstIndex; i < Section.FirstIndex + Section.NumTriangles * 3; i += 3)
		{
			FVector3f LocalVertex1 = LODModel.VertexBuffers.PositionVertexBuffer.VertexPosition(Indices[i]);
			FVector3f LocalVertex2 = LODModel.VertexBuffers.PositionVertexBuffer.VertexPosition(Indices[i + 1]);
			FVector3f LocalVertex3 = LODModel.VertexBuffers.PositionVertexBuffer.VertexPosition(Indices[i + 2]);
			
			
			FVector Vertex1 = ComponentTransform.TransformPosition(
				FVector(static_cast<double>(LocalVertex1.X), static_cast<double>(LocalVertex1.Y), static_cast<double>(LocalVertex1.Z)));
			FVector Vertex2 = ComponentTransform.TransformPosition(
				FVector(static_cast<double>(LocalVertex2.X), static_cast<double>(LocalVertex2.Y), static_cast<double>(LocalVertex2.Z)));
			FVector Vertex3 = ComponentTransform.TransformPosition(
				FVector(static_cast<double>(LocalVertex3.X), static_cast<double>(LocalVertex3.Y), static_cast<double>(LocalVertex3.Z)));

			triangleArray.Push(Triangle(Vertex1, Vertex2, Vertex3));
			// °¢ »ï°¢ÇüÀÇ ¼¼ Á¤Á¡ ÁÂÇ¥
			//UE_LOG(LogTemp, Error, TEXT("Triangle"));
			//UE_LOG(LogTemp, Log, TEXT("Triangle: Vertex1=(%s), Vertex2=(%s), Vertex3=(%s)"),
				 //*Vertex1.ToString(), *Vertex2.ToString(), *Vertex2.ToString());
		}
	}
}

FVector3d ASwordActor::GetNormalVector() {
	return FVector::CrossProduct(CurrentLeftToRight, CurrentStartToEnd);
}

FVector& ASwordActor::GetPreviousStartToEndVector() {
	return PreviousStartToEnd;
}
FVector& ASwordActor::GetCurrentStartToEndVector() {
	return CurrentStartToEnd;
}

FVector& ASwordActor::GetPreviousStartLocation() {
	return PreviousStartLocation;
};
FVector& ASwordActor::GetCurrentStartLocation() {
	return CurrentStartLocation;
}
FVector& ASwordActor::GetPreviousEndLocation() {
	return PreviousEndLocation;
}
FVector& ASwordActor::GetCurrentEndLocation() {
	return CurrentEndLocation;
}