# kgDoom
###### A doom port. With Lua.

#### Disclaimer
Point of this doom port is not to stay compatible or emulate original game accurately. Lua scripts in kgdoom.wad for original games do not reflect 100% original game behavior and are provided only for basic compatibility.

#### Install
To install on Switch, you currently have to use HBL.
Copy kgdoom.nro and kgdoom.wad to /switch/kgdoom/ on your SD card. Also copy any IWADs you have to this directory.
If you want to run PWAD mods, create directory /switch/kgdoom/pwads/ and copy these there.
When you start kgdoom, you will see WAD menu where you can pick IWAD, and optionally PWAD.

#### Map editing
This manual documents kgDoom Lua game API. I assume you already know how to make doom maps / mods and know Lua scripting.
Hexen map formap is recommended. Doom map format is present only for compatibility reasons.

### Terms
These are terms used in Lua API.

- mobj
  - Map object. Anything sprite-based in map. Items, monsters, players ...
  - Default properties and animations are defined by mobjtype.
- mobjtype
  - This is a template for mobj. When spawning mobjs, you refer to this type.
  - This defines all animations and default properties.
- player
  - This is a special mobj section that is only present for players.
  - Players have few extra properties that other mobjs do not have.
- sector
  - This allows access to map sectors.
- line
  - This allows access to map lines and sides.
- genericPlane
  - This is a generic plane (floor / ceiling) movement handler.
  - This is bound to a sector.
- genericCaller
  - This is a generic caller. It calls specified Lua callback periodically.
  - This is bound to a sector.
  - Used for example for light effects.
- textureScroll
  - This is used to add texture scroll effect to lines.
- material
  - Every texture can have specific material ID for extra effects.
  - Material is identified by integer from 0 to 255. 0 is default material.
  - Material ID assigned in TEXTUREx lump. Byte (uint8_t) at offset 8 (right after name) in texture definition.
    - ZDoom uses this offset as a uint16_t flags, but only MSB (bit 15) is defined.
  - Material ID is passed to `puff` mobj as `tag` when line or plane is hit.
  - Only textures from TEXTUREx can have materials assigned. Flat textures are always material 0.
    - Planned feature will solve this.

### Lua scripts
Lua scirpts are stored in WADs. There is only one recognized lump name: GAMELUA. You can have multiple GAMELUA lumps in multiple wads.
Lua scripts are loaded from first wad to last and from first lump to last one. This should allow for overrides (game mods).
First line in each GAMELUA can specify internal script name. Example: `---test.lua`. Every time Lua has to report error, it will use `test.lua` as a script name.
##### Stages
There are two stages. Loading stage and map stage. Every stage has its own set of global functions.
###### Loading stage
This is stage when all GAMELUA scripts are loaded and executed. At this point you should create all mobjtypes, register player class, weapons and keys. You can not do these later.
Content of GAMELUA ouside of any functions is executed at this stage.
###### Map stage
This is stage when level is already loaded and everything in map is spawned.
Only callbacks and action functions are called.

###### Beware
The is a weird bug(?) in Lua itself. If you make a syntax error, you will get `attempt to call a string value` error, which is not helpful at all.
The only Lua libraries available are `table`, `math` and `string`.
To make save games possible, do not store anything related to gamestate in global variables. Use provided fields in available API types.

### API
#### inventory & armor
- Every mobj can have its own inventory and armor.
- Inventory item can be any mobjtype that has maxcount other than zero.
  - When count reaches zero, item is removed from inventory.
  - Negative maxcount is undepletable and will be kept even if count reaches zero. Used for ammo, sice HUD expects ammo to exist even when depleted.
#### damage scale
- New damage resistence / weakness system was added.
  - There are up to 32 damage types.
  - Any mobj has its own damage scale for each mobj.
  - Before damage is subtracted from healt and armor, it is multiplied by corresponding scaler.
  - This does not affect damage thrust.
  - Damage can be scaled to 0% for no damage at all. Damage thrust is still unaffected.
  - Damage can be scaled to negative values for healing. Healing will not exceed maximum health defined in mobjtype for this mobj.
  - Since healing is not damage, here is no damage thrust anymore.
  - Damage types are indexed from 1, as Lua tables are.
  - Index values outside limits (0 or more than 32) are pure damage with no scaling.
#### values
- angles
  - Angles are specified in range 0 - 8191. This is a whole circle.
  - Angles can use decimal point.
- fixed
  - Any fixed point values can use decimal point in Lua.
#### globals
###### Loading stage
- function `createMobjType(table)`
  - Table should contain everything required to describe mobj type.
  - See Lua scripts in kgdoom.wad file, available in plaintext in `wadsrc/lua`.
  - See mobjtype section for list of properties.
  - Returns newly created mobjtype.
- function `setPlayerType(mobjtype)`
  - This function regiters primary player type.
  - There is only one player type requred, multiple calls will only redefine default player type.
- function `addWeaponType(mobjtype [, mobjtype [, mobjtype])`
  - This function expects weapon type as mobjtype and optionally one or two ammo types as mobjtype.
  - Every weapon that should be listed in weapon menu must be registered using this weapon.
  - If you want to have secret / scriptable weapons, you do not have to register them.
- function `addKeyType(mobjtype)`
  - This fucntion registers key as mobjtype. This registration is only for display in HUD.
###### Map stage
- function `doomRandom()`
  - This will return a prandom number from 0 to 255. It uses original doom random table.
- function `doomRandom(max)`
  - This will return a prandom number from 0 to max, including 0 and max. Only whole numbers are supported.
- function `doomRandom(min, max)`
  - This will return a prandom number from min to max, including min and max. Only whole numbers are supported.
