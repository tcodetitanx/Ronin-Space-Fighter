#include "Pawns/SpaceshipBase.h"
#include "Components/SphereComponent.h"
#include "Components/PointLightComponent.h"
#include "ProceduralMeshComponent.h"
#include "Components/SpaceshipMovementComponent.h"
#include "Components/HealthShieldComponent.h"
#include "Components/WeaponComponent.h"
#include "Components/TargetingComponent.h"
#include "Procedural/ProceduralShipMeshBuilder.h"

ASpaceshipBase::ASpaceshipBase()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	CollisionComp->SetSphereRadius(120.f);
	CollisionComp->SetCollisionProfileName(TEXT("Pawn"));
	RootComponent = CollisionComp;

	ShipMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ShipMesh"));
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
	InitializeShip();
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

	FLinearColor HullColor = GetTeamColor();
	FProceduralShipMeshBuilder::BuildForShipClass(ShipMesh, ShipClass, HullColor);

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
		return FLinearColor(0.15f, 0.25f, 0.6f);
	case ESpaceTeam::Omega:
		return FLinearColor(0.6f, 0.15f, 0.15f);
	default:
		return FLinearColor(0.4f, 0.4f, 0.4f);
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
		ShipStats.Acceleration = 2000.f;
		ShipStats.PitchRate = 80.f;
		ShipStats.YawRate = 60.f;
		ShipStats.RollRate = 120.f;
		ShipStats.LaserDamage = 8.f;
		ShipStats.LaserFireRate = 8.f;
		ShipStats.MissileDamage = 40.f;
		ShipStats.MissileCount = 4;
		CollisionComp->SetSphereRadius(120.f);
		break;
	case EShipClass::Interceptor:
		ShipStats.MaxHealth = 70.f;
		ShipStats.MaxShield = 30.f;
		ShipStats.MaxSpeed = 5500.f;
		ShipStats.BoostMaxSpeed = 9000.f;
		ShipStats.Acceleration = 3000.f;
		ShipStats.PitchRate = 100.f;
		ShipStats.YawRate = 80.f;
		ShipStats.RollRate = 150.f;
		ShipStats.LaserDamage = 6.f;
		ShipStats.LaserFireRate = 12.f;
		ShipStats.MissileDamage = 30.f;
		ShipStats.MissileCount = 2;
		CollisionComp->SetSphereRadius(100.f);
		break;
	case EShipClass::Bomber:
		ShipStats.MaxHealth = 200.f;
		ShipStats.MaxShield = 100.f;
		ShipStats.MaxSpeed = 2800.f;
		ShipStats.BoostMaxSpeed = 4500.f;
		ShipStats.Acceleration = 1200.f;
		ShipStats.PitchRate = 50.f;
		ShipStats.YawRate = 40.f;
		ShipStats.RollRate = 70.f;
		ShipStats.LaserDamage = 12.f;
		ShipStats.LaserFireRate = 5.f;
		ShipStats.MissileDamage = 80.f;
		ShipStats.MissileCount = 8;
		CollisionComp->SetSphereRadius(160.f);
		break;
	}
}
