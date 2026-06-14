#include "Actors/CapitalShip.h"
#include "Components/BoxComponent.h"
#include "Components/PointLightComponent.h"
#include "ProceduralMeshComponent.h"
#include "Components/HealthShieldComponent.h"
#include "Procedural/ProceduralShipMeshBuilder.h"
#include "Actors/LaserProjectile.h"
#include "Pawns/SpaceshipBase.h"
#include "Kismet/GameplayStatics.h"

ACapitalShip::ACapitalShip()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionComp = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision"));
	CollisionComp->SetBoxExtent(FVector(2500.f, 500.f, 250.f));
	CollisionComp->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	RootComponent = CollisionComp;

	ShipMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ShipMesh"));
	ShipMesh->SetupAttachment(RootComponent);
	ShipMesh->SetCastShadow(false);

	HealthComp = CreateDefaultSubobject<UHealthShieldComponent>(TEXT("Health"));
}

void ACapitalShip::BeginPlay()
{
	Super::BeginPlay();
	HealthComp->OnDamageTaken.AddDynamic(this, &ACapitalShip::OnShipDamaged);
}

void ACapitalShip::InitializeCapitalShip(ESpaceTeam InTeam)
{
	Team = InTeam;

	FLinearColor HullColor = (Team == ESpaceTeam::Alpha) ?
		FLinearColor(0.15f, 0.2f, 0.4f) : FLinearColor(0.4f, 0.15f, 0.15f);

	FProceduralShipMeshBuilder::BuildCapitalShipMesh(ShipMesh, HullColor);

	HealthComp->SetStats(2000.f, 500.f, 5.f, 10.f);

	Subsystems.Empty();

	FCapitalShipSubsystem Shield;
	Shield.Type = ESubsystemType::ShieldGenerator;
	Shield.MaxHealth = 400.f;
	Shield.CurrentHealth = 400.f;
	Shield.RelativeLocation = FVector(800.f, 0.f, 200.f);
	Subsystems.Add(Shield);

	FCapitalShipSubsystem Engine;
	Engine.Type = ESubsystemType::EngineArray;
	Engine.MaxHealth = 500.f;
	Engine.CurrentHealth = 500.f;
	Engine.RelativeLocation = FVector(-2000.f, 0.f, 0.f);
	Subsystems.Add(Engine);

	FCapitalShipSubsystem Weapons;
	Weapons.Type = ESubsystemType::WeaponBattery;
	Weapons.MaxHealth = 350.f;
	Weapons.CurrentHealth = 350.f;
	Weapons.RelativeLocation = FVector(0.f, 300.f, 100.f);
	Subsystems.Add(Weapons);

	FCapitalShipSubsystem Bridge;
	Bridge.Type = ESubsystemType::Bridge;
	Bridge.MaxHealth = 300.f;
	Bridge.CurrentHealth = 300.f;
	Bridge.RelativeLocation = FVector(400.f, 0.f, 400.f);
	Subsystems.Add(Bridge);

	for (int32 i = 0; i < Subsystems.Num(); ++i)
	{
		UPointLightComponent* Light = NewObject<UPointLightComponent>(this);
		Light->SetupAttachment(RootComponent);
		Light->SetRelativeLocation(Subsystems[i].RelativeLocation);
		Light->SetIntensity(5000.f);
		Light->SetAttenuationRadius(800.f);
		Light->SetLightColor(FLinearColor(0.f, 1.f, 0.f));
		Light->RegisterComponent();
		SubsystemLights.Add(Light);
	}
}

void ACapitalShip::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsDestroyed) return;

	FireTurrets(DeltaTime);

	AddActorLocalRotation(FRotator(0.f, 0.5f * DeltaTime, 0.f));
}

float ACapitalShip::GetHealthPercent() const
{
	return HealthComp ? HealthComp->GetHealthPercent() : 0.f;
}

