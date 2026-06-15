#include "Actors/SpaceEnvironment.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionConstant.h"
#include "ImageUtils.h"
#include "Misc/FileHelper.h"
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

	CreateSkybox();
	CreateStarfield();
	CreateNebula();
	CreateLighting();
	CreatePlanet();
}

void ASpaceEnvironment::CreateStarfield()
{
	FRandomStream Rand(12345);

	UStaticMesh* SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere"));
	UMaterialInterface* BaseMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));

	for (int32 i = 0; i < 50; ++i)
	{
		FVector Dir = FMath::VRand();
		float Distance = SpaceConstants::WorldBoundsRadius * (1.5f + Rand.FRand() * 0.5f);
		FVector Pos = Dir * Distance;

		UStaticMeshComponent* StarMesh = NewObject<UStaticMeshComponent>(this);
		StarMesh->SetupAttachment(RootComponent);
		StarMesh->SetStaticMesh(SphereMesh);
		StarMesh->SetWorldLocation(Pos);
		StarMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		float StarSize = Rand.FRandRange(10.f, 40.f);
		StarMesh->SetWorldScale3D(FVector(StarSize));

		if (BaseMat)
		{
			UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(BaseMat, this);
			if (DynMat)
			{
				float Brightness = Rand.FRandRange(1.f, 4.f);
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
	// Primary sun light
	UDirectionalLightComponent* SunLight = NewObject<UDirectionalLightComponent>(this);
	SunLight->SetupAttachment(RootComponent);
	SunLight->SetIntensity(4.f);
	SunLight->SetLightColor(FLinearColor(1.f, 0.95f, 0.85f));
	SunLight->SetWorldRotation(FRotator(-30.f, 45.f, 0.f));
	SunLight->RegisterComponent();

	// Fill light — aligned with the planet direction for sensible bounce
	UDirectionalLightComponent* FillLight = NewObject<UDirectionalLightComponent>(this);
	FillLight->SetupAttachment(RootComponent);
	FillLight->SetIntensity(0.6f);
	FillLight->SetLightColor(FLinearColor(0.6f, 0.7f, 1.f));
	FVector PlanetPos(-5000000.f, 3000000.f, -1500000.f);
	FRotator FillRot = (-PlanetPos).GetSafeNormal().Rotation();
	FillLight->SetWorldRotation(FillRot);
	FillLight->SetCastShadows(false);
	FillLight->RegisterComponent();

	// Skylight — low-intensity ambient fill so nothing is pure black
	USkyLightComponent* SkyLight = NewObject<USkyLightComponent>(this);
	SkyLight->SetupAttachment(RootComponent);
	SkyLight->SetIntensity(0.4f);
	SkyLight->SetLightColor(FLinearColor(0.5f, 0.55f, 0.7f));
	SkyLight->SourceType = ESkyLightSourceType::SLS_SpecifiedCubemap;
	SkyLight->bLowerHemisphereIsBlack = false;
	SkyLight->RegisterComponent();
	SkyLight->SetCaptureIsDirty();

	// Sun visual — opposite to light direction (light comes FROM the sun)
	UStaticMesh* SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere"));

	// Try unlit emissive material (no shadows on sun)
	UMaterialInterface* SunBaseMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/EngineMaterials/EmissiveMeshMaterial.EmissiveMeshMaterial"));
	if (!SunBaseMat)
	{
		SunBaseMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	}

	if (SphereMesh && SunBaseMat)
	{
		// Negate: light shines in FRotator(-30,45,0) direction, sun is opposite
		FVector SunDir = -FRotator(-30.f, 45.f, 0.f).Vector();
		FVector SunPos = SunDir * 500000.f;

		UStaticMeshComponent* SunMesh = NewObject<UStaticMeshComponent>(this);
		SunMesh->SetupAttachment(RootComponent);
		SunMesh->SetStaticMesh(SphereMesh);
		SunMesh->SetWorldLocation(SunPos);
		SunMesh->SetWorldScale3D(FVector(750.f));
		SunMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SunMesh->SetCastShadow(false);

		UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(SunBaseMat, this);
		if (DynMat)
		{
			// Very bright warm-white: bloom saturates center to white, edges show warm orange tint
			FLinearColor SunColor = FLinearColor(1.f, 0.9f, 0.7f) * 300.f;
			DynMat->SetVectorParameterValue(TEXT("Color"), SunColor);
			DynMat->SetVectorParameterValue(TEXT("EmissiveColor"), SunColor);
			SunMesh->SetMaterial(0, DynMat);
		}

		SunMesh->RegisterComponent();
	}
}

void ASpaceEnvironment::CreateSkybox()
{
	UStaticMesh* SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere"));
	if (!SphereMesh) return;

	UMaterialInterface* SkyMat = nullptr;

#if WITH_EDITORONLY_DATA
	FString TexturePath = FPaths::ProjectContentDir() / TEXT("Textures/SpaceSkybox.jpg");
	TArray<uint8> RawData;
	if (FFileHelper::LoadFileToArray(RawData, *TexturePath))
	{
		UTexture2D* SkyTex = FImageUtils::ImportBufferAsTexture2D(RawData);
		if (SkyTex)
		{
			UMaterial* Mat = NewObject<UMaterial>(GetTransientPackage(), NAME_None, RF_Transient);
			Mat->BlendMode = BLEND_Opaque;
			Mat->SetShadingModel(MSM_Unlit);
			Mat->TwoSided = true;

			UMaterialExpressionTextureSample* TexSample = NewObject<UMaterialExpressionTextureSample>(Mat);
			TexSample->Texture = SkyTex;
			TexSample->SamplerType = SAMPLERTYPE_Color;

			// Multiply texture by 0.3 — dims stars, pure black stays black
			UMaterialExpressionConstant* DimFactor = NewObject<UMaterialExpressionConstant>(Mat);
			DimFactor->R = 0.15f;

			UMaterialExpressionMultiply* Multiply = NewObject<UMaterialExpressionMultiply>(Mat);
			Multiply->A.Connect(0, TexSample);
			Multiply->B.Connect(0, DimFactor);

			auto* EdData = Mat->GetEditorOnlyData();
			if (EdData)
			{
				EdData->ExpressionCollection.Expressions.Add(TexSample);
				EdData->ExpressionCollection.Expressions.Add(DimFactor);
				EdData->ExpressionCollection.Expressions.Add(Multiply);
				EdData->EmissiveColor.Connect(0, Multiply);
			}

			Mat->PreEditChange(nullptr);
			Mat->PostEditChange();
			SkyMat = Mat;
		}
	}
#endif

	if (!SkyMat)
	{
		UMaterialInterface* FallbackMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
		if (!FallbackMat) return;
		SkyMat = FallbackMat;
	}

	UStaticMeshComponent* SkyMesh = NewObject<UStaticMeshComponent>(this);
	SkyMesh->SetupAttachment(RootComponent);
	SkyMesh->SetStaticMesh(SphereMesh);
	SkyMesh->SetWorldLocation(FVector::ZeroVector);
	SkyMesh->SetWorldScale3D(FVector(-500000.f));
	SkyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SkyMesh->SetCastShadow(false);
	SkyMesh->SetMaterial(0, SkyMat);
	SkyMesh->RegisterComponent();
}

void ASpaceEnvironment::CreatePlanet()
{
	UStaticMesh* SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere"));
	UMaterialInterface* BaseMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (!SphereMesh || !BaseMat) return;

	// Distant gas giant — purely decorative, impossibly far
	FVector PlanetPos = FVector(-5000000.f, 3000000.f, -1500000.f);

	UStaticMeshComponent* PlanetMesh = NewObject<UStaticMeshComponent>(this);
	PlanetMesh->SetupAttachment(RootComponent);
	PlanetMesh->SetStaticMesh(SphereMesh);
	PlanetMesh->SetWorldLocation(PlanetPos);
	PlanetMesh->SetWorldScale3D(FVector(80000.f));
	PlanetMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PlanetMesh->SetCastShadow(false);

	UMaterialInstanceDynamic* PlanetMat = UMaterialInstanceDynamic::Create(BaseMat, this);
	if (PlanetMat)
	{
		PlanetMat->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.05f, 0.15f, 0.25f) * 2.f);
		PlanetMesh->SetMaterial(0, PlanetMat);
	}
	PlanetMesh->RegisterComponent();

	// Atmospheric glow shell
	UStaticMeshComponent* AtmoMesh = NewObject<UStaticMeshComponent>(this);
	AtmoMesh->SetupAttachment(RootComponent);
	AtmoMesh->SetStaticMesh(SphereMesh);
	AtmoMesh->SetWorldLocation(PlanetPos);
	AtmoMesh->SetWorldScale3D(FVector(83000.f));
	AtmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AtmoMesh->SetCastShadow(false);

	UMaterialInstanceDynamic* AtmoMat = UMaterialInstanceDynamic::Create(BaseMat, this);
	if (AtmoMat)
	{
		AtmoMat->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.1f, 0.3f, 0.5f) * 3.f);
		AtmoMesh->SetMaterial(0, AtmoMat);
	}
	AtmoMesh->RegisterComponent();
}