- function `spawnMobj(mobjtype, x, y [, z [, angle [, action]]])`
  - Spawn new mobj at x and y and optionally z. Uses fixed point numbers.
  - Spawn angle can be specified.
  - If action is `true`, first frame action is executed. Enabled by default.
  - Returns spawned mobj.
- function `blockThingsIterator(test_mobj, func [, arg])`
  - Calls a callback `func` with optional argument `arg` for every mobj found inside radius of `test_mobj`.
  - Argument `arg` is user specified, and can be anything. It will be passed to callback function.
  - Callback should be function defined as `somecb(thing)` or `somecb(thing, arg)`.
  - If callback returns `false`, iteration will stop and any other return values will get returned by `blockThingsIterator` itself.
  - Beware: no z height checks are computed, you have to do this yourself it you need it.
- function `blockThingsIterator(x, y, func [, arg])`
  - Same as above, but with single point specified by fixed point x and y.
- function `blockThingsIterator(x, y, xe, ye, func [, arg])`
  - Same as above, but with manualy specified range. Also uses fixed points.
- function `globalPlayersIterator(func [, arg])`
  - Calls a callback `func` with optional argument `arg` for every player in level.
  - Callback should be function defined as `somecb(player)` or `somecb(player, arg)`.
  - Return logic is same as in `blockThingsIterator`.
- function `globalThingsIterator(func [, arg])`
  - Calls a callback `func` with optional argument `arg` for every mobj in level.
  - Callback should be function defined as `somecb(thing)` or `somecb(thing, arg)`.
  - Return logic is same as in `blockThingsIterator`.
- function `thingTagIterator(func [, arg])`
  - same as `globalThingsIterator` but with automatic TID filter.
- function `globalSectorsIterator(func [, arg])`
  - Calls a callback `func` with optional argument `arg` for every sector in level.
  - Callback should be function defined as `somecb(sector)` or `somecb(sector, arg)`.
  - Return logic is same as in `blockThingsIterator`.
- function `sectorTagIterator(tag, func [, arg])`
  - Same as `globalSectorsIterator` but with automatic sector tag filter.
- function `globalLinesIterator(func [, arg])`
  - Calls a callback `func` with optional argument `arg` for every line in level.
  - Callback should be function defined as `somecb(line)` or `somecb(line, arg)`.
  - Return logic is same as in `blockThingsIterator`.
- function `fakeContrast(enable)`
  - Set `false` to enable fake contrast in level.
  - Doom uses fake contrast on walls, so it is enabled by default.
- function `setBackground(patchlump)`
  - This function will set screen background to this gfx from any WAD.
  - Use of this function is not recommendes as specified image will only show trough holes in map (= bad map).
  - It is only used for Doom2 finale emulation.

#### mobj states
There are few animations that every mobjtype can have. Only spawn state has meaning for all of them, as it is state it gets set to on level load.
Other animations depend on flag combinations. Unused states can be used in any way you wish.
Animations are always specified as `_animName`.
Animations are creaded from one or more frames. Every frame can optionaly call any function with this `mobj` as argument.
- `_spawn`
  - Basic animation for all mobjs.
- `_see`
  - Set by internal `a.Look` when mobj has target to go after.
- `_pain`
  - Used on `__shootable` mobjs. This state is set when mobj is damaged, but with chance specified by `painChance`.
- `_melee`
  - Set by internal `a.Chase` when mobj is close enough to do melee damage to its target.
- `_missile`
  - Randomly set by internal `a.Chase`. Used to attack from distance.
- `_death`
  - Used on `__shootable` mobjs. This state is set when mobj is damaged and resulting health is less or equal to 0.
- `_xdeath`
  - Used on `__shootable` mobjs. This state is set when mobj is damaged and resulting health is less than negative default health (overkill).
- `_raise`
  - Not used by engine logic.
  - Used in Doom Lua scripts by Archvile to raise dead bodies.
- `_crush`
  - Used when there is not enough space in changing sector for this mobj and `__dropped` or `__corpse` is set.
- `_heal`
  - Not used by engine logic.
  - Used in Doom Lua scripts by Archvile itself to heal dead bodies.
- `_wRaise`
  - Weapon state. Used when raising selected weapon.
- `_wReady`
  - Weapon state. Used when holding weapon, waiting for fire.
- `_wLower`
  - Weapon state. Used when lowering weapon.
- `_wFireMain`
  - Weapon state. Used when firing using main attack.
- `_wFireAlt`
  - Weapon state. Used when firing using alternate attack.
- `_wFlashMain`
  - Weapon state. Added on top of normal animation if internal `a.WeaponFlash` is called.
- `_wFlashAlt`
  - Weapon state. Added on top of normal animation if internal `a.WeaponFlash` is called.

#### mobj flags
Mobj flags are boolean values that affect mobj behavior. Flags are always specified as `__flagName`.
- `__special`
  - Used for item pickup. Any mobj with `__pickup` enabled colliding with this one will call callback `action`.
  - Callback should be function defined as `somecb(mobj, spec)` or `somecb(mobj, spec, arg)`.
    - mobj is colliding thing (one with `__pickup` flag) and `spec` is this thing.
- `__solid`
  - This mobj is considered solid and can be collided agains.
- `__shootable`
  - This mobj can be damaged and is expected to have `health` and `painChance` set.
- `__noSector`
  - This mobj is not in sector links. This affects rendering. If enabled, this mobj will be invisible.
- `__noBlockmap`
  - This mobj is non in block links. This affects physics. Nothing can collide with this mobj. Usefull for GFX effects.
  - Projectiles use this. In Doom, nothing coolides with projectiles, but projectiles collide with other mobjs.
