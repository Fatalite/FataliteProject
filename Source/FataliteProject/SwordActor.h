// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "StaticMeshResources.h"
#include "CoreFwd.h"
#include "Rendering/SkinWeightVertexBuffer.h"
#include "SwordActor.generated.h"

struct Triangle {
	FVector x;
	FVector y;
	FVector z;

	Triangle(FVector a, FVector b, FVector c) {
		x = a;
		y = b;
		z = c;
	}
};


class AFataliteCharacter;

UCLASS()
class FATALITEPROJECT_API ASwordActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASwordActor();
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* SwordMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* PlaneComponent;

	UFUNCTION(BlueprintCallable, Category = "Sword")
	void AttachObjectToSwordSocket(class AActor* ObjectToAttach, FName SocketName);

	FVector3d StartCollisionVector;
	FVector3d EndCollisionVector;
	FVector PreviousStartToEnd = FVector(0.0f,0.0f,0.0f);
	FVector CurrentStartToEnd;
	FVector PreviousLeftToRight = FVector(0.0f, 0.0f, 0.0f);
	FVector CurrentLeftToRight;

	FVector PreviousStartLocation;
	FVector CurrentStartLocation;
	FVector PreviousEndLocation;
	FVector CurrentEndLocation;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void SetPlane(FRotator Rotation);
	UFUNCTION(BlueprintCallable, Category = "Sword")
	void OnSwordBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION(BlueprintCallable, Category = "Sword")
	void OnSwordEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void SaveStaticMeshTriangles(UStaticMeshComponent* StaticMeshComponent);
	FVector3d GetNormalVector();
	
	FVector& GetPreviousStartToEndVector();
	FVector& GetCurrentStartToEndVector();

	FVector& GetPreviousStartLocation();
	FVector& GetCurrentStartLocation();
	FVector& GetPreviousEndLocation();
	FVector& GetCurrentEndLocation();
};
