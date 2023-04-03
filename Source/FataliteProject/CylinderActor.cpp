#include "CylinderActor.h"
#include "Components/StaticMeshComponent.h"
#include "StaticMeshResources.h"
#include "CoreFwd.h"
#include "KismetProceduralMeshLibrary.h"
#include "Rendering/SkinWeightVertexBuffer.h"


// Sets default values
ACylinderActor::ACylinderActor()
{
    PrimaryActorTick.bCanEverTick = true;

    // Component Setting
    CylinderMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CylinderMeshComponent"));
    UpperProceduralMeshComponent = CreateDefaultSubobject <UProceduralMeshComponent>(TEXT("UpperProceduralMesh"));
    LowerProceduralMeshComponent = CreateDefaultSubobject <UProceduralMeshComponent>(TEXT("LowerProceduralMesh"));
    RootComponent = UpperProceduralMeshComponent;
    CylinderMeshComponent->SetupAttachment(RootComponent);
    LowerProceduralMeshComponent->SetupAttachment(RootComponent);

    // Load cylinder static mesh
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("StaticMesh'/Engine/BasicShapes/Cylinder.Cylinder'"));
        CylinderMeshComponent->SetStaticMesh(CylinderMesh.Object);

    // Load Materials
    static ConstructorHelpers::FObjectFinder<UMaterial> CyanMat(TEXT("Material'/Game/CyanWireFrameMat.CyanWireFrameMat'"));
    UpperProceduralMeshComponent->SetMaterial(0, CyanMat.Object);
    static ConstructorHelpers::FObjectFinder<UMaterial> VioletMat(TEXT("Material'/Game/VioletWireFrameMat.VioletWireFrameMat'"));
    LowerProceduralMeshComponent->SetMaterial(0, VioletMat.Object); 

    // Read the OBJ file
    
    FString FilePath("C:\\Users\\ugly2\\Downloads\\tetgen1.6.0\\tetgen1.6.0\\Hand.1.node");
    if (LoadNodes(FilePath, Vertices)) {
        UE_LOG(LogTemp, Log, TEXT("Succeed to load Vertice(Node) file: %s"), *FilePath);
    }
    else {
        UE_LOG(LogTemp, Error, TEXT("Failed to load Vertice(Node) file: %s"), *FilePath);
        return;
    }
    FilePath = "C:\\Users\\ugly2\\Downloads\\tetgen1.6.0\\tetgen1.6.0\\Hand.1.face";
    if (LoadFaces(FilePath, Triangles)) {
        UE_LOG(LogTemp, Log, TEXT("Succeed to load Triangles(Face) file: %s"), *FilePath);
    }
    else {
        UE_LOG(LogTemp, Error, TEXT("Failed to load Triangles(Face) file: %s"), *FilePath);
        return;
    }
    FilePath = "C:\\Users\\ugly2\\Downloads\\tetgen1.6.0\\tetgen1.6.0\\Hand.1.ele";
    if (LoadTetrahedrons(FilePath, Tetrahedra)) {
        UE_LOG(LogTemp, Log, TEXT("Succeed to load Tetrahedrons file: %s"), *FilePath);
    }
    else {
        UE_LOG(LogTemp, Error, TEXT("Failed to load Tetrahedrons file: %s"), *FilePath);
        return;
    }


    UpperProceduralMeshComponent->ClearAllMeshSections();
    UpperProceduralMeshComponent->CreateMeshSection_LinearColor(0, Vertices, Triangles, TArray<FVector>(), TArray<FVector2D>(), TArray<FLinearColor>(), TArray<FProcMeshTangent>(), false);
    LowerProceduralMeshComponent->CreateMeshSection_LinearColor(0, Vertices, Triangles, TArray<FVector>(), TArray<FVector2D>(), TArray<FLinearColor>(), TArray<FProcMeshTangent>(), false);
    LowerProceduralMeshComponent->ClearAllMeshSections();
    UpperProceduralMeshComponent->SetCollisionProfileName(TEXT("OverlapAll"));
    LowerProceduralMeshComponent->SetCollisionProfileName(TEXT("OverlapAll"));
    CylinderMeshComponent->SetCollisionProfileName(TEXT("OverlapAll"));
    UpperProceduralMeshComponent->bUseComplexAsSimpleCollision = false;
    LowerProceduralMeshComponent->bUseComplexAsSimpleCollision = false;
    LowerProceduralMeshComponent->AddCollisionConvexMesh(Vertices);
    UpperProceduralMeshComponent->AddCollisionConvexMesh(Vertices);
    UpperProceduralMeshComponent->SetSimulatePhysics(false); 
    LowerProceduralMeshComponent->SetSimulatePhysics(false);
    CylinderMeshComponent->SetSimulatePhysics(false);
    CylinderMeshComponent->SetVisibility(false);
}

