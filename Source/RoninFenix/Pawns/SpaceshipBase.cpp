#include "Pawns/SpaceshipBase.h"
#include "Pawns/PlayerSpaceship.h"
#include "Components/SphereComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SpaceshipMovementComponent.h"
#include "Components/HealthShieldComponent.h"
#include "Components/WeaponComponent.h"
#include "Components/TargetingComponent.h"
#include "Engine/StaticMesh.h"
#include "Actors/Asteroid.h"

ASpaceshipBase::ASpaceshipBase()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	CollisionComp->SetSphereRadius(120.f);
	CollisionComp->SetCollisionProfileName(TEXT("Pawn"));
	RootComponent = CollisionComp;

	ShipMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipMesh"));
	ShipMesh->SetupAttachment(RootComponent);
	ShipMesh->SetCastShadow(false);

	MovementComp = CreateDefaultSubobject<USpaceshipMovementComponent>(TEXT("Movement"));
	HealthComp = CreateDefaultSubobject<UHealthShieldComponent>(TEXT("Health"));
	WeaponComp = CreateDefaultSubobject<UWeaponComponent>(TEXT("Weapons"));
	TargetingComp = CreateDefaultSubobject<UTargetingComponent>(TEXT("Targeting"));

	EngineLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("EngineLight"));
	EngineLight->SetupAttachment(RootComponent);
	EngineLight->SetRelativeLocation(FVector(-200.f, 0.f, 0.f));
	EngineLight->SetIntensity(8000.f);
	EngineLight->SetAttenuationRadius(500.f);
	EngineLight->SetLightColor(FLinearColor(1.f, 0.5f, 0.2f));
}

void ASpaceshipBase::BeginPlay()
{
	Super::BeginPlay();
	HealthComp->OnDeath.AddDynamic(this, &ASpaceshipBase::OnShipDestroyed);

	CollisionComp->SetGenerateOverlapEvents(true);
	CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &ASpaceshipBase::OnShipOverlap);

	// InitializeShip is called by GameMode AFTER SetTeam/SetShipClass,
	// not here where Team is still Neutral
}

void ASpaceshipBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (EngineLight && MovementComp)
	{
		float SpeedPct = MovementComp->GetCurrentSpeed() / ShipStats.MaxSpeed;
		EngineLight->SetIntensity(2000.f + 8000.f * SpeedPct);
	}
}

void ASpaceshipBase::SetTeam(ESpaceTeam InTeam)
{
	Team = InTeam;
	if (WeaponComp) WeaponComp->OwnerTeam = Team;
	if (TargetingComp) TargetingComp->SetTeam(Team);
}

void ASpaceshipBase::SetShipClass(EShipClass InClass)
{
	ShipClass = InClass;
	ApplyShipClassStats();
}

void ASpaceshipBase::InitializeShip()
{
	ApplyShipClassStats();

	// Enemies move at half speed
	if (Team == ESpaceTeam::Omega)
	{
		ShipStats.MaxSpeed *= 0.5f;
		ShipStats.BoostMaxSpeed *= 0.5f;
		ShipStats.Acceleration *= 0.5f;
	}

	// Determine which mesh to load based on player vs AI
	FString MeshPath;
	bool bIsPlayer = IsA<APlayerSpaceship>();
	if (bIsPlayer)
	{
		MeshPath = TEXT("/Game/Meshes/mainShip/StaticMeshes/mainShip.mainShip");
	}
	else
	{
		// All AI ships use the same mesh — differentiated by team color
		MeshPath = TEXT("/Game/Meshes/enemyShip/StaticMeshes/enemyShip.enemyShip");
	}

	UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, *MeshPath);
	if (Mesh)
	{
		ShipMesh->SetStaticMesh(Mesh);
		ShipMesh->SetRelativeScale3D(FVector(4.f));
		ShipMesh->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));
		ShipMesh->SetVisibility(true);
		ShipMesh->SetHiddenInGame(false);
	}

	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);

	// Apply team color via dynamic material instance
	FLinearColor HullColor = GetTeamColor();
	if (ShipMesh->GetNumMaterials() > 0)
	{
		UMaterialInstanceDynamic* DynMat = ShipMesh->CreateAndSetMaterialInstanceDynamic(0);
		if (DynMat)
		{
			DynMat->SetVectorParameterValue(TEXT("BaseColor"), HullColor);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("InitializeShip: %s Team=%d Class=%d Mesh=%s Mats=%d Color=(%.2f,%.2f,%.2f)"),
		*GetName(), (int32)Team, (int32)ShipClass,
		Mesh ? TEXT("OK") : TEXT("FAIL"),
		ShipMesh->GetNumMaterials(),
		HullColor.R, HullColor.G, HullColor.B);

	MovementComp->Stats = ShipStats;
	HealthComp->SetStats(ShipStats.MaxHealth, ShipStats.MaxShield, ShipStats.ShieldRegenRate, ShipStats.ShieldRegenDelay);
	WeaponComp->SetWeaponStats(ShipStats.LaserDamage, ShipStats.LaserFireRate, ShipStats.MissileDamage, ShipStats.MissileCount);
	WeaponComp->OwnerTeam = Team;
	TargetingComp->SetTeam(Team);
}

