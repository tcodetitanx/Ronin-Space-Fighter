#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthShieldComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeathDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDamageTaken, float, Damage, AActor*, DamageCauser);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RONINFENIX_API UHealthShieldComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthShieldComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void ApplyDamage(float Damage, AActor* DamageInstigator);

	UFUNCTION(BlueprintCallable)
	float GetHealthPercent() const { return MaxHealth > 0.f ? CurrentHealth / MaxHealth : 0.f; }

	UFUNCTION(BlueprintCallable)
	float GetShieldPercent() const { return MaxShield > 0.f ? CurrentShield / MaxShield : 0.f; }

	UFUNCTION(BlueprintCallable)
	bool IsAlive() const { return CurrentHealth > 0.f; }

	UFUNCTION(BlueprintCallable)
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintCallable)
	float GetCurrentShield() const { return CurrentShield; }

	void SetStats(float InMaxHealth, float InMaxShield, float InShieldRegenRate, float InShieldRegenDelay);

	UPROPERTY(BlueprintAssignable)
	FOnDeathDelegate OnDeath;

	UPROPERTY(BlueprintAssignable)
	FOnDamageTaken OnDamageTaken;

private:
	UPROPERTY(EditAnywhere)
	float MaxHealth = 100.f;

	UPROPERTY(EditAnywhere)
	float MaxShield = 50.f;

	UPROPERTY(EditAnywhere)
	float ShieldRegenRate = 10.f;

	UPROPERTY(EditAnywhere)
	float ShieldRegenDelay = 3.f;

	float CurrentHealth = 100.f;
	float CurrentShield = 50.f;
	float TimeSinceLastDamage = 0.f;
};
