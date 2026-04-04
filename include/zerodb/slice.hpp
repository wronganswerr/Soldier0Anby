#pragma once

#include <cassert>
#include <cstddef>
#include <cstring>
#include <string>

namespace zerodb
{

    // 在基础变量上封装的统一对外的表现形式。（是一个视图）
    class Slice
    {
    private:
        const char *data_;
        size_t size_;

    public:
        // Create an empty slice.
        Slice() : data_(""), size_(0) {}

        // Create a slice that refers to d[0,n-1].
        Slice(const char *d, size_t n) : data_(d), size_(n) {}

        // Create a slice that refers to the contents of "s"
        Slice(const std::string &s) : data_(s.data()), size_(s.size()) {}

        // Create a slice that refers to s[0,strlen(s)-1]
        Slice(const char *s) : data_(s), size_(strlen(s)) {}

        // Intentionally copyable.
        Slice(const Slice &) = default;
        Slice &operator=(const Slice &) = default;

        const char *data() const
        {
            return data_;
        }

        size_t size() const
        {
            return size_;
        }

        bool empty() const
        {
            return size_ == 0;
        }

        const char *begin() const { return data(); }
        const char *end() const { return data() + size(); }

        // Return the ith byte in the referenced data.
        // REQUIRES: n < size()
        char operator[](size_t n) const
        {
            assert(n < size());
            return data_[n];
        }

        // Change this slice to refer to an empty array
        void clear()
        {
            data_ = "";
            size_ = 0;
        }

        // Drop the first "n" bytes from this slice.
        void remove_prefix(size_t n)
        {
            assert(n <= size());
            data_ += n;
            size_ -= n;
        }

        std::string ToString() const { return std::string(data_, size_); }

        // Three-way comparison.  Returns value:
        //   <  0 iff "*this" <  "b",
        //   == 0 iff "*this" == "b",
        //   >  0 iff "*this" >  "b"
        // 从实现上来看，使用字符串比较方法来逐字比较数据
        int compare(const Slice &b) const;

        // Return true iff "x" is a prefix of "*this"
        bool starts_with(const Slice &x) const
        {
            return ((size_ >= x.size_) && (memcmp(data_, x.data_, x.size_) == 0));
        }
    };

    inline bool operator==(const Slice &x, const Slice &y)
    {
        return ((x.size() == y.size()) &&
                (memcmp(x.data(), y.data(), x.size()) == 0));
    }

    inline bool operator!=(const Slice &x, const Slice &y) { return !(x == y); }

    inline int Slice::compare(const Slice &b) const
    {
        const size_t min_len = (size_ < b.size_) ? size_ : b.size_;
        int r = memcmp(data_, b.data_, min_len);
        if (r == 0)
        {
            if (size_ < b.size_)
                r = -1;
            else if (size_ > b.size_)
                r = +1;
        }
        return r;
    }

    inline Slice ExtractUserKey(const Slice &internal_key)
    {
        assert(internal_key.size() >= 8);
        return Slice(internal_key.data(), internal_key.size() - 8);
    }
}