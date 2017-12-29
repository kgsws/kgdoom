#ifndef __P_INTER__
#define __P_INTER__


#ifdef __GNUG__
#pragma interface
#endif


boolean P_GiveArmor(player_t* player, int armortype);
boolean P_GivePower(player_t* player, int power);
boolean P_GiveAmmo(player_t* player, ammotype_t	ammo, int num);


#endif

