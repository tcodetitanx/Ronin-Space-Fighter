#include "Components/HealthShieldComponent.h"

UHealthShieldComponent::UHealthShieldComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UHealthShieldComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!IsAlive()) return;

	TimeSinceLastDamage += DeltaTime;

	if (TimeSinceLastDamage >= ShieldRegenDelay && CurrentShield < MaxShield)
	{
		CurrentShield = FMath::Min(CurrentShield + ShieldRegenRate * DeltaTime, MaxShield);
	}
}

void UHealthShieldComponent::ApplyDamage(float Damage, AActor* DamageInstigator)
{
	if (!IsAlive()) return;

	TimeSinceLastDamage = 0.f;
	float RemainingDamage = Damage;

	if (CurrentShield > 0.f)
	{
		float ShieldDamage = FMath::Min(RemainingDamage, CurrentShield);
		CurrentShield -= ShieldDamage;
		RemainingDamage -= ShieldDamage;
	}

	if (RemainingDamage > 0.f)
	{
		CurrentHealth = FMath::Max(0.f, CurrentHealth - RemainingDamage);
	}

	OnDamageTaken.Broadcast(Damage, DamageInstigator);

	if (CurrentHealth <= 0.f)
	{
		OnDeath.Broadcast();
	}
}

void UHealthShieldComponent::SetStats(float InMaxHealth, float InMaxShield, float InShieldRegenRate, float InShieldRegenDelay)
{
	MaxHealth = InMaxHealth;
	MaxShield = InMaxShield;
	ShieldRegenRate = InShieldRegenRate;
	ShieldRegenDelay = InShieldRegenDelay;
	CurrentHealth = MaxHealth;
	CurrentShield = MaxShield;
}
