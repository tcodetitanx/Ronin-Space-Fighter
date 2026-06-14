#include "Components/TargetingComponent.h"
#include "Components/HealthShieldComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UTargetingComponent::UTargetingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UTargetingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Validate current target
	if (CurrentTarget)
	{
		UHealthShieldComponent* TargetHealth = CurrentTarget->FindComponentByClass<UHealthShieldComponent>();
		if (TargetHealth && !TargetHealth->IsAlive())
		{
			CurrentTarget = nullptr;
			LockOnProgress = 0.f;
			bMissileLocked = false;
		}
	}

	if (!CurrentTarget)
	{
		CurrentTarget = FindBestTarget();
		LockOnProgress = 0.f;
		bMissileLocked = false;
	}

	// Lock-on
	if (bLockingOn && CurrentTarget)
	{
		AActor* Owner = GetOwner();
		if (Owner)
		{
			FVector ToTarget = CurrentTarget->GetActorLocation() - Owner->GetActorLocation();
			float Distance = ToTarget.Size();
			float Angle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Owner->GetActorForwardVector(), ToTarget.GetSafeNormal())));

			if (Distance <= SpaceConstants::LockOnRange && Angle <= SpaceConstants::LockOnAngle)
			{
				LockOnProgress = FMath::Min(LockOnProgress + DeltaTime / LockOnTime, 1.f);
				bMissileLocked = LockOnProgress >= 1.f;
			}
			else
			{
				LockOnProgress = FMath::Max(0.f, LockOnProgress - DeltaTime * 2.f);
				bMissileLocked = false;
			}
		}
	}
	else
	{
		if (!bLockingOn)
		{
			LockOnProgress = FMath::Max(0.f, LockOnProgress - DeltaTime * 3.f);
			bMissileLocked = false;
		}
	}
}

AActor* UTargetingComponent::FindBestTarget() const
{
	AActor* Owner = GetOwner();
	if (!Owner) return nullptr;

	UWorld* World = GetWorld();
	if (!World) return nullptr;

	FVector OwnerLoc = Owner->GetActorLocation();
	FVector OwnerForward = Owner->GetActorForwardVector();
	AActor* BestTarget = nullptr;
	float BestScore = -1.f;

	TArray<AActor*> AllActors;
	UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), AllActors);

	for (AActor* Actor : AllActors)
	{
		if (Actor == Owner) continue;

		// Check for IFF tag via component
		UHealthShieldComponent* Health = Actor->FindComponentByClass<UHealthShieldComponent>();
		if (!Health || !Health->IsAlive()) continue;

		FVector ToActor = Actor->GetActorLocation() - OwnerLoc;
		float Distance = ToActor.Size();
		if (Distance > SpaceConstants::TargetingRange) continue;

		float DotProduct = FVector::DotProduct(OwnerForward, ToActor.GetSafeNormal());
		if (DotProduct < 0.3f) continue;

		float Score = DotProduct * (1.f - Distance / SpaceConstants::TargetingRange);
		if (Score > BestScore)
		{
			BestScore = Score;
			BestTarget = Actor;
		}
	}

	return BestTarget;
}

void UTargetingComponent::CycleTarget()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	UWorld* World = GetWorld();
	if (!World) return;

	FVector OwnerLoc = Owner->GetActorLocation();
	TArray<AActor*> AllActors;
	UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), AllActors);

	TArray<AActor*> ValidTargets;
	for (AActor* Actor : AllActors)
	{
		if (Actor == Owner) continue;
		UHealthShieldComponent* Health = Actor->FindComponentByClass<UHealthShieldComponent>();
		if (!Health || !Health->IsAlive()) continue;
		float Distance = FVector::Dist(OwnerLoc, Actor->GetActorLocation());
		if (Distance <= SpaceConstants::TargetingRange)
		{
			ValidTargets.Add(Actor);
		}
	}

	if (ValidTargets.Num() == 0)
	{
		CurrentTarget = nullptr;
		return;
	}

	int32 CurrentIdx = ValidTargets.Find(CurrentTarget);
	int32 NextIdx = (CurrentIdx + 1) % ValidTargets.Num();
	CurrentTarget = ValidTargets[NextIdx];
	LockOnProgress = 0.f;
	bMissileLocked = false;
}

void UTargetingComponent::StartLockOn()
{
	bLockingOn = true;
}

void UTargetingComponent::StopLockOn()
{
	bLockingOn = false;
}

float UTargetingComponent::GetDistanceToTarget() const
{
	if (!CurrentTarget || !GetOwner()) return 0.f;
	return FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
}

FVector UTargetingComponent::GetTargetScreenPosition() const
{
	return FVector::ZeroVector;
}
