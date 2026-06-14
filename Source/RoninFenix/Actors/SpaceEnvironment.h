#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "SpaceEnvironment.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;
class UDirectionalLightComponent;

UCLASS()
class RONINFENIX_API ASpaceEnvironment : public AActor
{
	GENERATED_BODY()

public:
	ASpaceEnvironment();

protected:
	virtual void BeginPlay() override;

private:
	void CreateStarfield();
	void CreateNebula();
	void CreateLighting();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> Root;

	UPROPERTY()
	TArray<TObjectPtr<UPointLightComponent>> StarLights;
};
