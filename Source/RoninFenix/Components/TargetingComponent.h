#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SpaceTypes.h"
#include "TargetingComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RONINFENIX_API UTargetingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTargetingComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	AActor* GetCurrentTarget() const { return CurrentTarget; }

	UFUNCTION(BlueprintCallable)
	bool HasMissileLock() const { return bMissileLocked; }

	UFUNCTION(BlueprintCallable)
	float GetLockOnProgress() const { return LockOnProgress; }

	UFUNCTION(BlueprintCallable)
	float GetDistanceToTarget() const;

	UFUNCTION(BlueprintCallable)
	FVector GetTargetScreenPosition() const;

	void SetTeam(ESpaceTeam InTeam) { OwnerTeam = InTeam; }
	void CycleTarget();
	void StartLockOn();
	void StopLockOn();

private:
	AActor* FindBestTarget() const;

	UPROPERTY()
	TObjectPtr<AActor> CurrentTarget;

	ESpaceTeam OwnerTeam = ESpaceTeam::Neutral;
	bool bLockingOn = false;
	bool bMissileLocked = false;
	float LockOnProgress = 0.f;
	float LockOnTime = 2.f;
};
