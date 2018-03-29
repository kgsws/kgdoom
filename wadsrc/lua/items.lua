-- kgsws' Lua Doom exports
-- pickup definitions

-- power IDs for tickers; random numbers
pw_invulnerability = 1354
pw_invisibility = 9423
pw_ironfeet = 5712
pw_infrared = 9321

function pickupBackpack(mobj, spec, arg)
	local left
	left = mobj.InventoryGive(spec.info)
	if left == 0 then
		mobj.InventorySetMax(MT_CLIP, MT_CLIP.maxcount * 2)
		mobj.InventorySetMax(MT_SHELL, MT_SHELL.maxcount * 2)
		mobj.InventorySetMax(MT_ROCKETAMMO, MT_ROCKETAMMO.maxcount * 2)
		mobj.InventorySetMax(MT_CELL, MT_CELL.maxcount * 2)
	end
	mobj.InventoryGive(MT_CLIP, MT_CLIP.arg[2])
	mobj.InventoryGive(MT_SHELL, MT_SHELL.arg[2])
	mobj.InventoryGive(MT_ROCKETAMMO, MT_ROCKETAMMO.arg[2])
	mobj.InventoryGive(MT_CELL, MT_CELL.arg[2])
	mobj.player.Message("Picked up a backpack full of ammo!")
	return pickup.item
end

function pickupMegasphere(mobj, spec, arg)
	mobj.armor = 200
	mobj.armortype = MT_BLUEARMOR
	mobj.health = mobj.info.health * 2
	mobj.player.Message("MegaSphere!")
	return pickup.power
end

function pickupHealth(mobj, spec, arg)
	local hp
	hp = mobj.health
	if hp >= mobj.info.health then
		return pickup.doNotPickup
	end
	hp = hp + spec.health
	if hp > 100 then
		mobj.health = 100
	else
		mobj.health = hp
	end
	mobj.player.Message(arg)
	return pickup.item
end

function pickupSoulSphere(mobj, spec, arg)
	local hp
	local hpp
	hpp = mobj.info.health
	hp = mobj.health + hpp
	if hp > hpp * 2 then
		mobj.health = hpp * 2
	else
		mobj.health = hp
	end
	mobj.player.Message("Supercharge!")
	return pickup.power
end

function pickupHealthBonus(mobj, spec, arg)
	local hp
	local hpp
	hpp = mobj.info.health
	hp = mobj.health + 1
	if hp > hpp * 2 then
		mobj.health = hpp * 2
	else
		mobj.health = hp
	end
	mobj.player.Message("Picked up a health bonus.")
	return pickup.item
end

function pickupArmorBonus(mobj, spec, arg)
	local ar
	ar = mobj.armor + 1
	if ar > 200 then
		mobj.armor = 200
	else
		mobj.armor = ar
	end
	if mobj.armortype == nil then
		mobj.armortype = MT_GREENARMOR
	end
	mobj.player.Message("Picked up an armor bonus.")
	return pickup.item
end

function pickupArmorGreen(mobj, spec, arg)
	if mobj.armor >= 100 then
		return pickup.doNotPickup
	end
	mobj.armor = 100
	mobj.armortype = MT_GREENARMOR
	mobj.player.Message("Picked up the armor.")
	return pickup.item
end

function pickupArmorBlue(mobj, spec, arg)
	if mobj.armortype == MT_BLUEARMOR and mobj.armor == 200 then
		return pickup.doNotPickup
	end
	mobj.armor = 200
	mobj.armortype = MT_BLUEARMOR
	mobj.player.Message("Picked up the MegaArmor!")
	return pickup.item
end

function removeSuit(mobj)
	mobj.DamageScale(1, 1)
end

function pickupSuit(mobj, spec, arg)
	mobj.DamageScale(1, 0)
	mobj.player.Message("Radiation Shielding Suit")
	mobj.TickerSet(pw_ironfeet, 60*35, "SUITA0", removeSuit)
	return pickup.power
end

function pickupMap(mobj, spec, arg)
	mobj.player.Message("Computer Area Map")
	mobj.player.map = 1
end

function removeInvuln(mobj)
	mobj.__invulnerable = false
	if mobj.TickerCheck(pw_infrared) then
		mobj.player.colormap = "COLORMAP:1"
	else
		mobj.player.colormap = "-"
	end
end

function pickupInvuln(mobj, spec, arg)
	mobj.__invulnerable = true
	mobj.player.Message("Invulnerability!")
	mobj.TickerSet(pw_invulnerability, 30*35, "PINVA0", removeInvuln)
	mobj.player.colormap = "COLORMAP:32"
	return pickup.power
end

function removeInvis(mobj)
	mobj.render = "!NORMAL"
end

function pickupInvis(mobj, spec, arg)
	mobj.render = "!SHADOW"
	mobj.player.Message("Partial Invisibility")
	mobj.TickerSet(pw_invisibility, 60*35, "PINSA0", removeInvis)
	return pickup.power
end

function removeVisor(mobj)
	mobj.player.Message("lights out")
	if not mobj.TickerCheck(pw_invulnerability) then
		mobj.player.colormap = "-"
	end
end

function pickupVisor(mobj, spec, arg)
	mobj.player.Message("Light Amplification Visor")
	mobj.TickerSet(pw_infrared, 120*35, "PVISA0", removeVisor)
	if not mobj.TickerCheck(pw_invulnerability) then
		mobj.player.colormap = "COLORMAP:1"
	end
	return pickup.power
end

function pickupBerserk(mobj, spec, arg)
	mobj.InventoryGive(spec.info)
	mobj.player.Message("Berserk!")
	mobj.player.SetWeapon(MT_FIST)
	if mobj.health < mobj.info.health then
		mobj.health = mobj.info.health
	end
	return pickup.power
