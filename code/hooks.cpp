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

#ifndef _HOOKS_CPP_
#define _HOOKS_CPP_

external void *OpenInventory_Address = 0;
external void *NotifyInventoryClosed_Address = 0;

external void OpenInventory_Hook();
external void NotifyInventoryClosed_Hook();

external void FASTCALL C_UIMenuEvents_OpenInventory
(C_UIMenuEvents *ptr, C_Actor *actor, InventoryMode mode)
{
  switch (mode) {
    case E_IM_Loot:
    case E_IM_Filter:
    case E_IM_Store:
    case E_IM_StoreReadOnly: {
      WHStaticsBundle *bundle = GetWHStaticsBundle();
      CCryAction *action = bundle->cryAction;
      
      action->vtable->PauseGame(action, true, 7, 0);
    } return;
    
    default:
      return;
  }
}

external void FASTCALL C_UIMenuEvents_NotifyInventoryClosed
(C_UIMenuEvents *ptr)
{
  WHStaticsBundle *bundle = GetWHStaticsBundle();
  CCryAction *action = bundle->cryAction;
  
  action->vtable->PauseGame(action, false, 7, 0);
}

#endif
