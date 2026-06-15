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
#include "Pawns/SpaceshipBase.h"
#include "Actors/CapitalShip.h"
#include "Framework/SpaceBattleGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"

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
	DrawShipOutlines();
	DrawEnemyDirectionArrows();
	DrawRadar();
	DrawScoreBoard();
	DrawKillFeed();
	DrawMatchTimer();
	DrawCapitalShipHealth();
	DrawControlsHelp();
}

void ASpaceBattleHUD::DrawCrosshair()
{
	APlayerSpaceship* Ship = GetPlayerShip();

	float CenterX = Canvas->ClipX * 0.5f;
	float CenterY = Canvas->ClipY * 0.5f;

	FVector2D Offset = FVector2D::ZeroVector;
	if (Ship)
	{
		Offset = Ship->GetReticleOffset();
	}

	float RX = CenterX + Offset.X;
	float RY = CenterY + Offset.Y;

	FLinearColor Bright(1.f, 1.f, 1.f, 0.9f);
	FLinearColor Dim(1.f, 1.f, 1.f, 0.5f);

	// --- Center dot ---
	DrawRect(Bright, RX - 2.f, RY - 2.f, 4.f, 4.f);

	// --- 4 chevrons pointing inward ---
	float ChevDist = 15.f;
	float ChevLen = 8.f;
	float ChevW = 6.f;
	// Top chevron (V pointing down)
	DrawLine(RX - ChevW, RY - ChevDist - ChevLen, RX, RY - ChevDist, Bright, 1.5f);
	DrawLine(RX + ChevW, RY - ChevDist - ChevLen, RX, RY - ChevDist, Bright, 1.5f);
	// Bottom chevron (V pointing up)
	DrawLine(RX - ChevW, RY + ChevDist + ChevLen, RX, RY + ChevDist, Bright, 1.5f);
	DrawLine(RX + ChevW, RY + ChevDist + ChevLen, RX, RY + ChevDist, Bright, 1.5f);
	// Left chevron (V pointing right)
	DrawLine(RX - ChevDist - ChevLen, RY - ChevW, RX - ChevDist, RY, Bright, 1.5f);
	DrawLine(RX - ChevDist - ChevLen, RY + ChevW, RX - ChevDist, RY, Bright, 1.5f);
	// Right chevron (V pointing left)
	DrawLine(RX + ChevDist + ChevLen, RY - ChevW, RX + ChevDist, RY, Bright, 1.5f);
	DrawLine(RX + ChevDist + ChevLen, RY + ChevW, RX + ChevDist, RY, Bright, 1.5f);

	// --- Outer diamond corners ---
	float DD = 30.f;
	float CL = 7.f;
	// Top
	DrawLine(RX - CL, RY - DD + CL, RX, RY - DD, Dim, 1.f);
	DrawLine(RX + CL, RY - DD + CL, RX, RY - DD, Dim, 1.f);
	// Bottom
	DrawLine(RX - CL, RY + DD - CL, RX, RY + DD, Dim, 1.f);
	DrawLine(RX + CL, RY + DD - CL, RX, RY + DD, Dim, 1.f);
	// Left
	DrawLine(RX - DD + CL, RY - CL, RX - DD, RY, Dim, 1.f);
	DrawLine(RX - DD + CL, RY + CL, RX - DD, RY, Dim, 1.f);
	// Right
	DrawLine(RX + DD - CL, RY - CL, RX + DD, RY, Dim, 1.f);
	DrawLine(RX + DD - CL, RY + CL, RX + DD, RY, Dim, 1.f);
}

