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

//NOTE(adm244): based on INI-Parser (https://github.com/adm244/INI-Parser)

#ifndef _CFG_PARSER_H_
#define _CFG_PARSER_H_

/////////////////////////// GLOBAL DATA ////////////////////////////
#ifndef assert
#define assert
#endif

#ifndef CFG_FILEPATH_MAX
#define CFG_FILEPATH_MAX MAX_PATH
#endif

#ifndef CFG_BUFFER_SIZE
#define CFG_BUFFER_SIZE 8192
#endif

#define CFG_STORAGE_MIN 64
#define CFG_KEY_LENGTH 128
#define CFG_VALUE_LENGTH 1024

#define CFG_MAKE_VERSION(major, minor, patch, build) ((major << 24) | (minor << 16) | (patch << 8) | (build))

enum cfg_value_type {
  CFG_VALUE_GARBAGE = 0,
  CFG_VALUE_INT,
  CFG_VALUE_FLOAT,
  CFG_VALUE_BOOL,
  CFG_VALUE_STRING,
  CFG_VALUE_SYMBOL
};

enum cfg_entry_type {
  CFG_ENTRY_GARBAGE = 0,
  CFG_ENTRY_KEYVALUE,
};

struct arena_t {
  void *base;
  void *end;
  void *cur;
  size_t count;
};

struct cfg_value_t {
  cfg_value_type type;
  union {
    int number;
    float real;
    bool boolean;
    char *str;
  };
};

struct cfg_entry_t {
  cfg_entry_type type;
  char *name;
  cfg_value_t value;
};

struct cfg_version_block_t {
  char *str;
};

struct cfg_settings_t {
  char filepath[CFG_FILEPATH_MAX];
  arena_t data;
  arena_t entries;
};

internal cfg_settings_t cfg_settings;

//////////////////////// UTILITY FUNCTIONS /////////////////////////
internal bool is_number(char c)
{
  return (c >= '0') && (c <= '9');
}

internal bool is_sign(char c)
{
  return (c == '-') || (c == '+');
}

internal bool is_whitespace(char c)
{
  return (c == 9) || (c == 10) || (c == 13) || (c == 32);
}

internal bool is_alphanum(char c)
{
  return ((c >= 48) && (c <= 57)) // digit
      || ((c >= 65) && (c <= 90)) // uppercase
      || ((c >= 97) && (c <= 122)); // lowercase
}

///////////////////////// ARENA FUNCTIONS //////////////////////////
internal inline void arena_init(arena_t *arena, void *base, size_t size)
{
  assert(arena);
  assert(base);
  assert(size > 0);
  
  arena->base = base;
  arena->end = ((char *)base + size);
  arena->cur = arena->base;
  arena->count = 0;
}

internal inline void arena_reset(arena_t *arena)
{
  assert(arena);
  
  arena->cur = arena->base;
  arena->count = 0;
}

internal void * arena_push(arena_t *arena, void *data, size_t size)
{
  assert(arena);
  assert(data);
  
  if (size == 0) {
    size = strlen((char *)data) + 1;
  }
  
  size_t free_space = ((char *)arena->end - arena->cur);
  if (free_space < size) {
    return 0;
  }
  
  memcpy_s(arena->cur, free_space, data, size);
  
  void *result = arena->cur;
  arena->cur = (char *)arena->cur + size;
  
  assert((arena->cur >= arena->base) && (arena->cur <= arena->end));
  
  ++arena->count;
  return result;
}

//////////////////////// PRIVATE FUNCTIONS /////////////////////////
internal void cfg_store_keyvalue(char *key, cfg_value_t *value)
{
  assert(key);
  assert(value);
  assert(value->type != CFG_VALUE_GARBAGE);
  
  cfg_entry_t entry;
  entry.type = CFG_ENTRY_KEYVALUE;
  entry.name = (char *)arena_push(&cfg_settings.data, key, 0);
  
  entry.value.type = value->type;
  switch (entry.value.type) {
    case CFG_VALUE_INT: {
      entry.value.number = value->number;
    } break;
    
    case CFG_VALUE_FLOAT: {
      entry.value.real = value->real;
    } break;
    
    case CFG_VALUE_BOOL: {
      entry.value.boolean = value->boolean;
    } break;
    
    case CFG_VALUE_STRING: {
      entry.value.str = (char *)arena_push(&cfg_settings.data, value->str, 0);
    } break;
    
    case CFG_VALUE_SYMBOL: {
      entry.value.str = (char *)arena_push(&cfg_settings.data, value->str, 0);
    } break;
    
    default: {
      entry.value.type = CFG_VALUE_GARBAGE;
    } break;
  }
  
  void *p = arena_push(&cfg_settings.entries, &entry, sizeof(cfg_entry_t));
  assert(p != 0);
}

