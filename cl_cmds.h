// clientside commands
// by kgsws

extern int local_player_predict;

//
// server command parsers

void CL_CmdConnected();
void CL_CmdDisconnected();
void CL_CmdChangeSector();
void CL_CmdChangeSidedef();
void CL_CmdSpawnMobj();
void CL_CmdSpawnPlayer();
void CL_CmdCeiling();
void CL_CmdFloor();
void CL_CmdPlayerInfo();
void CL_CmdUpdateMobj();
void CL_CmdRemoveMobj();
void CL_CmdDamageMobj();
void CL_CmdPlayerPickup();
void CL_CmdPlayerInventory();
void CL_CmdPlayerMessage();
void CL_CmdSound();

//
// client commands

void CL_Connect();
void CL_Disconnect();
void CL_Loaded();
void CL_RequestResend(uint32_t last);
void CL_KeepAlive();
void CL_Join(int team);
void CL_TicCmd(ticcmd_t *cmd);

//
// prediction

void CL_PredictPlayer();

