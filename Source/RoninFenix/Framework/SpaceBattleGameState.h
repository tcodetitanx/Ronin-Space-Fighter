#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SpaceTypes.h"
#include "SpaceBattleGameState.generated.h"

UCLASS()
class RONINFENIX_API ASpaceBattleGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	int32 GetTeamScore(ESpaceTeam Team) const;

	UFUNCTION(BlueprintCallable)
	void AddTeamScore(ESpaceTeam Team, int32 Points);

	UFUNCTION(BlueprintCallable)
	float GetMatchTimeRemaining() const { return MatchTimeRemaining; }

	UFUNCTION(BlueprintCallable)
	bool IsMatchOver() const { return bMatchOver; }

	void SetMatchOver(ESpaceTeam WinningTeam);
	ESpaceTeam GetWinningTeam() const { return WinTeam; }

	void TickMatchTime(float DeltaTime);
	void SetMatchDuration(float Duration) { MatchTimeRemaining = Duration; }

	void AddKillFeedEntry(const FString& KillerName, const FString& VictimName);
	const TArray<FString>& GetKillFeed() const { return KillFeed; }

private:
	int32 AlphaScore = 0;
	int32 OmegaScore = 0;
	float MatchTimeRemaining = 600.f;
	bool bMatchOver = false;
	ESpaceTeam WinTeam = ESpaceTeam::Neutral;

	UPROPERTY()
	TArray<FString> KillFeed;
};
