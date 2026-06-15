#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SpaceTypes.h"
#include "SpaceBattleHUD.generated.h"

class APlayerSpaceship;
class ASpaceBattleGameState;

UCLASS()
class RONINFENIX_API ASpaceBattleHUD : public AHUD
{
	GENERATED_BODY()

public:
	ASpaceBattleHUD();

	virtual void DrawHUD() override;

private:
	void DrawCrosshair();
	void DrawHealthShieldBars();
	void DrawSpeedIndicator();
	void DrawBoostBar();
	void DrawWeaponInfo();
	void DrawTargetingInfo();
	void DrawRadar();
	void DrawScoreBoard();
	void DrawKillFeed();
	void DrawMatchTimer();
	void DrawCapitalShipHealth();
	void DrawMatchOverScreen();
	void DrawRespawnTimer();
	void DrawShipOutlines();
	void DrawEnemyDirectionArrows();
	void DrawControlsHelp();

	void DrawBar(float X, float Y, float Width, float Height, float Percent, FLinearColor FillColor, FLinearColor BackColor);

	APlayerSpaceship* GetPlayerShip() const;
	ASpaceBattleGameState* GetBattleGameState() const;

	UPROPERTY()
	TObjectPtr<UFont> HUDFont;
};
