# Action Roguelike Game Demo — Unreal Engine 5

> A third-person action game built from scratch in C++ with Unreal Engine 5, demonstrating core gameplay systems including character locomotion, projectile combat, physics-driven destruction, and an interface-based interaction framework.

> 使用 C++ 与 Unreal Engine 5 从零构建的第三人称动作游戏 Demo，展示了角色运动、弹道战斗、物理破坏与基于接口的交互框架等核心游戏系统。

---

## Table of Contents / 目录

- [Overview](#overview)
- [Tech Stack](#tech-stack)
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

## Implemented Systems

### 1. Third-Person Character (`ASCharacter`)

- Spring-arm + camera rig with `bUsePawnControlRotation` for smooth mouse look
- World-space movement decoupled from camera yaw: `MoveForward` uses the controller's forward vector; `MoveRight` derives the right vector via `FRotationMatrix::GetScaledAxis(EAxis::Y)`, ensuring consistent strafe behavior at all camera angles
- `bOrientRotationToMovement = true` with `bUseControllerRotationYaw = false` — the character mesh faces the movement direction, not the camera
- Jump support via `ACharacter::Jump` / `StopJumping`
- Attack driven by `UAnimMontage` + `FTimerHandle`: the montage plays first, then the projectile spawns at the correct animation frame (0.2 s delay) from the `Muzzle_01` skeletal socket
- Delegates attack and interaction to dedicated components, keeping `ASCharacter` thin

### 2. Magic Projectile (`AAMagicProjectile`)

- `USphereComponent` with `"Projectile"` collision profile as the root
- `UProjectileMovementComponent`: initial speed 1000 cm/s, zero gravity, velocity-aligned rotation
- `UParticleSystemComponent` for in-flight VFX
- Dual hit response:
  - **Blocking hit** (`NotifyHit`): applies 20 point damage to the hit actor; visual effect and self-destruction are handled by Blueprint's `On Component Hit` — this path handles physics-simulated actors such as the explosive barrel, which use Block collision
  - **Overlap hit** (`OnActorOverlap`): applies 20 point damage via `UGameplayStatics::ApplyPointDamage`, then destroys self — for actors configured to Overlap with the projectile channel (e.g. enemies)

### 3. Explosive Barrel (`AExplosiveBarrel`)

- `UStaticMeshComponent` with `SetSimulatePhysics(true)` as root — fully physics-simulated rigid body
- `URadialForceComponent`: radius 750 cm, `bImpulseVelChange = true` (mass-independent velocity change), `bAutoActivate = false` (manual trigger only)
- `Explode()` (`public`, `BlueprintCallable`): fires the radial impulse and serves as the single entry point for explosion logic (particle effects, sounds can be added here later)
- `TakeDamage` override: calls `Explode()` when struck, ensuring a single FireImpulse regardless of how damage is delivered

### 4. Gameplay Interface & Interaction System

**`ISGameplayInterface` / `USGameplayInterface`**

UE5 `UINTERFACE`-based contract. Declares:
```cpp
UFUNCTION(BlueprintNativeEvent)
void Interact(APawn* InstigatorPawn);
```
Any actor can opt into the interaction system by implementing this interface — no inheritance coupling to the character or component.

**`USInteractionComponent`**

Attached to the character. On `PrimaryInteract()`:
1. Performs a **sphere sweep** (radius 30 cm) from the character's eye point along the aim direction (max 1000 cm) using `SweepMultiByObjectType` against `ECC_WorldDynamic`
2. Iterates hits; calls `ISGameplayInterface::Execute_Interact(HitActor, InstigatorPawn)` on the first actor that passes `Implements<USGameplayInterface>()`
3. Renders debug geometry (`DrawDebugSphere`, `DrawDebugLine`) in green on hit, red on miss — visible for 2 s

**`ASItemChest`**

Implements `ISGameplayInterface`. On interact: rotates the `LidMesh` to `TargetPitch` (default 110°) via `SetRelativeRotation`, opening the chest lid.

### 5. Input Bindings

Legacy axis/action input configured in `DefaultInput.ini`:

| Input | Binding | Action |
|---|---|---|
| W / S | `MoveForward` axis | Move forward / back |
| A / D | `MoveRight` axis | Strafe |
| Mouse X | `Turn` axis | Camera yaw |
| Mouse Y | `LookUp` axis | Camera pitch |
| Left Mouse Button | `PrimaryAttack` action | Fire projectile |
| Space | `Jump` action | Jump |
| E | `PrimaryInteract` action | Interact with world |

---

## Project Structure

```
Source/ActRouguelikeDemo/
├── Public/
│   ├── SCharacter.h            # Player character
│   ├── SInteractionComponent.h # Sphere-sweep interaction component
│   ├── SGameplayInterface.h    # UE5 interface for interactable actors
│   ├── SItemChest.h            # Interactable treasure chest
│   ├── AMagicProjectile.h      # Player-fired magic projectile
│   └── ExplosiveBarrel.h       # Physics barrel (reacts to damage)
└── Private/
    ├── SCharacter.cpp
    ├── SInteractionComponent.cpp
    ├── SGameplayInterface.cpp
    ├── SItemChest.cpp
    ├── AMagicProjectile.cpp
    └── ExplosiveBarrel.cpp
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

- [ ] Enemy AI with `UAIPerceptionComponent` and Behavior Trees
- [ ] Attribute system (health, mana) via `UAttributeComponent`
- [ ] UI HUD using `UMG` / `UUserWidget`
- [ ] Additional interactables and pick-up items
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
