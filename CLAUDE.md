# CLAUDE.md — Project Context for Claude

## Who I am

- C++ beginner, learning Unreal Engine 5 through hands-on practice
- Following Tom Looman's UE5 C++ Action Roguelike course (also taught as Stanford CS193U)
- Goal: understand UE5 architecture deeply (Actor/Component model, UObject reflection, collision, input) — not just "make it work"

## Project

Action Roguelike game demo built in UE5 C++. Third-person character with projectile combat, special abilities, physics destruction, and an interface-based interaction system.

- **Engine:** Unreal Engine 5.6
- **Language:** C++17 + Blueprints (Blueprint only used to extend C++ classes with VFX/particles, not for core logic)
- **Repo:** https://github.com/MournfulOx/ActionRoguelikeGameDemo_UE5
- **Platform:** Windows 11, Visual Studio 2022 / Rider

## Preferences & habits

**Communication**
- Talk to me in Chinese (I write in Chinese, respond in Chinese)
- Keep explanations beginner-friendly but technically accurate — I'm learning C++, so explain UE5-specific patterns when they appear
- Short and direct, no unnecessary padding

**Git workflow**
- Commit all related changes together with a descriptive message
- Always push after committing
- **Every push that adds or modifies features must also update:**
  - `README.md` — reflect the new/changed systems in Implemented Systems, Controls, Project Structure, and Roadmap
  - `CLAUDE.md` — update the "Current feature set" and "Roadmap" sections to stay in sync with actual project state

**GitHub Release format (参考 v0.1 风格，根据内容灵活调整)**
- One `###` section per major feature, each with a GIF embedded directly in the notes body via `<img>` tag (NOT as an Assets attachment)
- GIFs are uploaded by dragging into the GitHub web editor → auto-generates CDN `<img>` link
- If GIF not yet recorded, use `<!-- 录好后拖入 GitHub 编辑器替换此注释 -->` as placeholder
- Fixed footer: Tech Stack line → course credit line → `**Full Changelog**` link
- `gh` CLI: `& "C:\Program Files\GitHub CLI\gh.exe"` (not in PATH)

Release notes template:
```
## vX.X — Title

### Feature A
<!-- placeholder or <img> -->

### Feature B
<!-- placeholder or <img> -->

---

### What's in this build
- **System** — description

### Tech Stack
Unreal Engine 5.6 · C++ · Blueprints · Git LFS

---

*Based on Tom Looman's UE5 C++ Action Roguelike course, also taught at Stanford as CS193U.*

---

**Full Changelog**: https://github.com/MournfulOx/ActionRoguelikeGameDemo_UE5/commits/vX.X
```

**Code**
- Prefer C++ for core logic; Blueprint only for VFX/particle/animation extensions on top of C++ classes
- No comments unless the WHY is non-obvious
- Don't add abstractions beyond what the current task needs

**Tools**
- `gh` CLI is installed at `C:\Program Files\GitHub CLI\gh.exe` (not in PATH — call with full path or `& "C:\Program Files\GitHub CLI\gh.exe"`)
- Git LFS is active for `.uasset` / `.umap` files

## Current feature set (as of v0.4)

- Third-person character with spring-arm camera, WASD move, mouse look, jump
- Primary attack: magic projectile, camera line-trace aim correction (self-ignore fix)
- Blackhole ability (Q): Blueprint, RadialForce attraction, destroys simulating actors on overlap, 5s auto-destroy
- Dash/Teleport (R): C++ `ASDashProjectile`, teleports player to impact location with particle effects
- Crosshair HUD widget (UMG, `WBP_Crosshair`)
- Explosive barrel: physics-simulated, `URadialForceComponent`, `TakeDamage` → `Explode()`
- Interaction system (E): sphere sweep, `ISGameplayInterface` / `USGameplayInterface`
- Interactables: treasure chest (`ASItemChest`), lever (`BP_Lever`)
- Attribute system: `USAttributeComponent` with `Health`/`HealthMax`, clamp, dead-guard, `OnHealthChanged` 4-param multicast delegate, `IsFullHealth()`, `GetHealth()`, `GetHealthMax()`
- Player health bar UI: `WBP_PlayerHealth`, event-driven, `HealthMax`-aware init on Construct
- Damage popup widget: `WBP_DamagePopup`, Expose on Spawn `DamageAmount`, world-space follow + random screen offset, UMG scale+fade animation
- Test enemy: `BP_TestAttack`, timed attack, hit flash, damage popup, stops attacking on death
- Hit flash: `MF_HitFlashDemo` applied to player (`M_CharacterSimple`), target dummy, and test enemy via `SetScalarParameterValueOnMaterials("TimeToHit", ...)`
- Projectile audio: `UAudioComponent` (looping flight), `ImpactSound` (`PlaySoundAtLocation`) on `SProjectileBase`
- Casting particle: `SpawnEmitterAttached` at `Muzzle_01` on projectile spawn
- Camera shake: `PlayWorldCameraShake` on `Explode_Implementation` in `SProjectileBase`
- Health potion powerup: `ASPowerupActor` base (interact interface, 10s respawn timer) + `ASHealthPotion` child (heals pawn, ignores full health), `BP_HealthPotion` with `SM_PotionBottle`
- Dynamic materials: `M_HitFlashDemo`, `MF_HitFlashDemo`, `M_HealthBar`, `M_DissoveEffect` (wired via `UMaterialInstanceDynamic`, `DissolveAmount` 1.0→-1.0), `M_PBRDemo`, `M_SineWave`
- Enemy AI core: `ASAICharacter` + `ASAIController` + Behavior Tree / Blackboard; `NavMeshBoundsVolume` in level; `AIModule` + `GameplayTasks` added to Build.cs
- AI targeting via sight: `UPawnSensingComponent`; `OnSeePawn` → writes `TargetActor` to Blackboard (Note: deprecated in favor of AI Perception — warning only)
- BT Service `USBTService_ChackAttackRange`: distance + `LineOfSightTo` → `WithinAttackRange` bool
- Custom C++ BT Task `USBTTask_RangedAttack`: spawns projectile from `Muzzle_01` toward `TargetActor`; `Params.Instigator = MyPawn` prevents friendly fire
- EQS `Query_FindNearbyLocation`: Donut + Distance + Trace → smart repositioning near player; `QueryContext_TargetActor` for player reference
- `ASAIController::BeginPlay` guarded with `ensureMsgf` (C++ assert demo)
- **GameMode AI spawning** (`ASGameModeBase`): repeating timer → EQS query (`Query_FindBotSpawn`) → bot count check via `TActorIterator` → `SpawnActor`; `UCurveFloat` scales max bot count over time
- **AI death**: `USAttributeComponent` on `ASAICharacter`; on death → `StopLogic`, ragdoll, capsule disabled, dissolve via `UMaterialInstanceDynamic` (`M_DissoveEffect`), `SetLifeSpan`
- **AI friendly fire fix**: `OtherActor->IsA(GetInstigator()->GetClass())` check in `AMagicProjectile::OnActorOverlap`
- **Bot animation polish**: `Use Desired Rotation = true` on `BP_MinionRanged`; `Jog_Combat_BS` target weight interpolation = 8
- **AI flee/heal (Assignment 4)**: `USBTService_CheckHealth` writes `LowHealth` bool; `Query_FindHidingSpot` EQS (Donut + Trace hidden from player + Distance); `USBTTask_HealSelf` restores full health; Cooldown decorator (60s) on flee Sequence

## Roadmap (next up)

- Migrate `UPawnSensingComponent` → AI Perception (deprecation warning)
- Enhanced Input System migration
- Additional interactables and pick-ups
