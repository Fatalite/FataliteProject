#include "CylinderActor.h"
#include "Components/StaticMeshComponent.h"
#include "StaticMeshResources.h"
#include "CoreFwd.h"
#include "KismetProceduralMeshLibrary.h"
#include "Rendering/SkinWeightVertexBuffer.h"
//#include "FMyAsyncSimulationTask.h"
#include "RealtimeMeshComponent.h"
#include "RealtimeMeshLibrary.h"
#include "RealtimeMeshActor.h"
#include "Async/Async.h"


#define d 3
#define T float
#define TV TArray<T, 3>
#define T2 TArray<T, 2>
#define T4 TArray<T, 4>
#define Idp1 TArray<int, 4>
#define Id TArray<int, 3>
#define I3 TArray<int, 3>
#define I2 TArray<int, 2>
#define I4 TArray<int, 4>


// Sets default values
ACylinderActor::ACylinderActor()
{
    PrimaryActorTick.bCanEverTick = true;

    // Component Setting
    //CylinderMeshComponent = CreateDefaultSubobject <UStaticMeshComponent>(TEXT("ProceduralMesh"));
    UpperProceduralMeshComponent = CreateDefaultSubobject <UProceduralMeshComponent>(TEXT("UpperProceduralMesh"));
    RootComponent = UpperProceduralMeshComponent;
    LowerProceduralMeshComponent = CreateDefaultSubobject <UProceduralMeshComponent>(TEXT("LowerProceduralMesh"));
    LowerProceduralMeshComponent->SetupAttachment(RootComponent);
    //CylinderMeshComponent->SetupAttachment(RootComponent);
    CuttedSurfaceProceduralMeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("CuttedSurfacePMesh"));
    CuttedSurfaceProceduralMeshComponent->SetupAttachment(RootComponent);
    // Load Materials
    static ConstructorHelpers::FObjectFinder<UMaterial> CyanMat(TEXT("Material'/Game/CyanWireFrameMat.CyanWireFrameMat'"));
    static ConstructorHelpers::FObjectFinder<UMaterial> VioletMat(TEXT("Material'/Game/VioletWireFrameMat.VioletWireFrameMat'"));

    UpperProceduralMeshComponent->SetMaterial(0, CyanMat.Object);
    LowerProceduralMeshComponent->SetMaterial(0, VioletMat.Object);
    CuttedSurfaceProceduralMeshComponent->SetMaterial(0, VioletMat.Object);
    // Read the OBJ file

    FString FilePath("C:\\Users\\ugly2\\Downloads\\tetgen1.6.0\\tetgen1.6.0\\Tetrahedron.1.node");
    if (LoadNodes(FilePath, Vertices)) {
        UE_LOG(LogTemp, Log, TEXT("Succeed to load Vertice(Node) file: %s"), *FilePath);
    }
    else {
        UE_LOG(LogTemp, Error, TEXT("Failed to load Vertice(Node) file: %s"), *FilePath);
        return;
    }
    FilePath = "C:\\Users\\ugly2\\Downloads\\tetgen1.6.0\\tetgen1.6.0\\Tetrahedron.1.face";
    if (LoadFaces(FilePath, Triangles)) {
        UE_LOG(LogTemp, Log, TEXT("Succeed to load Triangles(Face) file: %s"), *FilePath);
    }
    else {
        UE_LOG(LogTemp, Error, TEXT("Failed to load Triangles(Face) file: %s"), *FilePath);
        return;
    }
    FilePath = "C:\\Users\\ugly2\\Downloads\\tetgen1.6.0\\tetgen1.6.0\\Tetrahedron.1.ele";
    if (LoadTetrahedrons(FilePath, Tetrahedra)) {
        UE_LOG(LogTemp, Log, TEXT("Succeed to load Tetrahedrons file: %s"), *FilePath);
    }
    else {
        UE_LOG(LogTemp, Error, TEXT("Failed to load Tetrahedrons file: %s"), *FilePath);
        return;
    }
    FilePath = "C:\\Users\\ugly2\\Downloads\\tetgen1.6.0\\tetgen1.6.0\\Tetrahedron.1.edge";
    if (LoadEdges(FilePath)) {
        UE_LOG(LogTemp, Log, TEXT("Succeed to load Tetrahedrons file: %s"), *FilePath);
    }
    else {
        UE_LOG(LogTemp, Error, TEXT("Failed to load Tetrahedrons file: %s"), *FilePath);
        return;
    }
    tetMesh.nodes_.clear();
    tetMesh.mesh_.clear();
    int width = 5;
    int height = 5;
    int depth = 5;
    float low = -100;
    float high = 100;
    float left = -100;
    float right = 100;
    float ff = 120;
    float nn = -100;
    float lw = (right - left) / width;
    float lh = (high - low) / height;
    float ld = (ff - nn) / depth;
    int offset = (width + 1) * (depth + 1);
    float OnGroundHeight = 250;
    for (int i = 0; i < height + 1; i++) {
        //cout << "height: " << low+i*lh << endl;
        for (int j = 0; j < width + 1; j++) {
            for (int k = 0; k < depth + 1; k++) {
                tetMesh.nodes_.push_back({ left + j * lw, low + i * lh,nn + k * ld + OnGroundHeight });
            }
        }
    }

    int tetIndex = 0;
    for (int k = 0; k < height; k++) {
        for (int i = 0; i < width; i++) {
            for (int j = 0; j < depth; j++) {
                tetMesh.mesh_.push_back({ tetIndex + offset + 1, tetIndex + 1, tetIndex, tetIndex + depth + 1 });
                tetMesh.mesh_.push_back({ tetIndex, tetIndex + offset, tetIndex + offset + 1, tetIndex + depth + 1 });
                tetMesh.mesh_.push_back({ tetIndex + depth + 1, tetIndex + offset, tetIndex + offset + 1, tetIndex + offset + depth + 1 });
                tetMesh.mesh_.push_back({ tetIndex + 1, tetIndex + depth + 1, tetIndex + offset + 1, tetIndex + depth + 2 });
                tetMesh.mesh_.push_back({ tetIndex + depth + 1, tetIndex + depth + 2, tetIndex + offset + depth + 2, tetIndex + offset + 1 });
                tetMesh.mesh_.push_back({ tetIndex + depth + 1, tetIndex + depth + 2 + offset, tetIndex + offset + depth + 1, tetIndex + offset + 1 });
                tetIndex++;
            }
            tetIndex++;
        }
        tetIndex += (depth + 1);
    }
    tetMesh.initializeSurfaceMesh();
    tetMesh.computeConnectedComponents();
    TArray<FVector> v;
    TArray<int32> t;
    for (array<double, 3> ai : tetMesh.nodes_) {
        v.Add(FVector({ ai[0],ai[1],ai[2] }));
    }
    for (array<int, 3> ai : tetMesh.surfaceMesh_) {
        t.Add(ai[0]);
        t.Add(ai[1]);
        t.Add(ai[2]);
    }

    UpperProceduralMeshComponent->ClearMeshSection(0);
    UpperProceduralMeshComponent->CreateMeshSection_LinearColor(0, v, t, TArray<FVector>(), TArray<FVector2D>(), TArray<FLinearColor>(), TArray<FProcMeshTangent>(), false);

    LowerProceduralMeshComponent->ClearAllMeshSections();
    UpperProceduralMeshComponent->SetCollisionProfileName(TEXT("OverlapAll"));
    LowerProceduralMeshComponent->SetCollisionProfileName(TEXT("OverlapAll"));
    UpperProceduralMeshComponent->bUseComplexAsSimpleCollision = false;
    LowerProceduralMeshComponent->bUseComplexAsSimpleCollision = false;
    LowerProceduralMeshComponent->AddCollisionConvexMesh(Vertices);
    UpperProceduralMeshComponent->AddCollisionConvexMesh(v);
    UpperProceduralMeshComponent->SetSimulatePhysics(false);
    LowerProceduralMeshComponent->SetSimulatePhysics(false);
    //LowerProceduralMeshComponent->ClearMeshSection(0);
    //LowerProceduralMeshComponent->CreateMeshSection_LinearColor(0, Vertices, Triangles, TArray<FVector>(), TArray<FVector2D>(), TArray<FLinearColor>(), TArray<FProcMeshTangent>(), false);
}
// Called when the game starts or when spawned
void ACylinderActor::BeginPlay()
{
    Super::BeginPlay();
    //Register the events
    UpperProceduralMeshComponent->OnComponentBeginOverlap.AddDynamic(this, &ACylinderActor::OnSwordBeginOverlap);
    UpperProceduralMeshComponent->OnComponentEndOverlap.AddDynamic(this, &ACylinderActor::OnSwordEndOverlap);
    
    
   /* for (Node n : Nodes) {
        tetMesh.nodes_.push_back({ n.Position.X,n.Position.Y ,n.Position.Z });
    }
    for (FTetrahedron ft : Tetrahedra) {
        tetMesh.mesh_.push_back({ ft.Vertices[0],ft.Vertices[1],ft.Vertices[2],ft.Vertices[3] });
    }*/
    

}

