#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SpaceTypes.h"
#include "SpaceBattlePlayerController.generated.h"

class APlayerSpaceship;

UCLASS()
class RONINFENIX_API ASpaceBattlePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ASpaceBattlePlayerController();

	void RequestRespawn();

	UFUNCTION(BlueprintCallable)
	float GetRespawnTimeRemaining() const { return RespawnTimer; }

	UFUNCTION(BlueprintCallable)
	bool IsWaitingToRespawn() const { return bWaitingToRespawn; }

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void OnPossess(APawn* InPawn) override;

private:
	UFUNCTION()
	void OnPlayerShipDestroyed();

	bool bWaitingToRespawn = false;
	float RespawnTimer = 0.f;
};
