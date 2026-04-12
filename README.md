# Hexcapade Alpha Release

## New Features
### Tiled Implementation
Rooms used in procedural generation are now created with Tiled. The algorithm was changed to utilize flood filling for hallways.
### Multistroke Gestures
The current gesture mapping is as follows:
- "Arrow" shape -> Fireball spell, deals damage to enemy
- Z like shape -> Freeze spell, freezes the enemy
- F-like shape -> Heal spell, used to heal your teammates
## Updated Features
1. The game now utilizes NetPhysics for networking and Box2D for collision handling.
2. Directional player sprites were added
3. Item pickup and deposit is now synched across all players.
