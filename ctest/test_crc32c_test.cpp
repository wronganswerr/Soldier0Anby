// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include <gtest/gtest.h>

#include <cstdint>
#include <cstring>

#include "crc32c.hpp"

TEST(CRC, StandardResults)
{
    // From rfc3720 section B.4.
    char buf[32];

    std::memset(buf, 0, sizeof(buf));
    ASSERT_EQ(0x8a9136aa, crc32c::Value(buf, sizeof(buf)));

    std::memset(buf, 0xff, sizeof(buf));
    ASSERT_EQ(0x62a8ab43, crc32c::Value(buf, sizeof(buf)));

    for (int i = 0; i < 32; i++)
    {
        buf[i] = i;
    }
    ASSERT_EQ(0x46dd794e, crc32c::Value(buf, sizeof(buf)));

    for (int i = 0; i < 32; i++)
    {
        buf[i] = 31 - i;
    }
    ASSERT_EQ(0x113fdb5c, crc32c::Value(buf, sizeof(buf)));

    uint8_t data[48] = {
        0x01,
        0xc0,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x14,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x04,
        0x00,
        0x00,
        0x00,
        0x00,
        0x14,
        0x00,
        0x00,
        0x00,
        0x18,
        0x28,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x02,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
    };
    ASSERT_EQ(0xd9963a56,
              crc32c::Value(reinterpret_cast<const char *>(data), sizeof(data)));
}

TEST(CRC, Values)
{
    ASSERT_NE(crc32c::Value("a", 1), crc32c::Value("foo", 3));
}

TEST(CRC, Extend)
{
    ASSERT_EQ(crc32c::Value("hello world", 11),
              crc32c::Extend(crc32c::Value("hello ", 6), "world", 5));
}

TEST(CRC, Mask)
{
    uint32_t crc = crc32c::Value("foo", 3);
    ASSERT_NE(crc, crc32c::Mask(crc));
    ASSERT_NE(crc, crc32c::Mask(crc32c::Mask(crc)));
    ASSERT_EQ(crc, crc32c::Unmask(crc32c::Mask(crc)));
    ASSERT_EQ(crc, crc32c::Unmask(crc32c::Unmask(crc32c::Mask(crc32c::Mask(crc)))));
}
