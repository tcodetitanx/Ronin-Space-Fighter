#include "Procedural/ProceduralShipMeshBuilder.h"
#include "ProceduralMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

void FProceduralShipMeshBuilder::BuildForShipClass(UProceduralMeshComponent* MeshComp, EShipClass ShipClass, FLinearColor HullColor)
{
	switch (ShipClass)
	{
	case EShipClass::Fighter:
		BuildFighterMesh(MeshComp, HullColor);
		break;
	case EShipClass::Interceptor:
		BuildInterceptorMesh(MeshComp, HullColor);
		break;
	case EShipClass::Bomber:
		BuildBomberMesh(MeshComp, HullColor);
		break;
	}
}

void FProceduralShipMeshBuilder::BuildFighterMesh(UProceduralMeshComponent* MeshComp, FLinearColor HullColor)
{
	FProcMeshData Data;
	FLinearColor DarkColor = HullColor * 0.5f;
	DarkColor.A = 1.f;
	FLinearColor EngineGlow(1.f, 0.4f, 0.1f, 1.f);

	// Nose cone
	AddCone(Data, FVector(150.f, 0.f, 0.f), FVector(350.f, 0.f, 0.f), 40.f, 12, HullColor);

	// Main fuselage
	AddCylinder(Data, FVector(-120.f, 0.f, 0.f), FVector(150.f, 0.f, 0.f), 50.f, 40.f, 12, HullColor);

	// Rear fuselage
	AddCylinder(Data, FVector(-200.f, 0.f, 0.f), FVector(-120.f, 0.f, 0.f), 45.f, 50.f, 12, DarkColor);

	// Left wing
	AddBox(Data, FVector(0.f, -150.f, 0.f), FVector(100.f, 80.f, 5.f), HullColor);

	// Right wing
	AddBox(Data, FVector(0.f, 150.f, 0.f), FVector(100.f, 80.f, 5.f), HullColor);

	// Wing tips - left
	AddBox(Data, FVector(-20.f, -240.f, 0.f), FVector(40.f, 15.f, 8.f), DarkColor);

	// Wing tips - right
	AddBox(Data, FVector(-20.f, 240.f, 0.f), FVector(40.f, 15.f, 8.f), DarkColor);

	// Left engine nacelle
	AddCylinder(Data, FVector(-220.f, -80.f, 0.f), FVector(-100.f, -80.f, 0.f), 18.f, 22.f, 8, DarkColor);

	// Right engine nacelle
	AddCylinder(Data, FVector(-220.f, 80.f, 0.f), FVector(-100.f, 80.f, 0.f), 18.f, 22.f, 8, DarkColor);

	// Engine glow - left
	AddCone(Data, FVector(-230.f, -80.f, 0.f), FVector(-220.f, -80.f, 0.f), 16.f, 8, EngineGlow);

	// Engine glow - right
	AddCone(Data, FVector(-230.f, 80.f, 0.f), FVector(-220.f, 80.f, 0.f), 16.f, 8, EngineGlow);

	// Cockpit canopy
	AddCone(Data, FVector(80.f, 0.f, 30.f), FVector(160.f, 0.f, 15.f), 25.f, 8, FLinearColor(0.2f, 0.3f, 0.5f, 1.f));

	ApplyToMeshComponent(MeshComp, Data);
}

