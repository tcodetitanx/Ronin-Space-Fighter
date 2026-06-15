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
	LightComp->SetIntensity(20000.f);
	LightComp->SetAttenuationRadius(800.f);
	LightComp->SetLightColor(FLinearColor(1.f, 0.5f, 0.1f));
	LightComp->SetCastShadows(false);
	MeshComp->SetCastShadow(false);

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

	UMaterialInterface* BaseMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (MeshComp && BaseMat)
	{
		UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(BaseMat, this);
		if (DynMat)
		{
			DynMat->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.8f, 0.8f, 0.8f));
			MeshComp->SetMaterial(0, DynMat);
		}
	}

	// Ion thruster flame on missile tail
	UStaticMesh* SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere"));
	UMaterialInterface* EmissiveMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/EngineMaterials/EmissiveMeshMaterial.EmissiveMeshMaterial"));
	if (!EmissiveMat) EmissiveMat = BaseMat;
	if (SphereMesh && EmissiveMat)
	{
		ThrusterFlame = NewObject<UStaticMeshComponent>(this);
		ThrusterFlame->SetupAttachment(RootComponent);
		ThrusterFlame->SetStaticMesh(SphereMesh);
		ThrusterFlame->SetRelativeLocation(FVector(-60.f, 0.f, 0.f));
		ThrusterFlame->SetRelativeScale3D(FVector(0.3f, 0.15f, 0.15f));
		ThrusterFlame->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ThrusterFlame->SetCastShadow(false);
		UMaterialInstanceDynamic* FlameMat = UMaterialInstanceDynamic::Create(EmissiveMat, this);
		if (FlameMat)
		{
			FLinearColor IonColor = FLinearColor(0.3f, 0.6f, 1.f) * 25.f;
			FlameMat->SetVectorParameterValue(TEXT("Color"), IonColor);
			FlameMat->SetVectorParameterValue(TEXT("EmissiveColor"), IonColor);
			ThrusterFlame->SetMaterial(0, FlameMat);
		}
		ThrusterFlame->RegisterComponent();
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

	FVector Start = GetActorLocation();
	FVector End = Start + Forward * Speed * DeltaTime;

	if (ArmTimer >= ArmingTime)
	{
		// Sweep along movement path to detect hits
		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);
		if (GetOwner()) Params.AddIgnoredActor(GetOwner());

		if (GetWorld()->SweepSingleByChannel(Hit, Start, End, FQuat::Identity,
			ECC_Visibility, FCollisionShape::MakeSphere(30.f), Params))
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
	}

	SetActorLocation(End);

	// Oscillate ion thruster flame
	if (ThrusterFlame)
	{
		float Time = GetWorld()->GetTimeSeconds();
		float Pulse = FMath::Sin(Time * 12.f) * 0.3f + 1.f;
		float Flicker = FMath::Sin(Time * 31.f) * 0.1f;
		float S = 0.3f * (Pulse + Flicker);
		ThrusterFlame->SetRelativeScale3D(FVector(S, S * 0.5f, S * 0.5f));
	}
}
