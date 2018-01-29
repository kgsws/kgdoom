-- kgsws' Lua Doom exports
-- Doom line actions

-- Plats: simple up

function platformCrushSimple(th, self)
	self.Reverse()
end

function setupPlatformRaise(sector, line)
	if sector.funcFloor ~= nil then
		return false
	end
	local spec
	local info
	local func
	spec = line.special
	if spec == 66 or spec == 15 then
		info = sector.floorheight + 24
		sector.SetDamage(line.frontsector)
	elseif spec == 67 or spec == 14 then
		info = sector.floorheight + 32
		sector.SetDamage(0, 0, 0)
	else
		info = sector.FindNextFloor()
		sector.SetDamage(0, 0, 0)
	end
	sector.floorpic = line.frontsector.floorpic
	func = sector.GenericFloor(info, 0.5, 0, nil, "dspstop", "dsstnmov")
	func.crush = platformCrushSimple
	return true
end

function platformRaise(mobj, line, side, act)
	local spec
	spec = line.special
	if act ~= doomLineType[spec] or mobj.player == nil then
		return false
	end
	local tag
	tag = line.tag
	if tag ~= 0 then
		sectorTagIterator(tag, setupPlatformRaise, line)
	else
		if not setupPlatformRaise(line.backsector, line) then
			return false
		end
	end
	if not doomLineRe[spec] then
		line.special = 0
	end
	line.DoButton("dsswtchn", "dsswtchn")
	return true	
end

linefunc[66] = platformRaise
doomLineType[66] = lnspec.use
doomLineRe[66] = true

linefunc[15] = platformRaise
doomLineType[15] = lnspec.use
doomLineRe[15] = false

linefunc[67] = platformRaise
doomLineType[67] = lnspec.use
doomLineRe[67] = true

linefunc[14] = platformRaise
doomLineType[14] = lnspec.use
doomLineRe[14] = false

linefunc[68] = platformRaise
doomLineType[68] = lnspec.use
doomLineRe[68] = true

linefunc[20] = platformRaise
doomLineType[20] = lnspec.use
doomLineRe[20] = false

linefunc[95] = platformRaise
doomLineType[95] = lnspec.cross
doomLineRe[95] = true

linefunc[22] = platformRaise
doomLineType[22] = lnspec.cross
doomLineRe[22] = false

linefunc[47] = platformRaise
doomLineType[47] = lnspec.hit
doomLineRe[47] = false

-- Plats: Down Wait Up Stay
-- [1] bottom height
-- [2] top height
-- [3] speed; negative = perpetual

function setupPlatformGoDown(self, info)
	local func
	func = self.sector.GenericFloor(info[1], info[3], 0, "dspstart", "dspstop")
	func.action = setupPlatformReturnUp
	func.arg = info
	return false
end

function setupPlatformGoUp(self, info)
	local func
	func = self.sector.GenericFloor(info[2], info[3], 0, "dspstart", "dspstop")
	func.action = setupPlatformReturnDown
	func.arg = info
	func.crush = platformCrush
	return false
end

function setupPlatformReturnUp(sector, info)
	sector.GenericCallerFloor(35*3, setupPlatformGoUp, info)
end

function setupPlatformReturnDown(sector, info)
	if info[3] < 0 then
		sector.GenericCallerFloor(35*3, setupPlatformGoDown, info)
	end
end

function platformCrush(th, self, info)
	setupPlatformGoDown(self, info)
	return false
end

function setupPlatformNormal(sector, spec)
	if sector.funcFloor ~= nil then
		if sector.funcFloor.isSuspended then
			sector.funcFloor.Suspend(false)
			return true
		end
		return false
	end
	local info
	local func
	info = {sector.FindLowestFloor(), 0, 0}
	if spec > 100 then
		info[3] = 8
	else
		info[3] = 4
	end
	if spec == 87 or spec == 53 then
		info[2] = sector.FindHighestFloor()
		if sector.floorheight > info[2] then
			info[2] = sector.floorheight
		end
		info[3] = -1
	else
		info[2] = sector.floorheight
	end
	func = sector.GenericFloor(info[1], info[3], 0, "dspstart", "dspstop")
	func.action = setupPlatformReturnUp
	func.arg = info
	return true
end

function platformDownWaitUpStay(mobj, line, side, act)
	local spec
	spec = line.special
	if act ~= doomLineType[spec] or mobj.player == nil then
		return false
	end
	local tag
	tag = line.tag
	if tag ~= 0 then
		sectorTagIterator(tag, setupPlatformNormal, spec)
	else
		if not setupPlatformNormal(line.backsector, spec) then
			return false
		end
	end
	if not doomLineRe[spec] then
		line.special = 0
	end
	line.DoButton("dsswtchn", "dsswtchn")
	return true	
end

linefunc[87] = platformDownWaitUpStay
doomLineType[87] = lnspec.cross
doomLineRe[87] = true

linefunc[53] = platformDownWaitUpStay
doomLineType[53] = lnspec.cross
doomLineRe[53] = false

linefunc[62] = platformDownWaitUpStay
doomLineType[62] = lnspec.use
doomLineRe[62] = true

linefunc[21] = platformDownWaitUpStay
doomLineType[21] = lnspec.use
doomLineRe[21] = false

linefunc[88] = platformDownWaitUpStay
doomLineType[88] = lnspec.cross
doomLineRe[88] = true

linefunc[10] = platformDownWaitUpStay
doomLineType[10] = lnspec.cross
doomLineRe[10] = false

linefunc[123] = platformDownWaitUpStay
doomLineType[123] = lnspec.use
doomLineRe[123] = true

linefunc[122] = platformDownWaitUpStay
doomLineType[122] = lnspec.use
doomLineRe[122] = false

linefunc[120] = platformDownWaitUpStay
doomLineType[120] = lnspec.cross
doomLineRe[120] = true

linefunc[121] = platformDownWaitUpStay
doomLineType[121] = lnspec.cross
doomLineRe[121] = false

-- Plats: Suspend

function setupPlatformSuspend(sector, spec)
	local func
	local act
	func = sector.funcFloor
	if func == nil then
		return false
	end
	act = func.action
	if act ~= setupPlatformReturnUp and act ~= setupPlatformReturnDown and act ~= setupPlatformGoDown and act ~= setupPlatformGoUp then
		return false
	end
	func.Suspend(true)
	return true
end

function platformSuspend(mobj, line, side, act)
	local spec
	spec = line.special
	if act ~= doomLineType[spec] or mobj.player == nil then
		return false
	end
	local tag
	tag = line.tag
	if tag ~= 0 then
		sectorTagIterator(tag, setupPlatformSuspend)
	else
		if not setupPlatformSuspend(line.backsector) then
			return false
		end
	end
	if not doomLineRe[spec] then
		line.special = 0
	end
	line.DoButton("dsswtchn", "dsswtchn")
	return true	
end

linefunc[89] = platformSuspend
doomLineType[89] = lnspec.cross
doomLineRe[89] = true

linefunc[54] = platformSuspend
doomLineType[54] = lnspec.cross
doomLineRe[54] = false