- `__ambush`
  - Used by internal `a.Look`.
  - Monsters with this flag won't react to `a.NoiseAlert` but wait for direct line of sight (even from behind) to wake up.
- `__spawnCeiling`
  - Used by some Doom decorations. Items with this flag will be spanwed on celing, but only at map loading.
- `__noGravity`
  - This mobj won't fall down due to gravity.
- `__dropOff`
  - This mobj can fall of tall edges.
- `__pickup`
  - This mobj can 'pickup' items. See `__special`.
- `__noClip`
  - This mobj has collision checks disabled.
- `__slide`
  - This mobj will slide along walls / other mobjs.
- `__float`
  - Used for internal `a.Chase` as a hit that this monster is flying.
- `__missile`
  - Used for projectiles. This mobj has special collision check logic. Any collision would cause its death. And eventual damage to collided mobj.
- `__dropped`
  - Used as a marker for dropped items. Engine either deletes this mobj, or sets state to `_crush` when there is not enough space in changing sector for this mobj.
  - Doom logic (Lua scripts) also use this flag to give half ammo from items dropped by monsters.
- `__noblood`
  - Bullet puffs on this mobj are spawned as is instead of having state set to `_pain`.
- `__corpse`
  - This flag is automatically set on things with `__shootable` flag set when killed (health <= 0).
  - Engine uses this flag set `_crush` state in changing sector when there is no room for this mobj.
- `__countKill`
  - This mobj is counted as killable for level end screen.
- `__countItem`
  - This mobj is counted as pickable for level end screen.
- `__notInDeathmatch`
  - This mobj will not get spawned in deathmatch. Only affects map loading.
- `__isMonster`
  - This mobj is marked as monster. This is purely informative, to be used for any checks you need.
- `__noRadiusDamage`
  - This mobj is immune to `RadiusDamage`.
- `__noRadiusDmgZ`
  - This mobj can be damaged by `RadiusDamage` at any Z distance. Only X and Y distance is checked.
- `__skullFly`
  - This mobj will damage anything it will collide with. Then it will be stopped, and its state will be set to `_spawn`.
  - Used on Doom lost souls.
- `__noZChange`
  - This mobj has 3D thing collision disabled.
  - Internaly set when `__missile` explodes.
- `__noZSpawnCheck`
  - This mobj won't have its floorz and ceilingz checked when spanwed.
  - This means that thing can spawn inside steps and wont get moved up.
  - This is how all things spawn when loading new map.
- `__troughMobj`
  - This mobj can't collide with other mobjs, and other mobjs can't collide with it.
  - Only level geometry collision is enabled.
- `__fullVolume`
  - This mobj has is's body sounds at full volume.
  - Used on bosses in Doom.
- `__noDeathPull`
  - There is a chance that death body will fall towards you instead of getting pushed by damage. This flag will disable it.
- `__invulnerable`
  - This mobj can't be hurt.
  - Used by invunlerability sphere in doom.
- `__wallBounce`
  - Used on projectiles. Projectile will bounce off walls.
- `__mobjBounce`
  - Used on projectiles. Projectile will bounce off other mobjs and do no damage to them.
- `__Monster`
  - Flag combination of `__isMonster`, `__countKill`, `__solid` and `__shootable`
- `__Projectile`
  - Flag combination of `__missile`, `__noBlockmap`, `__noGravity`, `__dropOff` and `__noZChange`

#### mobjtype
Mobjtype can contain these parameters. Default 0 / none, unless specified otherwise.
- `ednum`
  - Doom editor number. This is ID for map editor.
  - Integer.
- `health`
  - Default health. Used on `__shootable` items.
  - Integer.
- `reactionTime`
  - Default monster reacion time. See mobj.
  - Integer.
- `painChance`
  - Used on `__shootable` items. Chance to set `_pain` animation, where 0 is 0% and 256 is 100%
  - Integer.
- `speed`
  - Speed for projectile spawning, or internal `a.Chase` movement code.
  - Fixed point.
- `radius`
  - Mobj size - radius.
  - Fixed point.
- `height`
  - Mobj size - height.
  - Fixed point.
- `mass`
  - Mobj mass. Used on `__shootable` items when taking damage.
  - Can be used for any push strength calculation.
  - Integer.
- `damage`
  - Used on projectiles, or when `__skullFly` is set.
  - Negative damage specifies usage of original Doom random damage amount generator.
  - Integer.
- `gravity`
  - Gravity multiplier. Default is 1.
  - Fixed point.
- `bounce`
  - Bounce multiplier. If greater than 0, mobj can bounce of celings / floors.
  - Eeach bounce momentnum is inverted and multiplied by this value.
  - Fixed point.
- `viewz`
  - Rendering height offset when viewing trough this mobj.
  - Fixed point.
- `shootz`
  - Shooting height offset. Used for `SpawnMissile` and `LineAttack`
  - Fixed point.
- `bobz`
  - Only usefull for player classes. Amount of screen bouncing while walking.
  - Fixed point.
- `stepheight`
  - Maximal height difference this mobj can step up.
  - Fixed point, default 24.
- `species`
  - If other than 0, this mobj is added into specified species cathegory.
  - Same species can't hurt each other with **projectiles**. (Example: Imps won't hurt other imps, baron won't hurt knight.)
  - Integer.
- `block`
  - Blocking flags for this mobj. Only used when `__solid` is set.
  - This is a bit mask of blocking flags. This allows for custom blocking configuration.
  - Integer, 16 bit only. Default 0xFFFF (block everything).
- `pass`
  - See blocking. If specified bit is set, this mobj won't be blocked by other mobjs, lines or 3D floors with same `block` bit set as it normally would.
  - Integer, 16 bit only.
