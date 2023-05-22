// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "CuttableActor.generated.h"

struct Spring {
    int mass1;
    int mass2;
    double rest_length;
    double spring_const;
    Spring(int Mass1, int Mass2, double RestLength, double SpringConst) {
        mass1 = Mass1;
        mass2 = Mass2;
        rest_length = RestLength;
        spring_const = SpringConst;
    }
};

struct Node {
    FVector Position;
    FVector Velocity;
    double Mass;
    TArray<int32> ConnectedSprings;
    Node(FVector _Position, FVector _Velocity, double _Mass) {
        Position = _Position;
        Velocity = _Velocity;
        Mass = _Mass;
    }

};

UCLASS()
class FATALITEPROJECT_API ACuttableActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACuttableActor();
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UProceduralMeshComponent* ProceduralMeshComponent;
	TArray<Node> Nodes;
	TArray<Spring> Springs;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
    bool LoadFaces(FString filepath, TArray<int32>& fv);
    //bool LoadTetrahedrons(FString filepath);
    bool LoadNodes(FString filepath, TArray<FVector>& fv);
    bool LoadEdges(FString filepath);

    void AddNodes(Node n);
    void AddSprings(Spring s);
    void SimulatingMassSpringDamper(double dt);
    FVector Acceleration(int SizeStep, const FVector& pos = FVector::Zero(), const FVector& vel = FVector::Zero());
};
