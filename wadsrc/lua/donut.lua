-- kgsws' Lua Doom exports
-- Doom line actions

-- Donut

function checkNextDonut(line, nope)
	local nsec
	nsec = line.backsector
	if nsec ~= nil and nsec ~= nope then
		return false, nsec
	end
end

function checkNextHole(line, nope)
	local nsec
	nsec = line.backsector
	if nsec ~= nil then
		if nsec == nope then
			return false, line.frontsector
		else
			return false, nsec
		end
	end
	return false
end

function setupDonut(sector, spec)
	if sector.funcFloor ~= nil then
		return false
	end
	local snext
	local stex
	snext = sector.LineIterator(checkNextHole, sector)
	if snext == nil then
		return false
	end
	stex = snext.LineIterator(checkNextDonut, sector)
	if stex == nil then
		return false
	end
	sector.GenericFloor(stex.floorheight, 0.5, 0, nil, nil, "dsstnmov")
	local func
	func = snext.GenericFloor(stex.floorheight, 0.5, 0, nil, nil, nil, stex.floorpic)
	func.action = changeAfterLower
	func.arg = stex
	return true
end

function lineDonut(mobj, line, side, act)
	local spec
	spec = line.special
	if act ~= doomLineType[spec] or mobj.player == nil then
		return false
	end
	local tag
	tag = line.tag
	if tag ~= 0 then
		sectorTagIterator(tag, setupDonut, spec)
	else
		if not setupDonut(line.backsector, spec) then
			return false
		end
	end
	line.special = 0
	line.DoButton("dsswtchn", "dsswtchn")
	return true	
end

linefunc[9] = lineDonut
doomLineType[9] = lnspec.use