- `icon`
  - Icon to be used for this mobj. Usage depends on mobj type, or can be used for custom Lua scripts.
  - `addWeaponType` expects this to be set to valid icon. If not found or invalid, this weapon won't be listed in selection menu.
  - `addKeyType` expects this to be set to valid icon. If not found or invalid, this key won't be listed in HUD even when picked up.
  - String. Patch type lump in any of loaded WADs.
- `maxcount`
  - Max count if used as inventory item.
  - Integer.
- `translation`
  - Custom translation table. If specified, all colors will be tranlsated trough this table when rendering sprites of this mobj. Used on colored blood.
  - Translation table is 256 bytes long. Lump itself can be bigger and offset can be specified. Example: `BLOODMAP:1`. `BLOODMAP` is same as `BLOODMAP:0`.
  - String.
- `render`
  - Render style. For now there are only few supported internal styles.
  - `!NORMAL` no effects.
  - `!SHADOW` original doom invisibility fuzz effect.
  - `!HOLEY0` every pixel is skipped.
  - `!HOLEY1` every pixel is skipped, alternative version.
  - String.
- `action`
  - For now only used with `__special` flag.
  - Function or nil.
- `arg`
  - Argument for `action`.
  - Any Lua type.
- `action_crash`
  - Function to call when mobj hits floor (falls down). Callback should be defined as `somecb(thing)`. Used on player for fall down sound effect.
  - Function or nil.
- `seeSound`
  - Played by internal `a.Look` when target is found.
  - String. Sound lump from any WAD.
- `attackSound`
  - Played by internal `a.Chase` when melee attacking.
  - String. Sound lump from any WAD.
- `painSound`
  - Not used by engine. Supposed to be used in `_pain` animation.
  - String. Sound lump from any WAD.
- `activeSound`
  - Randomly player by internal `a.Chase`.
  - Played by player when pressing `use` key without anything to activate.
  - String. Sound lump from any WAD.
- `deathSound`
  - Not used by engine. Supposed to be used in `_death` animation.
  - String. Sound lump from any WAD.
- `xdeathSound`
  - Not used by engine. Supposed to be used in `_xdeath` animation.
  - String. Sound lump from any WAD.
- `bounceSound`
  - Played when thing is about to bounce. Used for `bounce`, `__wallBounce` and `__mobjBounce`.
  - String. Sound lump from any WAD.
- `damageScale`
  - Table of damage scalers.
  - Table of Fixed point numbers. Range from -2.7 to 9.0 with 0.05 steps. Or `true` for instant kill. Default is 1.0 for 100%.
- `damagetype`
  - Type of damage this mobj causes.
  - Integer.

#### mobj
Mobjs have all parameters copied from mobjtype when spawned. Most parameters can be modified on the fly.
- `angle`
  - Angle this mobj is facing.
  - Angle. (0 - 8191)
- `health`
  - Integer.
- `damage`
  - Integer.
- `damagetype`
  - Integer.
- `armor`
  - Armor points.
  - Integer.
- `armortype`
  - This specifies type of armor. Any mobjtype can be armor. HUD graphics is selected by `icon` field in mobjtype. Use `damage` field to specify armor protection. 100 = full protection. Doom uses 33 (green) and 50 (blue).
  - As you can see, armor is property of mobj and not player. This means that any monster can have depletable armor now.
  - Mobjtype or nil.
- `pitch`
  - This is a vertical angle (looking up / down).
  - Fixed point.
- `floorz`
  - Current sector floor height of this mobj. Does account for 3D floors, but does not account for standing on other mobjs.
  - Fixed point.
- `ceilingz`
  - Current sector ceiling height of this mobj. Does account for 3D floors, but does not account for standing under other mobjs.
  - Fixed point.
- `radius`
  - See mobjtype.
  - Fixed point.
- `height`
  - See mobjtype.
  - Fixed point.
- `momx` `momy` `momz`
  - Mobj momentnum. Movement vector in 3D space.
  - Fixed point.
- `gravity`
  - Current gravity multiplier for this mobj.
  - Fixed point.
- `bounce`
  - Current bounce multiplier for this mobj.
  - Fixed point.
- `movedir`
  - Only used for internal `a.Chase` movement logic.
  - Integer.
- `movecount`
  - Only used for internal `a.Chase` movement logic.
  - Integer.
- `target`
  - Current target of this mobj.
  - BEWARE, unlike original Doom behavior, this really specifies target. Even for projectiles.
  - Mobj or nil.
- `source`
  - Origin of this mobj. Internaly used only for projectiles.
  - BEWARE, unlike original Doom behavior, this specifies origin of projectile.
  - Mobj or nil.
- `attacker`
  - Last attacker that caused damage. Best used in `_pain`, `_death` or `_xdeath` animation.
  - Can be nil. Damage from sector causes nil attacker.
  - Mobj or nil.
- `mobj`
  - Free mobj slot for Lua use.
  - Mobj or nil.
- `reactiontime`
  - If used with internal `a.Chase`, `reactiontime` is lowered every call. Monsters with non-zero `reactiontime` will not attack.
  - If used on players, non-zero value freezes any player movement. Only shooting is allowed. Unless negative, `reactiontime` is lowered every tic.
  - Integer.
- `threshold`
  - If used with internal `a.Chase`, `threshold` is lowered every call. Monsters with non-zero `threshold` won't change its target even if attacted by something else.
  - Integer.
- `tics`
  - Tics for current animation frame. Lowered ever tic. When zero, next frame is set.
  - Integer.
- `tag`
  - Mobj TID specified in map editor, if hexen mode is used.
  - Integer.
- `speed`
  - See mobjtype.
  - Fixed point.
