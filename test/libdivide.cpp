///
/// @file  libdivide.cpp
/// @brief Test the branchfree divider of libdivide.h
///
/// Copyright (C) 2018 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <libdivide.h>
#include <stdint.h>

uint64_t dividends[20] =
{
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 63, 101, 511,
    1 << 5, 1 << 9, 1 << 20,
    (uint64_t) ((1ull << 31) - 1),
    (uint64_t) ((1ull << 63) - 1),
    (uint32_t) (~0ull),
    (uint64_t) (~0ull)
};

int main()
{
  for (int i = 0; i < 20; i++)
  {
    for (int j = 2; j < 10000; j++)
    {
      libdivide::divider<uint64_t, libdivide::BRANCHFREE> fast_d(j);
      uint64_t res1 = dividends[i] / j;
      uint64_t res2 = dividends[i] / fast_d;

      if (res1 != res2)
        return 1;
    }
  }

  for (int i = 0; i < 20; i++)
  {
    for (int j = 2; j < 20; j++)
    {
      libdivide::divider<uint64_t, libdivide::BRANCHFREE> fast_d(dividends[j]);
      uint64_t res1 = dividends[i] / dividends[j];
      uint64_t res2 = dividends[i] / fast_d;

      if (res1 != res2)
        return 1;
    }
  }

  return 0;
}
