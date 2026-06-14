#include "AI/SpaceshipAIController.h"
#include "Pawns/SpaceshipBase.h"
#include "Actors/CapitalShip.h"
#include "Components/SpaceshipMovementComponent.h"
#include "Components/WeaponComponent.h"
#include "Components/HealthShieldComponent.h"
#include "Components/TargetingComponent.h"
#include "Kismet/GameplayStatics.h"

ASpaceshipAIController::ASpaceshipAIController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ASpaceshipAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	ShipPawn = Cast<ASpaceshipBase>(InPawn);

	if (ShipPawn)
	{
		// Set home position near their capital ship
		float Direction = (ShipPawn->GetTeam() == ESpaceTeam::Alpha) ? -1.f : 1.f;
		HomePosition = FVector(Direction * SpaceConstants::CapitalShipSpawnDistance, 0.f, 10000.f);
		PatrolTarget = HomePosition + FMath::VRand() * PatrolRadius;
	}
}

void ASpaceshipAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!ShipPawn) return;
	if (!ShipPawn->GetHealthComp()->IsAlive()) return;

	DecisionTimer += DeltaTime;
	if (DecisionTimer >= 1.f)
	{
		DecisionTimer = 0.f;
		UpdateState();
	}

	switch (CurrentState)
	{
	case EAIState::Patrol:
		ExecutePatrol(DeltaTime);
		break;
	case EAIState::Engage:
		ExecuteEngage(DeltaTime);
		break;
	case EAIState::AttackCapitalShip:
		ExecuteAttackCapitalShip(DeltaTime);
		break;
	case EAIState::Evade:
		ExecuteEvade(DeltaTime);
		break;
	case EAIState::ReturnToBase:
		ExecuteReturnToBase(DeltaTime);
		break;
	}
}

void ASpaceshipAIController::UpdateState()
{
	if (!ShipPawn) return;

	float HealthPct = ShipPawn->GetHealthComp()->GetHealthPercent();

	// Leash: return to base if too far from home
	float DistFromHome = FVector::Dist(ShipPawn->GetActorLocation(), HomePosition);
	if (DistFromHome > MaxDistanceFromHome && CurrentState != EAIState::Evade && CurrentState != EAIState::ReturnToBase)
	{
		CurrentState = EAIState::ReturnToBase;
		return;
	}

	if (HealthPct < 0.2f && CurrentState != EAIState::Evade)
	{
		CurrentState = EAIState::Evade;
		EvadeTimer = 5.f;
		return;
	}

	ASpaceshipBase* Enemy = FindNearestEnemy();
	if (Enemy)
	{
		float Dist = FVector::Dist(ShipPawn->GetActorLocation(), Enemy->GetActorLocation());
		if (Dist < EngageRange)
		{
			CurrentTarget = Enemy;
			CurrentState = EAIState::Engage;
			return;
		}
	}

	if (FMath::RandRange(0, 10) < 3)
	{
		ACapitalShip* EnemyCap = FindEnemyCapitalShip();
		if (EnemyCap && !EnemyCap->IsDestroyed())
		{
			CurrentTarget = EnemyCap;
			CurrentState = EAIState::AttackCapitalShip;
			return;
		}
	}

	if (CurrentState != EAIState::Patrol)
	{
		CurrentState = EAIState::Patrol;
		PatrolTarget = HomePosition + FMath::VRand() * PatrolRadius;
	}
}

void ASpaceshipAIController::ExecutePatrol(float DeltaTime)
{
	float Dist = FVector::Dist(ShipPawn->GetActorLocation(), PatrolTarget);
	if (Dist < 1000.f)
	{
		PatrolTarget = HomePosition + FMath::VRand() * PatrolRadius;
	}

	FlyToward(PatrolTarget, DeltaTime, 500.f);
	ShipPawn->GetMovementComp()->SetThrottleInput(0.5f);
	ShipPawn->GetWeaponComp()->StopFiringLasers();
}

