-- kgsws' Lua Doom exports
-- Doom line actions

-- Teleports

PIT_StompThing =
function(thing, mobj)
	if thing ~= mobj and thing.z < mobj.z + mobj.height and thing.z + thing.height > mobj.z then
		thing.Damage(true, 0, mobj)
	end
end

function PIT_TeleportCheck(thing)
	if thing.__solid then
		return false, true
	end
end

function PIT_TeleportDest(thing)
	if thing.info == MT_TELEPORTMAN then
		return false, thing
	end
end

function setupTeleport(sector)
	local thing
	thing = sector.ThingIterator(PIT_TeleportDest)
	if thing ~= nil then
		return false, thing
	end
end

function lineTeleport(mobj, line, side, act)
	if act ~= lnspec.cross or not side or mobj.__missile then
		return false
	end
	if not doomLineType[line.special] and mobj.player ~= nil then
		return false
	end
	if line.tag ~= 0 then
		local dest
		dest = sectorTagIterator(line.tag, setupTeleport)
		if dest ~= nil then
			local radius
			radius = mobj.radius
			if mobj.player == nil and blockThingsIterator(dest.x - radius, dest.y - radius, dest.x + radius, dest.y + radius, PIT_TeleportCheck) then
				return false
			else
				mobj.reactiontime = 18
			end
			spawnMobj(MT_TFOG, mobj.x, mobj.y, mobj.z)
			mobj.Teleport(dest.x, dest.y, false)
			mobj.angle = dest.angle
			spawnMobj(MT_TFOG, mobj.x + finecosine[mobj.angle] * 20, mobj.y + finesine[mobj.angle] * 20, mobj.z)
			mobj.momx = 0
			mobj.momy = 0
			blockThingsIterator(dest.x - radius, dest.y - radius, dest.x + radius, dest.y + radius, PIT_StompThing, mobj)
			if not doomLineRe[line.special] then
				line.special = 0
			end
			return true
		end
	end
	return true
end

linefunc[97] = lineTeleport
doomLineType[97] = true
doomLineRe[97] = true

linefunc[39] = lineTeleport
doomLineType[39] = true
doomLineRe[39] = false

linefunc[126] = lineTeleport
doomLineType[126] = false
doomLineRe[126] = true

linefunc[125] = lineTeleport
doomLineType[125] = false
doomLineRe[125] = false

-- MT_TELEPORTMAN; teleport destination
mtype = {
	ednum = 14,
	__noBlockmap = true,
	__noGravity = true
}
MT_TELEPORTMAN = createMobjType(mtype)

-- MT_TFOG
mtype = {
	seeSound = "dstelept",
	ednum = 14,
	__noBlockmap = true,
	__noGravity = true,
	_spawn = {
		{"*TFOGA", 6, a.SoundSee},
		{"*TFOGB", 6},
		{"*TFOGA", 6},
		{"*TFOGB", 6},
		{"*TFOGC", 6},
		{"*TFOGD", 6},
		{"*TFOGE", 6},
		{"*TFOGF", 6},
		{"*TFOGG", 6},
		{"*TFOGH", 6},
		{"*TFOGI", 6},
		{"*TFOGJ", 6}
	}
}
MT_TFOG = createMobjType(mtype)

