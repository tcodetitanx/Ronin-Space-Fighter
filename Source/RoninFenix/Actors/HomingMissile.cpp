#include "Actors/HomingMissile.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/HealthShieldComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"

AHomingMissile::AHomingMissile()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	CollisionComp->SetSphereRadius(30.f);
	CollisionComp->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	RootComponent = CollisionComp;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->SetRelativeScale3D(FVector(1.5f, 0.3f, 0.3f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeMesh(TEXT("/Engine/BasicShapes/Cone"));
	if (ConeMesh.Succeeded())
	{
		MeshComp->SetStaticMesh(ConeMesh.Object);
		MeshComp->SetRelativeRotation(FRotator(0.f, 0.f, -90.f));
	}

	LightComp = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
	LightComp->SetupAttachment(RootComponent);
	LightComp->SetIntensity(3000.f);
	LightComp->SetAttenuationRadius(300.f);
	LightComp->SetLightColor(FLinearColor(1.f, 0.5f, 0.1f));

	InitialLifeSpan = SpaceConstants::MissileLifetime;
}

void AHomingMissile::BeginPlay()
{
	Super::BeginPlay();

	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
}

void AHomingMissile::Initialize(float InDamage, float InSpeed, AActor* InTarget, ESpaceTeam InTeam)
{
	Damage = InDamage;
	Speed = InSpeed;
	Target = InTarget;
	Team = InTeam;

	if (MeshComp)
	{
		UMaterialInterface* BaseMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
		if (BaseMat)
		{
			UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(BaseMat, this);
			if (DynMat)
			{
				DynMat->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.8f, 0.8f, 0.8f));
				MeshComp->SetMaterial(0, DynMat);
			}
		}
	}
}

void AHomingMissile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ArmTimer += DeltaTime;

	FVector Forward = GetActorForwardVector();

	if (Target && IsValid(Target))
	{
		FVector ToTarget = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		FRotator CurrentRot = Forward.Rotation();
		FRotator TargetRot = ToTarget.Rotation();
		FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, TurnRate);
		SetActorRotation(NewRot);
		Forward = NewRot.Vector();
	}

	SetActorLocation(GetActorLocation() + Forward * Speed * DeltaTime);

	if (ArmTimer < ArmingTime) return;

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
