/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

//IMPORTANT(adm244): SCRATCH VERSION JUST TO GET IT UP WORKING

#ifndef _NATIVE_TYPES_H_
#define _NATIVE_TYPES_H_

struct WHStaticsBundle;
struct CCryAction;

//------------- Functions -------------//
typedef WHStaticsBundle * (FASTCALL *GetWHStaticsBundle_t)();
typedef CCryAction * (FASTCALL *CCryAction_PauseGame_t)(CCryAction *ptr, bool pause, int channel, int unk04);

internal GetWHStaticsBundle_t GetWHStaticsBundle = 0;

//------------- Enumerables -------------//
enum InventoryMode {
  E_IM_Player = 0,
  E_IM_Map = 1,
  E_IM_Store = 2,
  E_IM_QuestReward = 3,
  E_IM_QuestDelivery = 4,
  E_IM_Loot = 5,
  E_IM_Shop = 6,
  E_IM_Pickpocket = 7,
  E_IM_StoreReadOnly = 8,
  E_IM_Filter = 10,
  E_IM_Repair = 11,
  E_IM_Sharpening = 14
};

//------------- Structures -------------//
#pragma pack(push, 8)

struct C_UIMenuEvents {
  void *vtable;
};

struct C_Actor {
  void *vtable;
};

struct CCryActionVFTable {
  void *unk00;
  void *unk08;
  void *unk10;
  void *unk18;
  void *unk20;
  void *unk28;
  void *unk30;
  void *unk38;
  void *unk40;
  void *unk48;
  void *unk50;
  void *unk58;
  void *PostUpdate; // 0x60
  void *unk68;
  CCryAction_PauseGame_t *PauseGame; // 0x70
  void *unk78;
  void *unk80;
  void *unk88;
  void *unk90;
  void *unk98;
  void *unk100;
  void *unk108;
  void *unk110;
  void *unk118;
  void *unk120;
  void *GetActorSystem; // 0x128
  void *unk130;
  void *unk138;
  void *GetActionMapManager; // 0x140
  
  u64 unk148[118];
};
assert_size(CCryActionVFTable, 0x498);

struct CCryAction {
  CCryActionVFTable *vtable;
};

struct C_EntityActions {
  void *vtable;
};

struct C_PlayerModule {
  void *vtable;
};

struct WHStaticsBundle {
  void *unk00;
  CCryAction *cryAction; // 0x08
  void *unk10;
  void *unk18;
  void *unk20;
  void *unk28;
  C_EntityActions *entityActions; // 0x30
  void *unk38;
  void *unk40;
  void *unk48;
  void *unk50;
  void *unk58;
  void *unk60;
  void *unk68;
  void *unk70;
  void *unk78;
  void *unk80;
  void *unk88;
  void *unk90;
  void *unk98;
  void *unkA0;
  void *unkA8;
  void *unkB0;
  void *unkB8;
  void *unkC0;
  void *unkC8;
  void *unkD0;
  void *unkD8;
  void *unkE0;
  C_PlayerModule *playerModule; // 0xE8
  void *unkF0;
  void *unkF8;
  void *unk100;
  void *unk108;
  void *unk110;
  void *unk118;
  void *unk120;
  void *unk128;
  void *unk130;
  void *unk138;
  void *unk140;
  void *unk148;
  void *unk150;
  void *unk158;
  void *unk160;
  void *unk168;
};
assert_size(WHStaticsBundle, 0x170);

#pragma pack(pop)

#endif
