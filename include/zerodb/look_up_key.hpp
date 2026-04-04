#include "slice.hpp"
#include "db_format.hpp"

// A helper class useful for DBImpl::Get()
class LookupKey
{
public:
    // Initialize *this for looking up user_key at a snapshot with
    // the specified sequence number.
    LookupKey(const Slice &user_key, SequenceNumber sequence);

    LookupKey(const LookupKey &) = delete;
    LookupKey &operator=(const LookupKey &) = delete;

    ~LookupKey();

    // Return a key suitable for lookup in a MemTable.
    Slice memtable_key() const { return Slice(start_, end_ - start_); }

    // Return an internal key (suitable for passing to an internal iterator)
    Slice internal_key() const { return Slice(kstart_, end_ - kstart_); }

    // Return the user key
    Slice user_key() const { return Slice(kstart_, end_ - kstart_ - 8); }

private:
    // We construct a char array of the form:
    //    klength  varint32               <-- start_
    //    userkey  char[klength]          <-- kstart_
    //    tag      uint64
    //                                    <-- end_
    // The array is a suitable MemTable key.
    // The suffix starting with "userkey" can be used as an InternalKey.
    const char *start_;
    const char *kstart_;
    const char *end_;
    char space_[200]; // Avoid allocation for short keys
};

inline LookupKey::~LookupKey()
{
    if (start_ != space_)
        delete[] start_;
}