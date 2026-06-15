# Action Roguelike Game Demo — Unreal Engine 5

> A third-person action game built from scratch in C++ with Unreal Engine 5, demonstrating core gameplay systems including character locomotion, projectile combat, special abilities, physics-driven destruction, and an interface-based interaction framework.

> 使用 C++ 与 Unreal Engine 5 从零构建的第三人称动作游戏 Demo，展示了角色运动、弹道战斗、特殊能力、物理破坏与基于接口的交互框架等核心游戏系统。

GIF demos for each ability are attached to the corresponding [GitHub Release](https://github.com/MournfulOx/ActionRoguelikeGameDemo_UE5/releases).

---

## Table of Contents / 目录

- [Overview](#overview)
- [Tech Stack](#tech-stack)
- [Demo](#demo)
- [Controls](#controls)
- [Implemented Systems](#implemented-systems)
- [Project Structure](#project-structure)
- [Development History](#development-history--开发历程)
- [Building & Running](#building--running)
- [Roadmap](#roadmap)

---

## Overview

This project is a hands-on learning demo built while studying Unreal Engine 5 C++ game development. The goal is to implement production-grade gameplay systems from first principles — no Blueprint-only shortcuts — in order to build a deep understanding of UE5's architecture: the Actor/Component model, the UObject reflection system, collision pipelines, and the input binding layer.

本项目是在学习 UE5 C++ 游戏开发过程中构建的实践 Demo。目标是从基础原理出发，纯 C++ 实现生产级游戏系统，深入理解 UE5 架构：Actor/Component 模型、UObject 反射系统、碰撞管线与输入绑定层。

---

## Tech Stack

| | |
|---|---|
| **Engine** | Unreal Engine 5.6 |
| **Language** | C++17 |
| **Build System** | Unreal Build Tool (UBT) |
| **IDE** | Rider / Visual Studio 2022 |
| **Version Control** | Git + Git LFS (for `.uasset` / `.umap` binaries) |
| **Platform** | Windows 11 x64 |

---

## Demo

See [Releases](https://github.com/MournfulOx/ActionRoguelikeGameDemo_UE5/releases) for full GIF demos of each ability.

---

## Controls

| Key | Action |
|---|---|
| W / A / S / D | Move |
| Mouse | Look / Aim |
| Left Mouse Button | Primary Attack (magic projectile) |
| Q | Blackhole Ability |
| R | Dash / Teleport |
| E | Interact with world |
| Space | Jump |

---

## Implemented Systems

### 1. Third-Person Character (`ASCharacter`)

- Spring-arm + camera rig with `bUsePawnControlRotation` for smooth mouse look
- World-space movement decoupled from camera yaw: `MoveForward` uses the controller's forward vector; `MoveRight` derives the right vector via `FRotationMatrix::GetScaledAxis(EAxis::Y)`, ensuring consistent strafe behavior at all camera angles
- `bOrientRotationToMovement = true` with `bUseControllerRotationYaw = false` — the character mesh faces the movement direction, not the camera
- Jump support via `ACharacter::Jump` / `StopJumping`
- Attack driven by `UAnimMontage` + `FTimerHandle`: the montage plays first, then the projectile spawns at the correct animation frame (0.2 s delay) from the `Muzzle_01` skeletal socket
- All three abilities share the same aim direction system: line trace from camera to crosshair, projectile rotated toward the impact point
- Delegates attack and interaction to dedicated components, keeping `ASCharacter` thin

### 2. Primary Attack — Magic Projectile (`AAMagicProjectile` + `BP_MagicProjectile`)

**Aim correction:** Line trace from camera origin along controller rotation (up to 2000 cm). Projectile rotation set via `FRotationMatrix::MakeFromX(ImpactPoint - HandLocation)` so the projectile always flies toward the crosshair target even when the muzzle socket is offset from the camera.

**C++ layer:**
- `USphereComponent` with `"Projectile"` collision profile as the root
- `UProjectileMovementComponent`: initial speed 1000 cm/s, zero gravity, velocity-aligned rotation
- `UParticleSystemComponent` for in-flight VFX
- Dual hit response:
  - **Blocking hit** (`NotifyHit`): applies 20 point damage to the hit actor — handles physics-simulated actors such as the explosive barrel
  - **Overlap hit** (`OnActorOverlap`): applies 20 point damage via `UGameplayStatics::ApplyPointDamage`, then destroys self

**Blueprint layer (`BP_MagicProjectile`):**
- `On Component Hit` event: spawns impact particle effect and destroys the projectile on any blocking collision
- Instigator ignored via `Ignore Actor when Moving` on `BeginPlay` to prevent self-collision

### 3. Blackhole Ability (`BP_BlackholeProjectile`)

Pure Blueprint projectile. Fired from `Muzzle_01` toward the camera crosshair (same aim system as primary attack, bound to **Q**).

- `URadialForceComponent` set to **attraction** mode continuously pulls nearby physics-simulated actors toward the center while active
- `On Component Begin Overlap`: any actor with physics simulation enabled (`IsSimulatingPhysics`) is destroyed on contact
- Self-destructs after **5 seconds** via a Blueprint `Delay` node, cleaning up the radial force field

### 4. Dash / Teleport (`ASDashProjectile` + `BP_DashProjectile`)

C++ class `ASDashProjectile` extends `AAMagicProjectile`. Bound to **R**.

**On spawn:**
- Starts a 2-second detonation timer (`TimerHandle_Detonate`) as a fallback if the projectile never hits anything

**On hit (`NotifyHit`) or timer expiry → `Explode()`:**
1. Guards against double-execution with `bExploded` flag
2. Clears the detonation timer
3. Stops projectile movement immediately (`MovementComp->StopMovementImmediately()`)
4. Spawns impact particle effect (`ImpactEffect`) at the projectile's location
5. Deactivates the in-flight trail (`EffectComp->DeactivateSystem()`)
6. Starts a short `TeleportDelay` (0.2 s) timer before moving the player

**`TeleportInstigator()`:**
- Spawns `TeleportExitEffect` at the instigator's current location (exit flash)
- Calls `InstigatorActor->TeleportTo(GetActorLocation() + FVector(0, 0, 50), ...)` — lifts player 50 cm above the impact point to avoid floor clipping
- Destroys the projectile actor

### 5. Attribute System (`USAttributeComponent`)

Component-based RPG-style attribute system. Attach to any Actor to give it health — character, enemy, or explosive barrel all share the same logic.

- `Health` / `HealthMax` stored as `protected` floats, exposed to Blueprint as read-only; only modifiable through `ApplyHealthChange`
- `ApplyHealthChange(InstigatorActor, Delta)` — clamps health between `0` and `HealthMax` via `FMath::Clamp`; dead-protection guard (`!isAlive() && Delta < 0`) prevents damage/effects after death
- `IsFullHealth()` — exposed to Blueprint for powerup/UI logic
- `DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnHealthChanged, ...)` — broadcasts `InstigatorActor`, `OwningComp`, `NewHealth`, `Delta` on every change; `BlueprintAssignable` so any system can subscribe without polling

### 6. Player Health UI (`WBP_PlayerHealth`)

Event-driven UMG health bar. Binds to `USAttributeComponent::OnHealthChanged` in `Construct` — updates progress bar and text only when health actually changes, not every frame.

- `Construct` (UMG BeginPlay) → casts owning pawn → gets `USAttributeComponent` → assigns delegate; also immediately sets bar to `Health / HealthMax` to handle non-100% starting health
- Progress bar value = `NewHealth / HealthMax` (reads runtime `HealthMax`, not class defaults)
- UI dependency is one-directional: UI listens to gameplay; gameplay never references UI

### 7. Damage Popup Widget (`WBP_DamagePopup`)

World-space floating damage numbers that follow the hit target and fade out.

- `Expose on Spawn` float `DamageAmount` — caller passes `Abs(Delta)` when creating the widget; displayed via `Set Text` on `Construct`
- `Event Tick`: projects `AttachTo` actor's world location to screen via `Project World to Screen`, divides by viewport scale for DPI correctness, adds a per-instance `ScreenOffset` (randomised at `Construct`) for Fortnite-style scatter
- UMG animation `PopupAnim`: scale punch `(1,1) → (1.4,1.4) → (0,0)` + alpha fade over 1 s; played on `Construct`
- Bound in `BP_Player`, `BP_TargetDummy`, and `BP_TestAttack` via `Bind Event to OnHealthChanged` + `Branch (Delta < 0)`

### 8. Hit Flash — Player & Enemies

Player character and all enemies flash on damage via `SetScalarParameterValueOnMaterials("TimeToHit", GetWorld()->TimeSeconds)`.

- `ASCharacter::OnHealthChanged`: triggers flash only when `Delta < 0`; same call disabled when `NewHealth <= 0` (dead-protection in `ApplyHealthChange`)
- `M_CharacterSimple` extended with `MF_HitFlashDemo` function call + `HitFlashColor` `VectorParameter` added to emissive channel — all `M_CharacterSimple_Inst` instances inherit automatically
- `BP_TestAttack` and `BP_TargetDummy` bind the same pattern in Blueprint

### 9. Projectile Audio (`ASProjectileBase`)

- `UAudioComponent` attached to root — looping flight sound assigned per Blueprint; auto-activates on spawn, auto-stops on destroy
- `USoundBase* ImpactSound` — played via `UGameplayStatics::PlaySoundAtLocation` inside `Explode_Implementation`

### 10. Casting Particle & Camera Shake

- **Casting effect**: `UGameplayStatics::SpawnEmitterAttached(CastingEffect, GetMesh(), "Muzzle_01")` fires at projectile spawn in `SpawnProjectile`; asset assigned in `BP_Character`
- **Camera shake**: `UGameplayStatics::PlayWorldCameraShake` called in `ASProjectileBase::Explode_Implementation` with the impact location; `TSubclassOf<UCameraShakeBase> ImpactCameraShake` assigned in `BP_MagicProjectile`

### 11. Health Potion Powerup (`ASPowerupActor` + `ASHealthPotion`)

Generic pickup/respawn framework designed for future powerup types.

**`ASPowerupActor` (base):**
- `USphereComponent` root (collision profile `OverlapAllDynamic` — detectable by the interaction sphere sweep)
- `UStaticMeshComponent` for visual mesh (asset assigned in Blueprint)
- `Interact_Implementation` calls `BlueprintNativeEvent OnActivated(Pawn)` — child classes override
- `HideAndCooldown()`: hides mesh, disables sphere collision, starts `CooldownDuration` (default 10 s) timer → `RestorePowerup()` re-enables both

**`ASHealthPotion` (child):**
- `OnActivated_Implementation`: gets `USAttributeComponent` from instigator; skips if `IsFullHealth()`; calls `ApplyHealthChange(+HealAmount)` then `HideAndCooldown()`

### 12. Crosshair UI Widget (UMG)

A `UUserWidget` Blueprint (`Content/UI/`) added to the player's HUD via the `BP_GameMode`. Displays a static crosshair at screen center to provide aim reference for all three projectile abilities.

### 13. Explosive Barrel (`AExplosiveBarrel` + `BP_ExplosiveBarrel`)

**C++ layer:**
- `UStaticMeshComponent` with `SetSimulatePhysics(true)` as root — fully physics-simulated rigid body
- `URadialForceComponent`: radius 750 cm, `bImpulseVelChange = true` (mass-independent velocity change), `bAutoActivate = false` (manual trigger only)
- `Explode()` (`public`, `BlueprintCallable`): fires the radial impulse — single entry point for explosion logic
- `TakeDamage` override: calls `Explode()` when struck by a projectile or triggered by a lever

**Blueprint layer (`BP_ExplosiveBarrel`):**
- Extends `Explode()` with destruction mesh swap and particle effect on detonation

### 14. Gameplay Interface & Interaction System

**`ISGameplayInterface` / `USGameplayInterface`**

UE5 `UINTERFACE`-based contract. Declares:
```cpp
UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
void Interact(APawn* InstigatorPawn);
```
Any actor can opt into the interaction system by implementing this interface — no inheritance coupling to the character or component.

**`USInteractionComponent`**

Attached to the character. On `PrimaryInteract()` (**E** key):
1. Performs a **sphere sweep** (radius 30 cm) from the character's eye point along the aim direction (max 1000 cm) using `SweepMultiByObjectType` against `ECC_WorldDynamic`
2. Iterates hits; calls `ISGameplayInterface::Execute_Interact(HitActor, InstigatorPawn)` on the first actor that passes `Implements<USGameplayInterface>()`
3. Renders debug geometry (`DrawDebugSphere`, `DrawDebugLine`) in green on hit, red on miss — visible for 2 s

**`ASItemChest` + `BP_TreasureChest`**

C++: implements `ISGameplayInterface`; on interact, rotates `LidMesh` to `TargetPitch` (default 110°) via `SetRelativeRotation`.
Blueprint: extends with lid-open animation and particle effect on interact.

**`BP_Lever`**

Pure Blueprint actor implementing `ISGameplayInterface`. Triggered by E key via the interaction system; controls one or more target actors (treasure chests or explosive barrels) — calls `Interact` or `Explode()` on targets through Blueprint event graph.

### 15. Dynamic Materials (`M_HitFlashDemo`, `ASTargetDummy`)

C++ driven material parameters for gameplay feedback — no Blueprint required.

**Hit Flash (`M_HitFlashDemo` / `MI_HitFlashDemo`):**
- Material has a `TimeToHit` scalar parameter. A sine-wave expression computes flash intensity from `(WorldTime - TimeToHit)`, producing a sharp white emissive burst that fades over ~0.8 s
- `ASTargetDummy::OnHealthChanged` calls `MeshComp->SetScalarParameterValueOnMaterials("TimeToHit", GetWorld()->TimeSeconds)` on any damage hit — the material reads the timestamp every frame and self-fades with no per-frame C++ tick

**Target Dummy (`ASTargetDummy` + `BP_TargetDummy`):**
- `UStaticMeshComponent` root + `USAttributeComponent` (100 HP)
- Binds `OnHealthChanged` in `BeginPlay`; flashes white on any damage via `TimeToHit` parameter
- Stateless hit feedback: no `UMaterialInstanceDynamic` allocation needed at runtime — `SetScalarParameterValueOnMaterials` updates all material instances on the mesh at once

**Material Function (`MF_HitFlashDemo`):**
- Encapsulates the hit flash node graph as a reusable chunk — drop into any material to add hit flash without duplicating the sine-wave logic

**UI Material (`M_HealthBar`):**
- Material Domain set to **User Interface**; used as a replacement health bar in UMG — demonstrates that UMG widgets can accept full material expressions rather than just static textures

**Additional material assets (learning / future use):**
- `M_PBRDemo` / `MI_PBRDemo_Inst` — PBR basics (base color, metallic, roughness)
- `M_DissoveEffect` — noise-texture dissolve, scalar-parameter driven
- `M_SineWave` — sine-wave utility material

### 17. Enemy AI — Behavior Tree (`ASAICharacter` + `ASAIController` + `USBTService_ChackAttackRange`)

C++ AI system built on UE5's Behavior Tree / Blackboard framework. A ranged minion enemy chases the player and stops when within attack range.

**AI Controller (`ASAIController`):**
- `BeginPlay` calls `RunBehaviorTree(BehaviorTree)` to start the tree; `BehaviorTree` asset reference is `EditDefaultsOnly` so it's assigned per Blueprint subclass (`BP_MinionController`)
- Immediately writes player's location and actor reference into the Blackboard: `SetValueAsVector("MoveToLocation", ...)` and `SetValueAsObject("TargetActor", ...)`

**Blackboard (`BB_MinionRanged`):**
- `SelfActor` (Object, auto-populated) — the AI pawn itself
- `MoveToLocation` (Vector) — cached player position
- `TargetActor` (Object, base class Actor) — player actor reference, tracked continuously by Move To task
- `WithinAttackRange` (Bool) — written every tick by the service

**Behavior Tree (`BT_MinionRanged`):**
```
ROOT
└── Selector  [Service: CheckAttackRange — ticks every ~0.5 s]
    ├── Sequence  [Decorator: "Outside Attack Range?" — Blackboard WithinAttackRange Is Not Set → abort Self]
    │   └── Move To Player  (tracks TargetActor actor reference, Acceptable Radius 5 cm)
    └── Wait 5 s   (fires when within attack range — placeholder for attack logic)
```
- **Selector** tries left child first; falls through to Wait only when the Decorator on the Sequence fails (i.e., `WithinAttackRange` is true)
- **Observer Aborts: Self** on the Decorator means Move To is immediately cancelled the moment the service sets `WithinAttackRange = true`, letting the Selector reach Wait without waiting for Move To to finish

**BT Service (`USBTService_ChackAttackRange`):**
- Overrides `TickNode` (called every ~0.5 s, configurable)
- Gets `TargetActor` from Blackboard via `GetValueAsObject` → cast to `AActor`
- Gets AI pawn via `OwnerComp.GetAIOwner()->GetPawn()`
- Computes `FVector::Distance`; if `< 500 cm` also runs `AIController::LineOfSightTo` for cover awareness
- Writes `bWithinRange && bHasLOS` to the Blackboard key bound via `FBlackboardKeySelector AttackRangeKey` (editable in the BT editor dropdown, not hardcoded)

**Build.cs additions:** `AIModule`, `GameplayTasks` (required for `UBTService` linkage).

**Level setup:** `NavMeshBoundsVolume` placed and scaled to cover the play area — provides pathfinding data used by the Move To task.

### 16. Input Bindings

Legacy axis/action input configured in `DefaultInput.ini`:

| Input | Binding | Action |
|---|---|---|
| W / S | `MoveForward` axis | Move forward / back |
| A / D | `MoveRight` axis | Strafe |
| Mouse X | `Turn` axis | Camera yaw |
| Mouse Y | `LookUp` axis | Camera pitch |
| Left Mouse Button | `PrimaryAttack` action | Fire magic projectile |
| Q | `BlackholeAttack` action | Fire blackhole projectile |
| R | `DashAttack` action | Fire dash/teleport projectile |
| Space | `Jump` action | Jump |
| E | `PrimaryInteract` action | Interact with world |

---

## Project Structure

```
Source/ActRouguelikeDemo/
├── Public/
│   ├── SCharacter.h            # Player character (3 abilities, interaction, hit flash)
│   ├── SInteractionComponent.h # Sphere-sweep interaction component
│   ├── SGameplayInterface.h    # UE5 interface for interactable actors
│   ├── SItemChest.h            # Interactable treasure chest
│   ├── SProjectileBase.h       # Projectile base (movement, VFX, audio, camera shake)
│   ├── AMagicProjectile.h      # Magic projectile (overlap damage, blocking hit)
│   ├── SDashProjectile.h       # Dash/teleport projectile
│   ├── SPowerupActor.h         # Powerup base (interact, respawn timer, hide/show)
│   ├── SHealthPotion.h         # Health potion (heals pawn, ignores full health)
│   ├── ExplosiveBarrel.h       # Physics barrel (reacts to damage)
│   ├── SAttributeComponent.h  # RPG attribute component (Health/HealthMax, delegate, dead-guard)
│   └── AI/
│       ├── SAICharacter.h      # AI character base class
│       ├── SAIController.h     # Runs BehaviorTree on BeginPlay, seeds Blackboard
│       └── SBTService_ChackAttackRange.h  # BT Service: distance + LOS check → WithinAttackRange
└── Private/
    ├── SCharacter.cpp
    ├── SInteractionComponent.cpp
    ├── SGameplayInterface.cpp
    ├── SItemChest.cpp
    ├── SProjectileBase.cpp
    ├── AMagicProjectile.cpp
    ├── SDashProjectile.cpp
    ├── SPowerupActor.cpp
    ├── SHealthPotion.cpp
    ├── ExplosiveBarrel.cpp
    ├── SAttributeComponent.cpp
    └── AI/
        ├── SAICharacter.cpp
        ├── SAIController.cpp
        └── SBTService_ChackAttackRange.cpp

Content/AI/
├── BP_MinionRanged.uasset          # Ranged AI minion Blueprint (mesh, AnimBP, AI controller ref)
├── BP_MinionController.uasset      # AI Controller Blueprint (assigns BT_MinionRanged asset)
├── BT_MinionRanged.uasset          # Behavior Tree: Selector → Move To Player / Wait
└── BB_MinionRanged.uasset          # Blackboard: SelfActor, MoveToLocation, TargetActor, WithinAttackRange

Content/Blueprint/
├── BP_MagicProjectile.uasset       # Primary projectile (audio, camera shake, casting FX)
├── BP_BlackholeProjectile.uasset   # Blackhole ability (RadialForce, auto-destroy)
├── BP_DashProjectile.uasset        # Dash ability (teleport effect config)
├── BP_Player.uasset                # Player Blueprint (ability refs, damage popup binding)
├── BP_HealthPotion.uasset          # Health potion pickup (SM_PotionBottle, 10s respawn)
├── BP_TestAttack.uasset            # Test enemy (timed attack, hit flash, damage popup)
└── BP_GameMode.uasset              # Game mode (HUD widget setup)

Content/UI/
├── WBP_Crosshair.uasset            # Crosshair HUD widget (UMG)
├── WBP_PlayerHealth.uasset         # Health bar (event-driven, HealthMax-aware init)
└── WBP_DamagePopup.uasset          # Floating damage numbers (world-space, random offset, animation)

Content/Material/
├── M_HitFlashDemo.uasset           # Hit flash — TimeToHit scalar param, sine-wave fade
├── MI_HitFlashDemo.uasset          # Material instance applied to target dummy mesh
├── MF_HitFlashDemo.uasset          # Reusable Material Function encapsulating hit flash logic
├── M_HealthBar.uasset              # UI material for health bar (Material Domain: User Interface)
├── M_DissoveEffect.uasset          # Dissolve — noise texture + scalar param
├── M_PBRDemo.uasset                # PBR basics reference material
├── MI_PBRDemo_Inst.uasset          # PBR material instance
└── M_SineWave.uasset               # Sine-wave utility material
```

---

## Development History / 开发历程

See [Releases](https://github.com/MournfulOx/ActionRoguelikeGameDemo_UE5/releases) for the full changelog with screenshots and build notes.

完整的版本历史、截图与开发笔记见 [Releases](https://github.com/MournfulOx/ActionRoguelikeGameDemo_UE5/releases) 页面。

---

## Building & Running

**Prerequisites:** Unreal Engine 5.6 installed via Epic Games Launcher, Visual Studio 2022 (or Rider) with the **Game Development with C++** workload.

```powershell
# 1. Clone the repository
git clone https://github.com/MournfulOx/ActionRoguelikeGameDemo_UE5.git
cd ActionRoguelikeGameDemo_UE5

# 2. Right-click the .uproject → "Generate Visual Studio project files"
#    (or use UnrealBuildTool directly)

# 3. Open the .sln in Visual Studio, set config to Development Editor | Win64, build

# 4. Launch the .uproject from the Epic Games Launcher or the compiled editor
```

---

## Roadmap

- [x] Crosshair HUD via `UMG` / `UUserWidget`
- [x] Attribute system (`USAttributeComponent`) — `Health`/`HealthMax`, clamp, dead-guard, multicast delegate
- [x] Player health bar UI (`WBP_PlayerHealth`) — event-driven UMG, `HealthMax`-aware
- [x] Dynamic materials — hit flash (`MF_HitFlashDemo`) on player, enemies, and target dummy
- [x] Damage popup widget (`WBP_DamagePopup`) — world-space follow, random scatter, UMG animation
- [x] Projectile audio — looped flight sound (`UAudioComponent`) + impact sound (`PlaySoundAtLocation`)
- [x] Casting particle effect (`SpawnEmitterAttached`) + world camera shake on impact
- [x] Health potion powerup (`ASPowerupActor` / `ASHealthPotion`) — interact to heal, 10 s respawn
- [x] Enemy AI — Behavior Tree + Blackboard + BT Service (`ASAICharacter`, `ASAIController`, `USBTService_ChackAttackRange`) — ranged minion chases player, stops within attack range, line-of-sight check
- [ ] Enemy attack logic (fire projectile at player when within range)
- [ ] Enhanced Input System migration (from legacy `BindAxis` / `BindAction`)
- [ ] Networked multiplayer replication

---

## About / 关于

This project is part of an ongoing effort to build a solid C++ foundation in Unreal Engine 5 by reimplementing core systems from scratch.

本项目是持续学习 UE5 C++ 开发、从零重新实现核心系统的系列实践之一。

**Author / 作者:** Jacky Feng  
**Engine:** Unreal Engine 5.6  
**Started:** 2025

---

*Course support: Based on [Tom Looman](https://www.tomlooman.com/)'s Unreal Engine C++ Action Roguelike course, also taught at Stanford University as CS193U.*  
*课程支持：基于 Tom Looman 的 UE C++ Action Roguelike 课程，该课程亦以 CS193U 形式在斯坦福大学开设。*
