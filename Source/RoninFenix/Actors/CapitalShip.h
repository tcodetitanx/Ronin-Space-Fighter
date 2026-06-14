#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpaceTypes.h"
#include "CapitalShip.generated.h"

class UProceduralMeshComponent;
class UBoxComponent;
class UHealthShieldComponent;
class UPointLightComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCapitalShipDestroyed, ESpaceTeam, Team);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSubsystemDestroyed, ESpaceTeam, Team, ESubsystemType, Subsystem);

UCLASS()
class RONINFENIX_API ACapitalShip : public AActor
{
	GENERATED_BODY()

public:
	ACapitalShip();

	virtual void Tick(float DeltaTime) override;

	void InitializeCapitalShip(ESpaceTeam InTeam);

	UFUNCTION(BlueprintCallable)
	ESpaceTeam GetTeam() const { return Team; }

	UFUNCTION(BlueprintCallable)
	bool IsDestroyed() const { return bIsDestroyed; }

	UFUNCTION(BlueprintCallable)
	float GetHealthPercent() const;

	UFUNCTION(BlueprintCallable)
	const TArray<FCapitalShipSubsystem>& GetSubsystems() const { return Subsystems; }

	UPROPERTY(BlueprintAssignable)
	FOnCapitalShipDestroyed OnCapitalShipDestroyed;

	UPROPERTY(BlueprintAssignable)
	FOnSubsystemDestroyed OnSubsystemDestroyed;

protected:
	virtual void BeginPlay() override;

private:
	void FireTurrets(float DeltaTime);
	AActor* FindNearestEnemy() const;

	UFUNCTION()
	void OnShipDamaged(float Damage, AActor* DamageCauser);

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> CollisionComp;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProceduralMeshComponent> ShipMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UHealthShieldComponent> HealthComp;

	UPROPERTY()
	TArray<TObjectPtr<UPointLightComponent>> SubsystemLights;

	ESpaceTeam Team = ESpaceTeam::Neutral;
	bool bIsDestroyed = false;

	UPROPERTY()
	TArray<FCapitalShipSubsystem> Subsystems;

	float TurretFireCooldown = 0.f;
	float TurretFireRate = 2.f;
	float TurretRange = 6000.f;
	float TurretDamage = 5.f;
};
