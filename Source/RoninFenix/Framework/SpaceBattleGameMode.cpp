#include "Framework/SpaceBattleGameMode.h"
#include "Framework/SpaceBattleGameState.h"
#include "Framework/SpaceBattlePlayerState.h"
#include "Framework/SpaceBattlePlayerController.h"
#include "Pawns/PlayerSpaceship.h"
#include "Pawns/AISpaceship.h"
#include "Actors/CapitalShip.h"
#include "Actors/Asteroid.h"
#include "Actors/SpaceEnvironment.h"
#include "Components/HealthShieldComponent.h"
#include "UI/SpaceBattleHUD.h"

ASpaceBattleGameMode::ASpaceBattleGameMode()
{
	DefaultPawnClass = APlayerSpaceship::StaticClass();
	PlayerControllerClass = ASpaceBattlePlayerController::StaticClass();
	GameStateClass = ASpaceBattleGameState::StaticClass();
	PlayerStateClass = ASpaceBattlePlayerState::StaticClass();
	HUDClass = ASpaceBattleHUD::StaticClass();

	PrimaryActorTick.bCanEverTick = true;
}

void ASpaceBattleGameMode::StartPlay()
{
	Super::StartPlay();

	SpawnEnvironment();
	SpawnCapitalShips();
	SpawnAIShips();
	SpawnAsteroids();

	ASpaceBattleGameState* GS = GetGameState<ASpaceBattleGameState>();
	if (GS)
	{
		GS->SetMatchDuration(600.f);
	}
}

void ASpaceBattleGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ASpaceBattleGameState* GS = GetGameState<ASpaceBattleGameState>();
	if (GS && !GS->IsMatchOver())
	{
		GS->TickMatchTime(DeltaTime);
		CheckWinConditions();
		HandleAIRespawns(DeltaTime);
	}
}

void ASpaceBattleGameMode::RestartPlayer(AController* NewPlayer)
{
	if (!NewPlayer) return;

	APlayerController* PC = Cast<APlayerController>(NewPlayer);
	if (!PC) return;

	if (PC->GetPawn()) return;

	FVector SpawnLoc = GetSpawnLocation(ESpaceTeam::Alpha);
	FRotator SpawnRot = FRotator(0.f, 90.f, 0.f);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	APlayerSpaceship* Ship = GetWorld()->SpawnActor<APlayerSpaceship>(APlayerSpaceship::StaticClass(), SpawnLoc, SpawnRot, SpawnParams);
	if (Ship)
	{
		Ship->SetTeam(ESpaceTeam::Alpha);
		Ship->SetShipClass(EShipClass::Fighter);
		PC->Possess(Ship);
	}

	ASpaceBattlePlayerState* PS = PC->GetPlayerState<ASpaceBattlePlayerState>();
	if (PS)
	{
		PS->SetTeam(ESpaceTeam::Alpha);
	}
}

void ASpaceBattleGameMode::SpawnCapitalShips()
{
	UWorld* World = GetWorld();
	if (!World) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Alpha capital ship
	FVector AlphaLoc(-SpaceConstants::CapitalShipSpawnDistance, 0.f, 10000.f);
	AlphaCapitalShip = World->SpawnActor<ACapitalShip>(ACapitalShip::StaticClass(), AlphaLoc, FRotator(0.f, 0.f, 0.f), SpawnParams);
	if (AlphaCapitalShip)
	{
		AlphaCapitalShip->InitializeCapitalShip(ESpaceTeam::Alpha);
		AlphaCapitalShip->OnCapitalShipDestroyed.AddDynamic(this, &ASpaceBattleGameMode::OnCapitalShipDestroyedHandler);
	}

	// Omega capital ship
	FVector OmegaLoc(SpaceConstants::CapitalShipSpawnDistance, 0.f, 10000.f);
	OmegaCapitalShip = World->SpawnActor<ACapitalShip>(ACapitalShip::StaticClass(), OmegaLoc, FRotator(0.f, 180.f, 0.f), SpawnParams);
	if (OmegaCapitalShip)
	{
		OmegaCapitalShip->InitializeCapitalShip(ESpaceTeam::Omega);
		OmegaCapitalShip->OnCapitalShipDestroyed.AddDynamic(this, &ASpaceBattleGameMode::OnCapitalShipDestroyedHandler);
	}
}

