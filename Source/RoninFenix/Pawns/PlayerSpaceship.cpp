#include "Pawns/PlayerSpaceship.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputModifiers.h"
#include "Components/SpaceshipMovementComponent.h"
#include "Components/WeaponComponent.h"
#include "Components/TargetingComponent.h"
#include "Kismet/GameplayStatics.h"

APlayerSpaceship::APlayerSpaceship()
{
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.f;
	CameraBoom->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	CameraBoom->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
	CameraBoom->SocketOffset = FVector(0.f, 0.f, 120.f);
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 10.f;
	CameraBoom->bEnableCameraRotationLag = true;
	CameraBoom->CameraRotationLagSpeed = 12.f;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->SetFieldOfView(90.f);

	FPostProcessSettings& PP = FollowCamera->PostProcessSettings;
	FollowCamera->PostProcessBlendWeight = 1.0f;

	PP.bOverride_AutoExposureMethod = true;
	PP.AutoExposureMethod = AEM_Histogram;

	PP.bOverride_AutoExposureBias = true;
	PP.AutoExposureBias = 0.f;

	PP.bOverride_AutoExposureMinBrightness = true;
	PP.AutoExposureMinBrightness = 0.02f;

	PP.bOverride_AutoExposureMaxBrightness = true;
	PP.AutoExposureMaxBrightness = 10.f;

	PP.bOverride_AutoExposureSpeedUp = true;
	PP.AutoExposureSpeedUp = 3.f;

	PP.bOverride_AutoExposureSpeedDown = true;
	PP.AutoExposureSpeedDown = 1.f;
}

void APlayerSpaceship::BeginPlay()
{
	Super::BeginPlay();
	CreateInputActions();

	// Load and start thruster audio loops
	// USoundWave* HumSound = LoadObject<USoundWave>(nullptr, TEXT("/Game/Sounds/ThrusterHum1.ThrusterHum1"));
	USoundWave* LowSound = LoadObject<USoundWave>(nullptr, TEXT("/Game/Sounds/ThrusterLowLoop1.ThrusterLowLoop1"));

	UE_LOG(LogTemp, Warning, TEXT("PlayerSpaceship BeginPlay: LowSound=%s"),
		LowSound ? TEXT("LOADED") : TEXT("NULL"));

	if (LowSound)
	{
		LowSound->bLooping = true;
		ThrusterLowAudio = UGameplayStatics::SpawnSoundAttached(LowSound, RootComponent);
		if (ThrusterLowAudio)
		{
			ThrusterLowAudio->SetPitchMultiplier(0.5f);
			ThrusterLowAudio->Play();
		}
	}

}

void APlayerSpaceship::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Steer ship based on reticle offset (normalized -1 to 1)
	if (GetMovementComp() && ReticleRadius > 0.f)
	{
		float NormYaw = ReticleOffset.X / ReticleRadius;
		float NormPitch = ReticleOffset.Y / ReticleRadius;
		GetMovementComp()->SetYawInput(NormYaw);
		GetMovementComp()->SetPitchInput(-NormPitch);
	}

	// Modulate thruster pitch based on speed
	if (GetMovementComp())
	{
		float BoostMax = GetMovementComp()->Stats.BoostMaxSpeed;
		float SpeedPercent = (BoostMax > 0.f) ? FMath::Clamp(GetMovementComp()->GetCurrentSpeed() / BoostMax, 0.f, 1.f) : 0.f;

		if (ThrusterHumAudio)
		{
			ThrusterHumAudio->SetPitchMultiplier(0.7f + SpeedPercent * 0.8f);
			ThrusterHumAudio->SetVolumeMultiplier(ThrusterHumVolume);
		}
		if (ThrusterLowAudio)
		{
			ThrusterLowAudio->SetPitchMultiplier(0.5f + SpeedPercent * 0.6f);
			ThrusterLowAudio->SetVolumeMultiplier(ThrusterLowVolume);
		}
	}

	// Apply live-editable camera settings
	CameraBoom->TargetArmLength = CameraDistance;
	CameraBoom->SocketOffset = FVector(0.f, 0.f, CameraHeight);

	// Trace from reticle screen position to find where it points in world space
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		int32 ViewX, ViewY;
		PC->GetViewportSize(ViewX, ViewY);

		float ReticleScreenX = ViewX * 0.5f + ReticleOffset.X;
		float ReticleScreenY = ViewY * 0.5f + ReticleOffset.Y;

		FVector WorldLoc, WorldDir;
		if (PC->DeprojectScreenPositionToWorld(ReticleScreenX, ReticleScreenY, WorldLoc, WorldDir))
		{
			FHitResult Hit;
			FVector TraceEnd = WorldLoc + WorldDir * 100000.f;
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(this);

			FVector AimTarget;
			if (GetWorld()->LineTraceSingleByChannel(Hit, WorldLoc, TraceEnd, ECC_Visibility, Params))
			{
				AimTarget = Hit.ImpactPoint;
			}
			else
			{
				AimTarget = TraceEnd;
			}

			GetWeaponComp()->SetAimPoint(AimTarget);
		}
	}

	if (bBarrelRolling)
	{
		BarrelRollTimer += DeltaTime;
		float RollSpeed = 720.f;
		GetMovementComp()->SetRollInput(BarrelRollDirection * RollSpeed / GetMovementComp()->Stats.RollRate);

		if (BarrelRollTimer >= 0.5f)
		{
			bBarrelRolling = false;
			BarrelRollTimer = 0.f;
			GetMovementComp()->SetRollInput(0.f);
		}
	}

}

