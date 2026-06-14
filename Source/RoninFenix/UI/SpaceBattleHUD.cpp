#include "UI/SpaceBattleHUD.h"
#include "Engine/Canvas.h"
#include "Pawns/PlayerSpaceship.h"
#include "Framework/SpaceBattleGameState.h"
#include "Framework/SpaceBattlePlayerController.h"
#include "Framework/SpaceBattlePlayerState.h"
#include "Components/SpaceshipMovementComponent.h"
#include "Components/HealthShieldComponent.h"
#include "Components/WeaponComponent.h"
#include "Components/TargetingComponent.h"
#include "Actors/CapitalShip.h"
#include "Framework/SpaceBattleGameMode.h"
#include "Kismet/GameplayStatics.h"

ASpaceBattleHUD::ASpaceBattleHUD()
{
}

void ASpaceBattleHUD::DrawHUD()
{
	Super::DrawHUD();

	ASpaceBattleGameState* GS = GetBattleGameState();
	if (GS && GS->IsMatchOver())
	{
		DrawMatchOverScreen();
		return;
	}

	ASpaceBattlePlayerController* PC = Cast<ASpaceBattlePlayerController>(GetOwningPlayerController());
	if (PC && PC->IsWaitingToRespawn())
	{
		DrawRespawnTimer();
		return;
	}

	DrawCrosshair();
	DrawHealthShieldBars();
	DrawSpeedIndicator();
	DrawBoostBar();
	DrawWeaponInfo();
	DrawTargetingInfo();
	DrawRadar();
	DrawScoreBoard();
	DrawKillFeed();
	DrawMatchTimer();
	DrawCapitalShipHealth();
	DrawControlsHelp();
}

void ASpaceBattleHUD::DrawCrosshair()
{
	float CenterX = Canvas->ClipX * 0.5f;
	float CenterY = Canvas->ClipY * 0.5f;
	float Size = 20.f;
	float Thickness = 2.f;

	FLinearColor CrosshairColor(0.f, 1.f, 0.f, 0.8f);

	// Cross lines
	DrawLine(CenterX - Size, CenterY, CenterX - Size * 0.4f, CenterY, CrosshairColor, Thickness);
	DrawLine(CenterX + Size * 0.4f, CenterY, CenterX + Size, CenterY, CrosshairColor, Thickness);
	DrawLine(CenterX, CenterY - Size, CenterX, CenterY - Size * 0.4f, CrosshairColor, Thickness);
	DrawLine(CenterX, CenterY + Size * 0.4f, CenterX, CenterY + Size, CrosshairColor, Thickness);

	// Circle
	int32 Segments = 32;
	float Radius = Size * 1.5f;
	for (int32 i = 0; i < Segments; ++i)
	{
		float A1 = 2.f * PI * i / Segments;
		float A2 = 2.f * PI * (i + 1) / Segments;
		DrawLine(
			CenterX + Radius * FMath::Cos(A1), CenterY + Radius * FMath::Sin(A1),
			CenterX + Radius * FMath::Cos(A2), CenterY + Radius * FMath::Sin(A2),
			CrosshairColor * 0.5f, 1.f);
	}
}

void ASpaceBattleHUD::DrawHealthShieldBars()
{
	APlayerSpaceship* Ship = GetPlayerShip();
	if (!Ship) return;

	UHealthShieldComponent* Health = Ship->GetHealthComp();
	if (!Health) return;

	float BarWidth = 250.f;
	float BarHeight = 16.f;
	float X = 20.f;
	float Y = Canvas->ClipY - 80.f;

	// Shield bar
	DrawBar(X, Y, BarWidth, BarHeight, Health->GetShieldPercent(),
		FLinearColor(0.2f, 0.5f, 1.f, 0.9f), FLinearColor(0.1f, 0.1f, 0.2f, 0.6f));

	FString ShieldText = FString::Printf(TEXT("SHIELD: %.0f"), Health->GetCurrentShield());
	DrawText(ShieldText, FLinearColor::White, X + 5.f, Y + 1.f, nullptr, 0.9f);

	// Health bar
	Y += BarHeight + 4.f;
	FLinearColor HealthColor = Health->GetHealthPercent() > 0.3f ?
		FLinearColor(0.2f, 1.f, 0.3f, 0.9f) : FLinearColor(1.f, 0.2f, 0.2f, 0.9f);
	DrawBar(X, Y, BarWidth, BarHeight, Health->GetHealthPercent(),
		HealthColor, FLinearColor(0.1f, 0.1f, 0.1f, 0.6f));

	FString HealthText = FString::Printf(TEXT("HULL: %.0f"), Health->GetCurrentHealth());
	DrawText(HealthText, FLinearColor::White, X + 5.f, Y + 1.f, nullptr, 0.9f);
}

