-- kgsws' Lua Doom exports
-- Doom line actions

-- Floor: Down Highest Neighbor Floor

function setupHighestNeighborFloor(sector, spec)
	local z
	local speed
	z = sector.FindHighestFloor()
	if spec == 70 or spec == 71 or spec == 98 or spec == 36 then
		speed = 4
		z = z + 8
	else
		speed = 1
	end
	if z > sector.floorheight then
		speed = z - sector.floorheight
	end
	sector.GenericFloor(z, speed, 0, nil, "dspstop", "dsstnmov")
end

function floorDownHighestNeighborFloor(mobj, line, side, act)
	local spec
	spec = line.special
	if act ~= doomLineType[spec] or mobj.player == nil then
		return false
	end
	local tag
	tag = line.tag
	if tag ~= 0 then
		sectorTagIterator(tag, setupHighestNeighborFloor, spec)
	else
		if not setupHighestNeighborFloor(line.backsector, spec) then
			return false
		end
	end
	if not doomLineRe[spec] then
		line.special = 0
	end
	line.DoButton("dsswtchn", "dsswtchn")
	return true	
end

linefunc[45] = floorDownHighestNeighborFloor
doomLineType[45] = lnspec.use
doomLineRe[45] = true

linefunc[102] = floorDownHighestNeighborFloor
doomLineType[102] = lnspec.use
doomLineRe[102] = false

linefunc[83] = floorDownHighestNeighborFloor
doomLineType[83] = lnspec.cross
doomLineRe[83] = true

linefunc[19] = floorDownHighestNeighborFloor
doomLineType[19] = lnspec.cross
doomLineRe[19] = false

linefunc[70] = floorDownHighestNeighborFloor
doomLineType[70] = lnspec.use
doomLineRe[70] = true

linefunc[71] = floorDownHighestNeighborFloor
doomLineType[71] = lnspec.use
doomLineRe[71] = false

linefunc[98] = floorDownHighestNeighborFloor
doomLineType[98] = lnspec.cross
doomLineRe[98] = true

linefunc[36] = floorDownHighestNeighborFloor
doomLineType[36] = lnspec.cross
doomLineRe[36] = false

-- Floor: Down Lowest Neighbor Floor

function floorTextureFind(line, info)
	local sec
	sec = line.backsector
	if sec ~= nil then
		if sec == info[2] then
			sec = line.frontsector
		end
		return sec.floorheight ~= info[1], sec
	end
end

function changeAfterLower(sector, sec)
	sector.SetDamage(sec)
end

function setupLowestNeighborFloor(sector, spec)
	local info
	local func
	local sec
	local tex
	info = {sector.FindLowestFloor(), sector}
	if spec == 84 or spec == 37 then
		sec = sector.LineIterator(floorTextureFind, info)
		if sec ~= nil then
			tex = sec.floorpic
		end
	end
	func = sector.GenericFloor(info[1], 1, 0, nil, "dspstop", "dsstnmov", tex)
	if sec ~= nil then
		func.action = changeAfterLower
		func.arg = sec
	end
end

function floorDownLowestNeighborFloor(mobj, line, side, act)
	local spec
	spec = line.special
	if act ~= doomLineType[spec] or mobj.player == nil then
		return false
	end
	local tag
	tag = line.tag
	if tag ~= 0 then
		sectorTagIterator(tag, setupLowestNeighborFloor, spec)
	else
		if not setupLowestNeighborFloor(line.backsector, spec) then
			return false
		end
	end
	if not doomLineRe[spec] then
		line.special = 0
	end
	line.DoButton("dsswtchn", "dsswtchn")
	return true	
end

linefunc[60] = floorDownLowestNeighborFloor
doomLineType[60] = lnspec.use
doomLineRe[60] = true

linefunc[23] = floorDownLowestNeighborFloor
doomLineType[23] = lnspec.use
doomLineRe[23] = false

linefunc[82] = floorDownLowestNeighborFloor
doomLineType[82] = lnspec.cross
doomLineRe[82] = true

linefunc[38] = floorDownLowestNeighborFloor
doomLineType[38] = lnspec.cross
doomLineRe[38] = false

linefunc[84] = floorDownLowestNeighborFloor
doomLineType[84] = lnspec.cross
doomLineRe[84] = true

linefunc[37] = floorDownLowestNeighborFloor
doomLineType[37] = lnspec.cross
doomLineRe[37] = false

-- Floor: Up Lowest Neighbor Ceiling

function setupLowestNeighborCeiling(sector, spec)
	local speed
	local z
	speed = 1
	z = sector.FindLowestCeiling()
	if spec == 65 or spec == 55 or spec == 94 or spec == 56 then
		z = z - 8
	end
	if z < sector.floorheight then
		speed = sector.floorheight - z
	end
	sector.GenericFloor(z, speed, 0, nil, "dspstop", "dsstnmov")
end

function floorUpLowestNeighborCeiling(mobj, line, side, act)
	local spec
	spec = line.special
	if act ~= doomLineType[spec] or mobj.player == nil then
		return false
	end
	local tag
	tag = line.tag
	if tag ~= 0 then
		sectorTagIterator(tag, setupLowestNeighborCeiling, spec)
	else
		if not setupLowestNeighborCeiling(line.backsector, spec) then
			return false
		end
	end
	if not doomLineRe[spec] then
		line.special = 0
	end
	line.DoButton("dsswtchn", "dsswtchn")
	return true	
