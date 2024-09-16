// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_FBPCHAT_FBPROTOCOL_H_
#define FLATBUFFERS_GENERATED_FBPCHAT_FBPROTOCOL_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 24 &&
              FLATBUFFERS_VERSION_MINOR == 3 &&
              FLATBUFFERS_VERSION_REVISION == 7,
             "Non-compatible flatbuffers version included");

namespace FBProtocol {

struct CPkt_Chat;
struct CPkt_ChatBuilder;

struct SPkt_Chat;
struct SPkt_ChatBuilder;

struct CPkt_Chat FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef CPkt_ChatBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_MESSAGE = 4
  };
  const ::flatbuffers::String *message() const {
    return GetPointer<const ::flatbuffers::String *>(VT_MESSAGE);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_MESSAGE) &&
           verifier.VerifyString(message()) &&
           verifier.EndTable();
  }
};

struct CPkt_ChatBuilder {
  typedef CPkt_Chat Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_message(::flatbuffers::Offset<::flatbuffers::String> message) {
    fbb_.AddOffset(CPkt_Chat::VT_MESSAGE, message);
  }
  explicit CPkt_ChatBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<CPkt_Chat> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<CPkt_Chat>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<CPkt_Chat> CreateCPkt_Chat(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::String> message = 0) {
  CPkt_ChatBuilder builder_(_fbb);
  builder_.add_message(message);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<CPkt_Chat> CreateCPkt_ChatDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const char *message = nullptr) {
  auto message__ = message ? _fbb.CreateString(message) : 0;
  return FBProtocol::CreateCPkt_Chat(
      _fbb,
      message__);
}

struct SPkt_Chat FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef SPkt_ChatBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_PLAYER_ID = 4,
    VT_MESSAGE = 6
  };
  uint32_t player_id() const {
    return GetField<uint32_t>(VT_PLAYER_ID, 0);
  }
  const ::flatbuffers::String *message() const {
    return GetPointer<const ::flatbuffers::String *>(VT_MESSAGE);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint32_t>(verifier, VT_PLAYER_ID, 4) &&
           VerifyOffset(verifier, VT_MESSAGE) &&
           verifier.VerifyString(message()) &&
           verifier.EndTable();
  }
};

struct SPkt_ChatBuilder {
  typedef SPkt_Chat Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_player_id(uint32_t player_id) {
    fbb_.AddElement<uint32_t>(SPkt_Chat::VT_PLAYER_ID, player_id, 0);
  }
  void add_message(::flatbuffers::Offset<::flatbuffers::String> message) {
    fbb_.AddOffset(SPkt_Chat::VT_MESSAGE, message);
  }
  explicit SPkt_ChatBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<SPkt_Chat> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<SPkt_Chat>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<SPkt_Chat> CreateSPkt_Chat(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    uint32_t player_id = 0,
    ::flatbuffers::Offset<::flatbuffers::String> message = 0) {
  SPkt_ChatBuilder builder_(_fbb);
  builder_.add_message(message);
  builder_.add_player_id(player_id);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<SPkt_Chat> CreateSPkt_ChatDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    uint32_t player_id = 0,
    const char *message = nullptr) {
  auto message__ = message ? _fbb.CreateString(message) : 0;
  return FBProtocol::CreateSPkt_Chat(
      _fbb,
      player_id,
      message__);
}

}  // namespace FBProtocol

#endif  // FLATBUFFERS_GENERATED_FBPCHAT_FBPROTOCOL_H_