void ASpaceBattleHUD::DrawSpeedIndicator()
{
	APlayerSpaceship* Ship = GetPlayerShip();
	if (!Ship) return;

	USpaceshipMovementComponent* Mov = Ship->GetMovementComp();
	if (!Mov) return;

	float X = Canvas->ClipX - 270.f;
	float Y = Canvas->ClipY - 80.f;

	float SpeedPercent = Mov->GetCurrentSpeed() / Mov->Stats.BoostMaxSpeed;
	DrawBar(X, Y, 250.f, 16.f, SpeedPercent,
		FLinearColor(1.f, 0.8f, 0.2f, 0.9f), FLinearColor(0.1f, 0.1f, 0.1f, 0.6f));

	FString SpeedText = FString::Printf(TEXT("SPEED: %.0f"), Mov->GetCurrentSpeed());
	DrawText(SpeedText, FLinearColor::White, X + 5.f, Y + 1.f, nullptr, 0.9f);

	// Throttle
	Y += 20.f;
	DrawBar(X, Y, 250.f, 8.f, Mov->GetThrottlePercent(),
		FLinearColor(0.5f, 0.5f, 0.5f, 0.7f), FLinearColor(0.05f, 0.05f, 0.05f, 0.4f));
}

void ASpaceBattleHUD::DrawBoostBar()
{
	APlayerSpaceship* Ship = GetPlayerShip();
	if (!Ship) return;

	USpaceshipMovementComponent* Mov = Ship->GetMovementComp();
	if (!Mov) return;

	float X = Canvas->ClipX - 270.f;
	float Y = Canvas->ClipY - 40.f;

	FLinearColor BoostColor = Mov->IsBoosting() ?
		FLinearColor(0.f, 0.8f, 1.f, 0.9f) : FLinearColor(0.3f, 0.6f, 0.8f, 0.7f);

	DrawBar(X, Y, 250.f, 12.f, Mov->GetBoostPercent(), BoostColor, FLinearColor(0.05f, 0.05f, 0.1f, 0.5f));

	FString BoostText = Mov->IsBoosting() ? TEXT("BOOST ACTIVE") : TEXT("BOOST");
	DrawText(BoostText, BoostColor, X + 5.f, Y, nullptr, 0.8f);
}

void ASpaceBattleHUD::DrawWeaponInfo()
{
	APlayerSpaceship* Ship = GetPlayerShip();
	if (!Ship) return;

	UWeaponComponent* Wep = Ship->GetWeaponComp();
	if (!Wep) return;

	float X = 20.f;
	float Y = Canvas->ClipY - 40.f;

	// Overheat bar
	FLinearColor HeatColor = Wep->IsOverheated() ?
		FLinearColor(1.f, 0.1f, 0.1f, 0.9f) : FLinearColor(1.f, 0.5f, 0.f, 0.7f);
	DrawBar(X, Y, 150.f, 12.f, Wep->GetOverheatPercent(), HeatColor, FLinearColor(0.1f, 0.05f, 0.f, 0.4f));

	FString HeatText = Wep->IsOverheated() ? TEXT("OVERHEATED!") : TEXT("HEAT");
	DrawText(HeatText, HeatColor, X + 5.f, Y, nullptr, 0.8f);

	// Missiles
	FString MissileText = FString::Printf(TEXT("MISSILES: %d"), Wep->GetMissilesRemaining());
	DrawText(MissileText, FLinearColor(0.8f, 0.8f, 1.f), X + 170.f, Y, nullptr, 0.9f);
}