void FProceduralShipMeshBuilder::BuildInterceptorMesh(UProceduralMeshComponent* MeshComp, FLinearColor HullColor)
{
	FProcMeshData Data;
	FLinearColor DarkColor = HullColor * 0.5f;
	DarkColor.A = 1.f;
	FLinearColor EngineGlow(0.3f, 0.5f, 1.f, 1.f);

	// Long pointed nose
	AddCone(Data, FVector(200.f, 0.f, 0.f), FVector(500.f, 0.f, 0.f), 30.f, 10, HullColor);

	// Slim fuselage
	AddCylinder(Data, FVector(-100.f, 0.f, 0.f), FVector(200.f, 0.f, 0.f), 35.f, 30.f, 10, HullColor);

	// Rear
	AddCylinder(Data, FVector(-180.f, 0.f, 0.f), FVector(-100.f, 0.f, 0.f), 30.f, 35.f, 10, DarkColor);

	// Swept wings - left
	AddQuad(Data,
		FVector(100.f, -30.f, 0.f),
		FVector(-50.f, -200.f, 0.f),
		FVector(-100.f, -200.f, 0.f),
		FVector(0.f, -30.f, 0.f),
		HullColor);

	// Swept wings - right
	AddQuad(Data,
		FVector(0.f, 30.f, 0.f),
		FVector(-100.f, 200.f, 0.f),
		FVector(-50.f, 200.f, 0.f),
		FVector(100.f, 30.f, 0.f),
		HullColor);

	// Vertical stabilizer
	AddQuad(Data,
		FVector(-50.f, 0.f, 30.f),
		FVector(-180.f, 0.f, 80.f),
		FVector(-180.f, 0.f, 30.f),
		FVector(-50.f, 0.f, 30.f),
		DarkColor);

	// Single big engine
	AddCylinder(Data, FVector(-250.f, 0.f, 0.f), FVector(-180.f, 0.f, 0.f), 20.f, 28.f, 8, DarkColor);
	AddCone(Data, FVector(-260.f, 0.f, 0.f), FVector(-250.f, 0.f, 0.f), 18.f, 8, EngineGlow);

	// Cockpit
	AddCone(Data, FVector(120.f, 0.f, 22.f), FVector(200.f, 0.f, 10.f), 18.f, 8, FLinearColor(0.2f, 0.3f, 0.5f, 1.f));

	ApplyToMeshComponent(MeshComp, Data);
}

void FProceduralShipMeshBuilder::BuildBomberMesh(UProceduralMeshComponent* MeshComp, FLinearColor HullColor)
{
	FProcMeshData Data;
	FLinearColor DarkColor = HullColor * 0.5f;
	DarkColor.A = 1.f;
	FLinearColor EngineGlow(1.f, 0.6f, 0.2f, 1.f);

	// Blunt nose
	AddCone(Data, FVector(100.f, 0.f, 0.f), FVector(200.f, 0.f, 0.f), 60.f, 12, HullColor);

	// Wide fuselage
	AddCylinder(Data, FVector(-150.f, 0.f, 0.f), FVector(100.f, 0.f, 0.f), 70.f, 60.f, 12, HullColor);

	// Rear
	AddCylinder(Data, FVector(-250.f, 0.f, 0.f), FVector(-150.f, 0.f, 0.f), 55.f, 70.f, 12, DarkColor);

	// Stubby wings - left
	AddBox(Data, FVector(-20.f, -130.f, 0.f), FVector(80.f, 60.f, 8.f), HullColor);

	// Stubby wings - right
	AddBox(Data, FVector(-20.f, 130.f, 0.f), FVector(80.f, 60.f, 8.f), HullColor);

	// Bomb bay (underside detail)
	AddBox(Data, FVector(-50.f, 0.f, -50.f), FVector(60.f, 40.f, 15.f), DarkColor);

	// Twin engines - left
	AddCylinder(Data, FVector(-280.f, -50.f, 0.f), FVector(-150.f, -50.f, 0.f), 22.f, 28.f, 8, DarkColor);
	AddCone(Data, FVector(-290.f, -50.f, 0.f), FVector(-280.f, -50.f, 0.f), 20.f, 8, EngineGlow);

	// Twin engines - right
	AddCylinder(Data, FVector(-280.f, 50.f, 0.f), FVector(-150.f, 50.f, 0.f), 22.f, 28.f, 8, DarkColor);
	AddCone(Data, FVector(-290.f, 50.f, 0.f), FVector(-280.f, 50.f, 0.f), 20.f, 8, EngineGlow);

	// Cockpit
	AddCone(Data, FVector(50.f, 0.f, 40.f), FVector(120.f, 0.f, 20.f), 30.f, 8, FLinearColor(0.2f, 0.3f, 0.5f, 1.f));

	ApplyToMeshComponent(MeshComp, Data);
}

