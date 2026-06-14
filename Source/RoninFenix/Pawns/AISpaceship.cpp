#include "Pawns/AISpaceship.h"
#include "AI/SpaceshipAIController.h"
#include "Components/HealthShieldComponent.h"
#include "Components/SpaceshipMovementComponent.h"
#include "Components/WeaponComponent.h"

AAISpaceship::AAISpaceship()
{
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = ASpaceshipAIController::StaticClass();
}

void AAISpaceship::BeginPlay()
{
	Super::BeginPlay();
}

void AAISpaceship::Respawn(FVector SpawnLocation, FRotator SpawnRotation)
{
	SetActorLocation(SpawnLocation);
	SetActorRotation(SpawnRotation);
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetActorTickEnabled(true);

	InitializeShip();
}
