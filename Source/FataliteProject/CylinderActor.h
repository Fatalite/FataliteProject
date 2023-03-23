// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "SwordActor.h"
#include "CylinderActor.generated.h"
// Represents a virtual node in the VNA
struct FVirtualNode
{
	FVector Position;
	TArray<int32> AssociatedVertices;
};

// Represents a tetrahedron in the VNA
struct FTetrahedron
{
	int32 VirtualNodes[4];
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
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	class UStaticMeshComponent* CylinderMeshComponent;
	FBoxSphereBounds Bounds;
	TArray<FVirtualNode> VirtualNodes;
	TArray<FTetrahedron> Tetrahedra;

	FVector3d StartCollisionVector;
	FVector3d EndCollisionVector;

	UFUNCTION(BlueprintCallable, Category = "Cylinder")
		void OnSwordBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION(BlueprintCallable, Category = "Cylinder")
		void OnSwordEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};