void FProceduralShipMeshBuilder::BuildCapitalShipMesh(UProceduralMeshComponent* MeshComp, FLinearColor HullColor)
{
	FProcMeshData Data;
	FLinearColor DarkColor = HullColor * 0.4f;
	DarkColor.A = 1.f;
	FLinearColor DetailColor = HullColor * 0.7f;
	DetailColor.A = 1.f;
	FLinearColor EngineGlow(0.8f, 0.5f, 0.2f, 1.f);

	// Main hull - long box shape
	AddBox(Data, FVector(0.f, 0.f, 0.f), FVector(2000.f, 400.f, 150.f), HullColor);

	// Bridge tower
	AddBox(Data, FVector(400.f, 0.f, 300.f), FVector(200.f, 100.f, 150.f), DetailColor);

	// Bridge cap
	AddBox(Data, FVector(400.f, 0.f, 480.f), FVector(120.f, 150.f, 30.f), DarkColor);

	// Forward section (wedge-like)
	AddCone(Data, FVector(1500.f, 0.f, 0.f), FVector(2500.f, 0.f, 0.f), 300.f, 6, HullColor);

	// Engine block
	AddBox(Data, FVector(-2000.f, 0.f, 0.f), FVector(200.f, 350.f, 120.f), DarkColor);

	// Engine nozzles
	for (int32 i = -2; i <= 2; ++i)
	{
		float Y = i * 100.f;
		AddCylinder(Data, FVector(-2300.f, Y, 0.f), FVector(-2200.f, Y, 0.f), 30.f, 40.f, 8, EngineGlow);
	}

	// Hangar bay (underside)
	AddBox(Data, FVector(500.f, 0.f, -200.f), FVector(400.f, 200.f, 50.f), DarkColor);

	// Turret mounts (top)
	for (int32 i = -1; i <= 1; ++i)
	{
		AddCylinder(Data, FVector(i * 600.f, 0.f, 150.f), FVector(i * 600.f, 0.f, 200.f), 30.f, 20.f, 8, DetailColor);
	}

	// Side trenches
	AddBox(Data, FVector(0.f, -420.f, 0.f), FVector(1500.f, 20.f, 50.f), DarkColor);
	AddBox(Data, FVector(0.f, 420.f, 0.f), FVector(1500.f, 20.f, 50.f), DarkColor);

	ApplyToMeshComponent(MeshComp, Data);
}

void FProceduralShipMeshBuilder::BuildAsteroidMesh(UProceduralMeshComponent* MeshComp, float Radius, int32 Seed)
{
	FProcMeshData Data;
	FRandomStream Rand(Seed);
	FLinearColor RockColor(0.3f, 0.25f, 0.2f, 1.f);

	const int32 LatSegments = 8;
	const int32 LonSegments = 12;

	for (int32 Lat = 0; Lat <= LatSegments; ++Lat)
	{
		float Theta = PI * Lat / LatSegments;
		for (int32 Lon = 0; Lon <= LonSegments; ++Lon)
		{
			float Phi = 2.f * PI * Lon / LonSegments;
			float Variation = Radius * (0.7f + 0.3f * Rand.FRand());
			FVector Pos(
				Variation * FMath::Sin(Theta) * FMath::Cos(Phi),
				Variation * FMath::Sin(Theta) * FMath::Sin(Phi),
				Variation * FMath::Cos(Theta)
			);
			FVector Normal = Pos.GetSafeNormal();
			float U = (float)Lon / LonSegments;
			float V = (float)Lat / LatSegments;

			float Shade = 0.7f + 0.3f * Rand.FRand();
			FLinearColor VertColor = RockColor * Shade;
			VertColor.A = 1.f;

			Data.Vertices.Add(Pos);
			Data.Normals.Add(Normal);
			Data.UVs.Add(FVector2D(U, V));
			Data.VertexColors.Add(VertColor);
		}
	}

	for (int32 Lat = 0; Lat < LatSegments; ++Lat)
	{
		for (int32 Lon = 0; Lon < LonSegments; ++Lon)
		{
			int32 Current = Lat * (LonSegments + 1) + Lon;
			int32 Next = Current + LonSegments + 1;

			Data.Triangles.Add(Current);
			Data.Triangles.Add(Next);
			Data.Triangles.Add(Current + 1);

			Data.Triangles.Add(Current + 1);
			Data.Triangles.Add(Next);
			Data.Triangles.Add(Next + 1);
		}
	}

	ApplyToMeshComponent(MeshComp, Data);
}

