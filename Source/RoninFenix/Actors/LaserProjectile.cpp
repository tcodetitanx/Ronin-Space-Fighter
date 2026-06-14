#include "Actors/LaserProjectile.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/HealthShieldComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"

ALaserProjectile::ALaserProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	CollisionComp->SetSphereRadius(20.f);
	CollisionComp->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CollisionComp->SetGenerateOverlapEvents(true);
	RootComponent = CollisionComp;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->SetRelativeScale3D(FVector(3.f, 0.3f, 0.3f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereMesh.Succeeded())
	{
		MeshComp->SetStaticMesh(SphereMesh.Object);
	}

	LightComp = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
	LightComp->SetupAttachment(RootComponent);
	LightComp->SetIntensity(5000.f);
	LightComp->SetAttenuationRadius(200.f);
	LightComp->SetLightColor(FLinearColor(0.2f, 1.f, 0.2f));

	InitialLifeSpan = SpaceConstants::LaserLifetime;
}

void ALaserProjectile::BeginPlay()
{
	Super::BeginPlay();

	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	CollisionComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	CollisionComp->SetGenerateOverlapEvents(true);
}

void ALaserProjectile::Initialize(float InDamage, float InSpeed, ESpaceTeam InTeam)
{
	Damage = InDamage;
	Speed = InSpeed;
	Team = InTeam;

	FLinearColor LaserColor = (InTeam == ESpaceTeam::Alpha) ?
		FLinearColor(0.2f, 1.f, 0.3f) : FLinearColor(1.f, 0.2f, 0.2f);

	if (LightComp)
	{
		LightComp->SetLightColor(LaserColor);
	}

	if (MeshComp)
	{
		UMaterialInterface* BaseMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
		if (BaseMat)
		{
			UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(BaseMat, this);
			if (DynMat)
			{
				DynMat->SetVectorParameterValue(TEXT("Color"), LaserColor * 5.f);
				MeshComp->SetMaterial(0, DynMat);
			}
		}
	}
}

void ALaserProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector Movement = GetActorForwardVector() * Speed * DeltaTime;
	SetActorLocation(GetActorLocation() + Movement);

	TArray<AActor*> OverlappingActors;
	CollisionComp->GetOverlappingActors(OverlappingActors);
	for (AActor* Actor : OverlappingActors)
	{
		if (Actor == this || Actor == GetOwner()) continue;

		UHealthShieldComponent* Health = Actor->FindComponentByClass<UHealthShieldComponent>();
		if (Health)
		{
			Health->ApplyDamage(Damage, GetOwner());
			Destroy();
			return;
		}
	}
}

