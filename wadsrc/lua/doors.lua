-- kgsws' Lua Doom exports
-- Doom line actions

-- Door: open, wait, close

function setupOpen(th, self, speed)
	local sound
	local sector
	local func
	if speed == 8 then
		sound = "dsbdopn"
	else
		sound = "dsdoropn"
	end
	sector = self.sector
	func = sector.GenericCeiling(sector.FindLowestCeiling() - 4, speed, 0, sound)
	func.action = setupWaitClose
	func.arg = speed
	return false
end

function setupClose(self, speed)
	local sound
	local sector
	local func
	if speed == 8 then
		sound = "dsbdcls"
	else
		sound = "dsdorcls"
	end
	sector = self.sector
	func = sector.GenericCeiling(sector.floorheight, speed, 0, sound)
	func.crush = setupOpen
	func.arg = speed
end

function setupWaitClose(sector, speed)
	sector.GenericCallerCeiling(35*4, setupClose, speed)
end

function setupDoorOpenWaitClose(sector, config)
	local spec
	local speed
	local sound0
	local sound1
	local func
	spec = config[2]
	func = sector.funcCeiling
	if spec < 100 then
		sound0 = "dsdoropn"
		sound1 = "dsdorcls"
		speed = 2
	else
		sound0 = "dsbdopn"
		sound1 = "dsbdcls"
		speed = 8
	end
	if func ~= nil then
		if (func.action == setupWaitClose or func.isCaller) and config[1].player ~= nil and config[3] then
			func = sector.GenericCeiling(sector.floorheight, speed, 0, sound1)
			func.crush = setupOpen
			func.arg = speed
			return true
		end
		if not func.isLowering or config[1].player == nil then
			return false
		end
	end
	func = sector.GenericCeiling(sector.FindLowestCeiling() - 4, speed, 0, sound0)
	func.action = setupWaitClose
	func.arg = speed
	return true
end

function lineDoorOpenWaitClose(mobj, line, side, act)
	local spec
	spec = line.special
	if act ~= doomLineType[spec] or (mobj.player == nil and spec > 4) then
		return false
	end
	local tag
	local config
	config = {mobj, spec, false}
	tag = line.tag
	if tag ~= 0 then
		sectorTagIterator(tag, setupDoorOpenWaitClose, config)
	else
		config[3] = true
		if not setupDoorOpenWaitClose(line.backsector, config) then
			return false
		end
	end
	if not doomLineRe[spec] then
		line.special = 0
	end
	line.DoButton("dsswtchn", "dsswtchn")
	return true
end

linefunc[1] = lineDoorOpenWaitClose
doomLineType[1] = lnspec.use
doomLineRe[1] = true

linefunc[117] = lineDoorOpenWaitClose
doomLineType[117] = lnspec.use
doomLineRe[117] = true

linefunc[63] = lineDoorOpenWaitClose
doomLineType[63] = lnspec.use
doomLineRe[63] = true

linefunc[114] = lineDoorOpenWaitClose
doomLineType[114] = lnspec.use
doomLineRe[114] = true

linefunc[29] = lineDoorOpenWaitClose
doomLineType[29] = lnspec.use
doomLineRe[29] = false

linefunc[111] = lineDoorOpenWaitClose
doomLineType[111] = lnspec.use
doomLineRe[111] = false

linefunc[90] = lineDoorOpenWaitClose
doomLineType[90] = lnspec.cross
doomLineRe[90] = true

linefunc[105] = lineDoorOpenWaitClose
doomLineType[105] = lnspec.cross
doomLineRe[105] = true

linefunc[4] = lineDoorOpenWaitClose
doomLineType[4] = lnspec.cross
doomLineRe[4] = false

linefunc[108] = lineDoorOpenWaitClose
doomLineType[108] = lnspec.cross
doomLineRe[108] = false

-- Door: open, stay

function setupDoorOpenStay(sector, config)
	if sector.funcCeiling ~= nil then
		return false
	end
	local spec
	local speed
	local sound
	local sound1
	local z
	spec = config[2]
	if spec < 104 and spec ~= 99 then
		sound = "dsdoropn"
		speed = 2
	else
		sound = "dsbdopn"
		speed = 8
	end
	z = sector.FindLowestCeiling() - 4
	if sector.ceilingheight ~= z then
		if sector.ceilingheight < z then
			sector.GenericCeiling(z, speed, 0, sound)
		end
		return true
	end
	return false
end