end

linefunc[64] = floorUpLowestNeighborCeiling
doomLineType[64] = lnspec.use
doomLineRe[64] = true

linefunc[101] = floorUpLowestNeighborCeiling
doomLineType[101] = lnspec.use
doomLineRe[101] = false

linefunc[91] = floorUpLowestNeighborCeiling
doomLineType[91] = lnspec.cross
doomLineRe[91] = true

linefunc[5] = floorUpLowestNeighborCeiling
doomLineType[5] = lnspec.cross
doomLineRe[5] = false

linefunc[24] = floorUpLowestNeighborCeiling
doomLineType[24] = lnspec.hit
doomLineRe[24] = false

linefunc[65] = floorUpLowestNeighborCeiling
doomLineType[65] = lnspec.use
doomLineRe[65] = true

linefunc[55] = floorUpLowestNeighborCeiling
doomLineType[55] = lnspec.use
doomLineRe[55] = false

linefunc[94] = floorUpLowestNeighborCeiling
doomLineType[94] = lnspec.cross
doomLineRe[94] = true

linefunc[56] = floorUpLowestNeighborCeiling
doomLineType[56] = lnspec.cross
doomLineRe[56] = false

-- Floor: Up Next Neighbor Floor

function setupNextNeighborFloor(sector, spec)
	local speed
	if spec > 128 then
		speed = 4
	else
		speed = 1
	end
	sector.GenericFloor(sector.FindNextFloor(), speed, 0, nil, "dspstop", "dsstnmov")
end

function floorUpNextNeighborFloor(mobj, line, side, act)
	local spec
	spec = line.special
	if act ~= doomLineType[spec] or mobj.player == nil then
		return false
	end
	local tag
	tag = line.tag
	if tag ~= 0 then
		sectorTagIterator(tag, setupNextNeighborFloor, spec)
	else
		if not setupNextNeighborFloor(line.backsector, spec) then
			return false
		end
	end
	if not doomLineRe[spec] then
		line.special = 0
	end
	line.DoButton("dsswtchn", "dsswtchn")
	return true	
end

linefunc[69] = floorUpNextNeighborFloor
doomLineType[69] = lnspec.use
doomLineRe[69] = true

linefunc[18] = floorUpNextNeighborFloor
doomLineType[18] = lnspec.use
doomLineRe[18] = false

linefunc[128] = floorUpNextNeighborFloor
doomLineType[128] = lnspec.cross
doomLineRe[128] = true

linefunc[119] = floorUpNextNeighborFloor
doomLineType[119] = lnspec.cross
doomLineRe[119] = false

linefunc[132] = floorUpNextNeighborFloor
doomLineType[132] = lnspec.use
doomLineRe[132] = true

linefunc[131] = floorUpNextNeighborFloor
doomLineType[131] = lnspec.use
doomLineRe[131] = false

linefunc[129] = floorUpNextNeighborFloor
doomLineType[129] = lnspec.cross
doomLineRe[129] = true

linefunc[130] = floorUpNextNeighborFloor
doomLineType[130] = lnspec.cross
doomLineRe[130] = false

-- Floor: Up By Value

function setupUpByValue(sector, line)
	local spec
	local tex
	local z
	spec = line.special
	z = 24
	if spec == 140 then
		z = 512
	elseif spec == 96 or spec == 30 then
		z = sector.GetShortestTexture(false)
	elseif spec == 93 or spec == 59 then
		sector.floorpic = line.frontsector.floorpic
		sector.SetDamage(line.frontsector)
	end
	sector.GenericFloor(z + sector.floorheight, 1, 0, nil, "dspstop", "dsstnmov")
end

function floorUpByValue(mobj, line, side, act)
	local spec
	spec = line.special
	if act ~= doomLineType[spec] or mobj.player == nil then
		return false
	end
	local tag
	tag = line.tag
	if tag ~= 0 then
		sectorTagIterator(tag, setupUpByValue, line)
	else
		if not setupUpByValue(line.backsector, line) then
			return false
		end
	end
	if not doomLineRe[spec] then
		line.special = 0
	end
	line.DoButton("dsswtchn", "dsswtchn")
	return true	
end

linefunc[92] = floorUpByValue
doomLineType[92] = lnspec.cross
doomLineRe[92] = true

linefunc[58] = floorUpByValue
doomLineType[58] = lnspec.cross
doomLineRe[58] = false

linefunc[93] = floorUpByValue
doomLineType[93] = lnspec.cross
doomLineRe[93] = true

linefunc[59] = floorUpByValue
doomLineType[59] = lnspec.cross
doomLineRe[59] = false

linefunc[96] = floorUpByValue
doomLineType[96] = lnspec.cross
doomLineRe[96] = true

linefunc[30] = floorUpByValue
doomLineType[30] = lnspec.cross
doomLineRe[30] = false

linefunc[140] = floorUpByValue
doomLineType[140] = lnspec.use
doomLineRe[140] = false

