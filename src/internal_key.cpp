#include "internal_key.hpp"



std::string InternalKey::DebugString() const
{
  ParsedInternalKey parsed;
  if (ParseInternalKey(rep_, &parsed))
  {
    return parsed.DebugString();
  }
  std::ostringstream ss;
  // ss << "(bad)" << EscapeString(rep_);
  return ss.str();
}