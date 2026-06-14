# RoninFenix - Space Battle Game

Unreal Engine 5.7 space combat game inspired by Star Wars Battlefront II space battles.

## Build

- Engine: UE 5.7 installed at `D:\UE_5.7`
- Build: `D:\UE_5.7\Engine\Build\BatchFiles\Build.bat RoninFenix Win64 Development "-project=D:\UEProjects\RoninFenix\RoninFenix.uproject"`
- Module dependencies: Core, CoreUObject, Engine, InputCore, EnhancedInput, ProceduralMeshComponent, AIModule, UMG, Slate, SlateCore

## Architecture

All gameplay is in C++ under `Source/RoninFenix/`:

- `SpaceTypes.h` — shared enums (ESpaceTeam, EShipClass, EWeaponType), structs (FShipStats), constants
- `Components/` — SpaceshipMovement, HealthShield, Weapon, Targeting
- `Pawns/` — SpaceshipBase (procedural mesh hull), PlayerSpaceship (camera + input), AISpaceship
- `Actors/` — LaserProjectile, HomingMissile, Asteroid, CapitalShip, SpaceEnvironment
- `AI/` — SpaceshipAIController (patrol/engage/attack capital ship/evade states)
- `Framework/` — SpaceBattleGameMode, PlayerController, GameState, PlayerState
- `UI/` — SpaceBattleHUD (canvas-drawn)
- `Procedural/` — ProceduralShipMeshBuilder (generates fighter/interceptor/bomber/capital ship/asteroid geometry)

## Game Design

- Two teams (Alpha/Omega) with capital ships and AI fighter squadrons
- Three ship classes: Fighter (balanced), Interceptor (fast/fragile), Bomber (slow/tough)
- Weapons: dual lasers with overheat + lock-on missiles
- Capital ships have destroyable subsystems (shields, engines, weapons, bridge)
- Win condition: destroy enemy capital ship or highest score when timer expires
- 10-minute match timer

## Controls

W/S = throttle, Mouse = steer, Q/E = roll, LMB = lasers, RMB = lock-on, MMB = missile, Shift = boost, Space = barrel roll, Tab = cycle target
