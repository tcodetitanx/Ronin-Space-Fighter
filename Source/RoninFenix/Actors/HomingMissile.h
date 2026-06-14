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
	float TurnRate = 8.f;
	ESpaceTeam Team = ESpaceTeam::Neutral;
	float ArmingTime = 0.3f;
	float ArmTimer = 0.f;

	// Ring buffer trail
	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> TrailDots;
	int32 TrailIndex = 0;
	float TrailTimer = 0.f;
	static constexpr int32 TrailCount = 20;
	static constexpr float TrailInterval = 0.02f;
};
