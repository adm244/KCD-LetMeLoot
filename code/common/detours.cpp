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

//TODO(adm244): this lib is a mess, laddie...

#ifndef _DETOURS_CPP
#define _DETOURS_CPP

#define PSAPI_VERSION 1
#include <psapi.h>

#define DETOUR_LENGTH 5
#define DETOUR_LENGTH_X64 12

internal void * FindWString(void *beginAddress, void *endAddress, wchar_t *str)
{
  size_t stringSize = wcslen(str) * sizeof(wchar_t);
  
  for (char *p = (char *)beginAddress; p < endAddress; ++p) {
    bool matched = true;
    char *str_p = (char *)str;
    
    for (size_t i = 0; i < stringSize; ++i) {
      if (str_p[i] != *(p + i)) {
        matched = false;
        break;
      }
    }
    
    if (matched)
      return p;
  }
  
  return 0;
}

//TODO(adm244): switch to multiple patterns search
internal u64 FindSignature(MODULEINFO *moduleInfo, char *pattern, char *mask, u64 offset)
{
  u64 startAddress = (u64)moduleInfo->lpBaseOfDll;
  u64 blockSize = moduleInfo->SizeOfImage;
  u64 patternSize = strlen(mask ? mask : pattern);
  u64 endAddress = startAddress + blockSize - patternSize;

  for (u64 i = startAddress; i < endAddress; ++i) {
    bool matched = true;
    for (u64 j = 0; j < patternSize; ++j) {
      if (mask && mask[j] == '?') continue;
      if (pattern[j] != *((char *)(i + j))) {
        matched = false;
        break;
      }
    }
    
    if (matched) return (i + offset);
  }
  
  return 0;
}

internal bool WriteMemory(void *dest, void *buffer, int length)
{
  DWORD oldProtection;
  BOOL result = VirtualProtect(dest, length, PAGE_EXECUTE_READWRITE, &oldProtection);
  if (!result) {
    return false;
  }
  
  memcpy(dest, buffer, length);
  
  result = VirtualProtect(dest, length, oldProtection, &oldProtection);
  if (!result) {
    return false;
  }
  
  return true;
}

internal void * RIPRel8(void *src, u8 opcode_length)
{
  i8 offset = *((i8 *)((u64)src + opcode_length));
  u64 rip = (u64)src + opcode_length + sizeof(u8);
  
  return (void *)(rip + offset);
}

internal void * RIPRel32(void *src, u8 opcode_length)
{
  i32 offset = *((i32 *)((u64)src + opcode_length));
  u64 rip = (u64)src + opcode_length + sizeof(u32);
  
  return (void *)(rip + offset);
}

internal i32 RIPRelOffset32(void *rip, void *dest)
{
  return (i32)dest - (i32)rip;
}

/// Assembles a relative 32-bit jump
internal void * JumpRel32(void *src, void *dest)
{
  u8 *p = (u8 *)src;
  u8 *pend = (u8 *)(p + DETOUR_LENGTH);

  // assemble jmp dest
  p[0] = 0xE9;
  *((i32 *)(&p[1])) = RIPRelOffset32(pend, dest);
  
  return pend;
  
}

/// Assembles an absolute indirect 64-bit jump
internal void * JumpAbsInd64(void *src, void *dest)
{
  u8 *p = (u8 *)src;
  u8 *pend = (u8 *)(p + DETOUR_LENGTH_X64);

  // assemble mov rax, qword ptr [dest]
  p[0] = 0x48;
  p[1] = 0xB8;
  *((u64 *)(&p[2])) = (u64)dest;
  
  // assemble jmp rax
  p[10] = 0xFF;
  p[11] = 0xE0;
  
  return pend;
}

/// Writes a detour to dest at src nop'ing extra space
internal bool WriteDetour(void *src, void *dest, int padding)
{
  DWORD oldProtection;
  //NOTE(adm244): funny how this bug never caused any problems since min length is 4KB by default anyway
  // it just never crossed that boundary, but still, let's be on a safe side and add padding size
  BOOL result = VirtualProtect(src, DETOUR_LENGTH + padding, PAGE_EXECUTE_READWRITE, &oldProtection);
  if (!result) {
    return false;
  }
  
  JumpRel32(src, dest);
  
  for (int i = DETOUR_LENGTH; i < (DETOUR_LENGTH + padding); ++i) {
    ((u8 *)src)[i] = 0x90;
  }
  
  result = VirtualProtect(src, DETOUR_LENGTH + padding, oldProtection, &oldProtection);
  if (!result) {
    return false;
  }
  
  return true;
}

internal bool WriteDetour64(void *src, void *dest, int padding)
{
  DWORD oldProtection;
  BOOL result = VirtualProtect(src, DETOUR_LENGTH_X64 + padding, PAGE_EXECUTE_READWRITE, &oldProtection);
  if (!result) {
    return false;
  }
  
  JumpAbsInd64(src, dest);
  
  for (int i = DETOUR_LENGTH_X64; i < (DETOUR_LENGTH_X64 + padding); ++i) {
    ((u8 *)src)[i] = 0x90;
  }
  
  result = VirtualProtect(src, DETOUR_LENGTH_X64 + padding, oldProtection, &oldProtection);
  if (!result) {
    return false;
  }
  
  return true;
}

#endif