// Called every frame
void ACylinderActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    TArray<AActor*> OverlappingActors;
    GetOverlappingActors(OverlappingActors);
    for (int j = 0; j < OverlappingActors.Num(); j++) {
        if (Cast<ASwordActor>(OverlappingActors[j])->StaticClass() == ASwordActor::StaticClass()) {
            
            if (ASwordActor* SwordActor = Cast<ASwordActor>(OverlappingActors[j])) {
                UE_LOG(LogTemp, Log, TEXT("Ticking"));
                cs = Cast<ASwordActor>(OverlappingActors[j])->GetCurrentStartLocation();
                ce = Cast<ASwordActor>(OverlappingActors[j])->GetCurrentEndLocation();
                
                
                triMesh.nodes_.push_back({ (cs[0]), (cs[1]), (cs[2]) });
                triMesh.nodes_.push_back({ (ce[0]), (ce[1]), (ce[2]) });
                int TriMeshSize = triMesh.nodes_.size();
                if (TriMeshSize >= 4) {
                    triMesh.mesh_.push_back({ TriMeshSize -4 ,TriMeshSize -2,TriMeshSize -3 });
                    triMesh.mesh_.push_back({ TriMeshSize -3,TriMeshSize -1,TriMeshSize -2 });
                }
                TArray<FVector> vs;
                TArray<int> ts;
                for (auto a : triMesh.nodes_) {
                    vs.Add(FVector(a[0], a[1], a[2]));
                }
                for (auto a : triMesh.mesh_) {
                    ts.Push(a[0]);
                    ts.Push(a[1]);
                    ts.Push(a[2]);
                }
                CuttedSurfaceProceduralMeshComponent->ClearMeshSection(0);
                CuttedSurfaceProceduralMeshComponent->CreateMeshSection_LinearColor(0, vs, ts, TArray<FVector>(), TArray<FVector2D>(), TArray<FLinearColor>(), TArray<FProcMeshTangent>(), false);
            }
        }
    }
    
}

