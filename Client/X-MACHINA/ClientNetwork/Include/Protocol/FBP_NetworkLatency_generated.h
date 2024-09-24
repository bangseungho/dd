// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_FBPNETWORKLATENCY_FBPROTOCOL_H_
#define FLATBUFFERS_GENERATED_FBPNETWORKLATENCY_FBPROTOCOL_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 24 &&
              FLATBUFFERS_VERSION_MINOR == 3 &&
              FLATBUFFERS_VERSION_REVISION == 7,
             "Non-compatible flatbuffers version included");

namespace FBProtocol {

struct CPkt_NetworkLatency;
struct CPkt_NetworkLatencyBuilder;

struct SPkt_NetworkLatency;
struct SPkt_NetworkLatencyBuilder;

struct CPkt_NetworkLatency FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef CPkt_NetworkLatencyBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_TIMESTAMP = 4
  };
  int64_t timestamp() const {
    return GetField<int64_t>(VT_TIMESTAMP, 0);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int64_t>(verifier, VT_TIMESTAMP, 8) &&
           verifier.EndTable();
  }
};

struct CPkt_NetworkLatencyBuilder {
  typedef CPkt_NetworkLatency Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_timestamp(int64_t timestamp) {
    fbb_.AddElement<int64_t>(CPkt_NetworkLatency::VT_TIMESTAMP, timestamp, 0);
  }
  explicit CPkt_NetworkLatencyBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<CPkt_NetworkLatency> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<CPkt_NetworkLatency>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<CPkt_NetworkLatency> CreateCPkt_NetworkLatency(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    int64_t timestamp = 0) {
  CPkt_NetworkLatencyBuilder builder_(_fbb);
  builder_.add_timestamp(timestamp);
  return builder_.Finish();
}

struct SPkt_NetworkLatency FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef SPkt_NetworkLatencyBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_TIMESTAMP = 4
  };
  int64_t timestamp() const {
    return GetField<int64_t>(VT_TIMESTAMP, 0);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int64_t>(verifier, VT_TIMESTAMP, 8) &&
           verifier.EndTable();
  }
};

struct SPkt_NetworkLatencyBuilder {
  typedef SPkt_NetworkLatency Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_timestamp(int64_t timestamp) {
    fbb_.AddElement<int64_t>(SPkt_NetworkLatency::VT_TIMESTAMP, timestamp, 0);
  }
  explicit SPkt_NetworkLatencyBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<SPkt_NetworkLatency> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<SPkt_NetworkLatency>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<SPkt_NetworkLatency> CreateSPkt_NetworkLatency(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    int64_t timestamp = 0) {
  SPkt_NetworkLatencyBuilder builder_(_fbb);
  builder_.add_timestamp(timestamp);
  return builder_.Finish();
}

}  // namespace FBProtocol

#endif  // FLATBUFFERS_GENERATED_FBPNETWORKLATENCY_FBPROTOCOL_H_