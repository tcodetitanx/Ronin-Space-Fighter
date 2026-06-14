#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SpaceTypes.h"
#include "Pawns/AISpaceship.h"
#include "Actors/CapitalShip.h"
#include "Actors/Asteroid.h"
#include "SpaceBattleGameMode.generated.h"

class APlayerSpaceship;
class ASpaceBattlePlayerController;

UCLASS()
class RONINFENIX_API ASpaceBattleGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASpaceBattleGameMode();

	virtual void StartPlay() override;
	virtual void Tick(float DeltaTime) override;

	void RespawnPlayer(ASpaceBattlePlayerController* PC);
	void OnShipKilled(AActor* Victim, AActor* Killer);

	UFUNCTION(BlueprintCallable)
	ACapitalShip* GetCapitalShip(ESpaceTeam Team) const;

	virtual void RestartPlayer(AController* NewPlayer) override;

private:
	void SpawnCapitalShips();
	void SpawnAIShips();
	void SpawnAsteroids();
	void SpawnEnvironment();
	void CheckWinConditions();
	void HandleAIRespawns(float DeltaTime);

	UFUNCTION()
	void OnCapitalShipDestroyedHandler(ESpaceTeam Team);

	UFUNCTION()
	void OnAIShipDestroyed();

	FVector GetSpawnLocation(ESpaceTeam Team) const;

	UPROPERTY()
	TObjectPtr<ACapitalShip> AlphaCapitalShip;

	UPROPERTY()
	TObjectPtr<ACapitalShip> OmegaCapitalShip;

	UPROPERTY()
	TArray<TObjectPtr<AAISpaceship>> AIShips;

	UPROPERTY()
	TArray<TObjectPtr<AAsteroid>> Asteroids;

	UPROPERTY()
	TMap<TObjectPtr<AAISpaceship>, float> RespawnTimers;
};
