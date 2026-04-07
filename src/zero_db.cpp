// 弃用！！！


#include "zero_db.hpp"
// type|key_size|key_data(ptr)|value_size|value_data(ptr)|

const int kNumNonTableCacheFiles = 10;

struct ZeroDB::Writer
{
    explicit Writer(Mutex *mu)
        : batch(nullptr), sync(false), done(false), cv(mu) {}

    Status status;
    WriteBatch *batch;
    bool sync;
    bool done;
    CondVar cv;
};

// 虽然没有IMPL后缀，但是它就是一个IMPL类，直接暴露接口给外部模块调用
Status ZeroDB::Open(InternalKeyComparator cmp, ZeroDB **dbptr)
{
    *dbptr = nullptr;
    ZeroDB *zero_db = new ZeroDB(cmp);
    zero_db->mutex_.Lock();

    zero_db->mem_ = new MemTable(zero_db->internal_comparator_);
    zero_db->mem_->Ref();

    zero_db->mutex_.unlock();

    *dbptr = zero_db;

    return Status::OK();
}

ZeroDB::ZeroDB(const Options &options, const std::string &dbname)
    : env_(options.env),
      internal_comparator_(options.comparator),
      internal_filter_policy_(options.filter_policy),
      options_(SanitizeOptions(dbname, &internal_comparator_,
                               &internal_filter_policy_, options)),
      mem_(nullptr),
      table_cache_(new TableCache(dbname_, options_, TableCacheSize(options_)))
{
}

ZeroDB::~ZeroDB()
{
    if (mem_ != nullptr)
        mem_->Unref();
}

ZeroDB::Put(const Slice &key, const Slice &value)
{
    WriteBatch write_batch;
    // ptr & size 拷贝一份到 rep_ 中，递增序列号
    write_batch.Put(key, value);
    return
}

ZeroDB::Write(WriteBatch &write_batch)
{
    // 使用mutex，支持外部的多线程调用
    Writer w(&mutex_);
    w.batch = write_batch;
    w.sync = false;
    w.done = false;
    MutexLock l(&mutex_);
    writers_.push_back(&w);
    while (!w.done && &w != writers_.front())
    {
        w.cv.Wait();
    }

    if (w.done)
    {
        return w.status;
    }

    Status status = MakeRoomForWrite(write_batch == nullptr);

    Writer *last_writer = &w;
}

ZeroDB::MakeRoomForWrite(bool force)
{
    // 检查是否可以写入到memtable层中
    mutex_.AssertHeld();
    assert(!writers_.empty());

    bool allow_delay = !force;

    Status s;

    while (true)
    {
        if (!bg_error_.ok())
        {
            s = bg_error_;
            break;
        }
        else if (allow_delay && versions_->NumLevelFiles(0) >= config::kL0_SlowdownWritesTrigger)
        {
            // We are getting close to hitting a hard limit on the number of
            // L0 files.  Rather than delaying a single write by several
            // seconds when we hit the hard limit, start delaying each
            // individual write by 1ms to reduce latency variance.  Also,
            // this delay hands over some CPU to the compaction thread in
            // case it is sharing the same core as the writer.
            // 写入延迟机制（write throttling）等待后台压缩线程
            mutex_.Unlock();
            env_->SleepForMicroseconds(1000);
            allow_delay = false; // Do not delay a single write more than once，每个任务等待不超过1次
            mutex_.Lock();
        }
        else if (!force && (mem_->ApproximateMemoryUsage() <= options_.write_buffer_size))
        {
            // 非强制 且 使用内存未到达限制的情况下 不进行压缩
            break;
        }
        else if (imm_ != nullptr)
        {
            // 等待后台写入到第0层的线程完成
            Log(options_.info_log, "Current memtable full; waiting...\n");
            background_work_finished_signal_.Wait();
        }
        else if (versions_->NumLevelFiles(0) >= config::kL0_StopWritesTrigger)
        {
            // 第0层的文件过多，等待压缩
            Log(options_.info_log, "Too many L0 files; waiting...\n");
            background_work_finished_signal_.Wait();
        }
        else
        {
            // Attempt to switch to a new memtable and trigger compaction of old
            assert(versions_->PrevLogNumber() == 0);
            uint64_t new_log_number = versions_->NewFileNumber();
            WritableFile *lfile = nullptr;
            s = env_->NewWritableFile(LogFileName(dbname_, new_log_number), &lfile);
            if (!s.ok())
            {
                // Avoid chewing through file number space in a tight loop.
                versions_->ReuseFileNumber(new_log_number);
                break;
            }

            delete log_;

            s = logfile_->Close();
            if (!s.ok())
            {
                // We may have lost some data written to the previous log file.
                // Switch to the new log file anyway, but record as a background
                // error so we do not attempt any more writes.
                //
                // We could perhaps attempt to save the memtable corresponding
                // to log file and suppress the error if that works, but that
                // would add more complexity in a critical code path.
                RecordBackgroundError(s);
            }
            delete logfile_;

            logfile_ = lfile;
            logfile_number_ = new_log_number;
            log_ = new log::Writer(lfile);
            imm_ = mem_;
            has_imm_.store(true, std::memory_order_release);
            mem_ = new MemTable(internal_comparator_);
            mem_->Ref();
            force = false; // Do not force another compaction if have room
            MaybeScheduleCompaction();
        }
    }
}

// Fix user-supplied options to be reasonable
template <class T, class V>
static void ClipToRange(T *ptr, V minvalue, V maxvalue)
{
    if (static_cast<V>(*ptr) > maxvalue)
        *ptr = maxvalue;
    if (static_cast<V>(*ptr) < minvalue)
        *ptr = minvalue;
}

Options SanitizeOptions(const std::string &dbname,
                        const InternalKeyComparator *icmp,
                        const InternalFilterPolicy *ipolicy,
                        const Options &src)
{
    Options result = src;
    result.comparator = icmp;
    result.filter_policy = (src.filter_policy != nullptr) ? ipolicy : nullptr;
    ClipToRange(&result.max_open_files, 64 + kNumNonTableCacheFiles, 50000);
    ClipToRange(&result.write_buffer_size, 64 << 10, 1 << 30);
    ClipToRange(&result.max_file_size, 1 << 20, 1 << 30);
    ClipToRange(&result.block_size, 1 << 10, 4 << 20);
    if (result.info_log == nullptr)
    {
        // Open a log file in the same directory as the db
        src.env->CreateDir(dbname); // In case it does not exist
        src.env->RenameFile(InfoLogFileName(dbname), OldInfoLogFileName(dbname));
        Status s = src.env->NewLogger(InfoLogFileName(dbname), &result.info_log);
        if (!s.ok())
        {
            // No place suitable for logging
            result.info_log = nullptr;
        }
    }

    if (result.block_cache == nullptr)
    {
        result.block_cache = NewLRUCache(8 << 20); // 这个参数是怎么来的？有什么依据
    }

    return result;
}