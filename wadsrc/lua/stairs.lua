-- kgsws' Lua Doom exports
-- Doom line actions

-- Stairs
-- [1] sector
-- [2] texture

function checkNextStair(line, info)
	local nsec
	nsec = line.backsector
	if nsec ~= nil and line.frontsector == info[1] and nsec.funcFloor == nil and nsec.floorpic == info[2] then
		return false, nsec
	end
end

function setupStairs(sector, spec)
	if sector.funcFloor ~= nil then
		return false
	end
	local z
	local speed
	local step
	local info
	if spec > 8 then
		speed = 4
		step = 16
	else
		speed = 0.25
		step = 8
	end
	z = sector.floorheight
	info = {sector, sector.floorpic}
	while sector ~= nil do
		z = z + step
		sector.GenericFloor(z, speed, 0, nil, "dspstop", "dsstnmov")
		sector = sector.LineIterator(checkNextStair, info)
		info[1] = sector
	end
end

function stairsBuild(mobj, line, side, act)
	local spec
	spec = line.special
	if act ~= doomLineType[spec] or mobj.player == nil then
		return false
	end
	local tag
	tag = line.tag
	if tag ~= 0 then
		sectorTagIterator(tag, setupStairs, spec)
	else
		if not setupStairs(line.backsector, spec) then
			return false
		end
	end
	if not doomLineRe[spec] then
		line.special = 0
	end
	line.DoButton("dsswtchn", "dsswtchn")
	return true	
end

linefunc[7] = stairsBuild
doomLineType[7] = lnspec.use
doomLineRe[7] = false

linefunc[8] = stairsBuild
doomLineType[8] = lnspec.cross
doomLineRe[8] = false

linefunc[127] = stairsBuild
doomLineType[127] = lnspec.use
doomLineRe[127] = false

linefunc[100] = stairsBuild
doomLineType[100] = lnspec.cross
doomLineRe[100] = false

