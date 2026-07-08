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
| Left Shift | Sprint |

---

## Implemented Systems

### 1. Third-Person Character (`ASCharacter`)

- Spring-arm + camera rig with `bUsePawnControlRotation` for smooth mouse look
- World-space movement decoupled from camera yaw: `MoveForward` uses the controller's forward vector; `MoveRight` derives the right vector via `FRotationMatrix::GetScaledAxis(EAxis::Y)`, ensuring consistent strafe behavior at all camera angles
- `bOrientRotationToMovement = true` with `bUseControllerRotationYaw = false` — the character mesh faces the movement direction, not the camera
- Jump support via `ACharacter::Jump` / `StopJumping`
- Owns a single `USActionComponent* ActionComp`; `PrimaryAttack()` / `BlackholeAttack()` / `DashAttack()` / `SprintStart()` / `SprintStop()` are one-line calls to `ActionComp->StartActionByName(this, "...")` / `StopActionByName(...)` — no ability logic, timers, or asset references live on the character anymore (see [Action System](#24-action-system--custom-gameplay-ability-system-alternative-usactioncomponent--usaction))
- All three attack abilities share the same aim direction system (camera-to-crosshair line trace), now implemented once in `USAction_ProjectileAttack` instead of being duplicated per-attack
- Delegates attack, interaction, and ability logic to dedicated components, keeping `ASCharacter` thin

### 2. Primary Attack — Magic Projectile (`AAMagicProjectile` + `BP_MagicProjectile`)

**Aim correction:** Line trace from camera origin along controller rotation (up to 2000 cm). Projectile rotation set via `FRotationMatrix::MakeFromX(ImpactPoint - HandLocation)` so the projectile always flies toward the crosshair target even when the muzzle socket is offset from the camera.

**C++ layer:**
- `USphereComponent` with `"Projectile"` collision profile as the root
- `UProjectileMovementComponent`: initial speed 1000 cm/s, zero gravity, velocity-aligned rotation
- `UParticleSystemComponent` for in-flight VFX
- Dual hit response:
  - **Blocking hit** (`NotifyHit`): falls back to `UGameplayStatics::ApplyDamage` for actors with no `USAttributeComponent` (e.g. standard `TakeDamage`-based actors)
  - **Overlap hit** (`OnActorOverlap`): calls `USGameplayFunctionLibrary::ApplyDirectionalDamage(GetInstigator(), OtherActor, DamageAmount, SweepResult)` — applies damage and, if the hit component is physics-simulated, a directional knockback impulse — then destroys self

**Blueprint layer (`BP_MagicProjectile`):**
- `On Component Hit` event: spawns impact particle effect and destroys the projectile on any blocking collision
- Instigator ignored via `Ignore Actor when Moving` on `BeginPlay` to prevent self-collision

### 3. Blackhole Ability (`ASBlackholeProjectile` + `BP_BlackholeProjectile`)

C++ projectile fired from `Muzzle_01` toward the camera crosshair (same aim system as primary attack, bound to **Q**, granted via `BP_ActionBlackhole`).

- `USphereComponent` root (`PullSphereComp`, 20 cm radius, blocks `ECC_WorldStatic` only) stops the projectile on walls; `UProjectileMovementComponent` (speed 800, zero gravity) flies it out
- `Tick`: stops flight once it travels `MaxRange`; every frame, pulls any `ASAICharacter` within `PullRadius` toward itself via `AddActorWorldOffset`, and pulls physics-simulated actors (barrels, cubes — excluding AI/self/instigator) via `AddForce` scaled by mass
- `ApplyPullDamage` (1 s repeating timer, started in `BeginPlay`): applies `DamagePerSecond` to every AI within `PullRadius` via `USGameplayFunctionLibrary::ApplyDamage`
- `NotifyHit` stops movement immediately on wall impact; `SetLifeSpan(LifeSpanDuration)` handles self-destruction

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

### 17. GameMode — Dynamic AI Spawning (`ASGameModeBase`)

Spawns ranged minions at runtime via EQS so the level is never hand-populated with enemies.

- `StartPlay()` (not `BeginPlay`) starts a repeating timer (`SpawnTimerInterval`, default 2 s) → `SpawnBotTimerElapsed()`
- Each tick runs `UEnvQueryManager::RunEQSQuery` (`Query_FindBotSpawn`, `RandomBest5Pct` mode); callback `OnQueryCompleted` (`UFUNCTION()` — required for `AddDynamic`) fires when the query finishes
- Before spawning, counts alive bots via `TActorIterator<ASAICharacter>` + `USAttributeComponent::isAlive()` (`EngineUtils.h` required for the iterator)
- Max bot count driven by `UCurveFloat* DifficultyCurve` keyed on `GetWorld()->TimeSeconds` — difficulty scales over time; falls back to `4` if no curve is assigned
- `FActorSpawnParameters::SpawnCollisionHandlingOverride = AdjustIfPossibleButAlwaysSpawn` ensures bots don't fail to spawn due to mild overlap at the chosen EQS point
- `AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned` set on `ASAICharacter` — without this, dynamically spawned bots have no controller

### 18. Enemy AI — Behavior Tree, EQS & Pawn Sensing (`ASAICharacter` + `ASAIController` + custom BT Task / Service)

C++ AI system built on UE5's Behavior Tree / Blackboard framework. A ranged minion enemy *spots* the player by sight, repositions to a smart firing spot near the player using an Environment Query, and fires projectiles when within attack range and line of sight.

**AI Controller (`ASAIController`):**
- `BeginPlay` calls `RunBehaviorTree(BehaviorTree)`, guarded by `ensureMsgf` so a missing tree assignment surfaces a clear assert instead of a silent failure (`BehaviorTree` is `EditDefaultsOnly`, assigned per Blueprint subclass `BP_MinionController`)
- No longer auto-targets the player on `BeginPlay` — targeting is now driven by sight (Pawn Sensing) so the enemy stays idle until it actually sees the player

**Sight — Pawn Sensing (`ASAICharacter`):**
- `UPawnSensingComponent` added in the constructor; binds `OnSeePawn` → `OnPawnSeen(APawn*)`
- `OnPawnSeen` writes the spotted pawn into the Blackboard `TargetActor` key and draws a `"PLAYER SPOTTED"` debug string above the bot
- *Note:* `UPawnSensingComponent` is deprecated in UE5 in favor of the AI Perception system (compiles with a warning) — migration is on the roadmap

**Blackboard (`BB_MinionRanged`):**
- `SelfActor` (Object, auto-populated) — the AI pawn itself
- `MoveToLocation` (Vector) — destination produced by the EQS query
- `TargetActor` (Object, base class Actor) — player actor reference, set by Pawn Sensing
- `WithinAttackRange` (Bool) — written every tick by the service

**Behavior Tree (`BT_MinionRanged`):**
```
ROOT
└── Selector  [Service: CheckAttackRange (~0.5 s) + Set Default Focus to TargetActor]
    ├── [Decorator: "Within Attack Range?" — WithinAttackRange Is Set]
    │   └── Selector → Cooldown → Loop → Attack Sequence (Wait · RangedAttack)
    ├── Move To Sequence
    │   ├── Find Nearby Location  (Run EQS Query → writes MoveToLocation)
    │   └── Move To Player        (Move To MoveToLocation)
    └── Wait 2 s
```
- **Set Default Focus** keeps the bot facing `TargetActor` so projectiles fire in the right direction
- When in range, the **Cooldown + Loop** fires a short burst of ranged attacks, then re-evaluates

**Custom C++ BT Task — Ranged Attack (`USBTTask_RangedAttack`):**
- Overrides `ExecuteTask`; resolves the AI's pawn via `OwnerComp.GetAIOwner()->GetPawn()`
- Reads the `Muzzle_01` skeletal socket location and the `TargetActor` from the Blackboard
- Aims by direction vector `TargetActor location − MuzzleLocation`, converted to a `FRotator` via `.Rotation()`
- Spawns `ProjectileClass` (`TSubclassOf<AActor>`, `EditAnywhere` — chosen per BT node) with `ESpawnActorCollisionHandlingMethod::AlwaysSpawn` so the projectile spawns even when the muzzle overlaps the bot; returns `Succeeded` / `Failed` accordingly

**Environment Query System (EQS) — smarter movement:**
- Enabled via *Editor Preferences → Experimental → EQS*
- Custom context `QueryContext_TargetActor` returns the Blackboard `TargetActor` so tests can measure against the player
- `Query_FindNearbyLocation`:
  - **Donut** generator centered on the player — rings of candidate points around the target
  - **Distance** test: at least 500 cm, prefer lesser — keeps the bot at a firing distance, not on top of the player
  - **Trace** test (Visibility, require *not* hit) — discards points with no line of sight to the player
- The BT runs the query, writes the best point to `MoveToLocation`, and `Move To` walks there — giving more natural repositioning than charging straight at the player

**BT Service (`USBTService_ChackAttackRange`):**
- Overrides `TickNode` (~0.5 s); reads `TargetActor`, computes `FVector::Distance` to the AI pawn
- If `< 500 cm`, also runs `AIController::LineOfSightTo`; writes `bWithinRange && bHasLOS` to the Blackboard key bound via `FBlackboardKeySelector AttackRangeKey` (editable in the BT editor, not hardcoded)

**AI Death:**
- `USAttributeComponent` added to `ASAICharacter`; `OnHealthChanged` bound in `PostInitializeComponents`
- On death (`NewHealth <= 0`): stops BT via `AIC->GetBrainComponent()->StopLogic("Killed")`; enables ragdoll (`SetAllBodiesSimulatePhysics(true)`, collision profile `"Ragdoll"`); disables capsule collision; sets `SetLifeSpan(DissolveDuration + 0.5f)`
- Hit flash on damage: `GetMesh()->SetScalarParameterValueOnMaterials("TimeToHit", ...)` — only fires while alive to avoid overwriting dissolve parameters
- **Dissolve effect**: on death, creates a `UMaterialInstanceDynamic` from `M_DissoveEffect`, applies it to all mesh slots; a 0.05 s looping timer drives `DissolveAmount` from `1.0` (visible) → `-1.0` (fully dissolved) over `DissolveDuration` seconds via `FMath::Lerp`; timer auto-clears when `Alpha >= 1.0`

**Friendly Fire Prevention:**
- `AMagicProjectile::OnActorOverlap` checks `OtherActor->IsA(GetInstigator()->GetClass())` — AI projectiles skip other AI units; player projectiles skip other player pawns
- `SBTTask_RangedAttack` sets `Params.Instigator = MyPawn` so the spawned projectile knows who fired it

**Bot Animation Polish (Blueprint):**
- `BP_MinionRanged` CharacterMovement: `Use Controller Rotation Yaw = false`, `Use Desired Rotation = true` — smooth rotation instead of instant snap
- `Jog_Combat_BS` blend space: `Target Weight Interpolation Per Second = 8` — fast blend eliminates foot sliding when the bot changes speed

**Build.cs additions:** `AIModule`, `GameplayTasks` (required for `UBTService` / `UBTTaskNode` linkage).

**Level setup:** `NavMeshBoundsVolume` placed and scaled to cover the play area — provides pathfinding data used by Move To and EQS.

### 20. 3D World-Space Health Bar (`USWorldUserWidget`)

Enemy health bar that floats above the enemy in world space, appears on first hit, and disappears when the enemy dies.

**C++ base class (`USWorldUserWidget`):**
- Extends `UUserWidget`; overrides `NativeTick` (`bCanEverTick = true` set in the Blueprint constructor)
- `UPROPERTY(meta=(BindWidget)) USizeBox* ParentSizeBox` — UMG slot anchored at `(0,0)`, sized to wrap the bar content; `SetRenderTranslation` moves it each tick
- `UPROPERTY(BlueprintReadOnly) AActor* AttachedActor` — set by C++ before `AddToViewport()`
- `UPROPERTY(EditAnywhere) FVector WorldOffset` — tunable vertical offset (e.g. above the head)
- `NativeTick`: calls `UGameplayStatics::ProjectWorldToScreen` to get pixel coords, then divides by `UWidgetLayoutLibrary::GetViewportScale` for DPI correction before applying `SetRenderTranslation`; if `AttachedActor` becomes invalid (`!IsValid`), calls `RemoveFromParent()`

**Integration in `ASAICharacter::OnHealthChanged`:**
- Widget spawned only on first damage (`ActiveHealthBar == nullptr`), via `CreateWidget<USWorldUserWidget>(GetWorld(), HealthBarWidgetClass)` (owner must be the world, not the AICharacter)
- `ActiveHealthBar->AttachedActor = this` assigned before `AddToViewport()` so the first `NativeTick` already has a valid actor reference
- Widget auto-removes itself when the actor is destroyed (handled inside `NativeTick`)

**Blueprint layer (`BP_MinionHealth_Widget`):**
- Parent class set to `USWorldUserWidget`; contains a Canvas Panel → SizeBox (`ParentSizeBox`) → Progress Bar wired to `AttributeComp->GetHealth() / GetHealthMax()`

### 21. Main HUD Framework (`WBP_Main_HUD`)

A single container widget that bundles all player-facing HUD elements.

- **`WBP_Main_HUD`**: added to viewport by `BP_OwnGameMode` on game start; uses a Canvas Panel to host child widgets at fixed screen positions
  - `WBP_PlayerHealth` — health bar (bottom-centre), anchored with explicit position + size (not stretch anchors)
  - `WBP_Crosshair` — static crosshair at screen centre
  - `WBP_Credits` — placeholder credit display (top-right)
  - `WBP_GameModeInfo` — top-left panel showing elapsed game time via `GameState->GetServerWorldTimeSeconds()` (multiplayer-safe)
- `WBP_GameModeInfo` uses a Text widget bound in Blueprint: `Get Game State` → cast to `AGameState` → `GetServerWorldTimeSeconds()` → format as `MM:SS`

### 22. Player Spawn via GameMode + PlayerStart

Replaced the manually placed player character in the level with a proper spawn setup.

- **`PlayerStart`** actor placed in the level — GameMode uses it as the preferred spawn point
- **`DefaultPawnClass`** set in `BP_OwnGameMode` to `BP_Player` — the engine spawns and possesses the player automatically at game start
- **Project Settings → Maps & Modes → Default Game Mode** points to `BP_OwnGameMode` so the setting is project-wide
- The old `BP_Player` actor dragged directly into the level is removed — it was never possessed correctly and caused two characters to appear

### 23. Debug Console Commands (`UFUNCTION(Exec)`)

`UFUNCTION(Exec)` makes a member function callable as a typed console command during play. Works on: `APlayerController`, `ACharacter` (while possessed), `AGameMode`, `ACheatManager`.

- **`ASCharacter::HealSelf(float Amount = 100.f)`** — calls `AttributeComp->ApplyHealthChange(this, Amount)`; type `HealSelf 50` in the console to restore 50 HP
- **`ASGameModeBase::KillAll()`** — iterates all `ASAICharacter` actors via `TActorIterator`, calls `AttributeComp->Kill(this)` on every living bot; useful for testing respawn and spawn rate
- **God Mode** — `CanBeDamaged` bool on `AActor` (built-in UE5); `USAttributeComponent::ApplyHealthChange` checks `!GetOwner()->CanBeDamaged()` and returns early; toggled via the built-in `God` console command (from `ACheatManager`)

### 24. Action System — Custom Gameplay Ability System Alternative (`USActionComponent` + `USAction`)

A lightweight, hand-rolled alternative to Epic's Gameplay Ability System (GAS) — built to learn the underlying concepts (grantable abilities, per-actor action instances, code separation) without GAS's setup overhead. Any actor can gain abilities simply by attaching `USActionComponent`.

**`USActionComponent` (`UActorComponent`):**
- `TArray<USAction*> Actions` — runtime instances owned by this component
- `TArray<TSubclassOf<USAction>> DefaultActions` (`EditAnywhere`) — actions granted automatically; `BeginPlay` loops over it and calls `AddAction()` for each, so abilities are data-driven per-Blueprint instead of wired by hand in the event graph
- `AddAction(TSubclassOf<USAction> ActionClass)` — `NewObject<USAction>(this, ActionClass)`, appends to `Actions`
- `StartActionByName(Instigator, FName ActionName)` / `StopActionByName(...)` — linear search by `ActionName`, calls the matching action's `StartAction` / `StopAction`

**`USAction` (`UObject`, `Blueprintable`):**
- `FName ActionName` — identifies the action for `StartActionByName` lookups
- `StartAction` / `StopAction` — `BlueprintNativeEvent`, so actions can be pure C++, pure Blueprint, or a C++ base extended in Blueprint
- Overrides `GetWorld()` by walking the Outer chain (`Cast<UActorComponent>(GetOuter())->GetWorld()`) — required because a plain `UObject` has no world context of its own, and `NewObject` sets the owning `USActionComponent` as the Outer

**`USAction_ProjectileAttack` (C++ subclass of `USAction`):**
- Consolidates the "play montage → wait → spawn projectile toward the crosshair" pattern that used to be duplicated across all three player attacks in `ASCharacter`
- `StartAction_Implementation`: plays `AttackAnim`, spawns `CastingEffect` at the `HandSocketName` socket, starts a timer (`AttackAnimDelay`) to `AttackDelay_Elapsed`
- `AttackDelay_Elapsed`: fetches the instigator's camera via `FindComponentByClass<UCameraComponent>()`, re-runs the camera-to-crosshair aim trace (same sphere-sweep + dot-product fallback logic previously in `ASCharacter::SpawnProjectile`), spawns `ProjectileClass`, then calls `StopAction`
- `BP_ActionMagicProjectile`, `BP_ActionBlackhole`, `BP_ActionDash` are thin, data-only Blueprint subclasses — each just assigns a different `ProjectileClass` / `AttackAnim` / `CastingEffect`

**`BP_ActionSprint` (Blueprint subclass of plain `USAction`):**
- Overrides `StartAction` / `StopAction` to add/remove a `BonusSpeed` value on `CharacterMovementComponent::MaxWalkSpeed` — no C++ needed since the logic is a single variable set

**Result:** `SCharacter.h` / `.cpp` no longer own `ProjectileClass`, `BlackholeProjectileClass`, `DashProjectileClass`, `AttackAnim`, `CastingEffect`, any `FTimerHandle`, or `SpawnProjectile()` — all of that now lives in the Action classes, and `BP_Player`'s `ActionComp` lists the four action Blueprints in `DefaultActions` to grant them at `BeginPlay`.

### 25. Gameplay Function Library — Centralized Damage Application (`USGameplayFunctionLibrary`)

A `UBlueprintFunctionLibrary` that replaces the "get `USAttributeComponent` → call `ApplyHealthChange`" pattern that had been copy-pasted across every projectile's overlap/hit handler.

- **`ApplyDamage(DamageCauser, TargetActor, DamageAmount)`** — `USAttributeComponent::GetAttributes(TargetActor)` + `ApplyHealthChange(DamageCauser, -DamageAmount)`, wrapped in one static call
- **`ApplyDirectionalDamage(DamageCauser, TargetActor, DamageAmount, const FHitResult& HitResult)`** — calls `ApplyDamage`, then if the hit component `IsSimulatingPhysics(BoneName)`, applies a knockback impulse: `AddImpulseAtLocation(Direction * 300000.f, ImpactPoint, BoneName)`
  - **Direction is `(HitResult.TraceEnd - HitResult.TraceStart).GetSafeNormal()`, not `HitResult.ImpactNormal`** — `ImpactNormal` only reflects the struck surface's geometry (e.g. hitting the side of a head knocks it sideways even when shot from the front), producing inconsistent-looking knockback; using the trace vector always knocks the target away from where the shot actually came from
- Adopted by `AAMagicProjectile::OnActorOverlap` and `ASAIProjectile::OnActorOverlap` (both have a `SweepResult` available) for directional knockback on physics-simulated hits (e.g. `ExplosiveBarrel`, AI ragdolls); `ASBlackholeProjectile::ApplyPullDamage` (timer-tick damage-over-time, no hit result available) uses plain `ApplyDamage`

### 19. AI Flee / Heal Behavior — Assignment 4

When health drops below 30 %, the bot breaks off combat, retreats to a hidden position, heals to full, and resumes fighting — but can only flee once every 60 seconds.

**BT Service (`USBTService_CheckHealth`):**
- `TickNode` reads `USAttributeComponent::GetHealth() / GetHealthMax()`; writes result to a `LowHealth` Bool Blackboard key
- `LowHealthFraction` (default 0.3) is `EditAnywhere` so the threshold can be tuned per BT node

**BT Task (`USBTTask_HealSelf`):**
- Calls `AttributeComp->ApplyHealthChange(AIPawn, GetHealthMax())` — `ApplyHealthChange` clamps internally so passing `HealthMax` always fills to full without overshooting
- Returns `Succeeded` immediately (healing is instantaneous)

**EQS — `Query_FindHidingSpot`:**
- **Donut** generator centered on the AI (`EnvQueryContext_Querier`) — candidate ring of points at 500–1500 cm
- **Trace test** from `QueryContext_TargetActor` (player) with `Bool Match = false` — keeps only points the player cannot see (occluded positions)
- **Distance test** (Score Only, negative factor) — prefers closer hiding spots to minimise travel time

**Behavior Tree changes:**
```
ROOT
└── Selector  [Service: CheckHealth · Service: CheckAttackRange · Set Default Focus]
    ├── Sequence  [Decorator: LowHealth = true · Decorator: Cooldown 60 s]  ← flee branch (highest priority)
    │   ├── Run EQS Query  (Query_FindHidingSpot → HideLocation)
    │   ├── Move To        (HideLocation)
    │   └── HealSelf
    ├── [Decorator: Within Attack Range?] Selector → Cooldown → Loop → Attack Sequence
    ├── Move To Sequence  (EQS find nearby location → move)
    └── Wait
```
- The Cooldown decorator on the flee Sequence prevents re-entry for 60 s after healing completes
- After healing, `LowHealth` becomes false (health is full), so the Selector falls through to the attack branch and combat resumes normally

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
│   ├── SBlackholeProjectile.h  # Blackhole projectile (pull, AoE tick damage)
│   ├── SAIProjectile.h         # AI ranged attack projectile
│   ├── SDashProjectile.h       # Dash/teleport projectile
│   ├── SPowerupActor.h         # Powerup base (interact, respawn timer, hide/show)
│   ├── SHealthPotion.h         # Health potion (heals pawn, ignores full health)
│   ├── ExplosiveBarrel.h       # Physics barrel (reacts to damage)
│   ├── SAttributeComponent.h  # RPG attribute component (Health/HealthMax, delegate, dead-guard)
│   ├── SWorldUserWidget.h      # World-space widget base (NativeTick, ProjectWorldToScreen, DPI scale)
│   ├── SActionComponent.h      # Action System: owns/grants USAction instances (DefaultActions, StartActionByName)
│   ├── SAction.h                # Action System: base UObject action (ActionName, StartAction/StopAction, GetWorld via Outer)
│   ├── SAction_ProjectileAttack.h  # Action: montage → delayed projectile spawn (shared by all 3 attacks)
│   ├── SGameplayFunctionLibrary.h  # Static damage helpers (ApplyDamage, ApplyDirectionalDamage w/ knockback)
│   └── AI/
│       ├── SAICharacter.h               # AI character + PawnSensing + AttributeComp + dissolve death
│       ├── SAIController.h              # Runs BehaviorTree on BeginPlay (ensureMsgf guard)
│       ├── SBTService_ChackAttackRange.h  # BT Service: distance + LOS → WithinAttackRange
│       ├── SBTService_CheckHealth.h     # BT Service: health fraction → LowHealth bool
│       ├── SBTTask_RangedAttack.h       # BT Task: spawn projectile at target from Muzzle_01
│       └── SBTTask_HealSelf.h           # BT Task: restore AI to full health
├── SGameModeBase.h                      # GameMode: dynamic AI spawning via EQS + difficulty curve
└── Private/
    ├── SCharacter.cpp
    ├── SInteractionComponent.cpp
    ├── SGameplayInterface.cpp
    ├── SItemChest.cpp
    ├── SProjectileBase.cpp
    ├── AMagicProjectile.cpp
    ├── SBlackholeProjectile.cpp
    ├── SAIProjectile.cpp
    ├── SDashProjectile.cpp
    ├── SPowerupActor.cpp
    ├── SHealthPotion.cpp
    ├── ExplosiveBarrel.cpp
    ├── SAttributeComponent.cpp
    ├── SGameModeBase.cpp
    ├── SWorldUserWidget.cpp
    ├── SActionComponent.cpp
    ├── SAction.cpp
    ├── SAction_ProjectileAttack.cpp
    ├── SGameplayFunctionLibrary.cpp
    └── AI/
        ├── SAICharacter.cpp
        ├── SAIController.cpp
        ├── SBTService_ChackAttackRange.cpp
        ├── SBTService_CheckHealth.cpp
        ├── SBTTask_RangedAttack.cpp
        └── SBTTask_HealSelf.cpp

Content/AI/
├── BP_MinionRanged.uasset           # Ranged AI minion (mesh, AnimBP, DissolveMaterial, rotation fix)
├── BP_MinionController.uasset       # AI Controller Blueprint (assigns BT_MinionRanged asset)
├── BT_MinionRanged.uasset           # Behavior Tree: flee→heal · sight · EQS reposition · ranged attack
├── BB_MinionRanged.uasset           # Blackboard: SelfActor, MoveToLocation, TargetActor, WithinAttackRange, LowHealth, HideLocation
├── Query_FindNearbyLocation.uasset  # EQS: Donut + Distance + Trace → smart firing spot near player
├── Query_FindHidingSpot.uasset      # EQS: Donut + Trace (hidden from player) + Distance → flee destination
├── Query_FindBotSpawn.uasset        # EQS: spawn location for GameMode bot spawner
└── QueryContext_TargetActor.uasset  # EQS context returning the Blackboard TargetActor

Content/Blueprint/
├── BP_OwnGameMode.uasset            # GameMode Blueprint (MinionClass, SpawnBotQuery, DifficultyCurve)

Content/Blueprint/
├── BP_MagicProjectile.uasset       # Primary projectile (audio, camera shake, casting FX)
├── BP_BlackholeProjectile.uasset   # Blackhole ability (RadialForce, auto-destroy)
├── BP_DashProjectile.uasset        # Dash ability (teleport effect config)
├── BP_Player.uasset                # Player Blueprint (ActionComp::DefaultActions, damage popup binding)
├── BP_HealthPotion.uasset          # Health potion pickup (SM_PotionBottle, 10s respawn)
├── BP_TestAttack.uasset            # Test enemy (timed attack, hit flash, damage popup)
└── BP_GameMode.uasset              # Game mode (HUD widget setup)

Content/Actions/
├── BP_ActionMagicProjectile.uasset # extends SAction_ProjectileAttack — primary attack config
├── BP_ActionBlackhole.uasset       # extends SAction_ProjectileAttack — blackhole attack config
├── BP_ActionDash.uasset            # extends SAction_ProjectileAttack — dash attack config
└── BP_ActionSprint.uasset          # extends SAction — adds/removes MaxWalkSpeed bonus

Content/UI/
├── WBP_Crosshair.uasset            # Crosshair HUD widget (UMG)
├── WBP_PlayerHealth.uasset         # Health bar (event-driven, HealthMax-aware init)
├── WBP_DamagePopup.uasset          # Floating damage numbers (world-space, random offset, animation)
├── WBP_Main_HUD.uasset             # HUD container (PlayerHealth + Crosshair + Credits + GameModeInfo)
├── WBP_GameModeInfo.uasset         # Game time display using GameState::GetServerWorldTimeSeconds
├── WBP_Credits.uasset              # Credit display placeholder
└── WBP_MinionHealth.uasset         # Enemy health bar (extends USWorldUserWidget, shown on first hit)

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
- [x] Enemy AI — Behavior Tree + Blackboard + BT Service (`ASAICharacter`, `ASAIController`, `USBTService_ChackAttackRange`) — ranged minion, line-of-sight check
- [x] Enemy attack logic — custom C++ BT Task (`USBTTask_RangedAttack`) fires a projectile at the player when within range
- [x] Smarter movement — EQS query (`Query_FindNearbyLocation`) repositions the bot to a firing spot near the player
- [x] Sight-based targeting — `UPawnSensingComponent` (`OnSeePawn`) so the enemy must see the player before engaging
- [x] GameMode dynamic AI spawning — `ASGameModeBase` + EQS + `UCurveFloat` difficulty scaling
- [x] AI takes damage / dies — `USAttributeComponent` on `ASAICharacter`, ragdoll + dissolve on death
- [x] AI friendly fire prevention — `IsA(GetInstigator()->GetClass())` check in projectile overlap
- [x] Bot animation polish — smooth rotation (`Use Desired Rotation`), blend space weight = 8
- [x] AI flee / heal behavior — `USBTService_CheckHealth` + EQS hiding spot + `USBTTask_HealSelf` + 60 s cooldown
- [x] 3D world-space health bar (`USWorldUserWidget`) — enemy health bar projected to screen, DPI-corrected, auto-removes on actor death
- [x] Main HUD framework (`WBP_Main_HUD`) — container bundling PlayerHealth, Crosshair, Credits, and GameModeInfo widgets
- [x] Game time display (`WBP_GameModeInfo`) — elapsed time via `GameState->GetServerWorldTimeSeconds()`
- [x] Proper player spawn — GameMode `DefaultPawnClass` + `PlayerStart`, no manually placed pawn
- [x] Debug console commands — `HealSelf(float)` and `KillAll()` via `UFUNCTION(Exec)`; God mode via `CanBeDamaged`
- [x] Shooting accuracy — camera-position sphere trace with DotProduct fallback for steep upward angles
- [x] Custom Action System (`USActionComponent` + `USAction`) — hand-rolled GAS alternative; `DefaultActions` grants abilities at `BeginPlay`; `USAction_ProjectileAttack` consolidates the 3 attack abilities; Sprint action added
- [x] Refactored all attack/ability logic out of `ASCharacter` into standalone `USAction` subclasses — character class no longer owns projectile classes, anim montages, VFX refs, or attack timers
- [x] Centralized damage application (`USGameplayFunctionLibrary`) — `ApplyDamage` / `ApplyDirectionalDamage`; fixed knockback direction bug (trace vector instead of impact normal) for consistent physics impulses on barrels and AI ragdolls
- [ ] Migrate Pawn Sensing → AI Perception (deprecation warning)
- [ ] Enhanced Input System migration (from legacy `BindAxis` / `BindAction`)
- [ ] Networked multiplayer replication

---

## About / 关于

This project is part of an ongoing effort to build a solid C++ foundation in Unreal Engine 5 by reimplementing core systems from scratch.

本项目是持续学习 UE5 C++ 开发、从零重新实现核心系统的系列实践之一。

**Author / 作者:** Jacky Feng  
**Engine:** Unreal Engine 5.6  
**Started:** 2026

---

*Course support: Based on [Tom Looman](https://www.tomlooman.com/)'s Unreal Engine C++ Action Roguelike course, also taught at Stanford University as CS193U.*  
*课程支持：基于 Tom Looman 的 UE C++ Action Roguelike 课程，该课程亦以 CS193U 形式在斯坦福大学开设。*
