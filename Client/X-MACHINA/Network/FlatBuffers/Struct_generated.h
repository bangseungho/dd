// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_STRUCT_FBPROTOCOL_H_
#define FLATBUFFERS_GENERATED_STRUCT_FBPROTOCOL_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 24 &&
              FLATBUFFERS_VERSION_MINOR == 3 &&
              FLATBUFFERS_VERSION_REVISION == 7,
             "Non-compatible flatbuffers version included");

#include "Enum_generated.h"
#include "Transform_generated.h"

namespace FBProtocol {

struct Player;
struct PlayerBuilder;

struct Player FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef PlayerBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_ID = 4,
    VT_NAME = 6,
    VT_PLAYER_TYPE = 8,
    VT_TRANS = 10,
    VT_SPINE_LOOK = 12
  };
  uint64_t id() const {
    return GetField<uint64_t>(VT_ID, 0);
  }
  const ::flatbuffers::String *name() const {
    return GetPointer<const ::flatbuffers::String *>(VT_NAME);
  }
  FBProtocol::OBJECTTYPE player_type() const {
    return static_cast<FBProtocol::OBJECTTYPE>(GetField<int8_t>(VT_PLAYER_TYPE, 0));
  }
  const FBProtocol::Transform *trans() const {
    return GetPointer<const FBProtocol::Transform *>(VT_TRANS);
  }
  const FBProtocol::Vector3 *spine_look() const {
    return GetPointer<const FBProtocol::Vector3 *>(VT_SPINE_LOOK);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint64_t>(verifier, VT_ID, 8) &&
           VerifyOffset(verifier, VT_NAME) &&
           verifier.VerifyString(name()) &&
           VerifyField<int8_t>(verifier, VT_PLAYER_TYPE, 1) &&
           VerifyOffset(verifier, VT_TRANS) &&
           verifier.VerifyTable(trans()) &&
           VerifyOffset(verifier, VT_SPINE_LOOK) &&
           verifier.VerifyTable(spine_look()) &&
           verifier.EndTable();
  }
};

struct PlayerBuilder {
  typedef Player Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_id(uint64_t id) {
    fbb_.AddElement<uint64_t>(Player::VT_ID, id, 0);
  }
  void add_name(::flatbuffers::Offset<::flatbuffers::String> name) {
    fbb_.AddOffset(Player::VT_NAME, name);
  }
  void add_player_type(FBProtocol::OBJECTTYPE player_type) {
    fbb_.AddElement<int8_t>(Player::VT_PLAYER_TYPE, static_cast<int8_t>(player_type), 0);
  }
  void add_trans(::flatbuffers::Offset<FBProtocol::Transform> trans) {
    fbb_.AddOffset(Player::VT_TRANS, trans);
  }
  void add_spine_look(::flatbuffers::Offset<FBProtocol::Vector3> spine_look) {
    fbb_.AddOffset(Player::VT_SPINE_LOOK, spine_look);
  }
  explicit PlayerBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<Player> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<Player>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<Player> CreatePlayer(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    uint64_t id = 0,
    ::flatbuffers::Offset<::flatbuffers::String> name = 0,
    FBProtocol::OBJECTTYPE player_type = FBProtocol::OBJECTTYPE_NONE,
    ::flatbuffers::Offset<FBProtocol::Transform> trans = 0,
    ::flatbuffers::Offset<FBProtocol::Vector3> spine_look = 0) {
  PlayerBuilder builder_(_fbb);
  builder_.add_id(id);
  builder_.add_spine_look(spine_look);
  builder_.add_trans(trans);
  builder_.add_name(name);
  builder_.add_player_type(player_type);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<Player> CreatePlayerDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    uint64_t id = 0,
    const char *name = nullptr,
    FBProtocol::OBJECTTYPE player_type = FBProtocol::OBJECTTYPE_NONE,
    ::flatbuffers::Offset<FBProtocol::Transform> trans = 0,
    ::flatbuffers::Offset<FBProtocol::Vector3> spine_look = 0) {
  auto name__ = name ? _fbb.CreateString(name) : 0;
  return FBProtocol::CreatePlayer(
      _fbb,
      id,
      name__,
      player_type,
      trans,
      spine_look);
}

}  // namespace FBProtocol

#endif  // FLATBUFFERS_GENERATED_STRUCT_FBPROTOCOL_H_