void APlayerSpaceship::CreateInputActions()
{
	IA_Throttle = NewObject<UInputAction>(this, TEXT("IA_Throttle"));
	IA_Throttle->ValueType = EInputActionValueType::Axis1D;

	IA_Steering = NewObject<UInputAction>(this, TEXT("IA_Steering"));
	IA_Steering->ValueType = EInputActionValueType::Axis2D;

	IA_Roll = NewObject<UInputAction>(this, TEXT("IA_Roll"));
	IA_Roll->ValueType = EInputActionValueType::Axis1D;

	IA_Fire = NewObject<UInputAction>(this, TEXT("IA_Fire"));
	IA_Fire->ValueType = EInputActionValueType::Boolean;

	IA_Missile = NewObject<UInputAction>(this, TEXT("IA_Missile"));
	IA_Missile->ValueType = EInputActionValueType::Boolean;

	IA_Boost = NewObject<UInputAction>(this, TEXT("IA_Boost"));
	IA_Boost->ValueType = EInputActionValueType::Boolean;

	IA_LockOn = NewObject<UInputAction>(this, TEXT("IA_LockOn"));
	IA_LockOn->ValueType = EInputActionValueType::Boolean;

	IA_BarrelRoll = NewObject<UInputAction>(this, TEXT("IA_BarrelRoll"));
	IA_BarrelRoll->ValueType = EInputActionValueType::Boolean;

	ShipMappingContext = NewObject<UInputMappingContext>(this, TEXT("IMC_Ship"));

	// W = Throttle Up
	{
		FEnhancedActionKeyMapping& Mapping = ShipMappingContext->MapKey(IA_Throttle, EKeys::W);
	}
	// S = Throttle Down
	{
		FEnhancedActionKeyMapping& Mapping = ShipMappingContext->MapKey(IA_Throttle, EKeys::S);
		UInputModifierNegate* Negate = NewObject<UInputModifierNegate>(this);
		Mapping.Modifiers.Add(Negate);
	}

	// Mouse XY = Steering (Pitch/Yaw)
	{
		ShipMappingContext->MapKey(IA_Steering, EKeys::Mouse2D);
	}

	// A = Roll Left
	{
		FEnhancedActionKeyMapping& Mapping = ShipMappingContext->MapKey(IA_Roll, EKeys::A);
		UInputModifierNegate* Negate = NewObject<UInputModifierNegate>(this);
		Mapping.Modifiers.Add(Negate);
	}
	// D = Roll Right
	{
		ShipMappingContext->MapKey(IA_Roll, EKeys::D);
	}

	// Left Mouse Button = Fire Lasers
	{
		ShipMappingContext->MapKey(IA_Fire, EKeys::LeftMouseButton);
	}

	// Middle Mouse Button = Fire Missile
	{
		ShipMappingContext->MapKey(IA_Missile, EKeys::MiddleMouseButton);
	}

	// Left Shift = Boost
	{
		ShipMappingContext->MapKey(IA_Boost, EKeys::LeftShift);
	}

	// Right Mouse Button = Lock On
	{
		ShipMappingContext->MapKey(IA_LockOn, EKeys::RightMouseButton);
	}

	// Space = Barrel Roll
	{
		ShipMappingContext->MapKey(IA_BarrelRoll, EKeys::SpaceBar);
	}
}

