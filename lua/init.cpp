/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include <fstream>
#include <iostream>

extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

#include "client.h"
#include "client_lua.h"
#include "constants_lua.h"
#include "frame.h"
#include "frame_lua.h"
#include "gamestore.h"
#include "imgmanager.h"
#include "replayer_lua.h"
#include "state_lua.h"

using torchcraft::replayer::GameStore;

extern "C" int new_gameStore(lua_State* L) {
  auto lost = luaL_checkint(L, 1);
  auto won = luaL_checkint(L, 2);
  GameStore* gs = new GameStore(lost, won);
  luaT_pushudata(L, gs, "torchcraft.GameStore");
  return 1;
}

extern "C" int free_gameStore(lua_State* L) {
  GameStore* gs = (GameStore*)luaT_checkudata(L, 1, "torchcraft.GameStore");
  delete gs;
  return 0;
}

extern "C" int factory_gameStore(lua_State* L) {
  GameStore* gs = new GameStore();
  luaT_pushudata(L, gs, "torchcraft.GameStore");
  return 1;
}

static const struct luaL_Reg replayer_f[] = {
    {"frameFromTable", frameFromTable},
    {"frameFromString", frameFromString},
    {"newReplayer", newReplayer},
    {"loadReplayer", loadReplayer},
    {"newGameStore", newGameStore},
    {"loadGameStore", loadGameStore},
    {"rawBitmapToTensor", rawBitmapToTensor},
    {nullptr, nullptr}};

int registerReplayer(lua_State* L) {
  lua_newtable(L);
  int replayer_stack_location = lua_gettop(L);

  luaL_newmetatable(L, "torchcraft.Frame");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaT_setfuncs(L, frame_m, 0);
  lua_setfield(L, -2, "Frame");

  luaL_newmetatable(L, "torchcraft.Replayer");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaT_setfuncs(L, replayer_m, 0);
  lua_setfield(L, -2, "Replayer");

  luaT_newlocalmetatable(
      L,
      "torchcraft.GameStore",
      nullptr,
      new_gameStore,
      free_gameStore,
      factory_gameStore,
      replayer_stack_location);
  luaL_newmetatable(L, "torchcraft.GameStore");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaT_setfuncs(L, gamestore_m, 0);
  lua_setfield(L, -2, "GameStore");
  lua_pop(L, 1);

  luaT_setfuncs(L, replayer_f, 0);

  return 1;
}

int registerClient(lua_State* L) {
  torchcraft::init();

  lua_newtable(L);
  torchcraft::registerClient(L, lua_gettop(L));
  torchcraft::registerState(L, lua_gettop(L));
  torchcraft::registerConstants(L, lua_gettop(L));
  return 1;
}

/*This is the entry point of the lua binding
 * General syntax is luaopen_packagename(lua_State *L)
 * It is called when calling "require torchcraft.replayer" from lua and register
 * all the c++ function for call from lua
 */
extern "C" int luaopen_torchcraft_tc_lib(lua_State* L) {
  registerClient(L);
  registerReplayer(L);
  lua_setfield(L, -2, "replayer");
  return 1;
}
