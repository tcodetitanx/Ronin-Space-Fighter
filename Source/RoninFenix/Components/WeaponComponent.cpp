#include "Components/WeaponComponent.h"
#include "Actors/LaserProjectile.h"
#include "Actors/HomingMissile.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"

UWeaponComponent::UWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (FireCooldown > 0.f)
	{
		FireCooldown -= DeltaTime;
	}

	// Heat management
	if (!bFiringLasers)
	{
		CurrentHeat = FMath::Max(0.f, CurrentHeat - CooldownRate * DeltaTime);
		if (bOverheated && CurrentHeat <= CooldownThreshold)
		{
			bOverheated = false;
		}
	}

	if (bFiringLasers && !bOverheated && FireCooldown <= 0.f)
	{
		FireLaser();
	}
}

void UWeaponComponent::StartFiringLasers()
{
	bFiringLasers = true;
}

void UWeaponComponent::StopFiringLasers()
{
	bFiringLasers = false;
}

void UWeaponComponent::FireLaser()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	UWorld* World = GetWorld();
	if (!World) return;

	FireCooldown = 1.f / LaserFireRate;
	CurrentHeat += HeatPerShot;

	if (CurrentHeat >= OverheatThreshold)
	{
		bOverheated = true;
		bFiringLasers = false;
		return;
	}

	FVector Forward = Owner->GetActorForwardVector();
	FVector Right = Owner->GetActorRightVector();
	float BarrelOffset = bAlternateBarrel ? 60.f : -60.f;
	bAlternateBarrel = !bAlternateBarrel;

	FVector SpawnLoc = Owner->GetActorLocation() + Forward * 300.f + Right * BarrelOffset;
	FRotator SpawnRot = Owner->GetActorRotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Owner;
	SpawnParams.Instigator = Cast<APawn>(Owner);
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ALaserProjectile* Laser = World->SpawnActor<ALaserProjectile>(ALaserProjectile::StaticClass(), SpawnLoc, SpawnRot, SpawnParams);
	if (Laser)
	{
		Laser->Initialize(LaserDamage, SpaceConstants::LaserSpeed, OwnerTeam);
	}
}

void UWeaponComponent::FireMissile(AActor* Target)
{
	if (MissilesRemaining <= 0 || !Target) return;

	AActor* Owner = GetOwner();
	if (!Owner) return;

	UWorld* World = GetWorld();
	if (!World) return;

	MissilesRemaining--;

	FVector SpawnLoc = Owner->GetActorLocation() + Owner->GetActorForwardVector() * 200.f - Owner->GetActorUpVector() * 50.f;
	FRotator SpawnRot = Owner->GetActorRotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Owner;
	SpawnParams.Instigator = Cast<APawn>(Owner);
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AHomingMissile* Missile = World->SpawnActor<AHomingMissile>(AHomingMissile::StaticClass(), SpawnLoc, SpawnRot, SpawnParams);
	if (Missile)
	{
		Missile->Initialize(MissileDamage, SpaceConstants::MissileSpeed, Target, OwnerTeam);
	}
}

void UWeaponComponent::SetWeaponStats(float InLaserDamage, float InFireRate, float InMissileDamage, int32 InMaxMissiles)
{
	LaserDamage = InLaserDamage;
	LaserFireRate = InFireRate;
	MissileDamage = InMissileDamage;
	MaxMissiles = InMaxMissiles;
	MissilesRemaining = MaxMissiles;
}