void  ACylinderActor::OnSwordBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
    ASwordActor* SwordActor = Cast<ASwordActor>(OtherActor);
    if (SwordActor) {
        IsOverlappingSword = true;
    }
}
void ACylinderActor::OnSwordEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
    
    IsOverlappingSword = false;
    if (IsCutted) return;
    IsCutted = true;
    ASwordActor* SwordActor = Cast<ASwordActor>(OtherActor);
        if (SwordActor) {
            
            Cutter3D<double> cutter;
            tetMesh = cutter.run(tetMesh, triMesh);

            TArray<FVector3d> v;
            TArray<int> t;
            set<int> UpperParticles;
            int idx = 0;
            idx = tetMesh.connectedComponents_[0];
            int qq = 0;
            UpperParticles.clear();
            tetMesh.computeConnectedComponents();
            for (int i = 0; i < tetMesh.connectedComponents_.size(); ++i) {
                if (tetMesh.connectedComponents_[i] == (int)idx) {
                    for (auto j : tetMesh.mesh_[i]) {
                        
                        UpperParticles.insert(j);
                    }
                }
            }
            if (UpperParticles.size()) {
                for (auto i : UpperParticles) {
                    tetMesh.nodes_[i] = add<double, 3>(tetMesh.nodes_[i], { 0, 0, +100 });
                }
            }

            for (auto a : tetMesh.nodes_) {
                v.Push({ a[0], a[1], a[2] });
            }
            for (auto a : tetMesh.surfaceMesh_) {
                t.Push(a[0]);
                t.Push(a[1]);
                t.Push(a[2]);
            }
            

            UpperProceduralMeshComponent->ClearMeshSection(0);
            UpperProceduralMeshComponent->CreateMeshSection_LinearColor(0, v, t, TArray<FVector>(), TArray<FVector2D>(), TArray<FLinearColor>(), TArray<FProcMeshTangent>(), false);
        }
    // Find Intersections
    TArray<FVector> AdditionalVertices;
    TArray<int32> AdditionalFaces;
    UE_LOG(LogTemp, Log, TEXT("End Overlap"));
    if (false) {
        for (FTetrahedron& ft : Tetrahedra) {
            //Find Previous Active Tetrahedron And Find Edge Intersections
            //FindIntersectionTetrahedrons(Cast<ASwordActor>(OtherActor)->GetPreviousStartLocation(), Cast<ASwordActor>(OtherActor)->GetPreviousEndLocation(),
            //    Cast<ASwordActor>(OtherActor)->GetCurrentStartLocation(), Cast<ASwordActor>(OtherActor)->GetCurrentEndLocation());
            //만약 이전 포인트 사면체랑 다르면, Face Point를 저장해줘야함.
                    //Current End Vector - Previous End Vector is Line Segment
                    //Triangle is Shared Face;
                    //Finding Shared Face between two Tetrahedrons
            TArray<int32> SharedVertices;
            for (int ii = 0; ii < 4; ii++) {
                for (int jj = 0; jj < 4; jj++) {
                    if (ft.Vertices[ii] == ft.Vertices[jj]) {
                        SharedVertices.Add(ft.Vertices[ii]);
                    }
                }
            }
            float x;
            float y;
            float z;
            /*
             RayTriangleIntersect(Cast<ASwordActor>(OverlappingActors[j])->GetPreviousEndLocation(),
                 Cast<ASwordActor>(OverlappingActors[j])->GetCurrentEndLocation() - Cast<ASwordActor>(OverlappingActors[j])->GetPreviousEndLocation(),
                 Vertices[SharedVertices[0]], Vertices[SharedVertices[1]], Vertices[SharedVertices[2]],
                 x, y, z);
            */
            //Now Not used;
            FVector NewFacePoint(x, y, z);
            //Look Up Fucking Table; 
            bool AB[6] = { 1,0,0,0,0,0 };
            for (int m = 0; m < 6; m++) {
                //UE_LOG(LogTemp, Error, TEXT("%d"), AB[m]);
            }
            //Case 0, Only Intersection of AB
            //AB == ft.EdgeIntersection
            if (false) {
                //UE_LOG(LogTemp, Error, TEXT("AB Edge!"));
                //Insert A,B,C,D Vertices
                AdditionalVertices.Add(Vertices[ft.Vertices[0]]);
                AdditionalVertices.Add(Vertices[ft.Vertices[1]]);
                AdditionalVertices.Add(Vertices[ft.Vertices[2]]);
                AdditionalVertices.Add(Vertices[ft.Vertices[3]]);
                //Insert Mid Point of AB,AC,AD,BC,BD,CD Vertices [4-9]
                AdditionalVertices.Add((Vertices[ft.Vertices[0]] + Vertices[ft.Vertices[1]]) / 2);
                AdditionalVertices.Add((Vertices[ft.Vertices[0]] + Vertices[ft.Vertices[2]]) / 2);
                AdditionalVertices.Add((Vertices[ft.Vertices[0]] + Vertices[ft.Vertices[3]]) / 2);
                AdditionalVertices.Add((Vertices[ft.Vertices[1]] + Vertices[ft.Vertices[2]]) / 2);
                AdditionalVertices.Add((Vertices[ft.Vertices[1]] + Vertices[ft.Vertices[3]]) / 2);
                AdditionalVertices.Add((Vertices[ft.Vertices[2]] + Vertices[ft.Vertices[3]]) / 2);
                //Insert Mid Point of (ABC, ABD) Vertices [10],[11]
                AdditionalVertices.Add((Vertices[ft.Vertices[0]] + Vertices[ft.Vertices[1]] + Vertices[ft.Vertices[2]]) / 2);
                AdditionalVertices.Add((Vertices[ft.Vertices[0]] + Vertices[ft.Vertices[1]] + Vertices[ft.Vertices[3]]) / 2);

                //Vertices Num = 12(0~11);
            // Face ACD, CBD(Not Split)
            //ACD
                AdditionalFaces.Add(0);
                AdditionalFaces.Add(2);
                AdditionalFaces.Add(3);
                //CBD
                AdditionalFaces.Add(1);
                AdditionalFaces.Add(3);
                AdditionalFaces.Add(2);
                //Splitted Face
                AdditionalFaces.Add(4);
                AdditionalFaces.Add(10);
                AdditionalFaces.Add(11);
                AdditionalFaces.Add(4);
                AdditionalFaces.Add(11);
                AdditionalFaces.Add(10);
                //Face ACB to 6 Faces
                AdditionalFaces.Add(0);
                AdditionalFaces.Add(10);
                AdditionalFaces.Add(5);

                AdditionalFaces.Add(5);
                AdditionalFaces.Add(10);
                AdditionalFaces.Add(2);

                AdditionalFaces.Add(10);
                AdditionalFaces.Add(7);
                AdditionalFaces.Add(2);

                AdditionalFaces.Add(10);
                AdditionalFaces.Add(1);
                AdditionalFaces.Add(7);

                AdditionalFaces.Add(4);
                AdditionalFaces.Add(1);
                AdditionalFaces.Add(10);

                AdditionalFaces.Add(0);
                AdditionalFaces.Add(4);
                AdditionalFaces.Add(10);
                //Face ADB to 6 Faces
                AdditionalFaces.Add(0);
                AdditionalFaces.Add(6);
                AdditionalFaces.Add(11);

                AdditionalFaces.Add(6);
                AdditionalFaces.Add(3);
                AdditionalFaces.Add(11);

                AdditionalFaces.Add(11);
                AdditionalFaces.Add(3);
                AdditionalFaces.Add(8);

                AdditionalFaces.Add(11);
                AdditionalFaces.Add(8);
                AdditionalFaces.Add(1);

                AdditionalFaces.Add(4);
                AdditionalFaces.Add(11);
                AdditionalFaces.Add(1);

                AdditionalFaces.Add(0);
                AdditionalFaces.Add(11);
                AdditionalFaces.Add(4);

            }

        }
            /*
            
            */
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
            Nodes.Push(Node(FVector(FCString::Atod(*lineElements[1]), FCString::Atod(*lineElements[2]), FCString::Atod(*lineElements[3])), FVector::Zero(), 1));
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
bool ACylinderActor::LoadEdges(FString FilePath){
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
            Nodes[FCString::Atoi(*lineElements[1]) - 1].ConnectedSprings.Add(FCString::Atoi(*lineElements[2]) - 1);
            Nodes[FCString::Atoi(*lineElements[2]) - 1].ConnectedSprings.Add(FCString::Atoi(*lineElements[1]) - 1);
            Springs.Add(
                Spring(
                    FCString::Atoi(*lineElements[1])-1, 
                    FCString::Atoi(*lineElements[2])-1, 
                    FVector::Distance(
                        Vertices[FCString::Atoi(*lineElements[1]) - 1], Vertices[FCString::Atoi(*lineElements[2]) - 1])
                    , 1));
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
bool ACylinderActor::SameSide(FVector v1, FVector v2, FVector v3, FVector v4, FVector p) {
    FVector Normal = FVector::CrossProduct(v2 - v1, v3 - v1);
    double dotV4 = FVector::DotProduct(Normal, v4 - v1);
    double dotP = FVector::DotProduct(Normal, p - v1);
    return ((dotV4 > 0) ? true : false) == ((dotP > 0) ? true : false);
}
bool ACylinderActor::PointInTetrahedron(FVector v1, FVector v2, FVector v3, FVector v4, FVector p) {
    return SameSide(v1, v2, v3, v4, p) &&
        SameSide(v2, v3, v4, v1, p) &&
        SameSide(v3, v4, v1, v2, p) &&
        SameSide(v4, v1, v2, v3, p);
}
void ACylinderActor::AddNodes(Node n) {
    Nodes.Add(n);
}
void ACylinderActor::AddSprings(Spring s) {
    Springs.Add(s);
}
void ACylinderActor::SimulatingMassSpringDamper(double dt) {

    vector<FVector> k1_pos(Nodes.Num());
    vector<FVector> k1_vel(Nodes.Num());
    vector<FVector> k2_pos(Nodes.Num());
    vector<FVector> k2_vel(Nodes.Num());


    vector<FVector> k3_pos(Nodes.Num());
    vector<FVector> k3_vel(Nodes.Num());
    vector<FVector> k4_pos(Nodes.Num());
    vector<FVector> k4_vel(Nodes.Num());

    
    ParallelFor(Nodes.Num(), [&](int32 i) {
        k1_pos[i] = Nodes[i].Velocity;
        k1_vel[i] = Acceleration(i);
        });

    // Parallelize k2 calculation
    ParallelFor(Nodes.Num(), [&](int32 i) {
        k2_pos[i] = Nodes[i].Velocity + 0.5 * dt * k1_vel[i];
        k2_vel[i] = Acceleration(i, Nodes[i].Position + 0.5 * dt * k1_pos[i], Nodes[i].Velocity + 0.5 * dt * k1_vel[i]);
        });

    // Parallelize k3 calculation
    ParallelFor(Nodes.Num(), [&](int32 i) {
        k3_pos[i] = Nodes[i].Velocity + 0.5 * dt * k2_vel[i];
        k3_vel[i] = Acceleration(i, Nodes[i].Position + 0.5 * dt * k2_pos[i], Nodes[i].Velocity + 0.5 * dt * k2_vel[i]);
        });

    // Parallelize k4 calculation
    ParallelFor(Nodes.Num(), [&](int32 i) {
        k4_pos[i] = Nodes[i].Velocity + dt * k3_vel[i];
        k4_vel[i] = Acceleration(i, Nodes[i].Position + dt * k3_pos[i], Nodes[i].Velocity + dt * k3_vel[i]);
        });

    // Parallelize position and velocity updates
    ParallelFor(Nodes.Num(), [&](int32 i) {
        Nodes[i].Position += dt / 6.0 * (k1_pos[i] + 2 * k2_pos[i] + 2 * k3_pos[i] + k4_pos[i]);
        Nodes[i].Velocity += dt / 6.0 * (k1_vel[i] + 2 * k2_vel[i] + 2 * k3_vel[i] + k4_vel[i]);
        });


}
FVector ACylinderActor::Acceleration(int i, const FVector& pos, const FVector& vel) {
    FVector force = FVector(0, 0, 9.8);

    for (int springIndex = 0; springIndex < Springs.Num(); ++springIndex) {
        const Spring& spring = Springs[springIndex];
        if (spring.mass1 == i || spring.mass2 == i) {
            int other_mass = (spring.mass1 == i) ? spring.mass2 : spring.mass1;
            FVector rel_pos = (pos == FVector::Zero()) ? Nodes[i].Position - Nodes[other_mass].Position : pos - Nodes[other_mass].Position;
            double distance = rel_pos.Size();
            FVector dir = rel_pos.GetSafeNormal();
            force += -spring.spring_const * (distance - spring.rest_length) * dir;
        }
    }

    force += (-1) * ((vel == FVector::Zero()) ? Nodes[i].Velocity : vel);
    return force / Nodes[i].Mass;
}