void ASpaceshipBase::OnShipDestroyed()
{
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);
}

FLinearColor ASpaceshipBase::GetTeamColor() const
{
	switch (Team)
	{
	case ESpaceTeam::Alpha:
		return FLinearColor(0.3f, 0.5f, 1.0f);
	case ESpaceTeam::Omega:
		return FLinearColor(0.6f, 0.15f, 0.15f);
	default:
		return FLinearColor(0.4f, 0.4f, 0.4f);
	}
}

void ASpaceshipBase::OnShipOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	AAsteroid* Asteroid = Cast<AAsteroid>(OtherActor);
	if (Asteroid && HealthComp && HealthComp->IsAlive())
	{
		HealthComp->ApplyDamage(30.f, Asteroid);
	}
}

void ASpaceshipBase::ApplyShipClassStats()
{
	switch (ShipClass)
	{
	case EShipClass::Fighter:
		ShipStats.MaxHealth = 100.f;
		ShipStats.MaxShield = 50.f;
		ShipStats.MaxSpeed = 4000.f;
		ShipStats.BoostMaxSpeed = 7000.f;
		ShipStats.Acceleration = 4000.f;
		ShipStats.PitchRate = 80.f;
		ShipStats.YawRate = 80.f;
		ShipStats.RollRate = 120.f;
		ShipStats.LaserDamage = 8.f;
		ShipStats.LaserFireRate = 8.f;
		ShipStats.MissileDamage = 40.f;
		ShipStats.MissileCount = 50;
		CollisionComp->SetSphereRadius(120.f);
		break;
	case EShipClass::Interceptor:
		ShipStats.MaxHealth = 70.f;
		ShipStats.MaxShield = 30.f;
		ShipStats.MaxSpeed = 5500.f;
		ShipStats.BoostMaxSpeed = 9000.f;
		ShipStats.Acceleration = 6000.f;
		ShipStats.PitchRate = 100.f;
		ShipStats.YawRate = 100.f;
		ShipStats.RollRate = 150.f;
		ShipStats.LaserDamage = 6.f;
		ShipStats.LaserFireRate = 12.f;
		ShipStats.MissileDamage = 30.f;
		ShipStats.MissileCount = 50;
		CollisionComp->SetSphereRadius(100.f);
		break;
	case EShipClass::Bomber:
		ShipStats.MaxHealth = 200.f;
		ShipStats.MaxShield = 100.f;
		ShipStats.MaxSpeed = 2800.f;
		ShipStats.BoostMaxSpeed = 4500.f;
		ShipStats.Acceleration = 2400.f;
		ShipStats.PitchRate = 50.f;
		ShipStats.YawRate = 50.f;
		ShipStats.RollRate = 70.f;
		ShipStats.LaserDamage = 12.f;
		ShipStats.LaserFireRate = 5.f;
		ShipStats.MissileDamage = 80.f;
		ShipStats.MissileCount = 50;
		CollisionComp->SetSphereRadius(160.f);
		break;
	}
}
