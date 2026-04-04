// 实现一个简单版本，支持kv对的 insert 和 query
#pragma once
#include "comparators/internal_key_comparator.hpp"
#include "skiplist.hpp"
#include "arena.hpp"
#include "status.hpp"
#include "iterator.hpp"
#include "look_up_key.hpp"
#include "internal_key.hpp"

class MemTable
{
private:
    friend class MemTableIterator;
    friend class MemTableBackwardIterator;

    struct KeyComparator
    {
        const InternalKeyComparator comparator;
        explicit KeyComparator(const InternalKeyComparator &c) : comparator(c) {}
        int operator()(const char *a, const char *b) const;
    };

    typedef SkipList<const char *, KeyComparator> Table;

    KeyComparator comparator_;
    int refs_;
    Arena arena_;
    Table table_;

public:
    explicit MemTable(const InternalKeyComparator &comparator);

    MemTable(const MemTable &) = delete;
    MemTable &operator=(const MemTable &) = delete;

    ~MemTable();

    // 引用计数
    // Increase reference count.
    void Ref() { ++refs_; }

    // Drop reference count.  Delete if no more references exist.
    void Unref()
    {
        --refs_;
        assert(refs_ >= 0);
        if (refs_ <= 0)
        {
            delete this;
        }
    }

    // Returns an estimate of the number of bytes of data in use by this
    // data structure. It is safe to call when MemTable is being modified.
    size_t ApproximateMemoryUsage();

    // Return an iterator that yields the contents of the memtable.
    //
    // The caller must ensure that the underlying MemTable remains live
    // while the returned iterator is live.  The keys returned by this
    // iterator are internal keys encoded by AppendInternalKey in the
    // db/format.{h,cc} module.
    Iterator *NewIterator();

    // Add an entry into memtable that maps key to value at the
    // specified sequence number and with the specified type.
    // Typically value will be empty if type==kTypeDeletion.
    void Add(SequenceNumber seq, ValueType type, const Slice &key,
             const Slice &value);

    // If memtable contains a value for key, store it in *value and return true.
    // If memtable contains a deletion for key, store a NotFound() error
    // in *status and return true.
    // Else, return false.
    bool Get(const LookupKey &key, std::string *value, Status *s);
};

