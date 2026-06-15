#include "AI/SpaceshipAIController.h"
#include "Pawns/SpaceshipBase.h"
#include "Pawns/PlayerSpaceship.h"
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
		// Assign role randomly: 50/50 attacker/defender
		Role = FMath::RandBool() ? EAIRole::Attacker : EAIRole::Defender;

		float OwnDir = (ShipPawn->GetTeam() == ESpaceTeam::Alpha) ? -1.f : 1.f;
		float EnemyDir = -OwnDir;

		if (Role == EAIRole::Defender)
		{
			HomePosition = FVector(OwnDir * SpaceConstants::CapitalShipSpawnDistance, 0.f, 10000.f);
			PatrolRadius = 8000.f;
			MaxDistanceFromHome = 15000.f;
		}
		else
		{
			HomePosition = FVector(EnemyDir * SpaceConstants::CapitalShipSpawnDistance * 0.6f, 0.f, 10000.f);
			PatrolRadius = 12000.f;
			MaxDistanceFromHome = 25000.f;
		}

		PatrolTarget = HomePosition + FMath::VRand() * PatrolRadius;

		ShipPawn->GetHealthComp()->OnDamageTaken.AddDynamic(this, &ASpaceshipAIController::OnTookDamage);
	}
}

void ASpaceshipAIController::OnTookDamage(float Damage, AActor* DamageCauser)
{
	LastDamageTime = GetWorld()->GetTimeSeconds();
}

void ASpaceshipAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!ShipPawn) return;
	if (!ShipPawn->GetHealthComp()->IsAlive()) return;

	bRecentlyDamaged = (GetWorld()->GetTimeSeconds() - LastDamageTime) < 3.f;

	DecisionTimer += DeltaTime;
	if (DecisionTimer >= DecisionInterval)
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
	case EAIState::ChasePlayer:
		ExecuteChasePlayer(DeltaTime);
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

	// Evade if recently took damage and health is low
	float HealthPct = ShipPawn->GetHealthComp()->GetHealthPercent();
	if (bRecentlyDamaged && HealthPct < 0.3f && CurrentState != EAIState::Evade)
	{
		CurrentState = EAIState::Evade;
		EvadeTimer = 3.f;
		return;
	}

	// Leash check
	float DistFromHome = FVector::Dist(ShipPawn->GetActorLocation(), HomePosition);
	if (DistFromHome > MaxDistanceFromHome && CurrentState != EAIState::Evade)
	{
		CurrentState = EAIState::ReturnToBase;
		return;
	}

	// Chance to chase player (12%)
	ASpaceshipBase* Player = FindPlayerShip();
	if (Player && FMath::RandRange(0, 99) < 12)
	{
		float DistToPlayer = FVector::Dist(ShipPawn->GetActorLocation(), Player->GetActorLocation());
		if (DistToPlayer < 15000.f)
		{
			CurrentTarget = Player;
			CurrentState = EAIState::ChasePlayer;
			ChasePlayerTimer = FMath::RandRange(5.f, 10.f);
			return;
		}
	}

	// Look for nearby enemy fighters
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

	// Attackers go after capital ship
	if (Role == EAIRole::Attacker && FMath::RandRange(0, 99) < 40)
	{
		ACapitalShip* EnemyCap = FindEnemyCapitalShip();
		if (EnemyCap && !EnemyCap->IsDestroyed())
		{
			CurrentTarget = EnemyCap;
			CurrentState = EAIState::AttackCapitalShip;
			return;
		}
	}

	// Default to patrol
	if (CurrentState != EAIState::Patrol)
	{
		CurrentState = EAIState::Patrol;
		PatrolTarget = HomePosition + FMath::VRand() * PatrolRadius;
	}
}