void ASpaceshipAIController::ExecuteEngage(float DeltaTime)
{
	if (!CurrentTarget || !IsValid(CurrentTarget))
	{
		CurrentState = EAIState::Patrol;
		return;
	}

	ASpaceshipBase* EnemyShip = Cast<ASpaceshipBase>(CurrentTarget);
	if (EnemyShip && !EnemyShip->GetHealthComp()->IsAlive())
	{
		CurrentTarget = nullptr;
		CurrentState = EAIState::Patrol;
		return;
	}

	float Dist = FVector::Dist(ShipPawn->GetActorLocation(), CurrentTarget->GetActorLocation());
	if (Dist > DisengageRange)
	{
		CurrentTarget = nullptr;
		CurrentState = EAIState::Patrol;
		return;
	}

	FlyToward(CurrentTarget->GetActorLocation(), DeltaTime, 800.f);

	FVector ToTarget = (CurrentTarget->GetActorLocation() - ShipPawn->GetActorLocation()).GetSafeNormal();
	float DotForward = FVector::DotProduct(ShipPawn->GetActorForwardVector(), ToTarget);

	if (DotForward > 0.9f && Dist < 4000.f)
	{
		ShipPawn->GetWeaponComp()->StartFiringLasers();
	}
	else
	{
		ShipPawn->GetWeaponComp()->StopFiringLasers();
	}

	if (Dist < 1500.f)
	{
		ShipPawn->GetMovementComp()->SetThrottleInput(-0.3f);
	}
	else
	{
		ShipPawn->GetMovementComp()->SetThrottleInput(1.f);
	}

	if (Dist < 600.f)
	{
		ShipPawn->GetMovementComp()->SetBoostInput(true);
		CurrentState = EAIState::Evade;
		EvadeTimer = 2.f;
	}
}

void ASpaceshipAIController::ExecuteAttackCapitalShip(float DeltaTime)
{
	ACapitalShip* Cap = Cast<ACapitalShip>(CurrentTarget);
	if (!Cap || Cap->IsDestroyed())
	{
		CurrentState = EAIState::Patrol;
		return;
	}

	FVector AttackPoint = Cap->GetActorLocation();
	const TArray<FCapitalShipSubsystem>& Subs = Cap->GetSubsystems();
	for (const FCapitalShipSubsystem& Sub : Subs)
	{
		if (!Sub.bDestroyed)
		{
			AttackPoint = Cap->GetActorLocation() + Cap->GetActorRotation().RotateVector(Sub.RelativeLocation);
			break;
		}
	}

	FlyToward(AttackPoint, DeltaTime, 1500.f);

	float Dist = FVector::Dist(ShipPawn->GetActorLocation(), AttackPoint);
	FVector ToTarget = (AttackPoint - ShipPawn->GetActorLocation()).GetSafeNormal();
	float DotForward = FVector::DotProduct(ShipPawn->GetActorForwardVector(), ToTarget);

	if (DotForward > 0.85f && Dist < 5000.f)
	{
		ShipPawn->GetWeaponComp()->StartFiringLasers();
		ShipPawn->GetMovementComp()->SetThrottleInput(1.f);
	}
	else
	{
		ShipPawn->GetWeaponComp()->StopFiringLasers();
	}

	if (Dist < 1000.f)
	{
		CurrentState = EAIState::Evade;
		EvadeTimer = 3.f;
	}
}

void ASpaceshipAIController::ExecuteEvade(float DeltaTime)
{
	EvadeTimer -= DeltaTime;
	ShipPawn->GetWeaponComp()->StopFiringLasers();
	ShipPawn->GetMovementComp()->SetThrottleInput(1.f);
	ShipPawn->GetMovementComp()->SetBoostInput(true);

	FVector EvadeDir = ShipPawn->GetActorForwardVector() + ShipPawn->GetActorRightVector() * FMath::Sin(GetWorld()->GetTimeSeconds() * 1.f) * 0.5f;
	FVector EvadeTarget = ShipPawn->GetActorLocation() + EvadeDir * 3000.f;
	FlyToward(EvadeTarget, DeltaTime, 0.f);

	if (EvadeTimer <= 0.f)
	{
		ShipPawn->GetMovementComp()->SetBoostInput(false);
		CurrentState = EAIState::Patrol;
	}
}

