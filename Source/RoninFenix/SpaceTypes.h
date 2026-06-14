#pragma once

#include "CoreMinimal.h"
#include "SpaceTypes.generated.h"

UENUM(BlueprintType)
enum class ESpaceTeam : uint8
{
	Alpha,
	Omega,
	Neutral
};

UENUM(BlueprintType)
enum class EShipClass : uint8
{
	Fighter,
	Interceptor,
	Bomber
};

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	Laser,
	Missile
};

UENUM(BlueprintType)
enum class ESubsystemType : uint8
{
	ShieldGenerator,
	EngineArray,
	WeaponBattery,
	Bridge
};

USTRUCT(BlueprintType)
struct FShipStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHealth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxShield = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ShieldRegenRate = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ShieldRegenDelay = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSpeed = 4000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BoostMaxSpeed = 8000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Acceleration = 2000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PitchRate = 80.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float YawRate = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RollRate = 120.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BoostDuration = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BoostCooldown = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LaserDamage = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LaserFireRate = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MissileDamage = 40.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MissileLockTime = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MissileCount = 4;
};

USTRUCT(BlueprintType)
struct FCapitalShipSubsystem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESubsystemType Type = ESubsystemType::ShieldGenerator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHealth = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentHealth = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector RelativeLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDestroyed = false;
};

namespace SpaceConstants
{
	constexpr float WorldBoundsRadius = 50000.f;
	constexpr float CapitalShipSpawnDistance = 30000.f;
	constexpr float PlayerSpawnDistance = 5000.f;
	constexpr int32 AIShipsPerTeam = 8;
	constexpr float RespawnDelay = 3.f;
	constexpr int32 ScoreToWin = 100;
	constexpr float LaserSpeed = 20000.f;
	constexpr float MissileSpeed = 8000.f;
	constexpr float LaserLifetime = 3.f;
	constexpr float MissileLifetime = 8.f;
	constexpr float LockOnRange = 8000.f;
	constexpr float LockOnAngle = 15.f;
	constexpr float TargetingRange = 12000.f;
}
