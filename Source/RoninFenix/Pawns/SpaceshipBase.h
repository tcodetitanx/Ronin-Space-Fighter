#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "SpaceTypes.h"
#include "SpaceshipBase.generated.h"

class UProceduralMeshComponent;
class USpaceshipMovementComponent;
class UHealthShieldComponent;
class UWeaponComponent;
class UTargetingComponent;
class USphereComponent;
class UPointLightComponent;

UCLASS()
class RONINFENIX_API ASpaceshipBase : public APawn
{
	GENERATED_BODY()

public:
	ASpaceshipBase();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	ESpaceTeam GetTeam() const { return Team; }

	UFUNCTION(BlueprintCallable)
	EShipClass GetShipClass() const { return ShipClass; }

	void SetTeam(ESpaceTeam InTeam);
	void SetShipClass(EShipClass InClass);
	void InitializeShip();

	UFUNCTION()
	void OnShipDestroyed();

	USpaceshipMovementComponent* GetMovementComp() const { return MovementComp; }
	UHealthShieldComponent* GetHealthComp() const { return HealthComp; }
	UWeaponComponent* GetWeaponComp() const { return WeaponComp; }
	UTargetingComponent* GetTargetingComp() const { return TargetingComp; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USphereComponent> CollisionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UProceduralMeshComponent> ShipMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USpaceshipMovementComponent> MovementComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UHealthShieldComponent> HealthComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UWeaponComponent> WeaponComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UTargetingComponent> TargetingComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UPointLightComponent> EngineLight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESpaceTeam Team = ESpaceTeam::Neutral;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EShipClass ShipClass = EShipClass::Fighter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FShipStats ShipStats;

	FLinearColor GetTeamColor() const;
	void ApplyShipClassStats();
};
