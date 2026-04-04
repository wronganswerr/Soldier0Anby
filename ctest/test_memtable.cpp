#include "memtable.hpp"
#include <iostream>
#include <gtest/gtest.h>

TEST(MemtableTest, sp)
{

    const InternalKeyComparator comparator(BytewiseComparator());
    MemTable *mem = nullptr;
    mem = new MemTable(comparator);
    mem->Ref();

    // 准备要插入的用户 key / value 与序列号
    const std::string user_key = "user-key-1";
    const std::string user_value = "the-value";
    const SequenceNumber seq1 = 100; // 任意正序号

    // 插入一个普通 value
    mem->Add(seq1, kTypeValue, Slice(user_key), Slice(user_value));

    // 用 LookupKey 查找刚插入的值
    std::string got;
    Status s;
    LookupKey lookup1(Slice(user_key), seq1); // 若你的 LookupKey 构造签名不同请相应调整
    bool found = mem->Get(lookup1, &got, &s);

    // 预期：找到且状态 ok，value 与插入一致
    EXPECT_TRUE(found);
    EXPECT_TRUE(s.ok()) << "Expected OK status, got: " << s.ToString();
    EXPECT_EQ(got, user_value);
    // 现在插入一个 deletion（同 user_key，较新的序号）
    const SequenceNumber seq2 = seq1 + 1;
    mem->Add(seq2, kTypeDeletion, Slice(user_key), Slice());

    // 再查一次（使用 seq2），根据你的 Get 实现会返回 true 且 status = NotFound
    std::string got2;
    Status s2;
    LookupKey lookup2(Slice(user_key), seq2);
    bool found2 = mem->Get(lookup2, &got2, &s2);

    // 预期：Get 返回 true（表示找到 entry），但状态表示 NotFound（删除标记）
    EXPECT_TRUE(found2);
    EXPECT_FALSE(s2.ok());
    // LevelDB 风格的 Status 提供 IsNotFound()
    EXPECT_TRUE(s2.IsNotFound()) << "Expected NotFound status, got: " << s2.ToString();

    mem->Unref();
}