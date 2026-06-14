#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpaceTypes.h"
#include "HomingMissile.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UPointLightComponent;

UCLASS()
class RONINFENIX_API AHomingMissile : public AActor
{
	GENERATED_BODY()

public:
	AHomingMissile();

	virtual void Tick(float DeltaTime) override;

	void Initialize(float InDamage, float InSpeed, AActor* InTarget, ESpaceTeam InTeam);

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> CollisionComp;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> MeshComp;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPointLightComponent> LightComp;

	UPROPERTY()
	TObjectPtr<AActor> Target;

	float Damage = 40.f;
	float Speed = 8000.f;
	float TurnRate = 3.f;
	ESpaceTeam Team = ESpaceTeam::Neutral;
	float ArmingTime = 0.3f;
	float ArmTimer = 0.f;
};
