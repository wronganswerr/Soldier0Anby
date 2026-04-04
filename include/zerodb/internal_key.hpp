#pragma once
#include <string>
#include "slice.hpp"
#include "db_format.hpp"

class InternalKey
{
private:
  std::string rep_;

public:
  InternalKey() {} // Leave rep_ as empty to indicate it is invalid
  InternalKey(const Slice &user_key, SequenceNumber s, ValueType t)
  {
    AppendInternalKey(&rep_, ParsedInternalKey(user_key, s, t));
  }

  bool DecodeFrom(const Slice &s)
  {
    rep_.assign(s.data(), s.size());
    return !rep_.empty();
  }

  Slice Encode() const
  {
    assert(!rep_.empty());
    return rep_;
  }

  Slice user_key() const { return ExtractUserKey(rep_); }

  void SetFrom(const ParsedInternalKey &p)
  {
    rep_.clear();
    AppendInternalKey(&rep_, p);
  }

  void Clear() { rep_.clear(); }

  std::string DebugString() const;
};



