#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "SpaceTypes.h"
#include "SpaceshipAIController.generated.h"

class ASpaceshipBase;
class ACapitalShip;

UENUM()
enum class EAIRole : uint8
{
	Defender,
	Attacker
};

UENUM()
enum class EAIState : uint8
{
	Patrol,
	Engage,
	AttackCapitalShip,
	ChasePlayer,
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
	void ExecuteChasePlayer(float DeltaTime);
	void ExecuteEvade(float DeltaTime);
	void ExecuteReturnToBase(float DeltaTime);

	void FlyToward(FVector TargetLocation, float DeltaTime);
	ASpaceshipBase* FindNearestEnemy() const;
	ASpaceshipBase* FindPlayerShip() const;
	ACapitalShip* FindEnemyCapitalShip() const;

	UFUNCTION()
	void OnTookDamage(float Damage, AActor* DamageCauser);

	UPROPERTY()
	TObjectPtr<ASpaceshipBase> ShipPawn;

	UPROPERTY()
	TObjectPtr<AActor> CurrentTarget;

	EAIRole Role = EAIRole::Defender;
	EAIState CurrentState = EAIState::Patrol;
	FVector PatrolTarget;
	float DecisionTimer = 0.f;
	float DecisionInterval = 2.f;
	float EvadeTimer = 0.f;
	float ChasePlayerTimer = 0.f;
	float EngageRange = 6000.f;
	float DisengageRange = 12000.f;
	float PatrolRadius = 8000.f;
	FVector HomePosition = FVector::ZeroVector;
	float MaxDistanceFromHome = 20000.f;

	// Smooth steering
	float CurrentYawInput = 0.f;
	float CurrentPitchInput = 0.f;
	float CurrentRollInput = 0.f;
	float SteerSmoothing = 4.f;

	// Damage awareness
	float LastDamageTime = -10.f;
	bool bRecentlyDamaged = false;
};