void ASpaceBattleHUD::DrawTargetingInfo()
{
	APlayerSpaceship* Ship = GetPlayerShip();
	if (!Ship) return;

	UTargetingComponent* Tgt = Ship->GetTargetingComp();
	if (!Tgt || !Tgt->GetCurrentTarget()) return;

	AActor* Target = Tgt->GetCurrentTarget();
	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return;

	FVector TargetLoc = Target->GetActorLocation();
	FVector2D ScreenPos;
	if (PC->ProjectWorldLocationToScreen(TargetLoc, ScreenPos))
	{
		float BoxSize = 40.f;
		FLinearColor TargetColor = Tgt->HasMissileLock() ?
			FLinearColor(1.f, 0.f, 0.f, 1.f) : FLinearColor(0.f, 1.f, 0.f, 0.7f);

		// Target bracket
		float CornerLen = BoxSize * 0.3f;

		// Top-left
		DrawLine(ScreenPos.X - BoxSize, ScreenPos.Y - BoxSize, ScreenPos.X - BoxSize + CornerLen, ScreenPos.Y - BoxSize, TargetColor, 2.f);
		DrawLine(ScreenPos.X - BoxSize, ScreenPos.Y - BoxSize, ScreenPos.X - BoxSize, ScreenPos.Y - BoxSize + CornerLen, TargetColor, 2.f);

		// Top-right
		DrawLine(ScreenPos.X + BoxSize - CornerLen, ScreenPos.Y - BoxSize, ScreenPos.X + BoxSize, ScreenPos.Y - BoxSize, TargetColor, 2.f);
		DrawLine(ScreenPos.X + BoxSize, ScreenPos.Y - BoxSize, ScreenPos.X + BoxSize, ScreenPos.Y - BoxSize + CornerLen, TargetColor, 2.f);

		// Bottom-left
		DrawLine(ScreenPos.X - BoxSize, ScreenPos.Y + BoxSize - CornerLen, ScreenPos.X - BoxSize, ScreenPos.Y + BoxSize, TargetColor, 2.f);
		DrawLine(ScreenPos.X - BoxSize, ScreenPos.Y + BoxSize, ScreenPos.X - BoxSize + CornerLen, ScreenPos.Y + BoxSize, TargetColor, 2.f);

		// Bottom-right
		DrawLine(ScreenPos.X + BoxSize, ScreenPos.Y + BoxSize - CornerLen, ScreenPos.X + BoxSize, ScreenPos.Y + BoxSize, TargetColor, 2.f);
		DrawLine(ScreenPos.X + BoxSize - CornerLen, ScreenPos.Y + BoxSize, ScreenPos.X + BoxSize, ScreenPos.Y + BoxSize, TargetColor, 2.f);

		// Distance
		FString DistText = FString::Printf(TEXT("%.0fm"), Tgt->GetDistanceToTarget() * 0.01f);
		DrawText(DistText, TargetColor, ScreenPos.X - 20.f, ScreenPos.Y + BoxSize + 5.f, nullptr, 0.8f);

		// Lock-on progress
		if (Tgt->GetLockOnProgress() > 0.f)
		{
			float LockRadius = BoxSize + 10.f;
			int32 TotalSegs = 32;
			int32 FilledSegs = FMath::RoundToInt(TotalSegs * Tgt->GetLockOnProgress());

			for (int32 i = 0; i < FilledSegs; ++i)
			{
				float A1 = 2.f * PI * i / TotalSegs - PI * 0.5f;
				float A2 = 2.f * PI * (i + 1) / TotalSegs - PI * 0.5f;
				DrawLine(
					ScreenPos.X + LockRadius * FMath::Cos(A1), ScreenPos.Y + LockRadius * FMath::Sin(A1),
					ScreenPos.X + LockRadius * FMath::Cos(A2), ScreenPos.Y + LockRadius * FMath::Sin(A2),
					FLinearColor(1.f, 0.f, 0.f, 0.9f), 2.f);
			}

			if (Tgt->HasMissileLock())
			{
				DrawText(TEXT("LOCK"), FLinearColor(1.f, 0.f, 0.f, 1.f), ScreenPos.X - 15.f, ScreenPos.Y - BoxSize - 20.f, nullptr, 1.f);
			}
		}
	}
}

