#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "SpaceTypes.h"
#include "SpaceBattlePlayerState.generated.h"

UCLASS()
class RONINFENIX_API ASpaceBattlePlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	int32 GetKills() const { return Kills; }

	UFUNCTION(BlueprintCallable)
	int32 GetDeaths() const { return Deaths; }

	UFUNCTION(BlueprintCallable)
	int32 GetAssists() const { return Assists; }

	UFUNCTION(BlueprintCallable)
	ESpaceTeam GetTeam() const { return Team; }

	void AddKill() { Kills++; }
	void AddDeath() { Deaths++; }
	void AddAssist() { Assists++; }
	void SetTeam(ESpaceTeam InTeam) { Team = InTeam; }

private:
	int32 Kills = 0;
	int32 Deaths = 0;
	int32 Assists = 0;
	ESpaceTeam Team = ESpaceTeam::Alpha;
};
