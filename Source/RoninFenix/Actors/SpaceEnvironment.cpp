#include "Actors/SpaceEnvironment.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "SpaceTypes.h"

ASpaceEnvironment::ASpaceEnvironment()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
}

void ASpaceEnvironment::BeginPlay()
{
	Super::BeginPlay();

	CreateStarfield();
	CreateNebula();
	CreateLighting();
}

void ASpaceEnvironment::CreateStarfield()
{
	FRandomStream Rand(12345);

	UStaticMesh* SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere"));
	UMaterialInterface* BaseMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));

	for (int32 i = 0; i < 200; ++i)
	{
		FVector Dir = FMath::VRand();
		float Distance = SpaceConstants::WorldBoundsRadius * (1.5f + Rand.FRand() * 0.5f);
		FVector Pos = Dir * Distance;

		UStaticMeshComponent* StarMesh = NewObject<UStaticMeshComponent>(this);
		StarMesh->SetupAttachment(RootComponent);
		StarMesh->SetStaticMesh(SphereMesh);
		StarMesh->SetWorldLocation(Pos);
		StarMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		float StarSize = Rand.FRandRange(20.f, 80.f);
		StarMesh->SetWorldScale3D(FVector(StarSize));

		if (BaseMat)
		{
			UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(BaseMat, this);
			if (DynMat)
			{
				float Brightness = Rand.FRandRange(2.f, 10.f);
				FLinearColor StarColor;
				float ColorRand = Rand.FRand();
				if (ColorRand < 0.5f)
					StarColor = FLinearColor(1.f, 1.f, 1.f) * Brightness;
				else if (ColorRand < 0.7f)
					StarColor = FLinearColor(1.f, 0.8f, 0.5f) * Brightness;
				else if (ColorRand < 0.85f)
					StarColor = FLinearColor(0.5f, 0.7f, 1.f) * Brightness;
				else
					StarColor = FLinearColor(1.f, 0.5f, 0.3f) * Brightness;

				DynMat->SetVectorParameterValue(TEXT("Color"), StarColor);
				StarMesh->SetMaterial(0, DynMat);
			}
		}

		StarMesh->RegisterComponent();
	}
}

void ASpaceEnvironment::CreateNebula()
{
	UStaticMesh* SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere"));
	UMaterialInterface* BaseMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));

	if (!SphereMesh || !BaseMat) return;

	FRandomStream Rand(54321);

	struct NebulaCloud
	{
		FVector Position;
		float Scale;
		FLinearColor Color;
	};

	NebulaCloud Clouds[] = {
		{ FVector(40000.f, 20000.f, 10000.f), 5000.f, FLinearColor(0.1f, 0.02f, 0.15f) },
		{ FVector(-30000.f, -15000.f, 5000.f), 4000.f, FLinearColor(0.02f, 0.05f, 0.15f) },
		{ FVector(10000.f, -40000.f, -8000.f), 6000.f, FLinearColor(0.15f, 0.05f, 0.02f) },
	};

	for (const NebulaCloud& Cloud : Clouds)
	{
		for (int32 i = 0; i < 5; ++i)
		{
			UStaticMeshComponent* NebMesh = NewObject<UStaticMeshComponent>(this);
			NebMesh->SetupAttachment(RootComponent);
			NebMesh->SetStaticMesh(SphereMesh);
			NebMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			FVector Offset = FMath::VRand() * Cloud.Scale * 0.5f;
			NebMesh->SetWorldLocation(Cloud.Position + Offset);

			float Size = Cloud.Scale * Rand.FRandRange(0.5f, 1.5f);
			NebMesh->SetWorldScale3D(FVector(Size * 0.01f));

			UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(BaseMat, this);
			if (DynMat)
			{
				DynMat->SetVectorParameterValue(TEXT("Color"), Cloud.Color * 0.5f);
				DynMat->SetScalarParameterValue(TEXT("Opacity"), 0.3f);
				NebMesh->SetMaterial(0, DynMat);
			}

			NebMesh->RegisterComponent();
		}
	}
}

void ASpaceEnvironment::CreateLighting()
{
	// Main sun light
	UDirectionalLightComponent* SunLight = NewObject<UDirectionalLightComponent>(this);
	SunLight->SetupAttachment(RootComponent);
	SunLight->SetIntensity(3.f);
	SunLight->SetLightColor(FLinearColor(1.f, 0.95f, 0.85f));
	SunLight->SetWorldRotation(FRotator(-30.f, 45.f, 0.f));
	SunLight->RegisterComponent();

	// Ambient fill
	UDirectionalLightComponent* FillLight = NewObject<UDirectionalLightComponent>(this);
	FillLight->SetupAttachment(RootComponent);
	FillLight->SetIntensity(0.5f);
	FillLight->SetLightColor(FLinearColor(0.3f, 0.4f, 0.6f));
	FillLight->SetWorldRotation(FRotator(20.f, -135.f, 0.f));
	FillLight->RegisterComponent();

	// Rim light from below
	UDirectionalLightComponent* RimLight = NewObject<UDirectionalLightComponent>(this);
	RimLight->SetupAttachment(RootComponent);
	RimLight->SetIntensity(0.3f);
	RimLight->SetLightColor(FLinearColor(0.2f, 0.15f, 0.3f));
	RimLight->SetWorldRotation(FRotator(60.f, 90.f, 0.f));
	RimLight->RegisterComponent();
}