void ASpaceBattleHUD::DrawRadar()
{
	APlayerSpaceship* Ship = GetPlayerShip();
	if (!Ship) return;

	float RadarSize = 100.f;
	float RadarX = Canvas->ClipX - RadarSize - 30.f;
	float RadarY = 30.f;
	float RadarCenterX = RadarX + RadarSize;
	float RadarCenterY = RadarY + RadarSize;
	float RadarRange = 15000.f;

	// Background
	for (int32 i = 0; i < 32; ++i)
	{
		float A1 = 2.f * PI * i / 32;
		float A2 = 2.f * PI * (i + 1) / 32;
		DrawLine(
			RadarCenterX + RadarSize * FMath::Cos(A1), RadarCenterY + RadarSize * FMath::Sin(A1),
			RadarCenterX + RadarSize * FMath::Cos(A2), RadarCenterY + RadarSize * FMath::Sin(A2),
			FLinearColor(0.f, 0.5f, 0.f, 0.5f), 1.f);
	}

	// Center dot (player)
	DrawRect(FLinearColor(0.f, 1.f, 0.f, 1.f), RadarCenterX - 2, RadarCenterY - 2, 4, 4);

	// Other ships
	FVector PlayerLoc = Ship->GetActorLocation();
	FVector PlayerFwd = Ship->GetActorForwardVector();
	FVector PlayerRight = Ship->GetActorRightVector();

	TArray<AActor*> AllPawns;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), AllPawns);

	for (AActor* Actor : AllPawns)
	{
		if (Actor == Ship) continue;
		ASpaceshipBase* OtherShip = Cast<ASpaceshipBase>(Actor);
		if (!OtherShip) continue;
		UHealthShieldComponent* Health = OtherShip->GetHealthComp();
		if (!Health || !Health->IsAlive()) continue;

		FVector ToShip = OtherShip->GetActorLocation() - PlayerLoc;
		float Distance = ToShip.Size();
		if (Distance > RadarRange) continue;

		float RelX = FVector::DotProduct(ToShip, PlayerFwd);
		float RelY = FVector::DotProduct(ToShip, PlayerRight);

		float NormX = (RelX / RadarRange) * RadarSize * 0.9f;
		float NormY = (RelY / RadarRange) * RadarSize * 0.9f;

		FLinearColor BlipColor = (OtherShip->GetTeam() == Ship->GetTeam()) ?
			FLinearColor(0.f, 0.f, 1.f, 0.9f) : FLinearColor(1.f, 0.f, 0.f, 0.9f);

		float DotX = RadarCenterX + NormY;
		float DotY = RadarCenterY - NormX;

		float DistFromCenter = FMath::Sqrt(FMath::Square(DotX - RadarCenterX) + FMath::Square(DotY - RadarCenterY));
		if (DistFromCenter <= RadarSize)
		{
			DrawRect(BlipColor, DotX - 2, DotY - 2, 4, 4);
		}
	}

	DrawText(TEXT("RADAR"), FLinearColor(0.f, 0.6f, 0.f, 0.7f), RadarX + RadarSize - 20.f, RadarY + RadarSize * 2.f + 5.f, nullptr, 0.8f);
}

void ASpaceBattleHUD::DrawScoreBoard()
{
	ASpaceBattleGameState* GS = GetBattleGameState();
	if (!GS) return;

	float CenterX = Canvas->ClipX * 0.5f;
	float Y = 15.f;

	FString AlphaScore = FString::Printf(TEXT("ALPHA: %d"), GS->GetTeamScore(ESpaceTeam::Alpha));
	FString OmegaScore = FString::Printf(TEXT("OMEGA: %d"), GS->GetTeamScore(ESpaceTeam::Omega));

	DrawText(AlphaScore, FLinearColor(0.3f, 0.5f, 1.f), CenterX - 120.f, Y, nullptr, 1.2f);
	DrawText(OmegaScore, FLinearColor(1.f, 0.3f, 0.3f), CenterX + 30.f, Y, nullptr, 1.2f);
}

