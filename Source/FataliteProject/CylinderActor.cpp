#include "CylinderActor.h"
#include "Components/StaticMeshComponent.h"
#include "StaticMeshResources.h"
#include "CoreFwd.h"
#include "KismetProceduralMeshLibrary.h"
#include "Rendering/SkinWeightVertexBuffer.h"


// Sets default values
ACylinderActor::ACylinderActor()
{
    // Set this actor to call Tick() every frame
    PrimaryActorTick.bCanEverTick = true;

    // Create cylinder static mesh component and attach it to the root component
    CylinderMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CylinderMeshComponent"));
    UpperProceduralMeshComponent = CreateDefaultSubobject <UProceduralMeshComponent>(TEXT("UpperProceduralMesh"));
    LowerProceduralMeshComponent = CreateDefaultSubobject <UProceduralMeshComponent>(TEXT("LowerProceduralMesh"));
    RootComponent = UpperProceduralMeshComponent;
    CylinderMeshComponent->SetupAttachment(RootComponent);
    LowerProceduralMeshComponent->SetupAttachment(RootComponent);
    // Load cylinder static mesh
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("StaticMesh'/Engine/BasicShapes/Cylinder.Cylinder'"));
    if (CylinderMesh.Succeeded())
    {
        CylinderMeshComponent->SetStaticMesh(CylinderMesh.Object);
    }
    UStaticMesh* StaticMesh = CylinderMeshComponent->GetStaticMesh();
    if (StaticMesh == nullptr) return;
    static ConstructorHelpers::FObjectFinder<UMaterial> CyanMat(TEXT("Material'/Game/CyanWireFrameMat.CyanWireFrameMat'"));
    UpperProceduralMeshComponent->SetMaterial(0, CyanMat.Object);

    static ConstructorHelpers::FObjectFinder<UMaterial> VioletMat(TEXT("Material'/Game/VioletWireFrameMat.VioletWireFrameMat'"));
    LowerProceduralMeshComponent->SetMaterial(0, VioletMat.Object);
    FStaticMeshLODResources& LODModel = StaticMesh->GetRenderData()->LODResources[0];
    FIndexArrayView Indices = LODModel.IndexBuffer.GetArrayView();
    const uint32 NumSections = LODModel.Sections.Num();

    //Importing File
    // Read the OBJ file
    TArray<FVector> Vertices;
    TArray<int32> Triangles;

    FString FilePath("C:\\Users\\ugly2\\Downloads\\tetgen1.6.0\\tetgen1.6.0\\cylinder.1.node");
    FString FileContent;
    if (!FFileHelper::LoadFileToString(FileContent, *FilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load Vertice(Node) file: %s"), *FilePath);
        return;
    }
    else {
        //UE_LOG(LogTemp, Error, TEXT("%s"), *FileContent);
        TArray<FString> lines;

        //TArray<int32> Triangles;
        int32 lineCount = FileContent.ParseIntoArray(lines, TEXT("\n"), true);
        FileContent.Empty();
        TArray<FString> lineElements;
        for (int i = 1; i < lineCount-1; i++) {
            FString Line = lines[i].TrimStartAndEnd();
            lineElements.Empty();
            int32 elementCount = Line.ParseIntoArray(lineElements, TEXT(" "), true);
            Vertices.Add(FVector(FCString::Atod(*lineElements[1]), FCString::Atod(*lineElements[3]), FCString::Atod(*lineElements[2])));
        }
        for (FVector fv : Vertices) {
            UE_LOG(LogTemp, Error, TEXT("%s"), *fv.ToString());
        }
    }

    FilePath = "C:\\Users\\ugly2\\Downloads\\tetgen1.6.0\\tetgen1.6.0\\cylinder.1.face";
    FileContent.Empty();

    if (!FFileHelper::LoadFileToString(FileContent, *FilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load Face(Triangles) file: %s"), *FilePath);
        return;
    }
    else {
        //UE_LOG(LogTemp, Error, TEXT("%s"), *FileContent);
        TArray<FString> lines;


        int32 lineCount = FileContent.ParseIntoArray(lines, TEXT("\n"), true);
        FileContent.Empty();
        TArray<FString> lineElements;
        for (int i = 1; i < lineCount-1; i++) {
            FString Line = lines[i].TrimStartAndEnd();
            lineElements.Empty();
            int32 elementCount = Line.ParseIntoArray(lineElements, TEXT(" "), true);
            UE_LOG(LogTemp, Error, TEXT("%d"), elementCount);
            UE_LOG(LogTemp, Error, TEXT("-----------------"));
            Triangles.Add((FCString::Atoi(*lineElements[1]) - 1));
            Triangles.Add(FCString::Atoi(*lineElements[3]) - 1);
            Triangles.Add(FCString::Atoi(*lineElements[2]) - 1);
            
        }
        for (int fv : Triangles) {
            UE_LOG(LogTemp, Error, TEXT("%d"), fv);
        }
    }
    UpperProceduralMeshComponent->ClearAllMeshSections();
    UpperProceduralMeshComponent->CreateMeshSection_LinearColor(0, Vertices, Triangles, TArray<FVector>(), TArray<FVector2D>(), TArray<FLinearColor>(), TArray<FProcMeshTangent>(), false);

    /*
    
    //LowerProceduralMeshComponent->AddToRoot();
    for (uint32 i = 0; i < NumSections; i++) {
        TArray< FVector > TmpVertices;
        TArray< int32 > TmpTriangles;
        TArray< FVector > TmpNormals;
        TArray< FVector2D > TmpUVs;
        TArray< FProcMeshTangent > TmpTangents;
        TArray<FColor> TmpColors;
        UKismetProceduralMeshLibrary::GetSectionFromStaticMesh(
            CylinderMeshComponent->GetStaticMesh(), 0, i, TmpVertices, TmpTriangles, TmpNormals, TmpUVs, TmpTangents);
        UpperProceduralMeshComponent->CreateMeshSection(
            i, TmpVertices, TmpTriangles, TmpNormals, TmpUVs, TmpColors, TmpTangents, false);
        //LowerProceduralMeshComponent->CreateMeshSection(
          //  i, TmpVertices, TmpTriangles, TmpNormals, TmpUVs, TmpColors, TmpTangents, false);
    }
    */
    UpperProceduralMeshComponent->SetCollisionProfileName(TEXT("OverlapAll"));
    CylinderMeshComponent->SetCollisionProfileName(TEXT("OverlapAll"));
    // Enable physics simulation for the mesh
    //CylinderMeshComponent->SetSimulatePhysics(true);
    //UpperProceduralMeshComponent->SetSimulatePhysics(true); 
    //LowerProceduralMeshComponent->SetSimulatePhysics(true);
    

}

// Called when the game starts or when spawned
void ACylinderActor::BeginPlay()
{
	Super::BeginPlay();
    //Register the events
    CylinderMeshComponent->OnComponentBeginOverlap.AddDynamic(this, &ACylinderActor::OnSwordBeginOverlap);
    CylinderMeshComponent->OnComponentEndOverlap.AddDynamic(this, &ACylinderActor::OnSwordEndOverlap);

    
}

// Called every frame
void ACylinderActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    

    // Apply forces to each vertex of the cylinder mesh
    // You need to implement your custom logic to calculate the forces you want to apply to each vertex
    UStaticMesh* StaticMesh = CylinderMeshComponent->GetStaticMesh();
    if (StaticMesh == nullptr) return;

    FStaticMeshLODResources& LODModel = StaticMesh->GetRenderData()->LODResources[0];
    FIndexArrayView Indices = LODModel.IndexBuffer.GetArrayView();
    const uint32 NumSections = LODModel.Sections.Num();
    FVector Force;

    for (uint32 SectionIndex = 0; SectionIndex < NumSections; ++SectionIndex)
    {
        const FStaticMeshSection& Section = LODModel.Sections[SectionIndex];
        for (uint32 i = 0; i < Section.NumTriangles * 3; i++)
        {
            uint32 VertexIndex = Indices[Section.FirstIndex + i];
            FVector3f VertexPosition = LODModel.VertexBuffers.PositionVertexBuffer.VertexPosition(VertexIndex);
            FVector tmp(VertexPosition.X, VertexPosition.Y, VertexPosition.Z);
            // Calculate the force you want to apply to the vertex (e.g., gravity, wind, etc.)
            // Force = ...

            // Apply force to the vertex using Chaos Physics
            //CylinderMeshComponent->AddForceAtLocation(Force, tmp);
            //UpperProceduralMeshComponent->AddForceAtLocation(Force, tmp);
            //LowerProceduralMeshComponent->AddForceAtLocation(Force, tmp);
        }
    }
}


void ACylinderActor::OnSwordBeginOverlap(
    UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) 
{
    return;

}


void ACylinderActor::OnSwordEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
    FVector3d TmpNormalVector = FVector3d(0, 0, 1);
        //Cast<ASwordActor>(OtherActor)->GetNormalVector();
    FVector3d TmpPositionVector = FVector3d(49350.0, 64310.0, 6480.0);
        //GetActorLocation();
    UE_LOG(LogTemp, Error, TEXT("%f,%f,%f"), TmpPositionVector.X, TmpPositionVector.Y, TmpPositionVector.Z);
        //Cast<ASwordActor>(OtherActor)->GetActorLocation();
    UKismetProceduralMeshLibrary::SliceProceduralMesh(
        UpperProceduralMeshComponent, TmpPositionVector, TmpNormalVector, false, LowerProceduralMeshComponent, 
        EProcMeshSliceCapOption::NoCap, nullptr);
    UE_LOG(LogTemp, Error, TEXT("Slice?"));
};