#pragma once

#include "CoreMinimal.h"
#include "Pawns/SpaceshipBase.h"
#include "PlayerSpaceship.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UAudioComponent;
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

	/** Distance of camera behind the ship (tweak live in Details panel) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float CameraDistance = 400.f;

	/** How far above the ship the camera sits — pushes the ship below the reticle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float CameraHeight = 120.f;

	/** Mouse steering multiplier (higher = faster turning) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controls")
	float SteeringSensitivity = 6.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float ThrusterHumVolume = 0.125f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float ThrusterLowVolume = 0.125f;

	/** Pixel offset of reticle from screen center (accumulated from mouse input) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Controls")
	FVector2D ReticleOffset = FVector2D::ZeroVector;

	/** Maximum pixel radius the reticle can move from center */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controls")
	float ReticleRadius = 200.f;

	/** Get current reticle offset for HUD drawing */
	FVector2D GetReticleOffset() const { return ReticleOffset; }
	float GetReticleRadius() const { return ReticleRadius; }

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

	// Engine exhaust ring buffer trail
	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> EngineTrailDots;
	int32 EngineTrailIndex = 0;
	float EngineTrailTimer = 0.f;
	static constexpr int32 EngineTrailCount = 30;
	static constexpr float EngineTrailInterval = 0.02f;

	UPROPERTY()
	TObjectPtr<UAudioComponent> ThrusterHumAudio;

	UPROPERTY()
	TObjectPtr<UAudioComponent> ThrusterLowAudio;
};