function lineDoorOpenStay(mobj, line, side, act)
	local spec
	spec = line.special
	if act ~= doomLineType[spec] or mobj.player == nil then
		return false
	end
	local tag
	local config
	config = {mobj, spec}
	tag = line.tag
	if tag ~= 0 then
		sectorTagIterator(tag, setupDoorOpenStay, config)
	else
		if not setupDoorOpenStay(line.backsector, config) then
			return false
		end
	end
	if not doomLineRe[spec] then
		line.special = 0
	end
	line.DoButton("dsswtchn", "dsswtchn")
	return true
end

linefunc[31] = lineDoorOpenStay
doomLineType[31] = lnspec.use
doomLineRe[31] = false

linefunc[118] = lineDoorOpenStay
doomLineType[118] = lnspec.use
doomLineRe[118] = false

linefunc[61] = lineDoorOpenStay
doomLineType[61] = lnspec.use
doomLineRe[61] = true

linefunc[115] = lineDoorOpenStay
doomLineType[115] = lnspec.use
doomLineRe[115] = true

linefunc[103] = lineDoorOpenStay
doomLineType[103] = lnspec.use
doomLineRe[103] = false

linefunc[112] = lineDoorOpenStay
doomLineType[112] = lnspec.use
doomLineRe[112] = false

linefunc[86] = lineDoorOpenStay
doomLineType[86] = lnspec.cross
doomLineRe[86] = true

linefunc[106] = lineDoorOpenStay
doomLineType[106] = lnspec.cross
doomLineRe[106] = true

linefunc[2] = lineDoorOpenStay
doomLineType[2] = lnspec.cross
doomLineRe[2] = false

linefunc[109] = lineDoorOpenStay
doomLineType[109] = lnspec.cross
doomLineRe[109] = false

linefunc[46] = lineDoorOpenStay
doomLineType[46] = lnspec.hit
doomLineRe[46] = true

-- Door: close, stay

function setupDoorCloseStay(sector, config)
	if sector.funcCeiling ~= nil then
		return false
	end
	local spec
	local speed
	local sound
	spec = config[2]
	if spec < 100 then
		sound = "dsdorcls"
		speed = 2
	else
		sound = "dsbdcls"
		speed = 8
	end
	sector.GenericCeiling(sector.floorheight, speed, 0, sound)
	return true
end

function lineDoorCloseStay(mobj, line, side, act)
	local spec
	spec = line.special
	if act ~= doomLineType[spec] or mobj.player == nil then
		return false
	end
	local tag
	local config
	config = {mobj, spec}
	tag = line.tag
	if tag ~= 0 then
		sectorTagIterator(tag, setupDoorCloseStay, config)
	else
		if not setupDoorCloseStay(line.backsector, config) then
			return false
		end
	end
	if not doomLineRe[spec] then
		line.special = 0
	end
	line.DoButton("dsswtchn", "dsswtchn")
	return true
end

linefunc[42] = lineDoorCloseStay
doomLineType[42] = lnspec.use
doomLineRe[42] = true

linefunc[116] = lineDoorCloseStay
doomLineType[116] = lnspec.use
doomLineRe[116] = true

linefunc[50] = lineDoorCloseStay
doomLineType[50] = lnspec.use
doomLineRe[50] = false

linefunc[113] = lineDoorCloseStay
doomLineType[113] = lnspec.use
doomLineRe[113] = false

linefunc[75] = lineDoorCloseStay
doomLineType[75] = lnspec.cross
doomLineRe[75] = true

linefunc[107] = lineDoorCloseStay
doomLineType[107] = lnspec.cross
doomLineRe[107] = true

linefunc[3] = lineDoorCloseStay
doomLineType[3] = lnspec.cross
doomLineRe[3] = false

linefunc[110] = lineDoorCloseStay
doomLineType[110] = lnspec.cross
doomLineRe[110] = false

-- Door: close, wait, open

function setupOpenAfterClose(self, speed)
	local sector
	sector = self.sector
	sector.GenericCeiling(sector.FindLowestCeiling() - 4, speed, 0, "dsdoropn")
end

function setupWaitOpen(sector, speed)
	sector.GenericCallerCeiling(35*30, setupOpenAfterClose, speed)
end

function setupDoorCloseWaitOpen(sector)
	if sector.funcCeiling ~= nil then
		return false
	end
	local speed
	local func
	speed = 2
	func = sector.GenericCeiling(sector.floorheight, speed, 0, "dsdorcls")
	func.action = setupWaitOpen
	func.arg = speed
	return true
end

