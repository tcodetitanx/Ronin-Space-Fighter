#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SpaceTypes.h"
#include "Sound/SoundWave.h"
#include "WeaponComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RONINFENIX_API UWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWeaponComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void StartFiringLasers();
	void StopFiringLasers();
	void FireMissile(AActor* Target);

	UFUNCTION(BlueprintCallable)
	int32 GetMissilesRemaining() const { return MissilesRemaining; }

	UFUNCTION(BlueprintCallable)
	float GetOverheatPercent() const { return bOverheated ? (OverheatRecoveryTimer / OverheatRecoveryTime) : (CurrentHeat / MaxHeat); }

	UFUNCTION(BlueprintCallable)
	bool IsOverheated() const { return bOverheated; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float LaserDamage = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float LaserFireRate = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float MissileDamage = 40.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	int32 MaxMissiles = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	ESpaceTeam OwnerTeam = ESpaceTeam::Neutral;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float LaserShotVolume = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.1", ClampMax = "3.0"))
	float LaserShotPitch = 1.f;

	void SetWeaponStats(float InLaserDamage, float InFireRate, float InMissileDamage, int32 InMaxMissiles);

	/** Set world-space aim point (from camera trace). Lasers converge here instead of firing straight ahead. */
	void SetAimPoint(const FVector& InAimPoint) { AimPoint = InAimPoint; bHasAimPoint = true; }
	void ClearAimPoint() { bHasAimPoint = false; }

private:
	void FireLaser();

	bool bFiringLasers = false;
	float FireCooldown = 0.f;
	int32 MissilesRemaining = 4;
	float CurrentHeat = 0.f;
	float MaxHeat = 500.f;
	float HeatPerShot = 8.f;
	float CooldownRate = 30.f;
	bool bOverheated = false;
	float OverheatThreshold = 500.f;
	float OverheatRecoveryTimer = 0.f;
	float OverheatRecoveryTime = 3.f;
	bool bAlternateBarrel = false;

	UPROPERTY()
	TObjectPtr<USoundWave> LaserSound;

	FVector AimPoint = FVector::ZeroVector;
	bool bHasAimPoint = false;
};
