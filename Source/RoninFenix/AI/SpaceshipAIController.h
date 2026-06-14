#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "SpaceTypes.h"
#include "SpaceshipAIController.generated.h"

class ASpaceshipBase;
class ACapitalShip;

UENUM()
enum class EAIState : uint8
{
	Patrol,
	Engage,
	AttackCapitalShip,
	Evade,
	ReturnToBase
};

UCLASS()
class RONINFENIX_API ASpaceshipAIController : public AAIController
{
	GENERATED_BODY()

public:
	ASpaceshipAIController();

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void OnPossess(APawn* InPawn) override;

private:
	void UpdateState();
	void ExecutePatrol(float DeltaTime);
	void ExecuteEngage(float DeltaTime);
	void ExecuteAttackCapitalShip(float DeltaTime);
	void ExecuteEvade(float DeltaTime);
	void ExecuteReturnToBase(float DeltaTime);

	void FlyToward(FVector TargetLocation, float DeltaTime, float DesiredDistance = 500.f);
	ASpaceshipBase* FindNearestEnemy() const;
	ACapitalShip* FindEnemyCapitalShip() const;

	UPROPERTY()
	TObjectPtr<ASpaceshipBase> ShipPawn;

	UPROPERTY()
	TObjectPtr<AActor> CurrentTarget;

	EAIState CurrentState = EAIState::Patrol;
	FVector PatrolTarget;
	float StateTimer = 0.f;
	float EngageRange = 5000.f;
	float DisengageRange = 8000.f;
	float PatrolRadius = 10000.f;
	float EvadeTimer = 0.f;
	float DecisionTimer = 0.f;
	FVector HomePosition = FVector::ZeroVector;
	float MaxDistanceFromHome = 15000.f;
};
