#pragma once
#include <deque>

#include "memtable.hpp"
#include "status.hpp"
#include "stdcxx.hpp"
#include "write_batch.hpp"
#include "version_set.hpp"
#include "env.hpp"
#include "options.hpp"
#include "filename.hpp"
#include "internal_filter_policy.hpp"
#include "cache.hpp"

class ZeroDB
{
private:
    /* data */

    Env *const env_;
    const InternalKeyComparator internal_comparator_;
    const InternalFilterPolicy internal_filter_policy_;
    
    MemTable *mem_; // memory table 内存级别的数据存储
    MemTable *imm_ GUARDED_BY(mutex_); // 等待写入到磁盘的内存

    Mutex mutex_;
    CondVar background_work_finished_signal_ GUARDED_BY(mutex_); // 锁保护，防御工程

    struct Writer;

    // Queue of writers.
    std::deque<Writer *> writers_ GUARDED_BY(mutex_);

    Status bg_error_ GUARDED_BY(mutex_); // 后台线程的状态

    VersionSet *const versions_ GUARDED_BY(mutex_); // 版本号管理

    const Options options_;
    const std::string dbname_;

    TableCache* const table_cache_;

public:
    static Status Open(InternalKeyComparator cmp, ZeroDB **dbptr);

    ZeroDB(const Options &options, const std::string &dbname);
    ZeroDB(const ZeroDB &) = delete;
    ZeroDB &operator=(const ZeroDB &) = delete;

    ~ZeroDB();

    Status Put(const Slice &key, const Slice &value);

    Status Write(WriteBatch &write_batch);

    Status MakeRoomForWrite(bool force /* compact even if there is room? */)
        EXCLUSIVE_LOCKS_REQUIRED(mutex_);

    // 暴露出外部接口
};

Options SanitizeOptions(const std::string &db,
                        const InternalKeyComparator *icmp,
                        const InternalFilterPolicy *ipolicy,
                        const Options &src);