end

-- backpack
mtype = {
	activeSound = "dsitemup",
	action = pickupBackpack,
	maxcount = 1,
	ednum = 8,
	radius = 20,
	height = 16,
	__special = true,
	_spawn = {
		{"BPAKA", -1}
	}
}
MT_BACKPACK = createMobjType(mtype)

-- megasphere
mtype = {
	activeSound = "dsgetpow",
	action = pickupMegasphere,
	ednum = 83,
	radius = 20,
	height = 16,
	__special = true,
	__countItem = true,
	_spawn = {
		{"*MEGAA", 6},
		{"*MEGAB", 6},
		{"*MEGAC", 6},
		{"*MEGAD", 6},
		"loop"
	}
}
createMobjType(mtype)

-- stimpack
mtype = {
	activeSound = "dsitemup",
	action = pickupHealth,
	arg = "Picked up a stimpack.",
	health = 10,
	ednum = 2011,
	radius = 20,
	height = 16,
	__special = true,
	_spawn = {
		{"STIMA", -1}
	}
}
createMobjType(mtype)

-- medikit
mtype = {
	activeSound = "dsitemup",
	action = pickupHealth,
	arg = "Picked up a medikit.",
	health = 25,
	ednum = 2012,
	radius = 20,
	height = 16,
	__special = true,
	_spawn = {
		{"MEDIA", -1}
	}
}
createMobjType(mtype)

-- soulsphere
mtype = {
	activeSound = "dsgetpow",
	action = pickupSoulSphere,
	ednum = 2013,
	radius = 20,
	height = 16,
	__special = true,
	__countItem = true,
	_spawn = {
		{"*SOULA", 6},
		{"*SOULB", 6},
		{"*SOULC", 6},
		{"*SOULD", 6},
		{"*SOULC", 6},
		{"*SOULB", 6},
		"loop"
	}
}
createMobjType(mtype)

-- health bonus
mtype = {
	activeSound = "dsitemup",
	action = pickupHealthBonus,
	ednum = 2014,
	radius = 20,
	height = 16,
	__special = true,
	__countItem = true,
	_spawn = {
		{"BON1A", 6},
		{"BON1B", 6},
		{"BON1C", 6},
		{"BON1D", 6},
		{"BON1C", 6},
		{"BON1B", 6},
		"loop"
	}
}
createMobjType(mtype)

-- armor bonus
mtype = {
	activeSound = "dsitemup",
	action = pickupArmorBonus,
	ednum = 2015,
	radius = 20,
	height = 16,
	__special = true,
	__countItem = true,
	_spawn = {
		{"BON2A", 6},
		{"BON2B", 6},
		{"BON2C", 6},
		{"BON2D", 6},
		{"BON2C", 6},
		{"BON2B", 6},
		"loop"
	}
}
createMobjType(mtype)

-- green armor
mtype = {
	activeSound = "dsitemup",
	action = pickupArmorGreen,
	damage = 33,
	ednum = 2018,
	radius = 20,
	height = 16,
	__special = true,
	_spawn = {
		{"ARM1A", 6},
		{"*ARM1B", 6},
		"loop"
	}
}
MT_GREENARMOR = createMobjType(mtype)

-- blue armor
mtype = {
	activeSound = "dsitemup",
	action = pickupArmorBlue,
	damage = 50,
	ednum = 2019,
	radius = 20,
	height = 16,
	__special = true,
	_spawn = {
		{"ARM2A", 6},
		{"*ARM2B", 6},
		"loop"
	}
}
MT_BLUEARMOR = createMobjType(mtype)

-- invulnerability
mtype = {
	activeSound = "dsgetpow",
	action = pickupInvuln,
	ednum = 2022,
	radius = 20,
	height = 16,
	__special = true,
	__countItem = true,
	_spawn = {
		{"*PINVA", 6},
		{"*PINVB", 6},
		{"*PINVC", 6},
		{"*PINVD", 6},
		"loop"
	}
}
MT_INVULN = createMobjType(mtype)

-- berserk
mtype = {
	activeSound = "dsgetpow",
	action = pickupBerserk,
	maxcount = 1,
	ednum = 2023,
	radius = 20,
	height = 16,
	__special = true,
	__countItem = true,
	_spawn = {
		{"*PSTRA", -1}
	}
}
MT_BERSERK = createMobjType(mtype)

-- invisibility
mtype = {
	activeSound = "dsgetpow",
	action = pickupInvis,
	ednum = 2024,
	radius = 20,
	height = 16,
	__special = true,
	__countItem = true,
	_spawn = {
		{"*PINSA", 6},
		{"*PINSB", 6},
		{"*PINSC", 6},
		{"*PINSD", 6},
		"loop"
	}
}
createMobjType(mtype)

-- suit
mtype = {
	activeSound = "dsgetpow",
	action = pickupSuit,
	ednum = 2025,
	radius = 20,
	height = 16,
	__special = true,
	_spawn = {
		{"*SUITA", -1}
	}
}
createMobjType(mtype)

-- map
mtype = {
	activeSound = "dsgetpow",
	action = pickupMap,
	ednum = 2026,
	radius = 20,
	height = 16,
	__special = true,
	_spawn = {
		{"*PMAPA", 6},
		{"*PMAPB", 6},
		{"*PMAPC", 6},
		{"*PMAPD", 6},
		{"*PMAPC", 6},
		{"*PMAPB", 6},
		"loop"
	}
}
createMobjType(mtype)

-- visor
mtype = {
	activeSound = "dsgetpow",
	action = pickupVisor,
	ednum = 2045,
	radius = 20,
	height = 16,
	__special = true,
	_spawn = {
		{"*PVISA", 6},
		{"PVISB", 6},
		"loop"
	}
}
createMobjType(mtype)

