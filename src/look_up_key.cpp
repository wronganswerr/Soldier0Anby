#include "look_up_key.hpp"

LookupKey::LookupKey(const Slice &user_key, SequenceNumber s)
{
    size_t usize = user_key.size();
    size_t needed = usize + 13; // A conservative estimate
    char *dst;
    if (needed <= sizeof(space_))
    {
        dst = space_;
    }
    else
    {
        dst = new char[needed];
    }
    start_ = dst;
    dst = EncodeVarint32(dst, usize + 8);
    kstart_ = dst;
    std::memcpy(dst, user_key.data(), usize);
    dst += usize;
    EncodeFixed64(dst, PackSequenceAndType(s, kValueTypeForSeek));
    dst += 8;
    end_ = dst;
}