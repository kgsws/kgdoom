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

### Lua scripts
Lua scirpts are stored in WADs. There is only one recognized lump name: GAMELUA. You can have as many GAMELUA lumps in multiple wads.
Lua scripts are loaded from first wad to last and from first lump to last one. This should allow for overrides (game mods).
First line in each GAMELUA can specify internal script name. Example: `---test.lua`. Every time Lua has to report error, it will use `test.lua` as a script name.
##### Stages
There are two stages. Loading stage and map stage. Every stage has its own set of global functions.
###### Loading stage
This is stage when all GAMELUA scripts are loaded and executed. At this point you should create all mobjtypes, register player class, weapons and keys. You can not do these later.
Contents of GAMELUA ouside of any functions is executed at this stage.
###### Map stage
This is stage when level is already loaded and everything in map is spawned.
Only callbacks and action functions are called.

###### Beware
The is a weird bug(?) in Lua itself. If you make a syntax error, you will get `attempt to call a string value` error, which is not helpful at all.

### API
#### inventory & armor
- Every mobj can have its own inventory and armor.
- Inventory item can be any mobjtype that has maxcount other than zero.
  - When count reaches zero, item is removed from inventory.
  - Negative maxcount is undepletable and will be kept even if count reaches zero. Used for ammo sice HUD expects ammo to exist even when depleted.
#### values
- angles
  - Angles are specified in range 0 - 8191. This is a whole circle.
- fixed
  - Any fixed point values can use decimal point in Lua.
#### globals
###### Loading stage
- function `createMobjType(table)`
  - Table should contain everything required to describe mobj type.
  - See Lua scripts in kgdoom.wad file.
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
Animations are always specified as `_animName`
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
  - Anything Lua type.
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
  - Table of Fixed point numbers. Range from -2.7 to 9.0 with 0.05 steps. Or `true` for instant kill.




## To be continued ...
##### It takes a while to make manual.