// Called when the game starts or when spawned
void ACylinderActor::BeginPlay()
{
	Super::BeginPlay();
    //Register the events
    UpperProceduralMeshComponent->OnComponentBeginOverlap.AddDynamic(this, &ACylinderActor::OnSwordBeginOverlap);
    UpperProceduralMeshComponent->OnComponentEndOverlap.AddDynamic(this, &ACylinderActor::OnSwordEndOverlap);

    
}

// Called every frame
void ACylinderActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
    TArray<AActor*> OverlappingActors;
    GetOverlappingActors(OverlappingActors);
    for (int j = 0; j < OverlappingActors.Num(); j++) {
        if (Cast<ASwordActor>(OverlappingActors[j])->StaticClass() == ASwordActor::StaticClass()) {
            FindIntersectionTetrahedrons(Cast<ASwordActor>(OverlappingActors[j])->GetPreviousStartLocation(), Cast<ASwordActor>(OverlappingActors[j])->GetPreviousEndLocation(),
                Cast<ASwordActor>(OverlappingActors[j])->GetCurrentStartLocation(), Cast<ASwordActor>(OverlappingActors[j])->GetCurrentEndLocation());


            UE_LOG(LogTemp, Error, TEXT("Number of Tetrahedrons: %d"), Tetrahedra.Num());
            int cnt = 0;
            TArray<FVector> v;
            TArray<int32> f;
            for (FTetrahedron& tet : Tetrahedra) {
                if (tet.isIntersection) {
                    cnt++;
                    v.Add(Vertices[tet.Vertices[0]]);
                    v.Add(Vertices[tet.Vertices[1]]);
                    v.Add(Vertices[tet.Vertices[2]]);
                    v.Add(Vertices[tet.Vertices[3]]);
                }
                else {

                }
                tet.ClearIntersectionInformation();

            }
            for (int i = 0; i <= v.Num(); i = i + 4) {
                //abc
                f.Add(i);
                f.Add(i + 2);
                f.Add(i + 1);
                
                //bcd
                f.Add(i + 1);
                f.Add(i + 3);
                f.Add(i + 2);
                //acd
                f.Add(i);
                f.Add(i + 3);
                f.Add(i + 2);
                //abd
                f.Add(i);
                f.Add(i + 3);
                f.Add(i + 1);
            }
            
            LowerProceduralMeshComponent->CreateMeshSection_LinearColor(LowerProceduralMeshComponent->GetNumSections()+1, v, f, TArray<FVector>(), TArray<FVector2D>(), TArray<FLinearColor>(), TArray<FProcMeshTangent>(), false);
            UE_LOG(LogTemp, Error, TEXT("Intersection Tetrahedron : %d"), cnt);
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
    if (Cast<ASwordActor>(OtherActor) != nullptr) {
        UpperProceduralMeshComponent->SetVisibility(false);
    }
};

bool ACylinderActor::LoadNodes(FString FilePath, TArray<FVector>& fv) {
    FString FileContent;
    if (!FFileHelper::LoadFileToString(FileContent, *FilePath))
    {
        return false;
    }
    else {
        TArray<FString> lines;
        int32 lineCount = FileContent.ParseIntoArray(lines, TEXT("\n"), true);
        FileContent.Empty();
        TArray<FString> lineElements;
        for (int i = 1; i < lineCount - 1; i++) {
            FString Line = lines[i].TrimStartAndEnd();
            lineElements.Empty();
            int32 elementCount = Line.ParseIntoArray(lineElements, TEXT(" "), true);
            fv.Add(FVector(FCString::Atod(*lineElements[1]), FCString::Atod(*lineElements[2]), FCString::Atod(*lineElements[3])));
        }
        return true;
    }
}
bool ACylinderActor::LoadFaces(FString FilePath, TArray<int32>& fv) {
    FString FileContent;
    if (!FFileHelper::LoadFileToString(FileContent, *FilePath))
    {
        return false;
    }
    else {
        TArray<FString> lines;
        int32 lineCount = FileContent.ParseIntoArray(lines, TEXT("\n"), true);
        FileContent.Empty();
        TArray<FString> lineElements;
        for (int i = 1; i < lineCount - 1; i++) {
            FString Line = lines[i].TrimStartAndEnd();
            lineElements.Empty();
            int32 elementCount = Line.ParseIntoArray(lineElements, TEXT(" "), true);
            fv.Add((FCString::Atoi(*lineElements[1]) - 1));
            fv.Add(FCString::Atoi(*lineElements[3]) - 1);
            fv.Add(FCString::Atoi(*lineElements[2]) - 1);
        }
        return true;
    }
}
bool ACylinderActor::LoadTetrahedrons(FString FilePath, TArray<FTetrahedron>& tt) {
    FString FileContent;
    if (!FFileHelper::LoadFileToString(FileContent, *FilePath))
    {
        return false;
    }
    else {
        TArray<FString> lines;
        int32 lineCount = FileContent.ParseIntoArray(lines, TEXT("\n"), true);
        FileContent.Empty();
        TArray<FString> lineElements;
        for (int i = 1; i < lineCount - 1; i++) {
            FString Line = lines[i].TrimStartAndEnd();
            lineElements.Empty();
            int32 elementCount = Line.ParseIntoArray(lineElements, TEXT(" "), true);
            tt.Add(FTetrahedron((FCString::Atoi(*lineElements[1]) - 1), (FCString::Atoi(*lineElements[2]) - 1), (FCString::Atoi(*lineElements[3]) - 1), (FCString::Atoi(*lineElements[4]) - 1)));
            
        }
        return true;
    }
}
bool ACylinderActor::RayTriangleIntersect(const FVector& origin, const FVector& direction,
    const FVector& v0, const FVector& v1, const FVector& v2,
    float& t, float& u, float& v) {
    
    const float EPSILON = std::numeric_limits<float>::epsilon();
    FVector edge1 = v1 - v0;
    FVector edge2 = v2 - v0;
    FVector h = direction.Cross(edge2);
    float a = edge1.Dot(h);

    if (std::abs(a) < EPSILON) {
        return false;
    }

    float f = 1.0f / a;
    FVector s = origin - v0;
    u = f * s.Dot(h);

    if (u < 0.0f || u > 1.0f) {
        return false;
    }

    FVector q = s.Cross(edge1);
    v = f * direction.Dot(q);

    if (v < 0.0f || u + v > 1.0f) {
        return false;
    }

    t = f * edge2.Dot(q);
    return t >= 0.0f && t <= direction.Length();
}

void ACylinderActor::FindIntersectionTetrahedrons(FVector& PreviousStartVector, FVector& PreviousEndVector, FVector& CurrentStartVector, FVector& CurrentEndVector) {
    for (FTetrahedron& tet : Tetrahedra) {
        //CASE0. 0 1(AB)
        tet.EdgeIntersection[0] = RayTriangleIntersect(Vertices[tet.Vertices[0]]+GetActorLocation(),
            Vertices[tet.Vertices[1]] - Vertices[tet.Vertices[0]], 
            PreviousStartVector, PreviousEndVector, CurrentStartVector, 
            tet.EdgeIntersectionPoint[0].X, tet.EdgeIntersectionPoint[0].Y, tet.EdgeIntersectionPoint[0].Z);

        if (tet.EdgeIntersection[0] == false) {
            tet.EdgeIntersection[0] = RayTriangleIntersect(Vertices[tet.Vertices[0]] + GetActorLocation(),
                Vertices[tet.Vertices[2]] - Vertices[tet.Vertices[0]],
                PreviousStartVector, PreviousEndVector, CurrentEndVector,
                tet.EdgeIntersectionPoint[0].X, tet.EdgeIntersectionPoint[0].Y, tet.EdgeIntersectionPoint[0].Z);
        }
        //CASE1. 0 2(AC)
        tet.EdgeIntersection[1] = RayTriangleIntersect(Vertices[tet.Vertices[0]] + GetActorLocation(),
            Vertices[tet.Vertices[2]] - Vertices[tet.Vertices[0]],
            PreviousStartVector, PreviousEndVector, CurrentStartVector,
            tet.EdgeIntersectionPoint[1].X, tet.EdgeIntersectionPoint[1].Y, tet.EdgeIntersectionPoint[1].Z);

        if (tet.EdgeIntersection[1] == false) {
            tet.EdgeIntersection[1] = RayTriangleIntersect(Vertices[tet.Vertices[0]] + GetActorLocation(),
                Vertices[tet.Vertices[2]] - Vertices[tet.Vertices[0]],
                PreviousStartVector, PreviousEndVector, CurrentEndVector,
                tet.EdgeIntersectionPoint[1].X, tet.EdgeIntersectionPoint[1].Y, tet.EdgeIntersectionPoint[1].Z);
        }
        //CASE2. 0 3(A D)
        tet.EdgeIntersection[2] = RayTriangleIntersect(Vertices[tet.Vertices[0]] + GetActorLocation(),
            Vertices[tet.Vertices[3]] - Vertices[tet.Vertices[0]],
            PreviousStartVector, PreviousEndVector, CurrentStartVector,
            tet.EdgeIntersectionPoint[2].X, tet.EdgeIntersectionPoint[2].Y, tet.EdgeIntersectionPoint[2].Z);

        if (tet.EdgeIntersection[2] == false) {
            tet.EdgeIntersection[2] = RayTriangleIntersect(Vertices[tet.Vertices[0]] + GetActorLocation(),
                Vertices[tet.Vertices[3]] - Vertices[tet.Vertices[0]],
                PreviousStartVector, PreviousEndVector, CurrentEndVector,
                tet.EdgeIntersectionPoint[2].X, tet.EdgeIntersectionPoint[2].Y, tet.EdgeIntersectionPoint[2].Z);
        }
        //CASE3. 1 2(B C)
        tet.EdgeIntersection[3] = RayTriangleIntersect(Vertices[tet.Vertices[1]] + GetActorLocation(),
            Vertices[tet.Vertices[2]] - Vertices[tet.Vertices[1]],
            PreviousStartVector, PreviousEndVector, CurrentStartVector,
            tet.EdgeIntersectionPoint[3].X, tet.EdgeIntersectionPoint[3].Y, tet.EdgeIntersectionPoint[3].Z);

        if (tet.EdgeIntersection[3] == false) {
            tet.EdgeIntersection[3] = RayTriangleIntersect(Vertices[tet.Vertices[1]] + GetActorLocation(),
                Vertices[tet.Vertices[2]] - Vertices[tet.Vertices[1]],
                PreviousStartVector, PreviousEndVector, CurrentEndVector,
                tet.EdgeIntersectionPoint[3].X, tet.EdgeIntersectionPoint[3].Y, tet.EdgeIntersectionPoint[3].Z);
        }
        //CASE4. 1 3(B D)
        tet.EdgeIntersection[4] = RayTriangleIntersect(Vertices[tet.Vertices[1]] + GetActorLocation(),
            Vertices[tet.Vertices[3]] - Vertices[tet.Vertices[1]],
            PreviousStartVector, PreviousEndVector, CurrentStartVector,
            tet.EdgeIntersectionPoint[4].X, tet.EdgeIntersectionPoint[4].Y, tet.EdgeIntersectionPoint[4].Z);

        if (tet.EdgeIntersection[4] == false) {
            tet.EdgeIntersection[4] = RayTriangleIntersect(Vertices[tet.Vertices[1]] + GetActorLocation(),
                Vertices[tet.Vertices[3]] - Vertices[tet.Vertices[1]],
                PreviousStartVector, PreviousEndVector, CurrentEndVector,
                tet.EdgeIntersectionPoint[4].X, tet.EdgeIntersectionPoint[4].Y, tet.EdgeIntersectionPoint[4].Z);
        }
        //CASE5. 2 3(C D)
        tet.EdgeIntersection[5] = RayTriangleIntersect(Vertices[tet.Vertices[2]] + GetActorLocation(),
            Vertices[tet.Vertices[3]] - Vertices[tet.Vertices[2]],
            PreviousStartVector, PreviousEndVector, CurrentStartVector,
            tet.EdgeIntersectionPoint[5].X, tet.EdgeIntersectionPoint[5].Y, tet.EdgeIntersectionPoint[5].Z);

        if (tet.EdgeIntersection[5] == false) {
            tet.EdgeIntersection[5] = RayTriangleIntersect(Vertices[tet.Vertices[2]] + GetActorLocation(),
                Vertices[tet.Vertices[3]] - Vertices[tet.Vertices[2]],
                PreviousStartVector, PreviousEndVector, CurrentEndVector,
                tet.EdgeIntersectionPoint[5].X, tet.EdgeIntersectionPoint[5].Y, tet.EdgeIntersectionPoint[5].Z);
        }
        if (tet.EdgeIntersection[0] || tet.EdgeIntersection[1] || tet.EdgeIntersection[2] || tet.EdgeIntersection[3] || tet.EdgeIntersection[4] || tet.EdgeIntersection[5]) {
            tet.isIntersection = true;
        }
    }
}
