#include "Framework/SpaceBattleGameState.h"

int32 ASpaceBattleGameState::GetTeamScore(ESpaceTeam Team) const
{
	switch (Team)
	{
	case ESpaceTeam::Alpha: return AlphaScore;
	case ESpaceTeam::Omega: return OmegaScore;
	default: return 0;
	}
}

void ASpaceBattleGameState::AddTeamScore(ESpaceTeam Team, int32 Points)
{
	switch (Team)
	{
	case ESpaceTeam::Alpha: AlphaScore += Points; break;
	case ESpaceTeam::Omega: OmegaScore += Points; break;
	default: break;
	}
}

void ASpaceBattleGameState::SetMatchOver(ESpaceTeam WinningTeam)
{
	bMatchOver = true;
	WinTeam = WinningTeam;
}

void ASpaceBattleGameState::TickMatchTime(float DeltaTime)
{
	if (!bMatchOver)
	{
		MatchTimeRemaining = FMath::Max(0.f, MatchTimeRemaining - DeltaTime);
	}
}

void ASpaceBattleGameState::AddKillFeedEntry(const FString& KillerName, const FString& VictimName)
{
	FString Entry = FString::Printf(TEXT("%s destroyed %s"), *KillerName, *VictimName);
	KillFeed.Insert(Entry, 0);
	if (KillFeed.Num() > 5)
	{
		KillFeed.SetNum(5);
	}
}