void ASpaceshipAIController::ExecutePatrol(float DeltaTime)
{
	float Dist = FVector::Dist(ShipPawn->GetActorLocation(), PatrolTarget);
	if (Dist < 1500.f)
	{
		PatrolTarget = HomePosition + FMath::VRand() * PatrolRadius;
	}

	FlyToward(PatrolTarget, DeltaTime);
	ShipPawn->GetMovementComp()->SetThrottleInput(0.6f);
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

	FlyToward(CurrentTarget->GetActorLocation(), DeltaTime);

	FVector ToTarget = (CurrentTarget->GetActorLocation() - ShipPawn->GetActorLocation()).GetSafeNormal();
	float DotForward = FVector::DotProduct(ShipPawn->GetActorForwardVector(), ToTarget);

	if (DotForward > 0.85f && Dist < 5000.f)
	{
		ShipPawn->GetWeaponComp()->StartFiringLasers();
	}
	else
	{
		ShipPawn->GetWeaponComp()->StopFiringLasers();
	}

	if (Dist < 800.f)
	{
		ShipPawn->GetMovementComp()->SetThrottleInput(0.3f);
	}
	else
	{
		ShipPawn->GetMovementComp()->SetThrottleInput(1.f);
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

	FlyToward(AttackPoint, DeltaTime);

	float Dist = FVector::Dist(ShipPawn->GetActorLocation(), AttackPoint);
	FVector ToTarget = (AttackPoint - ShipPawn->GetActorLocation()).GetSafeNormal();
	float DotForward = FVector::DotProduct(ShipPawn->GetActorForwardVector(), ToTarget);

	if (DotForward > 0.8f && Dist < 6000.f)
	{
		ShipPawn->GetWeaponComp()->StartFiringLasers();
		ShipPawn->GetMovementComp()->SetThrottleInput(1.f);
	}
	else
	{
		ShipPawn->GetWeaponComp()->StopFiringLasers();
		ShipPawn->GetMovementComp()->SetThrottleInput(0.8f);
	}

	if (Dist < 1200.f)
	{
		CurrentState = EAIState::Patrol;
		PatrolTarget = HomePosition + FMath::VRand() * PatrolRadius;
	}
}

void ASpaceshipAIController::ExecuteChasePlayer(float DeltaTime)
{
	ChasePlayerTimer -= DeltaTime;

	if (!CurrentTarget || !IsValid(CurrentTarget) || ChasePlayerTimer <= 0.f)
	{
		CurrentTarget = nullptr;
		CurrentState = EAIState::Patrol;
		return;
	}

	ASpaceshipBase* PlayerShip = Cast<ASpaceshipBase>(CurrentTarget);
	if (PlayerShip && !PlayerShip->GetHealthComp()->IsAlive())
	{
		CurrentTarget = nullptr;
		CurrentState = EAIState::Patrol;
		return;
	}

	float Dist = FVector::Dist(ShipPawn->GetActorLocation(), CurrentTarget->GetActorLocation());

	FlyToward(CurrentTarget->GetActorLocation(), DeltaTime);

	FVector ToTarget = (CurrentTarget->GetActorLocation() - ShipPawn->GetActorLocation()).GetSafeNormal();
	float DotForward = FVector::DotProduct(ShipPawn->GetActorForwardVector(), ToTarget);

	if (DotForward > 0.85f && Dist < 5000.f)
	{
		ShipPawn->GetWeaponComp()->StartFiringLasers();
	}
	else
	{
		ShipPawn->GetWeaponComp()->StopFiringLasers();
	}

	ShipPawn->GetMovementComp()->SetThrottleInput(1.f);
	if (Dist > 3000.f)
	{
		ShipPawn->GetMovementComp()->SetBoostInput(true);
	}
	else
	{
		ShipPawn->GetMovementComp()->SetBoostInput(false);
	}
}

void ASpaceshipAIController::ExecuteEvade(float DeltaTime)
{
	EvadeTimer -= DeltaTime;
	ShipPawn->GetWeaponComp()->StopFiringLasers();
	ShipPawn->GetMovementComp()->SetThrottleInput(1.f);
	ShipPawn->GetMovementComp()->SetBoostInput(true);

	// Jinking — only during evasion does the AI fly erratically
	float Time = GetWorld()->GetTimeSeconds();
	float JinkYaw = FMath::Sin(Time * 4.f) * 0.8f;
	float JinkPitch = FMath::Cos(Time * 3.f) * 0.5f;

	ShipPawn->GetMovementComp()->SetYawInput(JinkYaw);
	ShipPawn->GetMovementComp()->SetPitchInput(JinkPitch);
	ShipPawn->GetMovementComp()->SetRollInput(FMath::Sin(Time * 2.f) * 0.6f);

	if (EvadeTimer <= 0.f)
	{
		ShipPawn->GetMovementComp()->SetBoostInput(false);
		CurrentState = EAIState::Patrol;
		PatrolTarget = HomePosition + FMath::VRand() * PatrolRadius;
	}
}

void ASpaceshipAIController::ExecuteReturnToBase(float DeltaTime)
{
	FlyToward(HomePosition, DeltaTime);
	ShipPawn->GetMovementComp()->SetThrottleInput(1.f);

	if (FVector::Dist(ShipPawn->GetActorLocation(), HomePosition) < 5000.f)
	{
		CurrentState = EAIState::Patrol;
		PatrolTarget = HomePosition + FMath::VRand() * PatrolRadius;
	}
}

void ASpaceshipAIController::FlyToward(FVector TargetLocation, float DeltaTime)
{
	if (!ShipPawn) return;

	FVector ToTarget = (TargetLocation - ShipPawn->GetActorLocation()).GetSafeNormal();
	FVector Forward = ShipPawn->GetActorForwardVector();
	FVector Right = ShipPawn->GetActorRightVector();
	FVector Up = ShipPawn->GetActorUpVector();

	float YawDot = FVector::DotProduct(ToTarget, Right);
	float PitchDot = FVector::DotProduct(ToTarget, Up);

	float DesiredYaw = FMath::Clamp(YawDot * 2.f, -1.f, 1.f);
	float DesiredPitch = FMath::Clamp(-PitchDot * 2.f, -1.f, 1.f);

	// Bank into turns
	float DesiredRoll = -YawDot * 1.2f;
	float CurrentRollDot = FVector::DotProduct(FVector::UpVector, Right);
	float RollCorrection = FMath::Clamp((DesiredRoll - CurrentRollDot), -1.f, 1.f);

	// Smooth interpolation — linear, non-erratic flight
	CurrentYawInput = FMath::FInterpTo(CurrentYawInput, DesiredYaw, DeltaTime, SteerSmoothing);
	CurrentPitchInput = FMath::FInterpTo(CurrentPitchInput, DesiredPitch, DeltaTime, SteerSmoothing);
	CurrentRollInput = FMath::FInterpTo(CurrentRollInput, RollCorrection, DeltaTime, SteerSmoothing);

	ShipPawn->GetMovementComp()->SetYawInput(CurrentYawInput);
	ShipPawn->GetMovementComp()->SetPitchInput(CurrentPitchInput);
	ShipPawn->GetMovementComp()->SetRollInput(CurrentRollInput);
}

ASpaceshipBase* ASpaceshipAIController::FindNearestEnemy() const
{
	if (!ShipPawn) return nullptr;

	TArray<AActor*> AllPawns;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpaceshipBase::StaticClass(), AllPawns);

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

ASpaceshipBase* ASpaceshipAIController::FindPlayerShip() const
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return nullptr;
	return Cast<ASpaceshipBase>(PC->GetPawn());
}

ACapitalShip* ASpaceshipAIController::FindEnemyCapitalShip() const
{
	if (!ShipPawn) return nullptr;

	TArray<AActor*> CapShips;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACapitalShip::StaticClass(), CapShips);

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
