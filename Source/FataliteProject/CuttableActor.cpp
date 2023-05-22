// Fill out your copyright notice in the Description page of Project Settings.


#include "CuttableActor.h"
#include "vector"

using namespace std;
// Sets default values
ACuttableActor::ACuttableActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    // Component Setting
    //CylinderMeshComponent = CreateDefaultSubobject <UStaticMeshComponent>(TEXT("ProceduralMesh"));
    ProceduralMeshComponent = CreateDefaultSubobject <UProceduralMeshComponent>(TEXT("UpperProceduralMesh"));
    RootComponent = ProceduralMeshComponent;
    // Load Materials
    static ConstructorHelpers::FObjectFinder<UMaterial> CyanMat(TEXT("Material'/Game/CyanWireFrameMat.CyanWireFrameMat'"));
    ProceduralMeshComponent->SetMaterial(0, CyanMat.Object);
    // Read the OBJ file

    FString FilePath("C:\\Users\\ugly2\\Downloads\\tetgen1.6.0\\tetgen1.6.0\\cylinder.1.node");
    if (LoadNodes(FilePath, Vertices)) {
        UE_LOG(LogTemp, Log, TEXT("Succeed to load Vertice(Node) file: %s"), *FilePath);
    }
    else {
        UE_LOG(LogTemp, Error, TEXT("Failed to load Vertice(Node) file: %s"), *FilePath);
        return;
    }
    FilePath = "C:\\Users\\ugly2\\Downloads\\tetgen1.6.0\\tetgen1.6.0\\cylinder.1.face";
    if (LoadFaces(FilePath, Triangles)) {
        UE_LOG(LogTemp, Log, TEXT("Succeed to load Triangles(Face) file: %s"), *FilePath);
    }
    else {
        UE_LOG(LogTemp, Error, TEXT("Failed to load Triangles(Face) file: %s"), *FilePath);
        return;
    }
    FilePath = "C:\\Users\\ugly2\\Downloads\\tetgen1.6.0\\tetgen1.6.0\\cylinder.1.edge";
    if (LoadEdges(FilePath)) {
        UE_LOG(LogTemp, Log, TEXT("Succeed to load Tetrahedrons file: %s"), *FilePath);
    }
    else {
        UE_LOG(LogTemp, Error, TEXT("Failed to load Tetrahedrons file: %s"), *FilePath);
        return;
    }

    ProceduralMeshComponent->ClearAllMeshSections();
    ProceduralMeshComponent->CreateMeshSection_LinearColor(0, Vertices, Triangles, TArray<FVector>(), TArray<FVector2D>(), TArray<FLinearColor>(), TArray<FProcMeshTangent>(), false);

    ProceduralMeshComponent->SetCollisionProfileName(TEXT("OverlapAll"));
    ProceduralMeshComponent->bUseComplexAsSimpleCollision = false;
    ProceduralMeshComponent->AddCollisionConvexMesh(Vertices);
    ProceduralMeshComponent->SetSimulatePhysics(false);
}

// Called when the game starts or when spawned
void ACuttableActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACuttableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
    SimulatingMassSpringDamper(DeltaTime);
    TArray<FVector> v;
    TArray<int32> t;
    for (Node n : Nodes) {
        v.Add(n.Position);
    }
    ProceduralMeshComponent->ClearMeshSection(0);
    ProceduralMeshComponent->CreateMeshSection_LinearColor(0,
        v, Triangles, TArray<FVector>(), TArray<FVector2D>(), TArray<FLinearColor>(), TArray<FProcMeshTangent>(), false);
}

bool ACuttableActor::LoadNodes(FString FilePath, TArray<FVector>& fv) {
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
bool ACuttableActor::LoadFaces(FString FilePath, TArray<int32>& fv) {
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
bool ACuttableActor::LoadEdges(FString FilePath) {
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
                    FCString::Atoi(*lineElements[1]) - 1,
                    FCString::Atoi(*lineElements[2]) - 1,
                    FVector::Distance(
                        Vertices[FCString::Atoi(*lineElements[1]) - 1], Vertices[FCString::Atoi(*lineElements[2]) - 1])
                    , 1));
        }
        return true;
    }

}

void ACuttableActor::AddNodes(Node n) {
    Nodes.Add(n);
}
void ACuttableActor::AddSprings(Spring s) {
    Springs.Add(s);
}
void ACuttableActor::SimulatingMassSpringDamper(double dt) {

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
FVector ACuttableActor::Acceleration(int i, const FVector& pos, const FVector& vel) {
    FVector force = FVector(0, 0, -9.8);

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