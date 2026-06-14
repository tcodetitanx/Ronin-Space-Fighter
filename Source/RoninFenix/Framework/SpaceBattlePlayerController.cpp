#include "Framework/SpaceBattlePlayerController.h"
#include "Framework/SpaceBattleGameMode.h"
#include "Framework/SpaceBattlePlayerState.h"
#include "Pawns/PlayerSpaceship.h"
#include "Components/HealthShieldComponent.h"
#include "Framework/SpaceBattleGameState.h"

ASpaceBattlePlayerController::ASpaceBattlePlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	bShowMouseCursor = false;
}

void ASpaceBattlePlayerController::BeginPlay()
{
	Super::BeginPlay();

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
}

void ASpaceBattlePlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bWaitingToRespawn)
	{
		RespawnTimer -= DeltaTime;
		if (RespawnTimer <= 0.f)
		{
			bWaitingToRespawn = false;
			RequestRespawn();
		}
	}
}

void ASpaceBattlePlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	APlayerSpaceship* Ship = Cast<APlayerSpaceship>(InPawn);
	if (Ship)
	{
		Ship->GetHealthComp()->OnDeath.AddDynamic(this, &ASpaceBattlePlayerController::OnPlayerShipDestroyed);
	}
}

void ASpaceBattlePlayerController::OnPlayerShipDestroyed()
{
	ASpaceBattlePlayerState* PS = GetPlayerState<ASpaceBattlePlayerState>();
	if (PS)
	{
		PS->AddDeath();
	}

	bWaitingToRespawn = true;
	RespawnTimer = SpaceConstants::RespawnDelay;
}

void ASpaceBattlePlayerController::RequestRespawn()
{
	ASpaceBattleGameMode* GM = Cast<ASpaceBattleGameMode>(GetWorld()->GetAuthGameMode());
	if (GM)
	{
		GM->RespawnPlayer(this);
	}
}
