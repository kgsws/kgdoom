-- kgsws' Lua Doom exports
-- common action functions

function shadowAim(mobj, angle)
	local target
	target = mobj.target
	if target ~= nil then
		if target.render == "!SHADOW" then
			return angle + (doomRandom() - doomRandom()) * 4
		end
	end
	return angle
end

a.FaceTarget =
function(mobj)
	mobj.Face(mobj.target)
	mobj.angle = shadowAim(mobj, mobj.angle)
end

a.Crushed =
function(mobj)
	if mobj.info == MT_HEAD then
		mobj.translation = "BLOODMAP:0"
	elseif mobj.info == MT_BRUISER or mobj.info == MT_KNIGHT then
		mobj.translation = "BLOODMAP:1"
	end
	mobj.__solid = false
end

a.Metal =
function(mobj)
	mobj.SoundPickup("dsmetal")
	a.Chase(mobj)
end

a.Hoof =
function(mobj)
	mobj.SoundPickup("dshoof")
	a.Chase(mobj)
end

PIT_BossDeath =
function(thing, info)
	if thing.info == info and (thing.health > 0 or thing.tics ~= -1) then
		return false, true
	end
end

PIT_BossOpen =
function(sector)
	sector.GenericCeiling(sector.FindLowestCeiling() - 4, 8, 0, "dsbdopn")
end

PIT_BossLower =
function(sector)
	sector.GenericFloor(sector.FindLowestFloor(), 1, 0, "-", "-", "dsstnmov")
end

-- Loading map

function setupClose30(self)
	local sector
	sector = self.sector
	sector.GenericCeiling(sector.floorheight, 2, 0, "dsdorcls")
end

function setupOpen300(self)
	local sector
	sector = self.sector
	sector.GenericCeiling(sector.FindLowestCeiling() - 4, 2, 0, "dsdoropn")
end

function mapLoadSector(sector)
	local spec
	spec = sector.special
	-- secret
	if spec == 9 then
		sector.isSecret = true
	end
	-- damage
	if spec == 5 then
		sector.SetDamage(10, 1, 32)
	elseif spec == 7 then
		sector.SetDamage(5, 1, 32)
	elseif spec == 16 or spec == 4 or spec == 11 then
		sector.SetDamage(20, 1, 32)
	end
	-- doors
	if spec == 10 then
		sector.GenericCallerCeiling(35*30, setupClose30, 8)
	elseif spec == 14 then
		sector.GenericCallerCeiling(35*30, setupOpen300, 8)
	end
	-- light
	if spec == 1 then
		sector.GenericCaller(doomRandom(1, 64), lightRandomBlink, sector.FindMinimalLight())
	elseif spec == 2 or spec == 4 then
		local level
		level = sector.FindMinimalLight()
		if level == sector.lightlevel then
			level = 0
		end
		sector.GenericCaller(doomRandom(1, 8), lightSlowBlink, level)
	elseif spec == 3 then
		local level
		level = sector.FindMinimalLight()
		if level == sector.lightlevel then
			level = 0
		end
		sector.GenericCaller(doomRandom(1, 8), lightFastBlink, level)
	elseif spec == 8 then
		local level0
		local level1
		level0 = sector.lightlevel
		level1 = sector.FindMinimalLight()
		if level0 ~= level1 then
			if level0 > level1 then
				sector.special = level1
				level1 = level0
			else
				sector.special = level0
			end
			sector.GenericCaller(1, lightGlowBlink, level1)
		end
	elseif spec == 12 then
		local level
		level = sector.FindMinimalLight()
		if level == sector.lightlevel then
			level = 0
		end
		sector.GenericCaller(1, lightFastBlink, level)
	elseif spec == 13 then
		local level
		level = sector.FindMinimalLight()
		if level == sector.lightlevel then
			level = 0
		end
		sector.GenericCaller(1, lightSlowBlink, level)
	elseif spec == 17 then
		sector.special = sector.lightlevel + 16
		sector.GenericCaller(4, lightFlickerBlink, sector.FindMinimalLight())
	end
end

function mapLoadLine(line)
	if line.special == 48 then
		local func
		func = line.funcFront
		if func ~= nil then
			func.x = func.x + 1
		else
			line.SetScroller(1, 0, true)
			line.special = 0
		end
	end
	-- this is not Doom original, but freedoom uses it
	if line.special == 85 then
		local func
		func = line.funcFront
		if func ~= nil then
			func.x = func.x - 1
		else
			line.SetScroller(-1, 0, true)
			line.special = 0
		end
	end
end

function mapSetupPlayers(player)
	local mobj
	mobj = player.mo
	if game.deathmatch then
		mobj.InventoryGive(MT_BLUECARD)
		mobj.InventoryGive(MT_YELLOWCARD)
		mobj.InventoryGive(MT_REDCARD)
		mobj.InventoryGive(MT_BLUESKULL)
		mobj.InventoryGive(MT_YELLOWSKULL)
		mobj.InventoryGive(MT_REDSKULL)
	else
		mobj.InventoryTake(MT_BLUECARD)
		mobj.InventoryTake(MT_YELLOWCARD)
		mobj.InventoryTake(MT_REDCARD)
		mobj.InventoryTake(MT_BLUESKULL)
		mobj.InventoryTake(MT_YELLOWSKULL)
		mobj.InventoryTake(MT_REDSKULL)
	end
end

function mapLoaded()
	globalSectorsIterator(mapLoadSector)
	globalLinesIterator(mapLoadLine)
	globalPlayersIterator(mapSetupPlayers)
	-- test
	fakeContrast(false)
end

