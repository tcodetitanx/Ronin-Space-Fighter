#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpaceTypes.h"
#include "LaserProjectile.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UPointLightComponent;

UCLASS()
class RONINFENIX_API ALaserProjectile : public AActor
{
	GENERATED_BODY()

public:
	ALaserProjectile();

	virtual void Tick(float DeltaTime) override;

	void Initialize(float InDamage, float InSpeed, ESpaceTeam InTeam);

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> CollisionComp;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> MeshComp;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPointLightComponent> LightComp;

	float Damage = 8.f;
	float Speed = 20000.f;
	ESpaceTeam Team = ESpaceTeam::Neutral;
	float LifeRemaining = 3.f;
};