void ASpaceBattleHUD::DrawKillFeed()
{
	ASpaceBattleGameState* GS = GetBattleGameState();
	if (!GS) return;

	const TArray<FString>& Feed = GS->GetKillFeed();
	float X = Canvas->ClipX - 350.f;
	float Y = 250.f;

	for (int32 i = 0; i < Feed.Num() && i < 5; ++i)
	{
		float Alpha = 1.f - (float)i * 0.15f;
		DrawText(Feed[i], FLinearColor(1.f, 1.f, 1.f, Alpha), X, Y + i * 18.f, nullptr, 0.8f);
	}
}

void ASpaceBattleHUD::DrawMatchTimer()
{
	ASpaceBattleGameState* GS = GetBattleGameState();
	if (!GS) return;

	float Remaining = GS->GetMatchTimeRemaining();
	int32 Minutes = FMath::FloorToInt(Remaining / 60.f);
	int32 Seconds = FMath::FloorToInt(FMath::Fmod(Remaining, 60.f));

	FString TimeText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
	float CenterX = Canvas->ClipX * 0.5f;

	FLinearColor TimeColor = (Remaining < 60.f) ? FLinearColor(1.f, 0.3f, 0.3f) : FLinearColor(1.f, 1.f, 1.f);
	DrawText(TimeText, TimeColor, CenterX - 25.f, 40.f, nullptr, 1.3f);
}

void ASpaceBattleHUD::DrawCapitalShipHealth()
{
	ASpaceBattleGameMode* GM = Cast<ASpaceBattleGameMode>(GetWorld()->GetAuthGameMode());
	if (!GM) return;

	float X = 20.f;
	float Y = 20.f;

	// Alpha capital ship
	ACapitalShip* AlphaCap = GM->GetCapitalShip(ESpaceTeam::Alpha);
	if (AlphaCap)
	{
		DrawText(TEXT("ALPHA FLAGSHIP"), FLinearColor(0.3f, 0.5f, 1.f), X, Y, nullptr, 0.8f);
		DrawBar(X, Y + 16.f, 200.f, 10.f, AlphaCap->GetHealthPercent(),
			FLinearColor(0.3f, 0.5f, 1.f, 0.8f), FLinearColor(0.1f, 0.1f, 0.2f, 0.5f));
	}

	// Omega capital ship
	ACapitalShip* OmegaCap = GM->GetCapitalShip(ESpaceTeam::Omega);
	if (OmegaCap)
	{
		DrawText(TEXT("OMEGA FLAGSHIP"), FLinearColor(1.f, 0.3f, 0.3f), X, Y + 35.f, nullptr, 0.8f);
		DrawBar(X, Y + 51.f, 200.f, 10.f, OmegaCap->GetHealthPercent(),
			FLinearColor(1.f, 0.3f, 0.3f, 0.8f), FLinearColor(0.2f, 0.1f, 0.1f, 0.5f));
	}
}

void ASpaceBattleHUD::DrawMatchOverScreen()
{
	ASpaceBattleGameState* GS = GetBattleGameState();
	if (!GS) return;

	float CenterX = Canvas->ClipX * 0.5f;
	float CenterY = Canvas->ClipY * 0.5f;

	// Darken background
	DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.6f), 0, 0, Canvas->ClipX, Canvas->ClipY);

	ESpaceTeam Winner = GS->GetWinningTeam();
	FString WinText = (Winner == ESpaceTeam::Alpha) ? TEXT("ALPHA TEAM WINS!") : TEXT("OMEGA TEAM WINS!");
	FLinearColor WinColor = (Winner == ESpaceTeam::Alpha) ?
		FLinearColor(0.3f, 0.5f, 1.f) : FLinearColor(1.f, 0.3f, 0.3f);

	DrawText(WinText, WinColor, CenterX - 100.f, CenterY - 30.f, nullptr, 2.f);

	FString ScoreText = FString::Printf(TEXT("ALPHA: %d  |  OMEGA: %d"),
		GS->GetTeamScore(ESpaceTeam::Alpha), GS->GetTeamScore(ESpaceTeam::Omega));
	DrawText(ScoreText, FLinearColor::White, CenterX - 80.f, CenterY + 20.f, nullptr, 1.2f);
}

