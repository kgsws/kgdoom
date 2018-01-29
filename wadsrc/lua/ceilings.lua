-- kgsws' Lua Doom exports
-- Doom line actions

-- Ceiling crusher
-- [1] height
-- [2] speed
-- [3] sound
-- [4] stop sound

function ceilingDoCrush(th, self)
	if (game.time & 3) == 0 then
		th.Damage(10, 0)
	end
end

function setupCrusherReturn(sector, info)
	local func
	func = sector.GenericCeiling(info[1], info[2], 0, nil, info[4], info[3])
	func.action = setupCrusherDown
	func.arg = info
	func.crush = ceilingDoCrush
end

function setupCrusherDown(sector, info)
	local func
	func = sector.GenericCeiling(sector.floorheight + 8, info[2], info[2] / 8, nil, info[4], info[3])
	func.action = setupCrusherReturn
	func.arg = info
	func.crush = ceilingDoCrush
end

function setupCrusher(sector, spec)
	if sector.funcCeiling ~= nil then
		if sector.funcCeiling.isSuspended then
			sector.funcCeiling.Suspend(false)
			return true
		end
		return false
	end
	local info
	info = {sector.ceilingheight, 1, "dsstnmov", nil}
	if spec == 77 or spec == 6 then
		info[2] = 2
	end
	if spec == 141 then
		info[3] = nil
		info[4] = "dspstop"
	end
	setupCrusherDown(sector, info)
	return true
end

function ceilingCrusher(mobj, line, side, act)
	local spec
	spec = line.special
	if act ~= doomLineType[spec] or mobj.player == nil then
		return false
	end
	local tag
	tag = line.tag
	if tag ~= 0 then
		sectorTagIterator(tag, setupCrusher, spec)
	else
		if not setupCrusher(line.backsector, spec) then
			return false
		end
	end
	if not doomLineRe[spec] then
		line.special = 0
	end
	line.DoButton("dsswtchn", "dsswtchn")
	return true	
end

linefunc[49] = ceilingCrusher
doomLineType[49] = lnspec.use
doomLineRe[49] = false

linefunc[73] = ceilingCrusher
doomLineType[73] = lnspec.cross
doomLineRe[73] = true

linefunc[25] = ceilingCrusher
doomLineType[25] = lnspec.cross
doomLineRe[25] = false

linefunc[77] = ceilingCrusher
doomLineType[77] = lnspec.cross
doomLineRe[77] = true

linefunc[6] = ceilingCrusher
doomLineType[6] = lnspec.cross
doomLineRe[6] = false

linefunc[141] = ceilingCrusher
doomLineType[141] = lnspec.cross
doomLineRe[141] = false

-- Ceiling crusher pause

function setupCrusherSuspend(sector, spec)
	local func
	local act
	func = sector.funcFloor
	if func == nil then
		return false
	end
	act = func.action
	if act ~= setupCrusherDown and act ~= setupCrusherReturn then
		return false
	end
	func.Suspend(true)
	return true
end

function ceilingSuspend(mobj, line, side, act)
	local spec
	spec = line.special
	if act ~= doomLineType[spec] or mobj.player == nil then
		return false
	end
	local tag
	tag = line.tag
	if tag ~= 0 then
		sectorTagIterator(tag, setupCrusherSuspend)
	else
		if not setupCrusherSuspend(line.backsector) then
			return false
		end
	end
	if not doomLineRe[spec] then
		line.special = 0
	end
	line.DoButton("dsswtchn", "dsswtchn")
	return true	
end

linefunc[74] = ceilingSuspend
doomLineType[74] = lnspec.cross
doomLineRe[74] = true

linefunc[57] = ceilingSuspend
doomLineType[57] = lnspec.cross
doomLineRe[57] = false

-- Ceiling move, no crush

function setupCeilingMove(sector, spec)
	if sector.funcCeiling ~= nil then
		return false
	end
	local speed
	local dest
	if spec == 43 or spec == 41 then
		speed = 4
		dest = sector.floorheight
	elseif spec == 40 then
		speed = 1
		dest = sector.FindHighestCeiling()
	else
		speed = 1
		dest = sector.floorheight + 8
	end
	sector.GenericCeiling(dest, speed, 0, nil, nil, "dsstnmov")
	return true
end

function ceilingMove(mobj, line, side, act)
	local spec
	spec = line.special
	if act ~= doomLineType[spec] or mobj.player == nil then
		return false
	end
	local tag
	tag = line.tag
	if tag ~= 0 then
		sectorTagIterator(tag, setupCeilingMove, spec)
	else
		if not setupCeilingMove(line.backsector, spec) then
			return false
		end
	end
	if not doomLineRe[spec] then
		line.special = 0
	end
	line.DoButton("dsswtchn", "dsswtchn")
	return true	
end

linefunc[43] = ceilingMove
doomLineType[43] = lnspec.use
doomLineRe[43] = true

linefunc[41] = ceilingMove
doomLineType[41] = lnspec.use
doomLineRe[41] = false

linefunc[40] = ceilingMove
doomLineType[40] = lnspec.cross
doomLineRe[40] = false

linefunc[72] = ceilingMove
doomLineType[72] = lnspec.cross
doomLineRe[72] = true

linefunc[44] = ceilingMove
doomLineType[44] = lnspec.cross
doomLineRe[44] = false

