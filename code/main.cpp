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

/* RE notes:
  
  1) Pause world updates (call "CCryAction::PauseGame")
    There's console variable (CVar) named "wh_ui_InventoryPauseEnabled"
    If it's set then world updates are paused when inventory menu is open
    
    The way WH implemented this pause seems a bit weird, but this is how:
      1. Get (CVar *) to CVar named "t_GameScale"
        1.1. Obtain CXConsole object through global pointer (for 1.9.5-404-503 it's RVA:0x02886028)
        1.2. Call CXConsole::GetCVar through vftable (method offset is 0xB8)
      The result is (CXConsoleVariableFloatRef *) which is (CXNumericConsoleVariable<float&>)?
      2. Set new value for "t_GameScale"
        2.1. Call CXConsoleVariableFloatRef::Set(float)
    
    Now the weird thing about it, is that there's another CVar named "wh_ui_InventoryPauseRatio"
    Description says: "Inverse ratio of the inventory pause.
      The game time scale will get divided by this number when the inventory is entered."
    And what's really happening is that "game time scale" is a constant value set to 1.0f
    So, they perform (1.0 / "wh_ui_InventoryPauseRatio") and then set the result of that
      to be a new "t_GameScale" value
    It's clear that there is no such value of "wh_ui_InventoryPauseRatio" (except +infinity)
      that will result in "t_GameScale" value to be 0
    Which means that there's no way to actually "pause" world updates unless very small numbers
      are simply interpreted as 0 or something like that
    It seems they use 5000.0f for "wh_ui_InventoryPauseRatio" value (although it's default is 1000.0f?)
      so "t_GameScale" is set to ~0.0002
    
    Nevertheless, one of possible ways to pause world updates is to set CVar "t_GameScale" to 0
    But maybe there's a better solution like setting some variable to true\false which stops Update calls
      on world objects or something (let's call this an "UnrealEngine3 way" since I know it's what it does)
    
    Found a better solution...
    There's "CCryAction::PauseGame" virtual function (0x70) that can be used to pause\unpause game updates
    Pause menu (so called "ingame menu") is using this method to pause\unpause game
    We can get "CCryAction" object through (void **)(RVA:033F1740) or a function call (RVA:007D7B2C)
      and at offset (0x08) of result structure
    
    Here's signature:
    
    void __fastcall CCryAction::PauseGame
    (CCryAction *this, bool pause, int channel, int unk04);
    
    WH use channel(?) 0x07, but it seems we can use whatever
      as long as pause\unpause pair is using the same value
    unk04 should be 0x0, no idea what that is and don't care
    
  2) Hook inventory open (hook RVA:008C7F8C)
    Object "C_ScriptBindActor" has a virtual function "C_ScriptBindActor::OpenInventory" (RVA:00722B7C)
    This function is called from a LUA object when someone "opens an inventory"
    Problem is that we have to make sure that "someone" is a player
    We could call "C_ScriptBindActor::GetActor" (RVA:00722934) to get actor who's requested it
    "C_ScriptBindActor::OpenInventory" actually does this as it's first call
    Though I think it's better to hook "C_UIMenuEvents::OpenInventory" (RVA:008C7F8C) which is an actual
      "OpenInventory" call
    
    Here's some info on that:
    
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
    
    void __fastcall C_UIMenuEvents::OpenInventory
    (C_UIMenuEvents *this, C_Actor *actor, InventoryMode mode, u64 unk04, u64 inventoryID, u64 unk06);
    
    That way we have some extra things we could check on, like which invenvory mode is requested
    (although that info we could've got from CFunctionHandler since these are args passed from LUA)
    Nonetheless, this is what you call to "open inventory" and it's called from a few other places
      not only from within a LUA call, so more things to catch that way
    
    About that actor check...
    There's a separate class for a player "C_Player" inherited from "C_Human"
      which is inherited from "C_Actor"
    So that check is simply done by comparing vftables or RTTI info
    
    We should ignore E_IM_Player, E_IM_Map since WH manipulates with "t_GameScale" on them
    
    We can also use padding in "C_UIMenuEvents" at [0x6A-0x6F] (6 bytes) to store our data
    
  3) Hook inventory close (hook RVA:008C5ED4)
    There's a call to LUA function named "OnInventoryClosed" which is called at closing inventory
    Call is made from a subclass (offset 0x60) of "C_ScriptBindActor" virtual function (0x08)
    This function gets a "CScriptTable" object and calls "CScriptTable::CallFunction"(?) with
      a function name "OnInventoryClosed" and a pointer to store some return(?) value as params
    At the top they check a boolean passed as an argument to see if call to that LUA object is needed
    Not enough space to hook, so just replace entry in vftable (it's easier anyways)
    VFTable address: RVA:021BD6B8 (1.9.5-404-503)
    
    There's another place we can hook actually...
    "C_UIMenuEvents" object has a callback(?) function "C_UIMenuEvents::OnClosedInventory" that's
      being called when any inventory menu is closed which is technically what we need instead
    Hooking this function is a bit problematic, have to overwrite 2 function calls (doable)
    "C_UIMenuEvents::OnClosedInventory" (RVA:008C6648)
    
    Nah... Let's hook "C_UIMenuEvents::NotifyInventoryClosed" (RVA:008C5ED4)
    Though it's being called from a loading screen, but shouldn't be a problem
    
    We probably have to store some info in "C_UIMenuEvents" so we know should we unpause or not
*/

#include <windows.h>

#include "types.h"
#include "detours.cpp"

internal char *baseModuleName = "whgame.dll";
external void *baseAddress = 0;

#include "native_types.h"
#include "hooks.cpp"

internal INLINE void * RVA(u64 offset)
{
  if (!baseAddress) {
    return 0;
  }
  
  return (void *)((u64)baseAddress + offset);
}

internal bool DefineAddresses()
{
  GetWHStaticsBundle = (GetWHStaticsBundle_t)RVA(0x007D7B2C);
  if (!GetWHStaticsBundle) {
    return false;
  }
  
  OpenInventory_Address = RVA(0x008C7F8C);
  if (!OpenInventory_Address) {
    return false;
  }
  
  NotifyInventoryClosed_Address = RVA(0x008C5ED4);
  if (!NotifyInventoryClosed_Address) {
    return false;
  }
  
  return true;
}

internal bool InjectHooks()
{
  if (!WriteDetour64(OpenInventory_Address, &OpenInventory_Hook, 1)) {
    return false;
  }
  
  if (!WriteDetour64(NotifyInventoryClosed_Address, &NotifyInventoryClosed_Hook, 3)) {
    return false;
  }
  
  return true;
}

internal bool Initialize()
{
  if (!DefineAddresses()) {
    OutputDebugStringA("DefineAddresses failed");
    return false;
  }
  
  if (!InjectHooks()) {
    OutputDebugStringA("InjectHooks failed");
    return false;
  }
  
  return true;
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
  if (reason == DLL_PROCESS_ATTACH) {
    baseAddress = (void *)GetModuleHandle(baseModuleName);
    if (!baseAddress) {
      OutputDebugStringA("GetModuleHandle returned NULL");
      return FALSE;
    }
    
    if (!Initialize()) {
      return FALSE;
    }
  }
  
  return TRUE;
}
