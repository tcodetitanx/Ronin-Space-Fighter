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
	LightComp->SetIntensity(15000.f);
	LightComp->SetAttenuationRadius(500.f);
	LightComp->SetLightColor(FLinearColor(1.f, 0.0f, 0.0f));

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
		FLinearColor(1.f, 0.0f, 0.0f) : FLinearColor(1.f, 0.4f, 0.0f);

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
				DynMat->SetVectorParameterValue(TEXT("Color"), LaserColor * 50.f);
				MeshComp->SetMaterial(0, DynMat);
			}
		}
	}
}

void ALaserProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector Start = GetActorLocation();
	FVector End = Start + GetActorForwardVector() * Speed * DeltaTime;

	// Sweep along movement path to catch high-speed hits
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	if (GetOwner()) Params.AddIgnoredActor(GetOwner());

	if (GetWorld()->SweepSingleByChannel(Hit, Start, End, FQuat::Identity,
		ECC_Visibility, FCollisionShape::MakeSphere(20.f), Params))
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor)
		{
			UHealthShieldComponent* Health = HitActor->FindComponentByClass<UHealthShieldComponent>();
			if (Health)
			{
				Health->ApplyDamage(Damage, GetOwner());
			}
		}
		Destroy();
		return;
	}

	SetActorLocation(End);
}

