-- kgsws' Lua Doom exports
-- Doom line actions

-- Light to specific level

function lightChangeValue(sector, level)
	sector.foglevel = level
	return true
end

function lightChangeSpecific(mobj, line, side, act)
	local spec
	spec = line.special
	if act ~= doomLineType[spec] or mobj.player == nil then
		return false
	end
	if not doomLineRe[spec] then
		line.special = 0
	end
	local level
	if spec == 79 or spec == 35 or spec == 139 then
		level = 35
	else
		level = 255
	end
	local tag
	tag = line.tag
	if tag ~= 0 then
		sectorTagIterator(tag, lightChangeValue, level)
	else
		if not lightChangeValue(line.backsector, level) then
			return false
		end
	end
	line.DoButton("dsswtchn", "dsswtchn")
	return true
end

linefunc[139] = lightChangeSpecific
doomLineType[139] = lnspec.use
doomLineRe[139] = true

linefunc[79] = lightChangeSpecific
doomLineType[79] = lnspec.cross
doomLineRe[79] = true

linefunc[35] = lightChangeSpecific
doomLineType[35] = lnspec.cross
doomLineRe[35] = false

linefunc[138] = lightChangeSpecific
doomLineType[138] = lnspec.use
doomLineRe[138] = true

linefunc[81] = lightChangeSpecific
doomLineType[81] = lnspec.cross
doomLineRe[81] = true

linefunc[13] = lightChangeSpecific
doomLineType[13] = lnspec.cross
doomLineRe[13] = false

-- Light to sector level

function lightChangeLow(sector)
	sector.foglevel = sector.FindMinimalFog()
	return true
end

function lightChangeHigh(sector)
	sector.foglevel = sector.FindMaximalFog()
	return true
end

function lightChangeLevel(mobj, line, side, act)
	local spec
	spec = line.special
	if act ~= doomLineType[spec] or mobj.player == nil then
		return false
	end
	if not doomLineRe[spec] then
		line.special = 0
	end
	local func
	if spec == 104 then
		func = lightChangeLow
	else
		func = lightChangeHigh
	end
	local tag
	tag = line.tag
	if tag ~= 0 then
		sectorTagIterator(tag, func)
	else
		if not func(line.backsector) then
			return false
		end
	end
	line.DoButton("dsswtchn", "dsswtchn")
	return true
end

linefunc[80] = lightChangeSpecific
doomLineType[80] = lnspec.cross
doomLineRe[80] = true

linefunc[12] = lightChangeSpecific
doomLineType[12] = lnspec.cross
doomLineRe[12] = false

linefunc[104] = lightChangeSpecific
doomLineType[104] = lnspec.cross
doomLineRe[104] = false

-- Light: Blinking

function lightDoBlink(self, level)
	local sector
	sector = self.sector
	self.arg = sector.foglevel
	sector.foglevel = level
	if self.ticrate == 35 then
		self.ticrate = 5
	else
		self.ticrate = 35
	end
	return true
end

function lightChangeLow(sector)
	if sector.funcCustom ~= nil then
		return false
	end
	local low
	low = sector.FindMinimalLight()
	sector.GenericCaller(35, lightDoBlink, low)
	return true
end

function lightStartBlink(mobj, line, side, act)
	local spec
	spec = line.special
	if act ~= doomLineType[spec] or mobj.player == nil then
		return false
	end
	if not doomLineRe[spec] then
		line.special = 0
	end
	local level
	if spec == 79 or spec == 35 or spec == 139 then
		level = 35
	else
		level = 255
	end
	local tag
	tag = line.tag
	if tag ~= 0 then
		sectorTagIterator(tag, lightChangeValue, level)
	else
		if not lightChangeValue(line.backsector, level) then
			return false
		end
	end
	line.DoButton("dsswtchn", "dsswtchn")
	return true
end

linefunc[17] = lightStartBlink
doomLineType[17] = lnspec.cross
doomLineRe[17] = false

-- Lights from sectors

function lightRandomBlink(self, level)
	local sector
	local oldlevel
	sector = self.sector
	oldlevel = sector.foglevel
	self.arg = oldlevel
	if oldlevel > level then
		self.ticrate = doomRandom(1, 8)
	else
		self.ticrate = doomRandom(1, 64)
	end
	sector.foglevel = level
	return true
end

function lightSlowBlink(self, level)
	local sector
	local oldlevel
	sector = self.sector
	oldlevel = sector.foglevel
	self.arg = oldlevel
	if oldlevel > level then
		self.ticrate = 35
	else
		self.ticrate = 5
	end
	sector.foglevel = level
	return true
end

function lightFastBlink(self, level)
	local sector
	local oldlevel
	sector = self.sector
	oldlevel = sector.foglevel
	self.arg = oldlevel
	if oldlevel > level then
		self.ticrate = 15
	else
		self.ticrate = 5
	end
	sector.foglevel = level
	return true
end

function lightFlickerBlink(self, lolevel)
	local sector
	local hilevel
	local level
	local amount
	sector = self.sector
	hilevel = sector.special
	amount = doomRandom(3) * 16
	level = sector.foglevel
	if level - amount < lolevel then
		sector.foglevel = lolevel
	else
		sector.foglevel = hilevel - amount
	end
	return true
end

function lightGlowBlink(self, level0)
	local sector
	local level1
	local level
	sector = self.sector
	level1 = sector.special
	level = sector.foglevel
	if level0 > level1 then
		level = level - 8
		if level <= level1 then
			level = level1
			self.arg = level1
			sector.special = level0
		end
	else
		level = level + 8
		if level >= level1 then
			level = level1
			self.arg = level1
			sector.special = level0
		end
	end
	sector.foglevel = level
	return true
end

