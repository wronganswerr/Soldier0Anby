#include "db_format.hpp"

void AppendInternalKey(std::string *result, const ParsedInternalKey &key)
{
  result->append(key.user_key.data(), key.user_key.size());
  PutFixed64(result, PackSequenceAndType(key.sequence, key.type));
}

std::string ParsedInternalKey::DebugString() const
{
  std::ostringstream ss;
//   ss << '\'' << EscapeString(user_key.ToString()) << "' @ " << sequence << " : " << static_cast<int>(type);
  return ss.str();
}











