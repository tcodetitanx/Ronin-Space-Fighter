#pragma once

#include "CoreMinimal.h"
#include "Pawns/SpaceshipBase.h"
#include "PlayerSpaceship.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

UCLASS()
class RONINFENIX_API APlayerSpaceship : public ASpaceshipBase
{
	GENERATED_BODY()

public:
	APlayerSpaceship();

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaTime) override;

	UCameraComponent* GetFollowCamera() const { return FollowCamera; }

protected:
	virtual void BeginPlay() override;

private:
	void CreateInputActions();

	void HandleThrottle(const FInputActionValue& Value);
	void HandleThrottleStop(const FInputActionValue& Value);
	void HandleSteering(const FInputActionValue& Value);
	void HandleSteeringStop(const FInputActionValue& Value);
	void HandleRoll(const FInputActionValue& Value);
	void HandleFireStart(const FInputActionValue& Value);
	void HandleFireStop(const FInputActionValue& Value);
	void HandleMissile(const FInputActionValue& Value);
	void HandleBoostStart(const FInputActionValue& Value);
	void HandleBoostStop(const FInputActionValue& Value);
	void HandleCycleTarget(const FInputActionValue& Value);
	void HandleLockOnStart(const FInputActionValue& Value);
	void HandleLockOnStop(const FInputActionValue& Value);
	void HandleBarrelRoll(const FInputActionValue& Value);

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY()
	TObjectPtr<UInputMappingContext> ShipMappingContext;

	UPROPERTY()
	TObjectPtr<UInputAction> IA_Throttle;

	UPROPERTY()
	TObjectPtr<UInputAction> IA_Steering;

	UPROPERTY()
	TObjectPtr<UInputAction> IA_Roll;

	UPROPERTY()
	TObjectPtr<UInputAction> IA_Fire;

	UPROPERTY()
	TObjectPtr<UInputAction> IA_Missile;

	UPROPERTY()
	TObjectPtr<UInputAction> IA_Boost;

	UPROPERTY()
	TObjectPtr<UInputAction> IA_CycleTarget;

	UPROPERTY()
	TObjectPtr<UInputAction> IA_LockOn;

	UPROPERTY()
	TObjectPtr<UInputAction> IA_BarrelRoll;

	float BarrelRollTimer = 0.f;
	bool bBarrelRolling = false;
	float BarrelRollDirection = 1.f;
};
