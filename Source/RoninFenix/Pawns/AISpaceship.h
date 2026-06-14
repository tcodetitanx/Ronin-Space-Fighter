#pragma once

#include "CoreMinimal.h"
#include "Pawns/SpaceshipBase.h"
#include "AISpaceship.generated.h"

UCLASS()
class RONINFENIX_API AAISpaceship : public ASpaceshipBase
{
	GENERATED_BODY()

public:
	AAISpaceship();

	void Respawn(FVector SpawnLocation, FRotator SpawnRotation);

protected:
	virtual void BeginPlay() override;
};