void FProceduralShipMeshBuilder::AddCone(FProcMeshData& Data, FVector Base, FVector Tip, float Radius, int32 Segments, FLinearColor Color)
{
	FVector Axis = (Tip - Base).GetSafeNormal();
	FVector Perp1, Perp2;
	Axis.FindBestAxisVectors(Perp1, Perp2);

	int32 BaseIndex = Data.Vertices.Num();
	int32 TipIndex = BaseIndex + Segments;

	for (int32 i = 0; i < Segments; ++i)
	{
		float Angle = 2.f * PI * i / Segments;
		FVector Offset = (Perp1 * FMath::Cos(Angle) + Perp2 * FMath::Sin(Angle)) * Radius;
		FVector Pos = Base + Offset;
		FVector Normal = Offset.GetSafeNormal();

		Data.Vertices.Add(Pos);
		Data.Normals.Add(Normal);
		Data.UVs.Add(FVector2D((float)i / Segments, 0.f));
		Data.VertexColors.Add(Color);
	}

	Data.Vertices.Add(Tip);
	Data.Normals.Add(Axis);
	Data.UVs.Add(FVector2D(0.5f, 1.f));
	Data.VertexColors.Add(Color);

	for (int32 i = 0; i < Segments; ++i)
	{
		int32 Next = (i + 1) % Segments;
		Data.Triangles.Add(BaseIndex + i);
		Data.Triangles.Add(TipIndex);
		Data.Triangles.Add(BaseIndex + Next);
	}

	// Base cap
	int32 BaseCenterIndex = Data.Vertices.Num();
	Data.Vertices.Add(Base);
	Data.Normals.Add(-Axis);
	Data.UVs.Add(FVector2D(0.5f, 0.5f));
	Data.VertexColors.Add(Color);

	for (int32 i = 0; i < Segments; ++i)
	{
		int32 Next = (i + 1) % Segments;
		Data.Triangles.Add(BaseCenterIndex);
		Data.Triangles.Add(BaseIndex + Next);
		Data.Triangles.Add(BaseIndex + i);
	}
}

void FProceduralShipMeshBuilder::AddCylinder(FProcMeshData& Data, FVector Start, FVector End, float RadiusStart, float RadiusEnd, int32 Segments, FLinearColor Color)
{
	FVector Axis = (End - Start).GetSafeNormal();
	FVector Perp1, Perp2;
	Axis.FindBestAxisVectors(Perp1, Perp2);

	int32 BaseStart = Data.Vertices.Num();

	for (int32 i = 0; i <= Segments; ++i)
	{
		float Angle = 2.f * PI * i / Segments;
		FVector Dir = Perp1 * FMath::Cos(Angle) + Perp2 * FMath::Sin(Angle);
		FVector Normal = Dir;

		FVector PosStart = Start + Dir * RadiusStart;
		FVector PosEnd = End + Dir * RadiusEnd;

		float U = (float)i / Segments;

		Data.Vertices.Add(PosStart);
		Data.Normals.Add(Normal);
		Data.UVs.Add(FVector2D(U, 0.f));
		Data.VertexColors.Add(Color);

		Data.Vertices.Add(PosEnd);
		Data.Normals.Add(Normal);
		Data.UVs.Add(FVector2D(U, 1.f));
		Data.VertexColors.Add(Color);
	}

	for (int32 i = 0; i < Segments; ++i)
	{
		int32 Idx = BaseStart + i * 2;
		Data.Triangles.Add(Idx);
		Data.Triangles.Add(Idx + 2);
		Data.Triangles.Add(Idx + 1);

		Data.Triangles.Add(Idx + 1);
		Data.Triangles.Add(Idx + 2);
		Data.Triangles.Add(Idx + 3);
	}
}

void FProceduralShipMeshBuilder::AddBox(FProcMeshData& Data, FVector Center, FVector Extent, FLinearColor Color)
{
	FVector Min = Center - Extent;
	FVector Max = Center + Extent;

	FVector Corners[8] = {
		FVector(Min.X, Min.Y, Min.Z),
		FVector(Max.X, Min.Y, Min.Z),
		FVector(Max.X, Max.Y, Min.Z),
		FVector(Min.X, Max.Y, Min.Z),
		FVector(Min.X, Min.Y, Max.Z),
		FVector(Max.X, Min.Y, Max.Z),
		FVector(Max.X, Max.Y, Max.Z),
		FVector(Min.X, Max.Y, Max.Z),
	};

	struct FaceDef { int32 Indices[4]; FVector Normal; };
	FaceDef Faces[6] = {
		{{4, 5, 6, 7}, FVector(0, 0, 1)},
		{{3, 2, 1, 0}, FVector(0, 0, -1)},
		{{0, 1, 5, 4}, FVector(0, -1, 0)},
		{{2, 3, 7, 6}, FVector(0, 1, 0)},
		{{1, 2, 6, 5}, FVector(1, 0, 0)},
		{{3, 0, 4, 7}, FVector(-1, 0, 0)},
	};

	for (const FaceDef& Face : Faces)
	{
		int32 BaseIdx = Data.Vertices.Num();

		for (int32 i = 0; i < 4; ++i)
		{
			Data.Vertices.Add(Corners[Face.Indices[i]]);
			Data.Normals.Add(Face.Normal);
			Data.VertexColors.Add(Color);
		}

		Data.UVs.Add(FVector2D(0, 0));
		Data.UVs.Add(FVector2D(1, 0));
		Data.UVs.Add(FVector2D(1, 1));
		Data.UVs.Add(FVector2D(0, 1));

		Data.Triangles.Add(BaseIdx);
		Data.Triangles.Add(BaseIdx + 1);
		Data.Triangles.Add(BaseIdx + 2);

		Data.Triangles.Add(BaseIdx);
		Data.Triangles.Add(BaseIdx + 2);
		Data.Triangles.Add(BaseIdx + 3);
	}
}

