#pragma once

#include "coding.hpp"

#include <cstdio>
#include <sstream>


// Grouping of constants.  We may want to make some of these
// parameters set via options.
namespace config
{
  static const int kNumLevels = 7;

  // Level-0 compaction is started when we hit this many files.
  static const int kL0_CompactionTrigger = 4;

  // Soft limit on number of level-0 files.  We slow down writes at this point.
  static const int kL0_SlowdownWritesTrigger = 8;

  // Maximum number of level-0 files.  We stop writes at this point.
  static const int kL0_StopWritesTrigger = 12;

  // Maximum level to which a new compacted memtable is pushed if it
  // does not create overlap.  We try to push to level 2 to avoid the
  // relatively expensive level 0=>1 compactions and to avoid some
  // expensive manifest file operations.  We do not push all the way to
  // the largest level since that can generate a lot of wasted disk
  // space if the same key space is being repeatedly overwritten.
  static const int kMaxMemCompactLevel = 2;

  // Approximate gap in bytes between samples of data read during iteration.
  static const int kReadBytesPeriod = 1048576;

} // namespace config

class InternalKey;

// Value types encoded as the last component of internal keys.
// DO NOT CHANGE THESE ENUM VALUES: they are embedded in the on-disk
// data structures.
enum ValueType
{
  kTypeDeletion = 0x0,
  kTypeValue = 0x1
};
// kValueTypeForSeek defines the ValueType that should be passed when
// constructing a ParsedInternalKey object for seeking to a particular
// sequence number (since we sort sequence numbers in decreasing order
// and the value type is embedded as the low 8 bits in the sequence
// number in internal keys, we need to use the highest-numbered
// ValueType, not the lowest).
static const ValueType kValueTypeForSeek = kTypeValue;

typedef uint64_t SequenceNumber;

// We leave eight bits empty at the bottom so a type and sequence#
// can be packed together into 64-bits.
static const SequenceNumber kMaxSequenceNumber = ((0x1ull << 56) - 1);



struct ParsedInternalKey
{
  Slice user_key;
  SequenceNumber sequence;
  ValueType type;

  ParsedInternalKey() {} // Intentionally left uninitialized (for speed)
  ParsedInternalKey(const Slice &u, const SequenceNumber &seq, ValueType t)
      : user_key(u), sequence(seq), type(t) {}
  std::string DebugString() const;
};

// Return the length of the encoding of "key".
inline size_t InternalKeyEncodingLength(const ParsedInternalKey& key) {
  return key.user_key.size() + 8;
}

// Append the serialization of "key" to *result.
void AppendInternalKey(std::string* result, const ParsedInternalKey& key);

// Attempt to parse an internal key from "internal_key".  On success,
// stores the parsed data in "*result", and returns true.
//
// On error, returns false, leaves "*result" in an undefined state.
bool ParseInternalKey(const Slice& internal_key, ParsedInternalKey* result);


inline bool ParseInternalKey(const Slice &internal_key,
                             ParsedInternalKey *result)
{
  const size_t n = internal_key.size();
  if (n < 8)
    return false;
  uint64_t num = DecodeFixed64(internal_key.data() + n - 8);
  uint8_t c = num & 0xff;
  result->sequence = num >> 8;
  result->type = static_cast<ValueType>(c);
  result->user_key = Slice(internal_key.data(), n - 8);
  return (c <= static_cast<uint8_t>(kTypeValue));
}

static uint64_t PackSequenceAndType(uint64_t seq, ValueType t)
{
  assert(seq <= kMaxSequenceNumber);
  assert(t <= kValueTypeForSeek);
  return (seq << 8) | t;
}