void APlayerSpaceship::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (!ShipMappingContext)
	{
		CreateInputActions();
	}

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(ShipMappingContext, 0);
		}
	}

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EIC) return;

	EIC->BindAction(IA_Throttle, ETriggerEvent::Triggered, this, &APlayerSpaceship::HandleThrottle);
	EIC->BindAction(IA_Throttle, ETriggerEvent::Completed, this, &APlayerSpaceship::HandleThrottleStop);
	EIC->BindAction(IA_Steering, ETriggerEvent::Triggered, this, &APlayerSpaceship::HandleSteering);
	EIC->BindAction(IA_Steering, ETriggerEvent::Completed, this, &APlayerSpaceship::HandleSteeringStop);
	EIC->BindAction(IA_Roll, ETriggerEvent::Triggered, this, &APlayerSpaceship::HandleRoll);
	EIC->BindAction(IA_Roll, ETriggerEvent::Completed, this, &APlayerSpaceship::HandleRoll);

	EIC->BindAction(IA_Fire, ETriggerEvent::Started, this, &APlayerSpaceship::HandleFireStart);
	EIC->BindAction(IA_Fire, ETriggerEvent::Completed, this, &APlayerSpaceship::HandleFireStop);

	EIC->BindAction(IA_Missile, ETriggerEvent::Started, this, &APlayerSpaceship::HandleMissile);

	EIC->BindAction(IA_Boost, ETriggerEvent::Started, this, &APlayerSpaceship::HandleBoostStart);
	EIC->BindAction(IA_Boost, ETriggerEvent::Completed, this, &APlayerSpaceship::HandleBoostStop);

	EIC->BindAction(IA_LockOn, ETriggerEvent::Started, this, &APlayerSpaceship::HandleLockOnStart);
	EIC->BindAction(IA_LockOn, ETriggerEvent::Completed, this, &APlayerSpaceship::HandleLockOnStop);

	EIC->BindAction(IA_BarrelRoll, ETriggerEvent::Started, this, &APlayerSpaceship::HandleBarrelRoll);
}

void APlayerSpaceship::HandleThrottle(const FInputActionValue& Value)
{
	GetMovementComp()->SetThrottleInput(Value.Get<float>());
}

void APlayerSpaceship::HandleSteering(const FInputActionValue& Value)
{
	FVector2D Input = Value.Get<FVector2D>();
	// Accumulate mouse delta into reticle offset (negate Y so mouse-up = reticle-up)
	ReticleOffset.X += Input.X * SteeringSensitivity;
	ReticleOffset.Y -= Input.Y * SteeringSensitivity;

	// Clamp to radius
	float Mag = ReticleOffset.Size();
	if (Mag > ReticleRadius)
	{
		ReticleOffset = ReticleOffset.GetSafeNormal() * ReticleRadius;
	}
}

void APlayerSpaceship::HandleSteeringStop(const FInputActionValue& Value)
{
	// Reticle stays where it is — ship keeps steering toward it
}

void APlayerSpaceship::HandleThrottleStop(const FInputActionValue& Value)
{
	GetMovementComp()->SetThrottleInput(0.f);
}

void APlayerSpaceship::HandleRoll(const FInputActionValue& Value)
{
	if (!bBarrelRolling)
	{
		GetMovementComp()->SetRollInput(Value.Get<float>());
	}
}

void APlayerSpaceship::HandleFireStart(const FInputActionValue& Value)
{
	GetWeaponComp()->StartFiringLasers();
}

void APlayerSpaceship::HandleFireStop(const FInputActionValue& Value)
{
	GetWeaponComp()->StopFiringLasers();
}

void APlayerSpaceship::HandleMissile(const FInputActionValue& Value)
{
	if (GetTargetingComp()->HasMissileLock())
	{
		GetWeaponComp()->FireMissile(GetTargetingComp()->GetCurrentTarget());
	}
}

void APlayerSpaceship::HandleBoostStart(const FInputActionValue& Value)
{
	GetMovementComp()->SetBoostInput(true);
}

void APlayerSpaceship::HandleBoostStop(const FInputActionValue& Value)
{
	GetMovementComp()->SetBoostInput(false);
}

void APlayerSpaceship::HandleLockOnStart(const FInputActionValue& Value)
{
	GetTargetingComp()->StartLockOn();
}

void APlayerSpaceship::HandleLockOnStop(const FInputActionValue& Value)
{
	GetTargetingComp()->StopLockOn();
}

void APlayerSpaceship::HandleBarrelRoll(const FInputActionValue& Value)
{
	if (!bBarrelRolling)
	{
		bBarrelRolling = true;
		BarrelRollTimer = 0.f;
		BarrelRollDirection = FMath::RandBool() ? 1.f : -1.f;
	}
}
