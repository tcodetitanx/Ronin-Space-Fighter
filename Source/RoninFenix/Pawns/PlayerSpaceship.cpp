#include "Pawns/PlayerSpaceship.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputModifiers.h"
#include "Components/SpaceshipMovementComponent.h"
#include "Components/WeaponComponent.h"
#include "Components/TargetingComponent.h"

APlayerSpaceship::APlayerSpaceship()
{
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 800.f;
	CameraBoom->SetRelativeLocation(FVector(-100.f, 0.f, 100.f));
	CameraBoom->SetRelativeRotation(FRotator(-10.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 5.f;
	CameraBoom->bEnableCameraRotationLag = true;
	CameraBoom->CameraRotationLagSpeed = 8.f;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->SetFieldOfView(90.f);
}

void APlayerSpaceship::BeginPlay()
{
	Super::BeginPlay();
	CreateInputActions();
}

void APlayerSpaceship::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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

	IA_CycleTarget = NewObject<UInputAction>(this, TEXT("IA_CycleTarget"));
	IA_CycleTarget->ValueType = EInputActionValueType::Boolean;

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

	// Tab = Cycle Target
	{
		ShipMappingContext->MapKey(IA_CycleTarget, EKeys::Tab);
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

	EIC->BindAction(IA_CycleTarget, ETriggerEvent::Started, this, &APlayerSpaceship::HandleCycleTarget);

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
	GetMovementComp()->SetYawInput(Input.X * 0.5f);
	GetMovementComp()->SetPitchInput(Input.Y * 0.5f);
}

void APlayerSpaceship::HandleSteeringStop(const FInputActionValue& Value)
{
	GetMovementComp()->SetYawInput(0.f);
	GetMovementComp()->SetPitchInput(0.f);
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

void APlayerSpaceship::HandleCycleTarget(const FInputActionValue& Value)
{
	GetTargetingComp()->CycleTarget();
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
