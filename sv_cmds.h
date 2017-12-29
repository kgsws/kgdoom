// serverside commands
// by kgsws

int disable_player_think;

//
// client command parsers

void SV_CmdConnected();
int SV_CmdPacketLoss();
void SV_CmdLoaded(uint32_t check);
void SV_CmdJoin();
void SV_CmdTick();

//
// server commands

void SV_Disconnect(int cl, const char *msg);
void SV_KeepAlive(int cl);
void SV_ChangeSector(sector_t *sec, uint16_t info);
void SV_ChangeSidedef(side_t *side, uint8_t info);
void SV_SpawnMobj(mobj_t *mobj, uint32_t info);
void SV_SpawnPlayer(int plnum);

void SV_SectorDoor(vldoor_t *door);
void SV_SectorCeiling(ceiling_t *ceiling);
void SV_SectorFloor(floormove_t *floor);
void SV_SectorPlatform(plat_t *plat);

void SV_UpdateLocalPlayer(int cl);
void SV_UpdateOtherPlayers(int cl);
void SV_UpdateMobj(mobj_t *mo, uint32_t info);
void SV_RemoveMobj(mobj_t *mo);
void SV_PlayerPickup(player_t *pl, mobj_t *mo);
void SV_PlayerInventory(player_t *pl);
void SV_PlayerMessage(int pl, const char *msg, int sound);

void SV_StartLineSound(line_t *line, int sound);