void ASpaceBattleGameMode::SpawnAIShips()
{
	UWorld* World = GetWorld();
	if (!World) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	EShipClass Classes[] = { EShipClass::Fighter, EShipClass::Fighter, EShipClass::Interceptor, EShipClass::Bomber };

	for (int32 TeamIdx = 0; TeamIdx < 2; ++TeamIdx)
	{
		ESpaceTeam Team = (TeamIdx == 0) ? ESpaceTeam::Alpha : ESpaceTeam::Omega;

		for (int32 i = 0; i < SpaceConstants::AIShipsPerTeam; ++i)
		{
			FVector SpawnLoc = GetSpawnLocation(Team) + FMath::VRand() * 2000.f;
			FRotator SpawnRot = FMath::VRand().Rotation();

			AAISpaceship* AIShip = World->SpawnActor<AAISpaceship>(AAISpaceship::StaticClass(), SpawnLoc, SpawnRot, SpawnParams);
			if (AIShip)
			{
				AIShip->SetTeam(Team);
				AIShip->SetShipClass(Classes[i % 4]);
				AIShips.Add(AIShip);

				AIShip->GetHealthComp()->OnDeath.AddDynamic(this, &ASpaceBattleGameMode::OnAIShipDestroyed);
			}
		}
	}
}

void ASpaceBattleGameMode::SpawnAsteroids()
{
	UWorld* World = GetWorld();
	if (!World) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FRandomStream Rand(42);

	for (int32 i = 0; i < 40; ++i)
	{
		FVector SpawnLoc = FMath::VRand() * Rand.FRandRange(5000.f, SpaceConstants::WorldBoundsRadius * 0.8f);
		AAsteroid* Rock = World->SpawnActor<AAsteroid>(AAsteroid::StaticClass(), SpawnLoc, FRotator::ZeroRotator, SpawnParams);
		if (Rock)
		{
			float Radius = Rand.FRandRange(100.f, 600.f);
			Rock->InitializeAsteroid(Radius, i);
			Asteroids.Add(Rock);
		}
	}
}

