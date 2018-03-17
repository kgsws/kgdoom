-- kgsws' Lua Doom exports
-- key pickups

function pickupKey(mobj, spec, arg)
	if mobj.InventoryCheck(spec.info) > 0 then
		return pickup.doNotPickup
	end
	mobj.InventoryGive(spec.info)
	mobj.player.Message(arg)
	return pickup.key
end

-- blue key card
mtype = {
	activeSound = "dsitemup",
	maxcount = 1,
	action = pickupKey,
	arg = "Picked up a blue keycard.",
	ednum = 5,
	radius = 20,
	height = 8,
	icon = "STKEYS0",
	__special = true,
	__notInDeathmatch = true,
	_spawn = {
		{"BKEYA", 10},
		{"*BKEYB", 10},
		"loop"
	}
}
MT_BLUECARD = createMobjType(mtype)

-- yellow key card
mtype = {
	activeSound = "dsitemup",
	maxcount = 1,
	action = pickupKey,
	arg = "Picked up a yellow keycard.",
	ednum = 6,
	radius = 20,
	height = 8,
	icon = "STKEYS1",
	__special = true,
	__notInDeathmatch = true,
	_spawn = {
		{"YKEYA", 10},
		{"*YKEYB", 10},
		"loop"
	}
}
MT_YELLOWCARD = createMobjType(mtype)

-- red key card
mtype = {
	activeSound = "dsitemup",
	maxcount = 1,
	action = pickupKey,
	arg = "Picked up a red keycard.",
	ednum = 13,
	radius = 20,
	height = 8,
	icon = "STKEYS2",
	__special = true,
	__notInDeathmatch = true,
	_spawn = {
		{"RKEYA", 10},
		{"*RKEYB", 10},
		"loop"
	}
}
MT_REDCARD = createMobjType(mtype)

-- red skull
mtype = {
	activeSound = "dsitemup",
	maxcount = 1,
	action = pickupKey,
	arg = "Picked up a red skull key.",
	ednum = 38,
	radius = 20,
	height = 8,
	icon = "STKEYS5",
	__special = true,
	__notInDeathmatch = true,
	_spawn = {
		{"RSKUA", 10},
		{"*RSKUB", 10},
		"loop"
	}
}
MT_REDSKULL = createMobjType(mtype)

-- yellow skull
mtype = {
	activeSound = "dsitemup",
	maxcount = 1,
	action = pickupKey,
	arg = "Picked up a yellow skull key.",
	ednum = 39,
	radius = 20,
	height = 8,
	icon = "STKEYS4",
	__special = true,
	__notInDeathmatch = true,
	_spawn = {
		{"YSKUA", 10},
		{"*YSKUB", 10},
		"loop"
	}
}
MT_YELLOWSKULL = createMobjType(mtype)

-- blue skull
mtype = {
	activeSound = "dsitemup",
	maxcount = 1,
	action = pickupKey,
	arg = "Picked up a blue skull key.",
	ednum = 40,
	radius = 20,
	height = 8,
	icon = "STKEYS3",
	__special = true,
	__notInDeathmatch = true,
	_spawn = {
		{"BSKUA", 10},
		{"*BSKUB", 10},
		"loop"
	}
}
MT_BLUESKULL = createMobjType(mtype)

addKeyType(MT_BLUECARD)
addKeyType(MT_YELLOWCARD)
addKeyType(MT_REDCARD)
addKeyType(MT_BLUESKULL)
addKeyType(MT_YELLOWSKULL)
addKeyType(MT_REDSKULL)

