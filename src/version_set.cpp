#include "version_set.hpp"

#include <algorithm>
#include <cstdio>

int FindFile(const InternalKeyComparator &icmp,
             const std::vector<FileMetaData *> &files, const Slice &key)
{
    uint32_t left = 0;
    uint32_t right = files.size();
    while (left < right)
    {
        uint32_t mid = (left + right) / 2;
        const FileMetaData *f = files[mid];
        if (icmp.InternalKeyComparator::Compare(f->largest.Encode(), key) < 0)
        {
            // Key at "mid.largest" is < "target".  Therefore all
            // files at or before "mid" are uninteresting.
            left = mid + 1;
        }
        else
        {
            // Key at "mid.largest" is >= "target".  Therefore all files
            // after "mid" are uninteresting.
            right = mid;
        }
    }
    return right;
}

int VersionSet::NumLevelFiles(int level) const
{
    //获取某一层的文件数量（磁盘上的文件）
    assert(level >= 0);
    assert(level < config::kNumLevels);
    return current_->files_[level].size();
}