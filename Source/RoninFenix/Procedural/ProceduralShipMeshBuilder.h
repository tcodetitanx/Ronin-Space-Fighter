#pragma once

#include "CoreMinimal.h"
#include "SpaceTypes.h"

class UProceduralMeshComponent;

struct FProcMeshData
{
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FLinearColor> VertexColors;
};

class FProceduralShipMeshBuilder
{
public:
	static void BuildFighterMesh(UProceduralMeshComponent* MeshComp, FLinearColor HullColor);
	static void BuildInterceptorMesh(UProceduralMeshComponent* MeshComp, FLinearColor HullColor);
	static void BuildBomberMesh(UProceduralMeshComponent* MeshComp, FLinearColor HullColor);
	static void BuildCapitalShipMesh(UProceduralMeshComponent* MeshComp, FLinearColor HullColor);
	static void BuildAsteroidMesh(UProceduralMeshComponent* MeshComp, float Radius, int32 Seed);
	static void BuildForShipClass(UProceduralMeshComponent* MeshComp, EShipClass ShipClass, FLinearColor HullColor);

private:
	static void AddCone(FProcMeshData& Data, FVector Base, FVector Tip, float Radius, int32 Segments, FLinearColor Color);
	static void AddCylinder(FProcMeshData& Data, FVector Start, FVector End, float RadiusStart, float RadiusEnd, int32 Segments, FLinearColor Color);
	static void AddBox(FProcMeshData& Data, FVector Center, FVector Extent, FLinearColor Color);
	static void AddQuad(FProcMeshData& Data, FVector A, FVector B, FVector C, FVector D, FLinearColor Color);
	static void ApplyToMeshComponent(UProceduralMeshComponent* MeshComp, const FProcMeshData& Data, int32 SectionIndex = 0);
	static UMaterialInstanceDynamic* CreateHullMaterial(UProceduralMeshComponent* MeshComp, FLinearColor Color);
};
