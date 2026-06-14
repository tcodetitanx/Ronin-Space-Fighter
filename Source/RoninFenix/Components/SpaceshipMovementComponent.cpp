#include "Components/SpaceshipMovementComponent.h"

USpaceshipMovementComponent::USpaceshipMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	BoostRemaining = Stats.BoostDuration;
}

void USpaceshipMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AActor* Owner = GetOwner();
	if (!Owner) return;

	// Throttle
	CurrentThrottle = FMath::Clamp(CurrentThrottle + ThrottleInput * DeltaTime, 0.f, 1.f);

	// Boost
	if (bWantsBoosting && BoostRemaining > 0.f && BoostCooldownRemaining <= 0.f)
	{
		bIsBoosting = true;
		BoostRemaining -= DeltaTime;
		if (BoostRemaining <= 0.f)
		{
			bIsBoosting = false;
			BoostCooldownRemaining = Stats.BoostCooldown;
		}
	}
	else
	{
		bIsBoosting = false;
		if (BoostCooldownRemaining > 0.f)
		{
			BoostCooldownRemaining -= DeltaTime;
			if (BoostCooldownRemaining <= 0.f)
			{
				BoostRemaining = Stats.BoostDuration;
			}
		}
	}

	float TargetSpeed = bIsBoosting ? Stats.BoostMaxSpeed : Stats.MaxSpeed * CurrentThrottle;
	CurrentSpeed = FMath::FInterpTo(CurrentSpeed, TargetSpeed, DeltaTime, Stats.Acceleration / Stats.MaxSpeed);

	// Rotation
	FRotator CurrentRotation = Owner->GetActorRotation();
	FRotator DeltaRotation(
		PitchInput * Stats.PitchRate * DeltaTime,
		YawInput * Stats.YawRate * DeltaTime,
		RollInput * Stats.RollRate * DeltaTime
	);

	FQuat CurrentQuat = CurrentRotation.Quaternion();
	FQuat DeltaQuat = DeltaRotation.Quaternion();
	FQuat NewQuat = CurrentQuat * DeltaQuat;

	Owner->SetActorRotation(NewQuat.Rotator());

	// Movement
	FVector Forward = Owner->GetActorForwardVector();
	FVector NewLocation = Owner->GetActorLocation() + Forward * CurrentSpeed * DeltaTime;
	Owner->SetActorLocation(NewLocation);
}

void USpaceshipMovementComponent::SetThrottleInput(float Value)
{
	ThrottleInput = FMath::Clamp(Value, -1.f, 1.f);
}

void USpaceshipMovementComponent::SetPitchInput(float Value)
{
	PitchInput = FMath::Clamp(Value, -1.f, 1.f);
}

void USpaceshipMovementComponent::SetYawInput(float Value)
{
	YawInput = FMath::Clamp(Value, -1.f, 1.f);
}

void USpaceshipMovementComponent::SetRollInput(float Value)
{
	RollInput = FMath::Clamp(Value, -1.f, 1.f);
}

void USpaceshipMovementComponent::SetBoostInput(bool bBoosting)
{
	bWantsBoosting = bBoosting;
}

float USpaceshipMovementComponent::GetBoostPercent() const
{
	return Stats.BoostDuration > 0.f ? BoostRemaining / Stats.BoostDuration : 0.f;
}