void ASpaceBattleHUD::DrawHealthShieldBars()
{
	APlayerSpaceship* Ship = GetPlayerShip();
	if (!Ship) return;

	UHealthShieldComponent* Health = Ship->GetHealthComp();
	if (!Health) return;

	float CX = 25.f;
	float CY = Canvas->ClipY + 5.f;
	float ShieldRadius = 145.f;
	float HealthRadius = 120.f;
	int32 Segments = 40;
	float ArcThick = 6.f;
	float StartDeg = 270.f;
	float ArcSpan = 90.f;

	float ShieldPct = Health->GetShieldPercent();
	float HealthPct = Health->GetHealthPercent();
	int32 ShieldFilled = FMath::RoundToInt(Segments * ShieldPct);
	int32 HealthFilled = FMath::RoundToInt(Segments * HealthPct);

	FLinearColor ShieldColor(0.2f, 0.5f, 1.f, 0.9f);
	FLinearColor ShieldBg(0.1f, 0.1f, 0.2f, 0.4f);
	FLinearColor HealthColor = HealthPct > 0.3f ?
		FLinearColor(0.2f, 1.f, 0.3f, 0.9f) : FLinearColor(1.f, 0.2f, 0.2f, 0.9f);
	FLinearColor HealthBg(0.1f, 0.1f, 0.1f, 0.4f);

	for (int32 i = 0; i < Segments; ++i)
	{
		float A1 = FMath::DegreesToRadians(StartDeg + ArcSpan * i / Segments);
		float A2 = FMath::DegreesToRadians(StartDeg + ArcSpan * (i + 1) / Segments);

		// Shield background then fill
		DrawLine(CX + ShieldRadius * FMath::Cos(A1), CY + ShieldRadius * FMath::Sin(A1),
			CX + ShieldRadius * FMath::Cos(A2), CY + ShieldRadius * FMath::Sin(A2),
			(i < ShieldFilled) ? ShieldColor : ShieldBg, ArcThick);

		// Health background then fill
		DrawLine(CX + HealthRadius * FMath::Cos(A1), CY + HealthRadius * FMath::Sin(A1),
			CX + HealthRadius * FMath::Cos(A2), CY + HealthRadius * FMath::Sin(A2),
			(i < HealthFilled) ? HealthColor : HealthBg, ArcThick);
	}

	// Labels at end of arcs
	float LabelAngle = FMath::DegreesToRadians(StartDeg + ArcSpan + 5.f);
	float SLX = CX + ShieldRadius * FMath::Cos(LabelAngle);
	float SLY = CY + ShieldRadius * FMath::Sin(LabelAngle);
	float HLX = CX + HealthRadius * FMath::Cos(LabelAngle);
	float HLY = CY + HealthRadius * FMath::Sin(LabelAngle);

	FString ShieldText = FString::Printf(TEXT("SHD %.0f"), Health->GetCurrentShield());
	FString HealthText = FString::Printf(TEXT("HULL %.0f"), Health->GetCurrentHealth());
	DrawText(ShieldText, ShieldColor, SLX, SLY - 8.f, nullptr, 0.75f);
	DrawText(HealthText, HealthColor, HLX, HLY - 8.f, nullptr, 0.75f);
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
					FLinearColor(1.f, 0.f, 0.f, 0.9f), 4.f);
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
	float RadarX = Canvas->ClipX - RadarSize - 230.f;
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