internal bool cfg_parse_read_file(char *buffer, size_t size)
{
  assert(buffer);
  
  HANDLE file = CreateFileA(cfg_settings.filepath, GENERIC_READ, FILE_SHARE_READ, 0,
    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
  if (file == INVALID_HANDLE_VALUE) {
    OutputDebugStringA("Error: Cannot open file!\n");
    return false;
  }
  
  DWORD sizeRead;
  BOOL result = ReadFile(file, buffer, size, &sizeRead, 0);
  if (!result) {
    OutputDebugStringA("Error: Cannot read file!\n");
    return false;
  }
  
  CloseHandle(file);
  
  buffer[sizeRead] = '\0';
  //NOTE(adm244): replace CR with SPACE
  for (int i = 0; i < sizeRead; ++i) {
    if (buffer[i] == 13) {
      buffer[i] = ' ';
    }
  }
  
  return true;
}

internal char * cfg_nextline(char **buffer)
{
  assert(buffer);
  assert(*buffer);
  
  char *start = *buffer;
  char *p = start;
  
  if (*start == '\0') {
    return 0;
  }
  
  while ((*p != '\n') && (*p != '\0')) {
    ++p;
  }
  
  if (*p == '\0') {
    *buffer = p;
  } else {
    assert(*p == '\n');
    
    *p = '\0';
    *buffer = p + 1;
  }
  
  return start;
}

internal inline char * cfg_strip_whitespace_left(char *buffer)
{
  assert(buffer);
  
  char *p = buffer;
  while (is_whitespace(*p)) {
    ++p;
  }
  return p;
}

internal inline char * cfg_strip_whitespace_right(char *buffer)
{
  assert(buffer);
  
  char *start = buffer;
  char *last = 0;
  
  char *p = start;
  while (*p != '\0') {
    if (!is_whitespace(*p)) {
      last = p;
    }
    ++p;
  }
  
  if (last) {
    *(last + 1) = '\0';
  }
  
  return start;
}

internal char * cfg_strip_whitespace(char *buffer)
{
  assert(buffer);
  
  buffer = cfg_strip_whitespace_left(buffer);
  buffer = cfg_strip_whitespace_right(buffer);
  
  return buffer;
}

internal char * cfg_strip_comments(char *buffer)
{
  assert(buffer);
  
  char *start = 0;
  char *p = buffer;
  bool inside_value = false;
  
  while (*p != '\0') {
    if (*p == '\"') {
      inside_value = !inside_value;
    }
    
    if (!inside_value) {
    //NOTE(adm244): "--" marks a comment (lua comment)
      if (*p == '-') {
        if (*(p + 1) == '-') {
          start = p;
          break;
        }
      }
    }
    
    ++p;
  }
  
  if (start) {
    *start = '\0';
  }
  
  return buffer;
}

internal bool cfg_parse_keyvalue_key(char *buffer, char *dest, size_t size)
{
  assert(buffer);
  assert(dest);
  assert(size > 0);
  
  char *start = buffer;
  char *p = start;
  
  while (*p != '\0') {
    if (is_whitespace(*p)) {
      break;
    }
    if (*p == '=') {
      break;
    }
    ++p;
  }
  
  if (*p == '\0') {
    return false;
  }
  
  if (*p == '=') {
    strncpy_s(dest, size, buffer, (size_t)(p - start));
    return true;
  } else {
    char *end = p;
    ++p;
    
    while (*p != '\0') {
      if (!is_whitespace(*p)) {
        break;
      }
      ++p;
    }
    
    if (*p != '=') {
      return false;
    }
    
    strncpy_s(dest, size, buffer, (size_t)(end - start));
    return true;
  }
}

internal bool cfg_parse_keyvalue_value(char *buffer, char *dest, size_t size)
{
  assert(buffer);
  assert(dest);
  assert(size > 0);
  
  char *start = buffer;
  char *p = start;
  
  while (*p != '\0') {
    if (*p == '=') {
      break;
    }
    ++p;
  }
  
  if (*p != '=') {
    return false;
  }
  
  start = p + 1;
  p = start;
  
  while (*p != '\0') {
    if (!is_whitespace(*p)) {
      break;
    }
    ++p;
  }
  
  if (*p == '\0') {
    return false;
  }
  
  start = p;
  p = start;
  bool quoted = false;
  
  while (*p != '\0') {
    if (*p == '\"') {
      quoted = !quoted;
    }
    if (!quoted) {
      if (is_whitespace(*p)) {
        break;
      }
    }
    ++p;
  }
  
  if (quoted) {
    return false;
  }
  
  if (*p != '\0') {
    return false;
  }
  
  strcpy_s(dest, size, start);
  return true;
}

internal bool is_float(char *buffer)
{
  bool result = false;
  
  char *p = buffer;
  if (is_sign(*p)) {
    ++p;
  }
  
  while (*p != '\0') {
    if (*p == '.') {
      if (result) {
        return false;
      }
      result = true;
    } else {
      if (!is_number(*p)) {
        return false;
      }
    }
    
    ++p;
  }
  
  return result;
}

internal bool cfg_parse_value_integer(char *buffer, int *value)
{
  char *p = is_sign(buffer[0]) ? buffer + 1 : buffer;
  int number = 0;
  
  while (*p != '\0') {
    if (!is_number(*p)) {
      break;
    }
    
    //TODO(adm244): check for overflow
    number *= 10;
    number += (*p - '0');
    
    ++p;
  }
  
  if (*p != '\0') {
    return false;
  }
  
  if (buffer[0] == '-') {
    number = -number;
  }
  
  *value = number;
  return true;
}

internal bool cfg_parse_value(char *buffer, cfg_value_t *value)
{
  assert(buffer);
  assert(value);
  
  value->type = CFG_VALUE_GARBAGE;
  
  // try parse boolean
  if ((buffer[0] == 't') || (buffer[0] == 'f')) {
    if (strcmp(buffer, "true") == 0) {
      value->type = CFG_VALUE_BOOL;
      value->boolean = true;
      return true;
    }
    
    if (strcmp(buffer, "false") == 0) {
      value->type = CFG_VALUE_BOOL;
      value->boolean = false;
      return true;
    }
    
    return false;
  }
  
  // try parse string
  else if (buffer[0] == '\"') {
    char *start = &buffer[1];
    char *p = start;
    
    //NOTE(adm244): guaranteed to have a closing quote
    while (*p != '\"') {
      ++p;
    }
    
    *p = '\0';
    
    value->type = CFG_VALUE_STRING;
    value->str = start;
    return true;
  }
  
  // try parse float
  else if (is_float(buffer)) {
    if (sscanf_s(buffer, "%f\0", &value->real) > 0) {
      value->type = CFG_VALUE_FLOAT;
      return true;
    }
    
    return false;
  }
  
  // try parse integer
  else if (is_number(buffer[0]) || is_sign(buffer[0])) {
    int number;
    bool result = cfg_parse_value_integer(buffer, &number);
    if (!result) {
      return false;
    }
    
    value->type = CFG_VALUE_INT;
    value->number = number;
    
    return true;
  }
  
  // try parse symbol
  else if (is_alphanum(buffer[0]) || (buffer[0] == '_')) {
    char *start = &buffer[0];
    char *p = start;
    
    while (*p != '\0') {
      if (!is_whitespace(*p)) {
        break;
      }
      ++p;
    }
    
    start = p;
    p += 1;
    
    while (*p != '\0') {
      if (is_whitespace(*p)) {
        break;
      }
      ++p;
    }
    
    *p = '\0';
    
    value->type = CFG_VALUE_SYMBOL;
    value->str = start;
    
    return true;
  }
  
  return false;
}

internal bool cfg_parse_keyvalue(char *buffer)
{
  assert(buffer);
  
  char key[CFG_KEY_LENGTH];
  if (!cfg_parse_keyvalue_key(buffer, key, CFG_KEY_LENGTH)) {
    return false;
  }
  
  char value[CFG_VALUE_LENGTH];
  if (!cfg_parse_keyvalue_value(buffer, value, CFG_VALUE_LENGTH)) {
    return false;
  }
  
  cfg_value_t value_typed;
  if (!cfg_parse_value(value, &value_typed)) {
    return false;
  }
  
  cfg_store_keyvalue(key, &value_typed);
  
  return true;
}

internal inline bool cfg_parse_line(char *buffer)
{
  assert(buffer);
  
  if (!cfg_parse_keyvalue(buffer)) {
    return false;
  }
  
  return true;
}

internal bool cfg_parse_buffer(char *buffer, size_t size)
{
  assert(buffer);
  
  bool result = true;
  
  char *p = buffer;
  char *line;
  
  while (line = cfg_nextline(&p)) {
    line = cfg_strip_comments(line);
    line = cfg_strip_whitespace(line);
    
    if (line[0] == '\0')
      continue;
    
    if (!cfg_parse_line(line)) {
      result = false;
      continue;
    }
  }
  
  return result;
}

internal cfg_value_t * cfg_get_value(char *key)
{
  cfg_entry_t *entries = (cfg_entry_t *)cfg_settings.entries.base;
  cfg_value_t *value = 0;
  
  for (int i = 0; i < cfg_settings.entries.count; ++i) {
    if (strcmp(entries[i].name, key) == 0) {
      value = &entries[i].value;
      break;
    }
  }
  
  return value;
}

internal void cfg_version_read_blocks(char *str, cfg_version_block_t *blocks, int blocks_count)
{
  assert(str);
  assert(blocks);
  assert(blocks_count > 0);
  
  // 'xxx.xxx.xxx.xxx'
  if (str) {
    int i = 0;
    int block_number = 0;
    bool initialized = false;
    
    for (; str[i] != '\0'; ++i) {
      if (block_number >= blocks_count) {
        break;
      }
      
      if (!initialized) {
        blocks[block_number].str = &str[i];
        initialized = true;
      }
      
      if (str[i] == '.') {
        str[i] = '\0';
        initialized = false;
        ++block_number;
      }
    }
  }
}

internal u8 cfg_version_block_to_u8(cfg_version_block_t block)
{
  u8 result = 0;
  
  if (block.str) {
    block.str = cfg_strip_whitespace(block.str);
    
    int number;
    if (cfg_parse_value_integer(block.str, &number)) {
      if ((uint)number < 256) {
        result = (u8)number;
      } else {
        OutputDebugStringA("cfg_version_block_to_u8: Version part number is too high");
      }
    } else {
      OutputDebugStringA("cfg_version_block_to_u8: Cannot parse a version part number");
    }
  }
  
  return result;
}

///////////////////////// PUBLIC FUNCTIONS /////////////////////////
internal void cfg_init(const char *filepath, void *storage, size_t size)
{
  assert(filepath);
  assert(storage);
  assert(size >= CFG_STORAGE_MIN);
  
  strcpy_s(cfg_settings.filepath, CFG_FILEPATH_MAX, filepath);
  
  //NOTE(adm244): 3:1, 3 - data, 1 - entries
  size_t entries_size = (size / 4);
  size_t data_size = (size - entries_size - 1);
  
  arena_init(&cfg_settings.data, storage, data_size);
  arena_init(&cfg_settings.entries, ((char *)storage + data_size + 1), entries_size);
}

internal bool cfg_parse()
{
  assert(cfg_settings.filepath);
  
  arena_reset(&cfg_settings.data);
  arena_reset(&cfg_settings.entries);
  
  char buffer[CFG_BUFFER_SIZE];
  bool result = cfg_parse_read_file(buffer, CFG_BUFFER_SIZE);
  if (!result) {
    OutputDebugStringA("cfg_parse: cannot read file");
    return false;
  }
  
  result = cfg_parse_buffer(buffer, CFG_BUFFER_SIZE);
  if (!result) {
    OutputDebugStringA("cfg_parse: cannot parse buffer");
    return false;
  }
  
  return true;
}

internal u32 cfg_convert_version_to_u32(char *str)
{
  assert(str);
  
  u32 result = 0;
  
  // '1.9.5' or '1.2.3.4'
  str = cfg_strip_whitespace(str);
  
  cfg_version_block_t version_blocks[4] = {0};
  cfg_version_read_blocks(str, version_blocks, arraycount(version_blocks));
  
  u8 major = cfg_version_block_to_u8(version_blocks[0]);
  u8 minor = cfg_version_block_to_u8(version_blocks[1]);
  u8 patch = cfg_version_block_to_u8(version_blocks[2]);
  u8 build = cfg_version_block_to_u8(version_blocks[3]);
  
  return CFG_MAKE_VERSION(major, minor, patch, build);
}

internal int cfg_read_int(char *key, int default)
{
  assert(key);
  
  cfg_value_t *value = cfg_get_value(key);
  if (!value) {
    return default;
  }
  
  if (value->type == CFG_VALUE_INT) {
    return value->number;
  }
  
  if (value->type == CFG_VALUE_BOOL) {
    return value->boolean;
  }
  
  return default;
}

internal float cfg_read_float(char *key, float default)
{
  assert(key);
  
  cfg_value_t *value = cfg_get_value(key);
  if (!value) {
    return default;
  }
  
  if (value->type == CFG_VALUE_FLOAT) {
    return value->real;
  }
  
  if (value->type == CFG_VALUE_INT) {
    return (float)value->number;
  }
  
  return default;
}

internal bool cfg_read_bool(char *key, bool default)
{
  assert(key);
  
  cfg_value_t *value = cfg_get_value(key);
  if (!value) {
    return default;
  }
  
  if (value->type == CFG_VALUE_BOOL) {
    return value->boolean;
  }
  
  if (value->type == CFG_VALUE_INT) {
    return (value->number) ? true : false;
  }
  
  return default;
}

internal char * cfg_read_string(char *key, char *default)
{
  assert(key);
  assert(default);
  
  cfg_value_t *value = cfg_get_value(key);
  if (!value) {
    return default;
  }
  
  if (value->type == CFG_VALUE_STRING) {
    return value->str;
  }
  
  return default;
}

#endif