- `mass`
  - See mobjtype.
  - Integer.
- `render`
  - See mobjtype.
  - String.
- `block`
  - See mobjtype.
  - Integer, 16 bit only.
- `pass`
  - See mobjtype.
  - Integer, 16 bit only.
- `attacktype`
  - Last damage type dealed to this thing. Best used in `_pain`, `_death` or `_xdeath` animation.
  - Integer. See `damage types`.
- `x` `y` `z`
  - Current mobj position.
  - Fixed point. Read only.
- `player`
  - If mobj is player, this points to player type. See player type.
  - Player. Read only.
- `sector`
  - Current sector this mobj has its center in.
  - Sector. Read only.
- `info`
  - Mobjtype of this mobj.
  - Mobjtype. Read only.
- `translation`
  - See mobjtype.
  - String.
- `colormap`
  - See `translation` in mobjtype.
  - String.
Functions called from mobj. All functions are read only and can't be redefined.
- `Remove()`
  - Remove this mobj from game.
- `Face(mobj)`
  - Set `angle` and `pitch` to face specified mobj.
- `Face(x, y [, z])`
  - Set `angle` and possibly `pitch` to face specified location.
  - `x`, `y` and `z` are fixed point values.
- `Teleport(x, y, z)`
  - This is only way to directly modify mobjs `x`, `y` and `z`. This function only moves mobj to its new location, no teleport effects are applied.
  - `x`, `y` and `z` are fixed point values.
  - Alternatively `z` can be boolean. Use `true` for ceiling and `false` for floor.
- `CheckPosition([x, y, z])`
  - Check mobj position either at specified location, or at current location if no parameters are provided.
  - `x`, `y` and `z` are fixed point values.
  - Returns `true` if check passed, `false` otherwise.
- `SpawnMissile(mobjtype [, angle [, pitch [, z [, x [, y]]]]])`
  - Spawn projectile with this mobj as originator.
  - `angle` and `pitch` are absolute values. See `AttackAim`.
  - `z` is offset relative to `shootz` and mobj current `z`.
  - `x` and `y` are offsets relative to mobj current location with `angle` rotation.
  - `x`, `y` and `z` are fixed point values.
  - Returns newly created mobj projectile.
- `AttackAim([type [, target]])`
  - Calculate `angle` and `pitch` this mobj has to use in order to aim at target.
  - `target` can be mobj to specify one, internal `target` field is used instead.
  - `type` is boolean. `true` for hitscan `false` for projectile.
  - When used on player mobj, `angle` and `pitch` is determined by `sv_freeaim`.
  - Returns `angle` and `pitch`.
- `LineTarget([angle])`
  - 2D target search. Atempts to find any `__shootable` mobj in path from mobj at specified `angle`.
  - If `angle` is not specified, internal `angle` field is used instead.
  - Returns mobj or nil.
- `MeleeRange([target [, zCheck]])`
  - Checks if `target` is in melee range of this mobj.
  - If `target` is not specified, internal `target` field is used instead.
  - If `zCheck` is `false`, only 2D distance is checked. Defaults to `true`.
  - Returns `true` if `target` is in melee range.
- `Distance([target [, zCheck]])`
  - Checks `target` distance from this mobj.
  - If `target` is not specified, internal `target` field is used instead.
  - If `zCheck` is `false`, only 2D distance is checked. Defaults to `true`.
  - Returns `target` distance from this mobj.
- `Damage(amount, type [, source [, inflictor]])`
  - Damage this mobj with `amount` of damage and `type` damage type.
  - `source` is origin of damage (projectile, barrel, etc) used for thrust calculation.
  - `inflictor` is who caused this damage. Monsters can attack `inflictor` back.
  - `amount` can be `true` for instagib (`_xdeath`) or `false` for instakill (`_death`).
  - Returns `true` if mobj was killed (health is less or equal to 0), `false` otherwise.
- `LineAttack(pufftype, damage [, angle [, pitch [, z [, x [, range]]]]])`
  - Hitscan attack with `pufftype` mobjtype spawned at hit destination.
  - `angle` and `pitch` are absolute values. See `AttackAim`.
  - `z` is offset relative to `shootz` and mobj current `z`.
  - `x` is offset relative to mobj current location with `angle` rotation.
  - `range` defaults to 4096.
  - If hit mobj has no `__noblood` enabled `_pain` animation of spawned `pufftype` will be entered, if defined.
  - Newly spawned `pufftype` will have:
    - `target` field set to shooter (this mobj)
    - `source` to hit mobj (what is damaged) or nil
    - `damage` to actual damage dealt
    - `damagetype` to actual damage type
    - `health` to remaining health
    - `tag` texture material, if line or plane was hit, see `materials`
  - Returns target mobj or nil.
- `Thrust(speed [, angle])`
  - Thrust thing at either specified absolute `angle` or mobjs current `angle`.
  - `speed` is fixed point strength of thrust.
- `CheckSight([target])`
  - Check if mobj can see eighter specified `target` or mobjs current `target`.
  - Returns `true` if mobj can see `target`.
- `RadiusDamage(range, damage, damagetype [, attacker [, hurt_attacker]])`
  - Explosion damage.
  - `attacker` is who caused this explosion.
  - `hurt_attacker` can be set to `false` for attacker to be immune to this damage.
  - `damage` can be negative for implosion (pull instead of push) effects, but positive damage will be applied anyway.
- `DamageScale(damagetype)`
  - Returns current damage scale of this damage type for this mobj.
- `DamageScale(damagetype, scale)`
  - Modifies current damage scale of this damage type for this mobj.
