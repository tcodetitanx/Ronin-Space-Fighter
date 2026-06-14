#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Asteroid.generated.h"

class UProceduralMeshComponent;
class USphereComponent;

UCLASS()
class RONINFENIX_API AAsteroid : public AActor
{
	GENERATED_BODY()

public:
	AAsteroid();

	void InitializeAsteroid(float InRadius, int32 Seed);

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> CollisionComp;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProceduralMeshComponent> MeshComp;

	FRotator RotationSpeed;
	float CollisionDamage = 30.f;
};
