-- kgsws' Lua Doom exports
-- Doom ammo

function pickupAmmo(mobj, spec, arg)
	local left
	local count
	if spec.__dropped then
		count = arg[2] / 2
	else
		count = arg[2]
	end
	left = mobj.InventoryGive(arg[1], count)
	if left == arg[2] then
		return pickup.doNotPickup
	end
	mobj.player.Message(arg[3])
	return pickup.ammo
end

-- MT_CLIP
mtype = {
	activeSound = "dsitemup",
	maxcount = -200,
	ednum = 2007,
	radius = 20,
	height = 16,
	action = pickupAmmo,
	icon = "CLIPA0",
	__special = true,
	_spawn = {
		{"CLIPA", -1}
	}
}
MT_CLIP = createMobjType(mtype)
MT_CLIP.arg = {MT_CLIP, 10, "Picked up a clip."}

-- MT_CLIPBOX (custom name)
mtype = {
	activeSound = "dsitemup",
	ednum = 2048,
	radius = 20,
	height = 16,
	action = pickupAmmo,
	__special = true,
	arg = 50,
	_spawn = {
		{"AMMOA", -1}
	}
}
MT_CLIPBOX = createMobjType(mtype)
MT_CLIPBOX.arg = {MT_CLIP, 50, "Picked up a box of bullets."}

-- MT_SHELL (custom name)
mtype = {
	activeSound = "dsitemup",
	maxcount = -50,
	ednum = 2008,
	radius = 20,
	height = 16,
	action = pickupAmmo,
	icon = "SHELA0",
	__special = true,
	_spawn = {
		{"SHELA", -1}
	}
}
MT_SHELL = createMobjType(mtype)
MT_SHELL.arg = {MT_SHELL, 4, "Picked up 4 shotgun shells."}

-- MT_SHELLBOX (custom name)
mtype = {
	activeSound = "dsitemup",
	ednum = 2049,
	radius = 20,
	height = 16,
	action = pickupAmmo,
	__special = true,
	_spawn = {
		{"SBOXA", -1}
	}
}
MT_SHELLBOX = createMobjType(mtype)
MT_SHELLBOX.arg = {MT_SHELL, 20, "Picked up a box of shotgun shells."}

-- MT_ROCKETAMMO (custom name)
mtype = {
	activeSound = "dsitemup",
	maxcount = -50,
	ednum = 2010,
	radius = 20,
	height = 16,
	action = pickupAmmo,
	icon = "ROCKA0",
	__special = true,
	_spawn = {
		{"ROCKA", -1}
	}
}
MT_ROCKETAMMO = createMobjType(mtype)
MT_ROCKETAMMO.arg = {MT_ROCKETAMMO, 1, "Picked up a rocket."}

-- MT_ROCKETBOX (custom name)
mtype = {
	activeSound = "dsitemup",
	ednum = 2046,
	radius = 20,
	height = 16,
	action = pickupAmmo,
	__special = true,
	_spawn = {
		{"BROKA", -1}
	}
}
MT_ROCKETBOX = createMobjType(mtype)
MT_ROCKETBOX.arg = {MT_ROCKETAMMO, 5, "Picked up a box of rockets."}

-- MT_CELL (custom name)
mtype = {
	activeSound = "dsitemup",
	maxcount = -300,
	ednum = 2047,
	radius = 20,
	height = 16,
	action = pickupAmmo,
	icon = "CELLA0",
	__special = true,
	_spawn = {
		{"CELLA", -1}
	}
}
MT_CELL = createMobjType(mtype)
MT_CELL.arg = {MT_CELL, 20, "Picked up an energy cell."}

-- MT_CELLPACK (custom name)
mtype = {
	activeSound = "dsitemup",
	ednum = 17,
	radius = 20,
	height = 16,
	action = pickupAmmo,
	__special = true,
	_spawn = {
		{"CELPA", -1}
	}
}
MT_CELLPACK = createMobjType(mtype)
MT_CELLPACK.arg = {MT_CELL, 100, "Picked up an energy cell pack."}

