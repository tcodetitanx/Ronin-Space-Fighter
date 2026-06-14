#include "Actors/HomingMissile.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/HealthShieldComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"

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

	// Ring buffer trail — world-space glowing planes left behind as missile moves
	UStaticMesh* TrailMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane"));
	if (TrailMesh && BaseMat)
	{
		TrailDots.SetNum(TrailCount);
		for (int32 i = 0; i < TrailCount; ++i)
		{
			UStaticMeshComponent* Dot = NewObject<UStaticMeshComponent>(this);
			Dot->SetupAttachment(RootComponent);
			Dot->SetAbsolute(true, true, true);
			Dot->SetStaticMesh(TrailMesh);
			Dot->SetWorldLocation(GetActorLocation());
			Dot->SetWorldScale3D(FVector(0.08f));
			Dot->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Dot->SetCastShadow(false);
			Dot->SetVisibility(false);

			UMaterialInstanceDynamic* TrailMat = UMaterialInstanceDynamic::Create(BaseMat, this);
			if (TrailMat)
			{
				TrailMat->SetVectorParameterValue(TEXT("Color"), FLinearColor(1.f, 0.5f, 0.1f) * 15.f);
				Dot->SetMaterial(0, TrailMat);
			}
			Dot->RegisterComponent();
			TrailDots[i] = Dot;
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

	// Update ring buffer trail
	TrailTimer += DeltaTime;
	if (TrailTimer >= TrailInterval && TrailDots.Num() > 0)
	{
		TrailTimer = 0.f;
		TrailDots[TrailIndex]->SetWorldLocation(GetActorLocation());
		TrailDots[TrailIndex]->SetVisibility(true);
		TrailIndex = (TrailIndex + 1) % TrailDots.Num();
	}

	// Billboard: orient trail planes to face camera
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (PC->PlayerCameraManager)
		{
			FVector CamLoc = PC->PlayerCameraManager->GetCameraLocation();
			for (int32 i = 0; i < TrailDots.Num(); ++i)
			{
				if (TrailDots[i] && TrailDots[i]->IsVisible())
				{
					FVector ToCamera = CamLoc - TrailDots[i]->GetComponentLocation();
					if (ToCamera.SizeSquared() > 1.f)
					{
						TrailDots[i]->SetWorldRotation(FRotationMatrix::MakeFromZ(ToCamera.GetSafeNormal()).Rotator());
					}
				}
			}
		}
	}
}