void ASpaceBattleHUD::DrawRespawnTimer()
{
	ASpaceBattlePlayerController* PC = Cast<ASpaceBattlePlayerController>(GetOwningPlayerController());
	if (!PC) return;

	float CenterX = Canvas->ClipX * 0.5f;
	float CenterY = Canvas->ClipY * 0.5f;

	DrawText(TEXT("DESTROYED"), FLinearColor(1.f, 0.2f, 0.2f), CenterX - 60.f, CenterY - 30.f, nullptr, 1.5f);

	FString RespawnText = FString::Printf(TEXT("Respawning in %.1f..."), PC->GetRespawnTimeRemaining());
	DrawText(RespawnText, FLinearColor::White, CenterX - 70.f, CenterY + 10.f, nullptr, 1.f);
}

void ASpaceBattleHUD::DrawControlsHelp()
{
	float X = 20.f;
	float Y = Canvas->ClipY * 0.5f - 100.f;
	float LineH = 16.f;
	FLinearColor HelpColor(0.5f, 0.5f, 0.5f, 0.4f);

	DrawText(TEXT("W/S - Throttle"), HelpColor, X, Y, nullptr, 0.7f); Y += LineH;
	DrawText(TEXT("Mouse - Steer"), HelpColor, X, Y, nullptr, 0.7f); Y += LineH;
	DrawText(TEXT("Q/E - Roll"), HelpColor, X, Y, nullptr, 0.7f); Y += LineH;
	DrawText(TEXT("LMB - Fire Lasers"), HelpColor, X, Y, nullptr, 0.7f); Y += LineH;
	DrawText(TEXT("RMB - Lock On"), HelpColor, X, Y, nullptr, 0.7f); Y += LineH;
	DrawText(TEXT("MMB - Fire Missile"), HelpColor, X, Y, nullptr, 0.7f); Y += LineH;
	DrawText(TEXT("Shift - Boost"), HelpColor, X, Y, nullptr, 0.7f); Y += LineH;
	DrawText(TEXT("Space - Barrel Roll"), HelpColor, X, Y, nullptr, 0.7f); Y += LineH;
	DrawText(TEXT("Tab - Cycle Target"), HelpColor, X, Y, nullptr, 0.7f);
}

void ASpaceBattleHUD::DrawBar(float X, float Y, float Width, float Height, float Percent, FLinearColor FillColor, FLinearColor BackColor)
{
	Percent = FMath::Clamp(Percent, 0.f, 1.f);
	DrawRect(BackColor, X, Y, Width, Height);
	DrawRect(FillColor, X, Y, Width * Percent, Height);

	// Border
	DrawLine(X, Y, X + Width, Y, FLinearColor(0.5f, 0.5f, 0.5f, 0.3f), 1.f);
	DrawLine(X, Y + Height, X + Width, Y + Height, FLinearColor(0.5f, 0.5f, 0.5f, 0.3f), 1.f);
	DrawLine(X, Y, X, Y + Height, FLinearColor(0.5f, 0.5f, 0.5f, 0.3f), 1.f);
	DrawLine(X + Width, Y, X + Width, Y + Height, FLinearColor(0.5f, 0.5f, 0.5f, 0.3f), 1.f);
}

APlayerSpaceship* ASpaceBattleHUD::GetPlayerShip() const
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return nullptr;
	return Cast<APlayerSpaceship>(PC->GetPawn());
}

ASpaceBattleGameState* ASpaceBattleHUD::GetBattleGameState() const
{
	return GetWorld() ? GetWorld()->GetGameState<ASpaceBattleGameState>() : nullptr;
}
