/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#pragma once

#include <set>
#include <vector>

#include "constants.h"
#include "frame.h"
#include "refcount.h"

// Flatbuffer messages
namespace torchcraft {
namespace fbs {
struct HandshakeServer;
struct Frame;
struct EndGame;
}
}

namespace torchcraft {

// Aliases for basic replayer types provided for convenience
typedef replayer::Unit Unit;
typedef replayer::Order Order;
typedef replayer::Resources Resources;
typedef replayer::Bullet Bullet;
typedef replayer::Action Action;
typedef replayer::Frame Frame;

class State : public RefCounted {
 public:
  // setup
  int lag_frames; // number of frames from order to execution
  std::vector<uint8_t> map_data; // 2D. 255 where not available
  int map_data_size[2];
  std::vector<bool> buildable_data; // 2D, build tile resolution
  int buildable_data_size[2];
  std::string map_name; // Name on the current map
  int player_id;
  int neutral_id;
  bool replay;

  // game state
  Frame* frame; // this will allow for easy reset (XXX)
  std::string frame_string;
  std::vector<int> deaths;
  int frame_from_bwapi;
  int battle_frame_count; // if micro mode

  bool game_ended; // did the game end?
  bool game_won;

  // if micro mode
  bool battle_just_ended;
  bool battle_won;
  bool waiting_for_restart;
  int last_battle_ended;

  // if with image
  std::string img_mode;
  int screen_position[2]; // position of screen {x, y} in pixels. {0, 0} is
  // top-left
  std::vector<uint8_t> visibility;
  int visibility_size[2];
  std::vector<uint8_t> image; // RGB
  int image_size[2];

  // Alive units in this frame. Used to detect end-of-battle in micro mode. If
  // the current frame is the end of a battle, this will contain all units that
  // were alive when the battle ended (which is not necessarily the current
  // frame due to frame skipping on the serv side). Note that this map ignores
  // onlyConsiderUnits_.
  // Maps unit id to player id
  std::unordered_map<int32_t, int32_t> aliveUnits;

  // Like aliveUnits, but containing only units of types in onlyConsiderUnits.
  // If onlyConsiderUnits is empty, this map is invalid.
  std::unordered_map<int32_t, int32_t> aliveUnitsConsidered;

  // Bots might want to use this map instead of frame->units because:
  // - Unknown unit types are not present (e.g. map revealers)
  // - Units reported as dead are not present (important if the server performs
  //   frame skipping. In that case, frame->units will still contain all units
  //   that have died since the last update.
  // - In micro mode and with frame skipping, deaths are only applied until the
  //   battle is considered finished, i.e. it corresponds to aliveUnits.
  std::unordered_map<int32_t, std::vector<Unit>> units;

  State(
      bool microBattles = false,
      std::set<BW::UnitType> onlyConsiderTypes = std::set<BW::UnitType>());
  ~State();
  State(const State&) = delete;
  State& operator=(const State&) = delete;

  bool microBattles() const {
    return microBattles_;
  }
  const std::set<BW::UnitType>& onlyConsiderTypes() const {
    return onlyConsiderTypes_;
  }
  void setMicroBattles(bool microBattles) {
    microBattles_ = microBattles;
  }
  void setOnlyConsiderTypes(std::set<BW::UnitType> types) {
    onlyConsiderTypes_ = std::move(types);
    aliveUnitsConsidered.clear();
  }

  void reset();
  std::vector<std::string> update(
      const torchcraft::fbs::HandshakeServer* handshake);
  std::vector<std::string> update(const torchcraft::fbs::Frame* frame);
  std::vector<std::string> update(const torchcraft::fbs::EndGame* end);
  void trackAliveUnits(
      std::vector<std::string>& upd,
      const std::set<BW::UnitType>& considered);

 private:
  bool setRawImage(const torchcraft::fbs::Frame* frame);
  void postUpdate(std::vector<std::string>& upd);
  bool checkBattleFinished(std::vector<std::string>& upd);

  bool microBattles_;
  std::set<BW::UnitType> onlyConsiderTypes_;
};

} // namespace torchcraft