- `ResetFlags([defaults])`
  - Resets this mobj flags.
  - If no `defaults` is specified, all flags are cleared.
  - If `defaults` is `true`, original mobjtype flags are set.
  - `defaults` value of `false` is reserved, do not use.
- `SoundBody(...)` `SoundWeapon(...)` `SoundPickup(...)`
  - Play sound at channel picked by function name.
  - If multiple sounds are specified, one will be picked at random with equal chance.
- `InventoryGive(mobjtype [, amount])`
  - Give `amount` or 1 of `mobjtype` to this mobj inventory.
  - `amount` of zero can be used for depletable items to only create inventory slot.
  - Returns overflow amount - how much was left from `amount` if `maxamount` was reached.
- `InventoryTake(mobjtype [, amount])`
  - Take `amount` or 1 of `mobjtype` from this mobj inventory.
  - Returns overflow amount - how much was not taken if mobj had less than `amount`.
- `InventoryCheck(mobjtype)`
  - Returns current amount and `maxcount` of `mobjtype` this mobj has in inventory.
- `InventorySetMax(mobjtype, maxcount)`
  - Sets new `maxcount` of `mobjtype` for this mobj inventory.
- `TickerSet(id, ticrate, icon, func [, arg])`
  - Starts new ticker on this mobj. Any mobj can have multiple tickers with unique `id`.
  - `id` can be any integer value. It is used to in other ticker functions.
  - `ticrate` is periodicity of this ticker.
  - `icon` is optional (string) icon. This icon will be shown on HUD if ticker is used on player. Can be nil for no icon.
  - `func` callback that will be called after `ticrate` of tics. Callback should be function defined as `somecb(mobj)` or `somecb(mobj, arg)`.
    - By default, ticker will be removed after `ticrate` tics passed. If callback returns `true`, tics will be reset and ticker will continue to run.
- `TickerRemove(id)`
  - Remove active ticker with `id` from this mobj. Ticker is cancelled and callback won't be called.
- `TickerCheck(id)`
  - Returns `true` if mobj has active ticker with specified `id`. `false` otherwise.

#### player
- `mo`
  - Body of this player.
  - Can be modified to give player new body. This allows to morph player into something else.
  - Mobj.
- `refire`
  - Counter of `a.Refire` calls for player weapon.
  - Integer.
- `colormap`
  - See `translation` in mobjtype.
  - This, however applies color remap for everything this player can see.
  - String.
- `extralight`
  - Add brightness for player view. Used by weapon flash.
  - Can be used to make things darker.
  - Integer.
- `deltaviewheight`
  - Part of player viewheight smoothing. Used when hitting floor hard to sink view lower.
  - Fixed point.
- `map`
  - State of player automap.
    0. Player can see only what found.
    1. Player can see undiscovered areas in gray.
    2. Player can see full map.
    3. Player can see full map and all things.
  - Integer.
- `hideStatusBar`
  - Can be used to hide entire HUD.
  - This will also disable weapon selection menu.
  - Boolean.
- `forceWeapon`
  - If set to `true`, weapon selection menu will be disabled.
  - Boolean.
Player functions.
- `Message(text)`
  - Message for player in top left corner. Used for pickups.
  - `text` is string.
- `SetWeapon(mobjtype [, forced])`
  - Set player weapon to `mobjtype`. This can be any weapon, even secret one not listed in weapon selection.
  - If `forced` is `true`, weapon will be forced instantly without lowering (or even finishing shooting) of old one.
  - This function with `forced` set to `true` must be used on newly spawned players. Otherwise players won't be able to change weapon.
- `WeaponRefire(offset)`
  - Fire player weapon again, with `offset` of frames skipped from original shooting animation.
  - `offset` is integer.
- `WeaponFlash(offset)`
  - Start player flash animation with `offset` of frames skipped from original flash animation.
  - `offset` is integer.

#### sector
Sectors are part of the map. Most parameters can be modified on the fly.
- `floorheight`
  - Fixed point.
- `ceilingheight`
  - Fixed point.
- `floorpic`
  - Floor texture.
  - String. Flat texture name.
- `ceilingpic`
  - Ceiling texture.
  - String. Flat texture name.
- `lightlevel`
  - Integer. Range 0 - 255.
- `special`
  - Editor assigned special effect number.
  - Integer. Range 0 - 65535.
- `tag`
  - Editor assigned sector tag.
  - Integer. Range 0 - 65535.
- `isSecret`
  - If set to true, this sector will be considered secret.
  - Intended to be set in level setup callback.
  - Boolean.
- `color`
  - See `translation` in mobjtype.
  - Sector colormap. This allows for custom colored lights.
  - String.
- `colormap`
  - Similar to colormap, but it requires entire shading range.
  - Used to create sector fog effects.
  - String.
- `funcFloor`
  - See `generic callers`.
  - Generic caller or nil.
- `funcCeiling`
  - See `generic callers`.
  - Generic caller or nil.
- `funcCustom`
  - See `generic callers`.
  - Generic caller or nil.
Sector functions.
- `FindLowestFloor()`
  - Returns lowest floor around this sector.
  - Returns fixed point.
- `FindHighestFloor()`
  - Returns highest floor around this sector.
  - Returns fixed point.
- `FindNextFloor()`
  - Returns next highest floor around this sector.
  - Returns fixed point.
- `FindLowestCeiling()`
  - Returns highest ceiling around this sector.
  - Returns fixed point.
- `FindHighestCeiling()`
  - Returns highest ceiling around this sector.
  - Returns fixed point.
- `FindMaximalLight()`
  - Returns highest light around this sector.
  - Returns integer.
- `FindMinimalLight()`
  - Returns highest light around this sector.
  - Returns integer.
