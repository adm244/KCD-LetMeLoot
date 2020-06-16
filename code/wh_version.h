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

#ifndef _WH_VERSION_H_
#define _WH_VERSION_H_

// major.minor.patch.build
#define MAKE_VERSION(major, minor, patch) CFG_MAKE_VERSION(major, minor, patch, 0)

//------------- Enumerables -------------//
enum PlatformType_e {
  Platform_Unknown = 0,
  Platform_GOG,
  Platform_STEAM,
};

enum GameVersion_e {
  Game_Unknown = 0,
  Game_NotSupported,
  Game_196,
  Game_195,
  Game_194,
  Game_193,
  Game_192,
  //Game_191_390CC,
  //Game_190
};

//------------- Structures -------------//
struct GameInfo_t {
  PlatformType_e platform;
  GameVersion_e version;
};

//------------- Global Data -------------//
internal GameInfo_t g_GameInfo = {
  (PlatformType_e)Platform_Unknown,
  (GameVersion_e)Game_Unknown
};

//------------- Functions -------------//
internal PlatformType_e GetPlatformType()
{
  PlatformType_e result = Platform_Unknown;
  
  HMODULE galaxyModule = GetModuleHandle("Galaxy64.dll");
  HMODULE steamModule = GetModuleHandle("steam_api64.dll");
  if (!(galaxyModule && steamModule)) {
    if (galaxyModule) {
      result = Platform_GOG;
    } else if (steamModule) {
      result = Platform_STEAM;
    } else {
      OutputDebugStringA("GetPlatformType: Unknown platform");
    }
  } else {
    OutputDebugStringA("GetPlatformType: Cannot accurately detect a platform");
  }
  
  return result;
}

internal u32 GetGameVersionNumber()
{
  u32 result = Game_Unknown;
  
  char buffer[KB(16)];
  char *string_nil = "nil";
  
  //IMPORTANT(adm244): double check a filepath
  cfg_init("system.cfg", buffer, sizeof(buffer));
  if (cfg_parse()) {
    char *string_version = cfg_read_string("wh_sys_version", string_nil);
    //NOTE(adm244): aware of pointer comparison,
    // 'string_version' _will_ point to 'string_nil' if parsing fails
    if (string_version != string_nil) {
      result = cfg_convert_version_to_u32(string_version);
    } else {
      OutputDebugStringA("GetGameVersionNumber: Cannot get version string");
    }
  } else {
    OutputDebugStringA("GetGameVersionNumber: Cannot parse system.cfg");
  }
  
  return result;
}

internal GameVersion_e GetGameVersion()
{
  GameVersion_e result = Game_Unknown;
  
  u32 version_binary = GetGameVersionNumber();
  switch (version_binary) {
    case MAKE_VERSION(1,9,6): {
      result = Game_196;
    } break;
    
    case MAKE_VERSION(1,9,5): {
      result = Game_195;
    } break;
    
    case MAKE_VERSION(1,9,4): {
      result = Game_194;
    } break;
    
    case MAKE_VERSION(1,9,3): {
      result = Game_193;
    } break;
    
    case MAKE_VERSION(1,9,2): {
      result = Game_192;
    } break;
    
    case MAKE_VERSION(1,9,1):
    case MAKE_VERSION(1,9,0):
    case MAKE_VERSION(1,8,2):
    case MAKE_VERSION(1,8,1):
    case MAKE_VERSION(1,7,1):
    case MAKE_VERSION(1,7,0):
    case MAKE_VERSION(1,6,2):
    case MAKE_VERSION(1,6,0):
    case MAKE_VERSION(1,5,0):
    case MAKE_VERSION(1,4,3):
    case MAKE_VERSION(1,4,2):
    case MAKE_VERSION(1,4,1):
    case MAKE_VERSION(1,4,0):
    case MAKE_VERSION(1,3,0):
    case MAKE_VERSION(1,2,5):
    case MAKE_VERSION(1,2,0):
    case MAKE_VERSION(1,1,0):
    case MAKE_VERSION(1,0,0): {
      result = Game_NotSupported;
    } break;
    
    case Game_Unknown: {
      OutputDebugStringA("GetGameVersion: Cannot get a version number");
    } break;
    
    default: {
      OutputDebugStringA("GetGameVersion: Unknown version");
    } break;
  }
  
  return result;
}

internal bool IsSupportedVersion()
{
  g_GameInfo.platform = GetPlatformType();
  g_GameInfo.version = GetGameVersion();
  
  if (g_GameInfo.platform == Platform_Unknown) {
    OutputDebugStringA("IsSupportedVersion: Unknown platform detected");
    return false;
  }
  
  switch (g_GameInfo.version) {
    case Game_Unknown: {
      OutputDebugStringA("IsSupportedVersion: Unknown game version detected");
    } return false;
  
    case Game_NotSupported: {
      OutputDebugStringA("IsSupportedVersion: Unsupported game version detected");
    } return false;
  }
  
  return true;
}

#endif
