// new pickup handling
// by kgsws

#ifndef __INFO_C__

// palette flash
#define BONUSADD 6

void P_TouchSpecialThing(mobj_t *special, mobj_t *toucher);

#else
//
// all pickup functions

void A_PickGreenArmor();
void A_PickBlueArmor();

void A_PickHealthBonus();
void A_PickArmorBonus();

void A_PickKey();

void A_PickHealth10();
void A_PickHealth25();

void A_PickSoulSphere();
void A_PickMegaSphere();

void A_PickPower();

void A_PickAmmo();

void A_PickBackpack();

void A_PickWeapon();

#endif
