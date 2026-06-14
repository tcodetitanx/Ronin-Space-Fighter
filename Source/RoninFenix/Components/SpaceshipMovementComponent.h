#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SpaceTypes.h"
#include "SpaceshipMovementComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RONINFENIX_API USpaceshipMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USpaceshipMovementComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SetThrottleInput(float Value);
	void SetPitchInput(float Value);
	void SetYawInput(float Value);
	void SetRollInput(float Value);
	void SetBoostInput(bool bBoosting);

	UFUNCTION(BlueprintCallable)
	float GetCurrentSpeed() const { return CurrentSpeed; }

	UFUNCTION(BlueprintCallable)
	float GetThrottlePercent() const { return ThrottleInput; }

	UFUNCTION(BlueprintCallable)
	float GetBoostPercent() const;

	UFUNCTION(BlueprintCallable)
	bool IsBoosting() const { return bIsBoosting; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship Stats")
	FShipStats Stats;

private:
	float ThrottleInput = 0.f;
	float PitchInput = 0.f;
	float YawInput = 0.f;
	float RollInput = 0.f;
	bool bWantsBoosting = false;
	bool bIsBoosting = false;
	float CurrentSpeed = 0.f;
	float BoostRemaining = 0.f;
	float BoostCooldownRemaining = 0.f;
	float CurrentThrottle = 0.5f;
};