void ACapitalShip::OnShipDamaged(float Damage, AActor* DamageCauser)
{
	if (bIsDestroyed) return;

	float MinDist = TNumericLimits<float>::Max();
	int32 ClosestIdx = -1;

	FVector HitLoc = DamageCauser ? DamageCauser->GetActorLocation() : GetActorLocation();
	for (int32 i = 0; i < Subsystems.Num(); ++i)
	{
		if (Subsystems[i].bDestroyed) continue;
		FVector WorldLoc = GetActorLocation() + GetActorRotation().RotateVector(Subsystems[i].RelativeLocation);
		float Dist = FVector::Dist(HitLoc, WorldLoc);
		if (Dist < MinDist)
		{
			MinDist = Dist;
			ClosestIdx = i;
		}
	}

	if (ClosestIdx >= 0)
	{
		Subsystems[ClosestIdx].CurrentHealth -= Damage;
		if (Subsystems[ClosestIdx].CurrentHealth <= 0.f && !Subsystems[ClosestIdx].bDestroyed)
		{
			Subsystems[ClosestIdx].bDestroyed = true;
			Subsystems[ClosestIdx].CurrentHealth = 0.f;

			if (ClosestIdx < SubsystemLights.Num() && SubsystemLights[ClosestIdx])
			{
				SubsystemLights[ClosestIdx]->SetLightColor(FLinearColor(1.f, 0.f, 0.f));
				SubsystemLights[ClosestIdx]->SetIntensity(10000.f);
			}

			OnSubsystemDestroyed.Broadcast(Team, Subsystems[ClosestIdx].Type);

			if (Subsystems[ClosestIdx].Type == ESubsystemType::ShieldGenerator)
			{
				HealthComp->SetStats(HealthComp->GetCurrentHealth(), 0.f, 0.f, 0.f);
			}
		}
	}

	if (!HealthComp->IsAlive())
	{
		bIsDestroyed = true;
		OnCapitalShipDestroyed.Broadcast(Team);
	}
}

void ACapitalShip::FireTurrets(float DeltaTime)
{
	// Check if weapon battery is still functional
	for (const FCapitalShipSubsystem& Sub : Subsystems)
	{
		if (Sub.Type == ESubsystemType::WeaponBattery && Sub.bDestroyed) return;
	}

	TurretFireCooldown -= DeltaTime;
	if (TurretFireCooldown > 0.f) return;

	AActor* Target = FindNearestEnemy();
	if (!Target) return;

	TurretFireCooldown = 1.f / TurretFireRate;

	UWorld* World = GetWorld();
	if (!World) return;

	FVector TurretPositions[] = {
		GetActorLocation() + GetActorRotation().RotateVector(FVector(600.f, 0.f, 200.f)),
		GetActorLocation() + GetActorRotation().RotateVector(FVector(0.f, 0.f, 200.f)),
		GetActorLocation() + GetActorRotation().RotateVector(FVector(-600.f, 0.f, 200.f)),
	};

	for (const FVector& TurretPos : TurretPositions)
	{
		FVector ToTarget = (Target->GetActorLocation() - TurretPos).GetSafeNormal();
		FRotator SpawnRot = ToTarget.Rotation();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		ALaserProjectile* Laser = World->SpawnActor<ALaserProjectile>(ALaserProjectile::StaticClass(), TurretPos, SpawnRot, SpawnParams);
		if (Laser)
		{
			ESpaceTeam LaserTeam = Team;
			Laser->Initialize(TurretDamage, SpaceConstants::LaserSpeed * 0.7f, LaserTeam);
		}
	}
}

AActor* ACapitalShip::FindNearestEnemy() const
{
	UWorld* World = GetWorld();
	if (!World) return nullptr;

	TArray<AActor*> AllPawns;
	UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), AllPawns);

	AActor* Nearest = nullptr;
	float NearestDist = TurretRange;

	for (AActor* Actor : AllPawns)
	{
		ASpaceshipBase* Ship = Cast<ASpaceshipBase>(Actor);
		if (!Ship || Ship->GetTeam() == Team) continue;

		UHealthShieldComponent* Health = Ship->GetHealthComp();
		if (!Health || !Health->IsAlive()) continue;

		float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
		if (Dist < NearestDist)
		{
			NearestDist = Dist;
			Nearest = Actor;
		}
	}

	return Nearest;
}
