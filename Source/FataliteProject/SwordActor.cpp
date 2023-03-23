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
	PlaneComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaneComponent"));
	PlaneComponent->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("StaticMesh'/Engine/BasicShapes/Plane.Plane'"));
	if (PlaneMesh.Succeeded())
	{
		PlaneComponent->SetStaticMesh(PlaneMesh.Object);
		
	}
	PlaneComponent->AddRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));

	SwordMesh->SetCollisionProfileName(TEXT("OverlapAll"));
	PlaneComponent->SetCollisionProfileName(TEXT("NoCollision"));
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
	UE_LOG(LogTemp, Log, TEXT("Start Collide"));
	if (OtherActor != nullptr && OtherActor != this && OtherComp != nullptr)
	{
		
		// This is the point where the sword started to collide with the other mesh
		StartCollisionVector = SweepResult.TraceStart;
		UE_LOG(LogTemp, Error, TEXT("%s"), *SweepResult.GetActor()->GetFName().ToString());
		UE_LOG(LogTemp, Error, TEXT("%s"), *StartCollisionVector.ToString());
		// You can now use the collision point for your purposes
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
			UE_LOG(LogTemp, Error, TEXT("Not Parent"));
			FString tmp = OtherActor->GetFName().ToString();
			//std::string a = std::string(TCHAR_TO_UTF8(*tmp));
			UE_LOG(LogTemp, Error, TEXT("%s"), *tmp);
			// Perform a line trace to find the point where the sword stopped intersecting the other mesh
			FVector Start = GetActorLocation();
			FVector End = Start + (GetActorForwardVector() * 100.0f); // Adjust TraceDistance according to your needs
			FHitResult HitResult;
			FCollisionQueryParams TraceParams(FName(TEXT("Sword Trace")), true, this);
			TraceParams.bTraceComplex = true;
			TraceParams.bReturnPhysicalMaterial = true;

			// Perform the line trace
			if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, TraceParams))
			{
				// This is the point where the sword stopped intersecting the other mesh
				EndCollisionVector = HitResult.TraceEnd;
				// You can now use the end collision point for your purposes
			}
			else {
				UE_LOG(LogTemp, Error, TEXT("Cant get End Vector"));
			}

			FVector ParentForwardVector;
			UE_LOG(LogTemp, Error, TEXT("Cast for End Collide"));
			ParentForwardVector = ParentCharacter->GetActorRightVector();
			FVector StartToEndVector = EndCollisionVector - StartCollisionVector;
			UE_LOG(LogTemp, Error, TEXT("%s"), *StartToEndVector.ToString());
			FVector CuttingPlaneNormalVector = FVector::CrossProduct(StartToEndVector, ParentForwardVector);
			//PlaneNormalVector = CuttingPlaneNormalVector;
			//Get Static Mesh's Global Triangles
			UStaticMeshComponent* param = Cast<UStaticMeshComponent>(OtherActor->GetComponentByClass(UStaticMeshComponent::StaticClass()));
			//UE_LOG(LogTemp, Error, TEXT("%s"), *param->GetFName().ToString());
			if (param) {
				SaveStaticMeshTriangles(param);
			}
			else {
				UE_LOG(LogTemp, Error, TEXT("NO STATIC MESH"));
			}
			


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
	
		
		//UE_LOG(LogTemp, Error, TEXT("%f,%f,%f"), PlaneNormalVector.X, PlaneNormalVector.Y, PlaneNormalVector.Z);
		return PlaneNormalVector;
}