void ASpaceBattleHUD::DrawShipOutlines()
{
	APlayerSpaceship* Ship = GetPlayerShip();
	if (!Ship) return;

	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return;

	TArray<AActor*> AllPawns;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpaceshipBase::StaticClass(), AllPawns);

	for (AActor* Actor : AllPawns)
	{
		if (Actor == Ship) continue;
		ASpaceshipBase* OtherShip = Cast<ASpaceshipBase>(Actor);
		if (!OtherShip) continue;
		UHealthShieldComponent* Health = OtherShip->GetHealthComp();
		if (!Health || !Health->IsAlive()) continue;

		float Dist = FVector::Dist(Ship->GetActorLocation(), OtherShip->GetActorLocation());
		if (Dist > 12000.f) continue;

		FVector2D ScreenPos;
		if (!PC->ProjectWorldLocationToScreen(OtherShip->GetActorLocation(), ScreenPos)) continue;

		if (ScreenPos.X < -100.f || ScreenPos.X > Canvas->ClipX + 100.f ||
			ScreenPos.Y < -100.f || ScreenPos.Y > Canvas->ClipY + 100.f) continue;

		bool bAlly = (OtherShip->GetTeam() == Ship->GetTeam());
		FLinearColor OutlineColor = bAlly ?
			FLinearColor(0.f, 0.8f, 0.f, 0.6f) : FLinearColor(1.f, 0.2f, 0.2f, 0.6f);

		float BoxSize = FMath::Clamp(3000.f / Dist * 35.f, 18.f, 60.f);
		float Corner = BoxSize * 0.4f;
		float Thick = 1.5f;

		// Top-left
		DrawLine(ScreenPos.X - BoxSize, ScreenPos.Y - BoxSize, ScreenPos.X - BoxSize + Corner, ScreenPos.Y - BoxSize, OutlineColor, Thick);
		DrawLine(ScreenPos.X - BoxSize, ScreenPos.Y - BoxSize, ScreenPos.X - BoxSize, ScreenPos.Y - BoxSize + Corner, OutlineColor, Thick);
		// Top-right
		DrawLine(ScreenPos.X + BoxSize - Corner, ScreenPos.Y - BoxSize, ScreenPos.X + BoxSize, ScreenPos.Y - BoxSize, OutlineColor, Thick);
		DrawLine(ScreenPos.X + BoxSize, ScreenPos.Y - BoxSize, ScreenPos.X + BoxSize, ScreenPos.Y - BoxSize + Corner, OutlineColor, Thick);
		// Bottom-left
		DrawLine(ScreenPos.X - BoxSize, ScreenPos.Y + BoxSize - Corner, ScreenPos.X - BoxSize, ScreenPos.Y + BoxSize, OutlineColor, Thick);
		DrawLine(ScreenPos.X - BoxSize, ScreenPos.Y + BoxSize, ScreenPos.X - BoxSize + Corner, ScreenPos.Y + BoxSize, OutlineColor, Thick);
		// Bottom-right
		DrawLine(ScreenPos.X + BoxSize, ScreenPos.Y + BoxSize - Corner, ScreenPos.X + BoxSize, ScreenPos.Y + BoxSize, OutlineColor, Thick);
		DrawLine(ScreenPos.X + BoxSize - Corner, ScreenPos.Y + BoxSize, ScreenPos.X + BoxSize, ScreenPos.Y + BoxSize, OutlineColor, Thick);
	}
}