void FProceduralShipMeshBuilder::AddQuad(FProcMeshData& Data, FVector A, FVector B, FVector C, FVector D, FLinearColor Color)
{
	int32 BaseIdx = Data.Vertices.Num();
	FVector Normal = FVector::CrossProduct(B - A, D - A).GetSafeNormal();

	Data.Vertices.Add(A);
	Data.Vertices.Add(B);
	Data.Vertices.Add(C);
	Data.Vertices.Add(D);

	for (int32 i = 0; i < 4; ++i)
	{
		Data.Normals.Add(Normal);
		Data.VertexColors.Add(Color);
	}

	Data.UVs.Add(FVector2D(0, 0));
	Data.UVs.Add(FVector2D(1, 0));
	Data.UVs.Add(FVector2D(1, 1));
	Data.UVs.Add(FVector2D(0, 1));

	// Front face
	Data.Triangles.Add(BaseIdx);
	Data.Triangles.Add(BaseIdx + 1);
	Data.Triangles.Add(BaseIdx + 2);

	Data.Triangles.Add(BaseIdx);
	Data.Triangles.Add(BaseIdx + 2);
	Data.Triangles.Add(BaseIdx + 3);

	// Back face
	Data.Triangles.Add(BaseIdx + 2);
	Data.Triangles.Add(BaseIdx + 1);
	Data.Triangles.Add(BaseIdx);

	Data.Triangles.Add(BaseIdx + 3);
	Data.Triangles.Add(BaseIdx + 2);
	Data.Triangles.Add(BaseIdx);
}

void FProceduralShipMeshBuilder::ApplyToMeshComponent(UProceduralMeshComponent* MeshComp, const FProcMeshData& Data, int32 SectionIndex)
{
	if (!MeshComp || Data.Vertices.Num() == 0) return;

	FProcMeshData DoubleSided;
	DoubleSided.Vertices = Data.Vertices;
	DoubleSided.Triangles = Data.Triangles;
	DoubleSided.UVs = Data.UVs;
	DoubleSided.VertexColors = Data.VertexColors;

	int32 VertCount = Data.Vertices.Num();
	for (int32 i = 0; i < VertCount; ++i)
	{
		DoubleSided.Vertices.Add(Data.Vertices[i]);
		DoubleSided.Normals.Add(-Data.Normals[i]);
		DoubleSided.UVs.Add(Data.UVs[i]);
		DoubleSided.VertexColors.Add(Data.VertexColors[i]);
	}
	DoubleSided.Normals.Insert(Data.Normals, 0);

	int32 TriCount = Data.Triangles.Num();
	for (int32 i = 0; i < TriCount; i += 3)
	{
		DoubleSided.Triangles.Add(Data.Triangles[i] + VertCount);
		DoubleSided.Triangles.Add(Data.Triangles[i + 2] + VertCount);
		DoubleSided.Triangles.Add(Data.Triangles[i + 1] + VertCount);
	}

	TArray<FProcMeshTangent> Tangents;

	MeshComp->CreateMeshSection_LinearColor(
		SectionIndex,
		DoubleSided.Vertices,
		DoubleSided.Triangles,
		DoubleSided.Normals,
		DoubleSided.UVs,
		DoubleSided.VertexColors,
		Tangents,
		true
	);

	UMaterialInstanceDynamic* Mat = CreateHullMaterial(MeshComp, Data.VertexColors.Num() > 0 ? Data.VertexColors[0] : FLinearColor::White);
	if (Mat)
	{
		MeshComp->SetMaterial(SectionIndex, Mat);
	}
}

UMaterialInstanceDynamic* FProceduralShipMeshBuilder::CreateHullMaterial(UProceduralMeshComponent* MeshComp, FLinearColor Color)
{
	UMaterialInterface* BaseMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (!BaseMat) return nullptr;

	UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(BaseMat, MeshComp);
	return DynMat;
}