- `GetShortestTexture(location)`
  - Returns shortest texture height from any sector double sided lines.
  - Set `location` = `true` to use top texture or `false` to use bottom texture.
  - Returns integer.
- `GenericFloor(stopz, speed [, crush_speed [, start_sound [, stop_sound [, move_sound [, new_texture]]]]])`
  - Add or replace generic floor movement for this sector.
  - `stopz` is fixed point, target floor height.
  - `speed` is fixed point, movement speed. Units per tic.
  - `crush_speed` is fixed point for speed change when there is solid thing to crush.
    - Crushing is optional, setting 0 or nil will make movement to stop.
  - `*_sound` is sfx lump to play.
  - `new_texture` is flat texture to set after movement finished.
  - Returns generic caller.
- `GenericCeiling(stopz, speed [, crush_speed [, start_sound [, stop_sound [, move_sound [, new_texture]]]]])`
  - See `GenericFloor`. Similar but handles ceiling movement.
- `GenericCaller(ticrate, func [, arg])`
  - Create generic caller for this sector. Uses `custom` slot.
  - Calls a callback `func` with optional argument `arg` every `ticrate` tics.
  - Callback should be function defined as `somecb(sector)` or `somecb(sector, arg)`.
    - Callback can return `false` to be stopped and removed.
  - Two other variants exists. `GenericCallerFloor` and `GenericCallerCeiling`, each for its corresponding slot.
  - Returns generic caller.
- `ThingIterator(func [, arg])`
  - Calls a callback `func` with optional argument `arg` for every `mobj` in this sector.
  - Callback should be function defined as `somecb(mobj)` or `somecb(mobj, arg)`.
  - If callback returns `false`, iteration will stop and any other return values will get returned by `ThingIterator` itself.
- `LineIterator(func [, arg])`
  - Calls a callback `func` with optional argument `arg` for every `line` this sector has.
  - Callback should be function defined as `somecb(line)` or `somecb(line, arg)`.
  - If callback returns `false`, iteration will stop and any other return values will get returned by `LineIterator` itself.
- `SetDamage(damage, damage_type, ticrate [, in_air])`
- `SetDamage(source_sector)`
  - Make this sector damaging.
  - `damage` is amount per `ticrate`.
  - `damage_type` is type of damage. See `damage types`
  - If `in_air` is `true`, any `mobj` will get damaged even when not touching this sector floor.
  - If `source_sector` is used, all parameters are copied from source sector instead.
- `AddFloor(sector_model, line_model [, blocking])`
  - Add 3D floor to this sector.
  - `sector_model` model for added 3D planes. Keep in mind that ceiling and floor is inverted.
  - `line_model` is model for added 3D sides. Also, rederstyle from this line is copied to 3D planes.
  - `blocking` is optional, defaults to 0xFFFF. See `blocking`.
- `SoundBody(...)` `SoundWeapon(...)` `SoundPickup(...)`
  - See same functions for `mobj`.

#### line
Lines are part of the map. Most parameters can be modified on the fly.
In Doom, many lines have two sides. Each side has unique texture slots.
Access to specific side is controled by `side`.
- `side`
  - This variable is used to select which side to access.
  - `true` for front side, `false` for back side.
  - Boolean.
- `midtexture`
  - Currently selected sides middle texture.
  - String. Wall texture.
- `toptexture`
  - Currently selected sides top texture.
  - String. Wall texture.
- `bottexture`
  - Currently selected sides bottom texture.
  - String. Wall texture.
- `special`
  - Line special.
  - Integer.
- `arg0` `arg1` `arg2` `arg3` `arg4`
  - Line special arguments.
  - These can be set in Hexen map format only, but can be used in Doom format.
- `tag`
  - Line tag.
  - This can be set in Doom map format only, but can be used in Hexen format.
- `render`
  - Line render style. See `render` in `mobjtype`.
- `horizon`
  - If set `true` this line is rendered as an infinite horizon. Floor and ceiling will stretch to infinity.
  - Can be used only on single sided lines.
  - Boolean.
- `sectorFront`
  - Front sector from perspective of current `side`.
  - Sector. Read only.
- `sectorBack`
  - Back sector from perspective of current `side`.
  - Sector. Read only.
- `frontsector`
  - Front sector of this line.
  - Sector. Read only.
- `backsector`
  - Front sector of this line.
  - Sector. Read only.
- `func`
  - Current `side` caller.
  - See `line side callers`.
  - Line side caller.
- `funcFront`
  - Front side caller
  - See `line side callers`.
  - Line side caller.
- `funcBack`
  - Back side caller.
  - See `line side callers`.
  - Line side caller.
Line functions.
- `SetScroller(x, y [, side])`
  - Add `line side caller` with texture scrolling effect.
  - `x` and `y` are relative offsets added every tic.
  - Optional `side` can be used to specify line `side`. Otherwise current `side` is used.
  - Returs `line side caller`.
- `DoButton([sound_press [, sound_release [, release_tics]]])`
  - Do button texture swap effect on current line `side`. See `switch textures`.
    - Nothing will happen if line does not have `switch texture`.
  - If special is not zero, button will be "unpressed" later.
    - Note: In Hexen map format, special will be automaticaly cleared if line is not repeatable.
  - `sound_press` use sound. String, sfx lump.
  - `sound_release` release sound. String, sfx lump.
  - `release_tics` time to "unpress" this button. Integer, default 35.
- `SoundBody(...)` `SoundWeapon(...)` `SoundPickup(...)`
  - See same functions for `mobj`.