void ASpaceBattleHUD::DrawEnemyDirectionArrows()
{
	APlayerSpaceship* Ship = GetPlayerShip();
	if (!Ship) return;

	APlayerController* PC = GetOwningPlayerController();
	if (!PC || !PC->PlayerCameraManager) return;

	float ScreenW = Canvas->ClipX;
	float ScreenH = Canvas->ClipY;
	float CenterX = ScreenW * 0.5f;
	float CenterY = ScreenH * 0.5f;
	float Margin = 50.f;
	float MaxDist = 25000.f;

	FRotator CamRot = PC->PlayerCameraManager->GetCameraRotation();
	FVector CamLoc = PC->PlayerCameraManager->GetCameraLocation();
	FVector CamRight = FRotationMatrix(CamRot).GetUnitAxis(EAxis::Y);
	FVector CamUp = FRotationMatrix(CamRot).GetUnitAxis(EAxis::Z);

	TArray<AActor*> AllPawns;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpaceshipBase::StaticClass(), AllPawns);

	for (AActor* Actor : AllPawns)
	{
		if (Actor == Ship) continue;
		ASpaceshipBase* OtherShip = Cast<ASpaceshipBase>(Actor);
		if (!OtherShip) continue;
		if (OtherShip->GetTeam() == Ship->GetTeam()) continue;
		UHealthShieldComponent* Health = OtherShip->GetHealthComp();
		if (!Health || !Health->IsAlive()) continue;

		float Dist = FVector::Dist(Ship->GetActorLocation(), OtherShip->GetActorLocation());
		if (Dist > MaxDist) continue;

		FVector2D ScreenPos;
		bool bOnScreen = PC->ProjectWorldLocationToScreen(OtherShip->GetActorLocation(), ScreenPos);

		if (bOnScreen && ScreenPos.X > Margin && ScreenPos.X < ScreenW - Margin &&
			ScreenPos.Y > Margin && ScreenPos.Y < ScreenH - Margin)
		{
			continue;
		}

		FVector ToEnemy = (OtherShip->GetActorLocation() - CamLoc).GetSafeNormal();
		float DotRight = FVector::DotProduct(ToEnemy, CamRight);
		float DotUp = FVector::DotProduct(ToEnemy, CamUp);

		FVector2D Dir(DotRight, -DotUp);
		if (Dir.SizeSquared() < 0.001f) continue;
		Dir.Normalize();

		float EdgeX = CenterX + Dir.X * (CenterX - Margin);
		float EdgeY = CenterY + Dir.Y * (CenterY - Margin);
		EdgeX = FMath::Clamp(EdgeX, Margin, ScreenW - Margin);
		EdgeY = FMath::Clamp(EdgeY, Margin, ScreenH - Margin);

		float Alpha = FMath::Clamp(1.f - (Dist / MaxDist), 0.15f, 0.9f);
		FLinearColor ArrowColor(1.f, 0.1f, 0.1f, Alpha);

		float Angle = FMath::Atan2(Dir.Y, Dir.X);
		float ArrowSize = 14.f;

		FVector2D Tip(EdgeX + FMath::Cos(Angle) * ArrowSize,
			EdgeY + FMath::Sin(Angle) * ArrowSize);
		FVector2D BaseL(EdgeX + FMath::Cos(Angle + 2.5f) * ArrowSize * 0.7f,
			EdgeY + FMath::Sin(Angle + 2.5f) * ArrowSize * 0.7f);
		FVector2D BaseR(EdgeX + FMath::Cos(Angle - 2.5f) * ArrowSize * 0.7f,
			EdgeY + FMath::Sin(Angle - 2.5f) * ArrowSize * 0.7f);

		DrawLine(Tip.X, Tip.Y, BaseL.X, BaseL.Y, ArrowColor, 2.5f);
		DrawLine(Tip.X, Tip.Y, BaseR.X, BaseR.Y, ArrowColor, 2.5f);
		DrawLine(BaseL.X, BaseL.Y, BaseR.X, BaseR.Y, ArrowColor, 2.5f);
	}
}

void ASpaceBattleHUD::DrawControlsHelp()
{
	float X = 20.f;
	float Y = Canvas->ClipY * 0.5f - 100.f;
	float LineH = 16.f;
	FLinearColor HelpColor(0.5f, 0.5f, 0.5f, 0.4f);

	DrawText(TEXT("W/S - Throttle"), HelpColor, X, Y, nullptr, 0.7f); Y += LineH;
	DrawText(TEXT("Mouse - Steer"), HelpColor, X, Y, nullptr, 0.7f); Y += LineH;
	DrawText(TEXT("A/D - Roll"), HelpColor, X, Y, nullptr, 0.7f); Y += LineH;
	DrawText(TEXT("LMB - Fire Lasers"), HelpColor, X, Y, nullptr, 0.7f); Y += LineH;
	DrawText(TEXT("RMB - Lock On"), HelpColor, X, Y, nullptr, 0.7f); Y += LineH;
	DrawText(TEXT("MMB - Fire Missile"), HelpColor, X, Y, nullptr, 0.7f); Y += LineH;
	DrawText(TEXT("Shift - Boost"), HelpColor, X, Y, nullptr, 0.7f); Y += LineH;
	DrawText(TEXT("Space - Barrel Roll"), HelpColor, X, Y, nullptr, 0.7f);
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
