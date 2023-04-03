// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "SwordActor.h"
#include "CylinderActor.generated.h"

struct FTetrahedron
{
    int32 Vertices[4];
    bool isIntersection;
    bool EdgeIntersection[6];
    FVector3f EdgeIntersectionPoint[6];

    FTetrahedron()
    {
        for (int i = 0; i < 6; i++) {
            EdgeIntersection[i] = false;
        }
        for (int i = 0; i < 4; ++i)
        {
            Vertices[i] = -1;
        }
        isIntersection = false;
    }

    FTetrahedron(int32 A, int32 B, int32 C, int32 D)
    {
        for (int i = 0; i < 6; i++) {
            EdgeIntersection[i] = false;
        }
        isIntersection = false;
        Vertices[0] = A;
        Vertices[1] = B;
        Vertices[2] = C;
        Vertices[3] = D;
    }

    void ClearIntersectionInformation() {
        isIntersection = false;
        for (int i = 0; i < 6; i++) {
            EdgeIntersection[i] = false;
        }
        for (int i = 0; i < 6; i++) {
            EdgeIntersectionPoint[i] = FVector3f();
        }
    }

};


UCLASS()
class FATALITEPROJECT_API ACylinderActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACylinderActor();
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UProceduralMeshComponent* UpperProceduralMeshComponent;
	UProceduralMeshComponent* LowerProceduralMeshComponent;
    TArray<FTetrahedron> Tetrahedra;
    TArray<FVector> Vertices;
    TArray<int32> Triangles;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	class UStaticMeshComponent* CylinderMeshComponent;

	UFUNCTION(BlueprintCallable, Category = "Cylinder")
	void OnSwordBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION(BlueprintCallable, Category = "Cylinder")
	void OnSwordEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
    
    //LOAD TETRAHEDRON FILES
    bool LoadFaces(FString filepath, TArray<int32>& fv);
    bool LoadTetrahedrons(FString filepath, TArray<FTetrahedron>& tt);
    bool LoadNodes(FString filepath, TArray<FVector>& fv);
    bool LoadEdges(FString filepath, TArray<FVector>& fv);
    bool RayTriangleIntersect(const FVector& origin, const FVector& direction,
        const FVector& v0, const FVector& v1, const FVector& v2,
        float& t, float& u, float& v);
    void FindIntersectionTetrahedrons(FVector& PreviousStartVector, FVector& PreviousEndVector, FVector& CurrentStartVector, FVector& CurrentEndVector);
};