#### generic caller
Generic callers are either plane movement functions, or sector bound periodic function caller.
Every sector has 3 slots for generic callers. Two of these are reserved for planes (floor and ceiling).
Movement functions are one-go, meaning caller will be removed and its `action` called, if any, when movement finished.
You can start new movement in callback `action` for same sector. Doom uses it for permanet crushers.
- `sector`
  - Sector this caller is bound to. This will always be first callback parameter.
  - Sector. Read only.
- `ticrate`
  - Ticrate for function caller. Can be modified for extra effects.
  - Integer. Available for function caller only.
- `action`
  - Callback function.
  - For plane movement, callback should be function defined as `somecb(sector)` or `somecb(sector, arg)`.
  - For generic caller, callback should be function defined as `somecb(self)` or `somecb(self, arg)` where `self` is `generic caller` that caused it.
  - For plane movement, `action` is called when movement ends.
  - For caller, `action` is called every `ticrate` tics. Callback can return `false` to be stopped and removed.
    - Caller can be replaced with another one in its own callback.
- `arg`
  - Argument for `action`.
  - Any Lua type.
- `crush`
  - Callback function. Should be defined as `somecb(thing, self)` or `somecb(thing, self, arg)` where `self` is `generic caller` that caused it.
  - This callback will be called when plane movement is about to crush a `__solid` thing.
    - This will be only called on things that won't fit new sector height difference.
  - Available for plane movement caller only.
- `isFloor`
  - Set to `true` if this caller is floor plane movement.
  - Boolean. Read only.
- `isCeiling`
  - Set to `true` if this caller is ceiling plane movement.
  - Boolean. Read only.
- `isCaller`
  - Set to `true` if this caller is generic function caller.
  - Boolean. Read only.
- `isSuspended`
  - Set to `true` if this caller is paused.
  - Boolean. Read only.
- `isRaising`
  - Set to `true` if plane is moving up.
  - Boolean. Read only.
- `isLowering`
  - Set to `true` if plane is moving down.
  - Boolean. Read only.
Functions.
- `Stop()`
  - Stop and destroy this caller. No callback will be called.
- `Suspend(susp)`
  - Set `susp` to `true` to suspend this caller.
  - Set `susp` to `false` to wake up this caller.
  - Suspended callers do nothing, but are still present with all settings left intact.
- `Reverse()`
  - Reverse plane movement direction. New destination is origin now.
  - Used for example in Doom doors in `crush` callback.
  - Available for plane movement caller only.

#### line side caller
Similar to generic caller but bound to line side instead of sector.
Currently, there is only texture scoller.
- `x`
  - Amount to scroll in `x` direction.
  - Fixed point.
- `y`
  - Amount to scroll in `y` direction.
  - Fixed point.
Functions.
- `Stop()`
  - Stop and remove this caller.

### global callbacks
There are few callbacks that are expected to exist as global Lua functions.
- `mapLoaded()`
  - This callback is called everytime new map is loaded, including all things and spawned players.
- `playerSpawn(player)`
  - `player` is cause of this callback.
  - This callback is called everytime player respawns - new player or dead player.
  - This won't be called on level transition sice player is considered already spawned.
  - Basic player inventory is ussualy setup here.
  - Beware! Players can spawn even before other map things. This function can get called before map is fully loaded.

### global variables
Global variable `a` contains few hardcoded action functions. These are intended to be used in mobj states.
This table is writable and functions can be added or replaced.
- `Look`
  - Used for enemy logic.
  - Check if monster can see or hear enemy player. If so, jumps to `_see` animation.
- `Chase`
  - Used for enemy logic. Almost original Doom monster logic.
  - Atempt to move `mobj` to it's `target`.
  - Randomly plays `activeSound` and jums to `_missile` animation, if defined.
  - Jumps to `_melee` animation if defined and close enough to `target`.
  - Jumps to `_spawn` animation if `target` is not valid thing to kill.
- `Fall`
  - Unset `__solid` flag.
- `FaceTarget`
  - Set `angle` and `pitch` to aim at `target`.
- `SoundSee` `SoundAttack` `SoundPain` `SoundActive` `SoundDeath` `SoundXDeath`
  - Play specified sound slot at `body` channel.
- `WeaponRaise`
  - Weapon rising, should be used in `_wRaise` animation.
- `WeaponReady`
  - Weapon ready to shoot. Should be used in `_wReady` animation.
- `WeaponReadyForced`
  - Same as `WeaponReady` but player can't change weapon.
- `WeaponLower`
  - Weapon lowering. Should be used in `_wLower` animation.
- `WeaponFlash`
  - Start weapon flash animation. Limited alternative to `WeaponFlash` function for `player`.
- `WeaponRefire`
  - Refire this weapon.
  - Refire calls are counted and count is available in `player` as `refire` integer.
  - Usually used to make constant firing inaccurate.
- `NoiseAlert`
  - Alert listening monsters.

Global variable `game` contains various game state information.
This table is read only and its variables too.
- `map`
  - String. Current map name.
- `time`
  - Integer. Number of tics elapsed from lever start.
- `DoomExit(secret)`
  - Function to exit level doom way. Levels are ordered as in original Doom. Depends on game IWAD.
  - Set `secret` for secret exit.
- `Exit(map_lump)`
  - Exit to any level named by string `map_lump`.
  - This should be primary way to exit custom games.
  - This won't affect Doom ordering. For Doom ordering it is like `Exit` was never called.
- `deathMatch`
  - Boolean, set to `true` if game is deathmatch.
- `doomEpisode`
  - Integer, current doom episode.
  - Used in Doom Lua scripts.

### sound channels
Every sound source has multiple sound channels. Every channel can play single sound at a time.
Use different channels to play multiple sounds from same source.
Channels are: `body` `weapon` and `pickup`.


##### I hope that's all
Keep in mind this is beta release and some API might change. I am open to API suggestions.
