# Automatic Plasma Turret — Unreal Engine 5 C++ Showcase

An automatic plasma turret created in **Unreal Engine 5.8.2** as part of the **LABA IT — Unreal Engine — C++** course and my first C++ gameplay component.

This repository showcases the mechanic through a demonstration video, gameplay screenshots, and C++ source files.

## Demo Video

[▶ Watch the showcase on YouTube](https://youtu.be/rqlqKLW4R_Q)

The showcase presents player tracking, automatic plasma fire, obstacle-based visibility checks, projectile trails, muzzle flashes, and firing audio.

## Features

- Player detection within a configurable range.
- Smooth horizontal head rotation with configurable speed.
- Line-of-sight checks from the muzzle using the Visibility collision channel.
- Tracking and firing stop when the player hides behind an obstacle or leaves detection range.
- Automatic reacquisition when the player becomes visible within range again.
- Timer-based automatic firing with configurable shots per second.
- Range and visibility checked again immediately before each shot.
- Configurable projectile class.
- Plasma projectiles with glowing cores and Niagara trails.
- Niagara muzzle flashes and firing sounds triggered after successful projectile creation.
- Projectile destruction on blocking impact and a five-second maximum lifetime.
- Optional debug trace visualization controlled by a preprocessor macro.

## Scene Preview

![Turret and player in the arena with cover obstacles](automatic-plasma-turret-Unreal%20C%2B%2B/screenshot-1.png)

![First-person view of the turret in the test arena](automatic-plasma-turret-Unreal%20C%2B%2B/screenshot-2.png)

![Plasma turret firing towards the player](automatic-plasma-turret-Unreal%20C%2B%2B/screenshot-3.png)

## C++ Implementation

### TurretComponent — Detection, Tracking, and Firing

`UTurretComponent` finds the head and muzzle through component tags when gameplay begins. Each frame, it checks the player's distance and performs a visibility trace. When the player is visible within range, the component rotates the head in yaw using `RInterpConstantTo` and starts the firing timer.

`StartFiring()` keeps a single repeating timer active. The interval is calculated as `1 / ShotsPerSecond`. `StopFiring()` clears the timer when the player is unavailable, hidden, or out of range.

Before creating each projectile, `Fire()` checks the required references, current range, and line of sight. It spawns the selected projectile class at the muzzle's world position and rotation, assigns the turret as its owner, and triggers the selected muzzle flash and sound after a successful spawn.

[View TurretComponent.h](automatic-plasma-turret-Unreal%20C%2B%2B/TurretComponent.h) · [View TurretComponent.cpp](automatic-plasma-turret-Unreal%20C%2B%2B/TurretComponent.cpp)

### TurretProjectile — Collision, Movement, and Impact

`ATurretProjectile` creates a sphere collision component, a visual static mesh, and a `UProjectileMovementComponent` in its constructor. The movement component moves the collision sphere along the projectile's local +X axis without gravity.

The visual mesh follows the sphere and has its own collision disabled. `OnProjectileStop` calls a function that destroys the projectile after a blocking impact. A five-second lifespan also removes projectiles that do not hit anything.

Blueprint settings supply the final appearance, Niagara trail, speed overrides, and collision responses used in the showcase.

[View TurretProjectile.h](automatic-plasma-turret-Unreal%20C%2B%2B/TurretProjectile.h) · [View TurretProjectile.cpp](automatic-plasma-turret-Unreal%20C%2B%2B/TurretProjectile.cpp)

### Blueprints and Niagara

C++ handles the turret gameplay logic. Blueprints assemble the turret meshes and assign projectile, audio, and effect assets. Niagara provides the plasma trail and muzzle flash.

## Aiming Laser Experiment

An optional Niagara aiming laser was also implemented. C++ updates its endpoints from the muzzle's forward direction and shortens the beam at the first surface blocking Visibility.

The laser was disabled in the final showcase by removing the `TurretLaser` component tag. A continuous beam represents the current aiming direction, while previously fired plasma projectiles continue along their own trajectories. During turret rotation, the effects visually diverged, so the final presentation uses plasma projectiles and muzzle flashes instead.

The laser code remains included for reference.

## Integration Notes

These files are shared as learning and showcase material, rather than a standalone Unreal Engine project.

- The source uses the `ARENASHOOTER_API` module macro; adapting it to another project requires the corresponding module macro.
- The owning module must include **Niagara** in its build dependencies.
- The rotating head requires the component tag `TurretHead`, and the muzzle requires `TurretMuzzle`.
- The muzzle's local **+X axis** defines the launch direction.
- The player capsule and cover obstacles must block **Visibility** for the current detection logic.
- Blueprint collision settings are needed to make projectiles ignore other projectiles and visibility traces while blocking intended targets.
- Materials, Niagara systems, Blueprints, audio, and other Unreal assets must be configured separately; they are not included in this source-only showcase.

Debug trace drawing can be enabled with `TURRET_DEBUG_TRACES` set to `1` in `TurretComponent.cpp`, or disabled with `0`. This controls visualization only, not the visibility checks.

## Asset Credits

Turret model: **Modular Drone Set (Mix N Match)** by **ArtWorld**, available on [Fab](https://www.fab.com/listings/95951784-32ed-4536-8584-c499747f9130).

Third-party assets and audio remain subject to their respective licenses.

## Repository Contents

- A link to the published showcase video and a local MP4 recording.
- Three gameplay screenshots.
- `TurretComponent.h` and `TurretComponent.cpp`.
- `TurretProjectile.h` and `TurretProjectile.cpp`.

No `.uproject` or `.uasset` files are included, so this repository cannot be opened directly as an Unreal Engine project. Other ArenaShooter mechanics visible in the scene are outside the scope of this showcase.