function lineDoorCloseWaitOpen(mobj, line, side, act)
	local spec
	spec = line.special
	if act ~= doomLineType[spec] or mobj.player == nil then
		return false
	end
	local tag
	tag = line.tag
	if tag ~= 0 then
		sectorTagIterator(tag, setupDoorCloseWaitOpen)
	else
		if not setupDoorCloseWaitOpen(line.backsector) then
			return false
		end
	end
	if not doomLineRe[spec] then
		line.special = 0
	end
	line.DoButton("dsswtchn", "dsswtchn")
	return true
end

linefunc[76] = lineDoorCloseWaitOpen
doomLineType[76] = lnspec.cross
doomLineRe[76] = true

linefunc[16] = lineDoorCloseWaitOpen
doomLineType[16] = lnspec.cross
doomLineRe[16] = false

-- Door: locked types

function lineDoorLockedOpenWaitClose(mobj, line, side, act)
	local spec
	spec = line.special
	if act ~= doomLineType[spec] or mobj.player == nil then
		return false
	end
	local key
	local msg
	if spec == 26 then
		key = mobj.InventoryCheck(MT_BLUECARD) > 0 or mobj.InventoryCheck(MT_BLUESKULL) > 0
		msg = "You need a blue key to open this door."
	elseif spec == 27 then
		key = mobj.InventoryCheck(MT_YELLOWCARD) > 0 or mobj.InventoryCheck(MT_YELLOWSKULL) > 0
		msg = "You need a yellow key to open this door."
	else
		key = mobj.InventoryCheck(MT_REDCARD) > 0 or mobj.InventoryCheck(MT_REDSKULL) > 0
		msg = "You need a red key to open this door."
	end
	if not key then
		mobj.player.Message(msg)
		mobj.SoundBody("dsnoway")
		return false
	end
	return lineDoorOpenWaitClose(mobj, line, side, act)
end

function lineDoorLockedOpenStay(mobj, line, side, act)
	local spec
	spec = line.special
	if act ~= doomLineType[spec] or mobj.player == nil then
		return false
	end
	local key
	local msg
	if spec == 99 or spec == 133 or spec == 32 then
		key = mobj.InventoryCheck(MT_BLUECARD) > 0 or mobj.InventoryCheck(MT_BLUESKULL) > 0
		msg = "You need a blue key to activate this object."
	elseif spec == 136 or spec == 137 or spec == 34 then
		key = mobj.InventoryCheck(MT_YELLOWCARD) > 0 or mobj.InventoryCheck(MT_YELLOWSKULL) > 0
		msg = "You need a yellow key to activate this object."
	else
		key = mobj.InventoryCheck(MT_REDCARD) > 0 or mobj.InventoryCheck(MT_REDSKULL) > 0
		msg = "You need a red key to activate this object."
	end
	if not key then
		mobj.player.Message(msg)
		mobj.SoundBody("dsnoway")
		return false
	end
	lineDoorOpenStay(mobj, line, side, act)
end

linefunc[26] = lineDoorLockedOpenWaitClose
doomLineType[26] = lnspec.use
doomLineRe[26] = true

linefunc[27] = lineDoorLockedOpenWaitClose
doomLineType[27] = lnspec.use
doomLineRe[27] = true

linefunc[28] = lineDoorLockedOpenWaitClose
doomLineType[28] = lnspec.use
doomLineRe[28] = true

linefunc[32] = lineDoorLockedOpenStay
doomLineType[32] = lnspec.use
doomLineRe[32] = false

linefunc[33] = lineDoorLockedOpenStay
doomLineType[33] = lnspec.use
doomLineRe[33] = false

linefunc[34] = lineDoorLockedOpenStay
doomLineType[34] = lnspec.use
doomLineRe[34] = false

linefunc[99] = lineDoorLockedOpenStay
doomLineType[99] = lnspec.use
doomLineRe[99] = true

linefunc[133] = lineDoorLockedOpenStay
doomLineType[133] = lnspec.use
doomLineRe[133] = false

linefunc[134] = lineDoorLockedOpenStay
doomLineType[134] = lnspec.use
doomLineRe[134] = true

linefunc[135] = lineDoorLockedOpenStay
doomLineType[135] = lnspec.use
doomLineRe[135] = false

linefunc[136] = lineDoorLockedOpenStay
doomLineType[136] = lnspec.use
doomLineRe[136] = true

linefunc[137] = lineDoorLockedOpenStay
doomLineType[137] = lnspec.use
doomLineRe[137] = false