void ASpaceshipAIController::ExecuteReturnToBase(float DeltaTime)
{
	FlyToward(HomePosition, DeltaTime, 2000.f);
	ShipPawn->GetMovementComp()->SetThrottleInput(1.f);

	if (FVector::Dist(ShipPawn->GetActorLocation(), HomePosition) < 5000.f)
	{
		CurrentState = EAIState::Patrol;
		PatrolTarget = HomePosition + FMath::VRand() * PatrolRadius;
	}
}

void ASpaceshipAIController::FlyToward(FVector TargetLocation, float DeltaTime, float DesiredDistance)
{
	if (!ShipPawn) return;

	FVector ToTarget = TargetLocation - ShipPawn->GetActorLocation();
	FVector TargetDir = ToTarget.GetSafeNormal();
	FVector Forward = ShipPawn->GetActorForwardVector();
	FVector Right = ShipPawn->GetActorRightVector();
	FVector Up = ShipPawn->GetActorUpVector();

	float YawDot = FVector::DotProduct(TargetDir, Right);
	float PitchDot = FVector::DotProduct(TargetDir, Up);
	float ForwardDot = FVector::DotProduct(TargetDir, Forward);

	float YawInput = FMath::Clamp(YawDot * 1.5f, -1.f, 1.f);
	float PitchInput = FMath::Clamp(-PitchDot * 1.5f, -1.f, 1.f);

	float RollTarget = -YawDot * 1.f;
	float CurrentRollDot = FVector::DotProduct(FVector::UpVector, Right);
	float RollInput = FMath::Clamp((RollTarget - CurrentRollDot) * 1.f, -1.f, 1.f);

	ShipPawn->GetMovementComp()->SetYawInput(YawInput);
	ShipPawn->GetMovementComp()->SetPitchInput(PitchInput);
	ShipPawn->GetMovementComp()->SetRollInput(RollInput);
}

ASpaceshipBase* ASpaceshipAIController::FindNearestEnemy() const
{
	if (!ShipPawn) return nullptr;

	UWorld* World = GetWorld();
	if (!World) return nullptr;

	TArray<AActor*> AllPawns;
	UGameplayStatics::GetAllActorsOfClass(World, ASpaceshipBase::StaticClass(), AllPawns);

	ASpaceshipBase* Nearest = nullptr;
	float NearestDist = EngageRange;

	for (AActor* Actor : AllPawns)
	{
		ASpaceshipBase* Ship = Cast<ASpaceshipBase>(Actor);
		if (!Ship || Ship == ShipPawn || Ship->GetTeam() == ShipPawn->GetTeam()) continue;
		if (!Ship->GetHealthComp()->IsAlive()) continue;

		float Dist = FVector::Dist(ShipPawn->GetActorLocation(), Ship->GetActorLocation());
		if (Dist < NearestDist)
		{
			NearestDist = Dist;
			Nearest = Ship;
		}
	}

	return Nearest;
}

ACapitalShip* ASpaceshipAIController::FindEnemyCapitalShip() const
{
	if (!ShipPawn) return nullptr;

	UWorld* World = GetWorld();
	if (!World) return nullptr;

	TArray<AActor*> CapShips;
	UGameplayStatics::GetAllActorsOfClass(World, ACapitalShip::StaticClass(), CapShips);

	for (AActor* Actor : CapShips)
	{
		ACapitalShip* Cap = Cast<ACapitalShip>(Actor);
		if (Cap && Cap->GetTeam() != ShipPawn->GetTeam() && !Cap->IsDestroyed())
		{
			return Cap;
		}
	}

	return nullptr;
}
