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

## Current feature set (as of v0.3)

- Third-person character with spring-arm camera, WASD move, mouse look, jump
- Primary attack: magic projectile, camera line-trace aim correction
- Blackhole ability (Q): Blueprint, RadialForce attraction, destroys simulating actors on overlap, 5s auto-destroy
- Dash/Teleport (R): C++ `ASDashProjectile`, teleports player to impact location with particle effects
- Crosshair HUD widget (UMG, `WBP_Crosshair`)
- Explosive barrel: physics-simulated, `URadialForceComponent`, `TakeDamage` → `Explode()`
- Interaction system (E): sphere sweep, `ISGameplayInterface` / `USGameplayInterface`
- Interactables: treasure chest (`ASItemChest`), lever (`BP_Lever`)
- Attribute system: `USAttributeComponent` with `Health`/`HealthMax`, `OnHealthChanged` 4-param multicast delegate
- Player health bar UI: `WBP_PlayerHealth`, event-driven (binds to `OnHealthChanged` in Construct)
- Test enemy: `BP_TestAttack`, timed attack toward player, SAttributeComponent, health delegate, `bAttackEnabled` toggle
- Dynamic materials (Lecture 8):
  - `M_HitFlashDemo` with `TimeToHit` scalar parameter, sine-wave self-fading emissive flash
  - `ASTargetDummy`: static mesh + AttributeComponent, drives hit flash via `SetScalarParameterValueOnMaterials("TimeToHit", ...)` in `OnHealthChanged`
  - `MF_HitFlashDemo`: reusable Material Function encapsulating hit flash node graph
  - `M_HealthBar`: UI material (Material Domain: User Interface) for UMG health bar
  - `M_DissoveEffect`: noise-texture dissolve material (asset ready, not yet wired to gameplay)
  - `M_PBRDemo`, `M_SineWave`: learning/utility materials

## Roadmap (next up)

- Enemy AI with `UAIPerceptionComponent` + Behavior Trees
- Additional interactables and pick-ups
- Enhanced Input System migration