void ASpaceBattleGameMode::SpawnEnvironment()
{
	UWorld* World = GetWorld();
	if (!World) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	World->SpawnActor<ASpaceEnvironment>(ASpaceEnvironment::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
}

void ASpaceBattleGameMode::OnShipKilled(AActor* Victim, AActor* Killer)
{
	ASpaceBattleGameState* GS = GetGameState<ASpaceBattleGameState>();
	if (!GS) return;

	ASpaceshipBase* VictimShip = Cast<ASpaceshipBase>(Victim);
	if (!VictimShip) return;

	ESpaceTeam VictimTeam = VictimShip->GetTeam();
	ESpaceTeam ScoringTeam = (VictimTeam == ESpaceTeam::Alpha) ? ESpaceTeam::Omega : ESpaceTeam::Alpha;
	GS->AddTeamScore(ScoringTeam, 1);

	FString KillerName = Killer ? Killer->GetName() : TEXT("Unknown");
	FString VictimName = Victim->GetName();
	GS->AddKillFeedEntry(KillerName, VictimName);
}

void ASpaceBattleGameMode::RespawnPlayer(ASpaceBattlePlayerController* PC)
{
	if (!PC) return;

	ASpaceBattleGameState* GS = GetGameState<ASpaceBattleGameState>();
	if (GS && GS->IsMatchOver()) return;

	APawn* OldPawn = PC->GetPawn();
	if (OldPawn)
	{
		OldPawn->Destroy();
	}

	FVector SpawnLoc = GetSpawnLocation(ESpaceTeam::Alpha) + FMath::VRand() * 1000.f;
	FRotator SpawnRot = FRotator::ZeroRotator;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	APlayerSpaceship* NewShip = GetWorld()->SpawnActor<APlayerSpaceship>(APlayerSpaceship::StaticClass(), SpawnLoc, SpawnRot, SpawnParams);
	if (NewShip)
	{
		NewShip->SetTeam(ESpaceTeam::Alpha);
		NewShip->SetShipClass(EShipClass::Fighter);
		PC->Possess(NewShip);
	}
}

ACapitalShip* ASpaceBattleGameMode::GetCapitalShip(ESpaceTeam Team) const
{
	return (Team == ESpaceTeam::Alpha) ? AlphaCapitalShip.Get() : OmegaCapitalShip.Get();
}

void ASpaceBattleGameMode::CheckWinConditions()
{
	ASpaceBattleGameState* GS = GetGameState<ASpaceBattleGameState>();
	if (!GS || GS->IsMatchOver()) return;

	if (AlphaCapitalShip && AlphaCapitalShip->IsDestroyed())
	{
		GS->SetMatchOver(ESpaceTeam::Omega);
		return;
	}

	if (OmegaCapitalShip && OmegaCapitalShip->IsDestroyed())
	{
		GS->SetMatchOver(ESpaceTeam::Alpha);
		return;
	}

	if (GS->GetMatchTimeRemaining() <= 0.f)
	{
		int32 AlphaScore = GS->GetTeamScore(ESpaceTeam::Alpha);
		int32 OmegaScore = GS->GetTeamScore(ESpaceTeam::Omega);
		ESpaceTeam Winner = (AlphaScore >= OmegaScore) ? ESpaceTeam::Alpha : ESpaceTeam::Omega;
		GS->SetMatchOver(Winner);
	}
}

void ASpaceBattleGameMode::HandleAIRespawns(float DeltaTime)
{
	TArray<AAISpaceship*> ToRespawn;

	for (auto& Pair : RespawnTimers)
	{
		Pair.Value -= DeltaTime;
		if (Pair.Value <= 0.f)
		{
			ToRespawn.Add(Pair.Key);
		}
	}

	for (AAISpaceship* Ship : ToRespawn)
	{
		RespawnTimers.Remove(Ship);
		FVector SpawnLoc = GetSpawnLocation(Ship->GetTeam()) + FMath::VRand() * 2000.f;
		Ship->Respawn(SpawnLoc, FMath::VRand().Rotation());
	}

	for (AAISpaceship* Ship : AIShips)
	{
		if (Ship && !Ship->GetHealthComp()->IsAlive() && !RespawnTimers.Contains(Ship))
		{
			RespawnTimers.Add(Ship, SpaceConstants::RespawnDelay + FMath::RandRange(0.f, 3.f));

			OnShipKilled(Ship, nullptr);
		}
	}
}

void ASpaceBattleGameMode::OnCapitalShipDestroyedHandler(ESpaceTeam Team)
{
	ASpaceBattleGameState* GS = GetGameState<ASpaceBattleGameState>();
	if (!GS) return;

	ESpaceTeam Winner = (Team == ESpaceTeam::Alpha) ? ESpaceTeam::Omega : ESpaceTeam::Alpha;
	GS->SetMatchOver(Winner);
}

void ASpaceBattleGameMode::OnAIShipDestroyed()
{
	// Handled in HandleAIRespawns via health check
}

FVector ASpaceBattleGameMode::GetSpawnLocation(ESpaceTeam Team) const
{
	float Direction = (Team == ESpaceTeam::Alpha) ? -1.f : 1.f;
	return FVector(Direction * SpaceConstants::PlayerSpawnDistance, 0.f, 10000.f);
}
