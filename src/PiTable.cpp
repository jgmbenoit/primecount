///
/// @file  PiTable.cpp
/// @brief The PiTable class is a compressed lookup table of prime
///        counts. Each bit of the lookup table corresponds to an
///        integer that is not divisible by 2, 3 and 5. The 8 bits of
///        each byte correspond to the offsets { 1, 7, 11, 13, 17, 19,
///        23, 29 }. Since our lookup table uses the uint64_t data
///        type, one array element (8 bytes) corresponds to an
///        interval of size 30 * 8 = 240.
///
/// Copyright (C) 2024 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <PiTable.hpp>
#include <primecount-internal.hpp>
#include <primesieve.hpp>
#include <Vector.hpp>
#include <imath.hpp>
#include <macros.hpp>
#include <min.hpp>

#include <stdint.h>
#include <algorithm>

namespace primecount {

/// Compressed PrimePi(x) lookup table for x < 64 * 240.
/// This lookup table has a size of 2 KiB, the 8 bits of each byte
/// correspond to the offsets { 1, 7, 11, 13, 17, 19, 23, 29 }.
/// This table has been generated by printing out the content of
/// the sieve_ vector that is filled in PiTable::init().
///
/// Array format: { bit_count, bits }
/// @bit_count: PrimePi(5) + count of 1-bits < current_index.
/// @bits: 64-bit word whose 1-bits correspond to primes.
///
const Array<PiTable::pi_t, 128> PiTable::pi_cache_ =
{{
  {    3, 0xF93DDBB67EEFDFFEull }, {   52, 0x9EEDA6EAF31E4FD5ull },
  {   92, 0xA559DD3BD3D30CE6ull }, {  128, 0x56A61E78BD92676Aull },
  {  162, 0x554C2ADE2DADE356ull }, {  196, 0xF8A154039FF0A3D9ull },
  {  228, 0x3A13F666E944FD2Eull }, {  263, 0x54BF11453A2B4CB8ull },
  {  293, 0x4F8CBCC8B37AC18Cull }, {  325, 0xEF17C19B71715821ull },
  {  357, 0x468C83E5081A9654ull }, {  382, 0x87588F9265AEFB72ull },
  {  417, 0xA0E3266581D892D2ull }, {  444, 0x99EB813C26C73811ull },
  {  473, 0x4D33F3243E88518Dull }, {  503, 0x4C58B42AA71C8B5Aull },
  {  532, 0xC383DC8219F6264Eull }, {  562, 0x02CDCDB50238F12Cull },
  {  590, 0x307A4C570C944AB2ull }, {  617, 0xF8246C44CBF10B43ull },
  {  646, 0x8DEA735CA8950119ull }, {  675, 0xC41E22A6502B9624ull },
  {  700, 0x9C742F3AD40648D1ull }, {  729, 0x2E1568BF88056A07ull },
  {  757, 0x14089851B7E35560ull }, {  783, 0x2770494D45AA5A86ull },
  {  811, 0x618302ABCAD593D2ull }, {  840, 0xADA9C22287CE2405ull },
  {  867, 0xB01689D1784D8C18ull }, {  893, 0x522434C0A262C757ull },
  {  919, 0x4308218D32405AAEull }, {  942, 0x60E119D9B6D2B634ull },
  {  973, 0x947A44D060391A67ull }, { 1000, 0x105574A88388099Aull },
  { 1023, 0x32C8231E685DA127ull }, { 1051, 0x38B14873440319E0ull },
  { 1075, 0x1CB59861572AE6C3ull }, { 1106, 0x2902AC8F81C5680Aull },
  { 1130, 0x2E644E1194E3471Aull }, { 1158, 0x1006C514DC3DCB14ull },
  { 1184, 0xE34730E982B129E9ull }, { 1214, 0xB430300A25C31934ull },
  { 1237, 0x4C8ED84446E5C16Cull }, { 1265, 0x818992787024225Dull },
  { 1289, 0xA508E9861B265682ull }, { 1315, 0x104AC2B029C3D300ull },
  { 1337, 0xC760421DA13859B2ull }, { 1364, 0x8BC61A44C88C2722ull },
  { 1389, 0x0931A610461A8182ull }, { 1409, 0x15A9D8D2182F54F0ull },
  { 1438, 0x91500EC0F60C2E06ull }, { 1462, 0xC319653818C126CDull },
  { 1489, 0x4A84D62D2A8B9356ull }, { 1518, 0xC476E0092CA50A61ull },
  { 1543, 0x1B6614E808D83C6Aull }, { 1570, 0x073110366302A4B0ull },
  { 1592, 0xA08AC312424892D5ull }, { 1615, 0x5C788582A4742D9Full },
  { 1645, 0xE8021D1461B0180Dull }, { 1667, 0x30831C4901C11218ull },
  { 1686, 0xF40C0FD888A13367ull }, { 1715, 0xB1474266D7588898ull },
  { 1743, 0x155941180896A816ull }, { 1765, 0xA1AAB3E1522A44B5ull },
  { 1794, 0x0CA5111855624559ull }, { 1818, 0x0AD654BE00673CA3ull },
  { 1847, 0x7E08150C826B3620ull }, { 1871, 0x840A61D078019156ull },
  { 1893, 0x50A0560E57012CA8ull }, { 1916, 0x1063206C478C980Bull },
  { 1939, 0x750B88570CE409C4ull }, { 1965, 0x022439C28252C20Bull },
  { 1986, 0xA3D629317A25562Cull }, { 2016, 0x328A0CB018B1E120ull },
  { 2038, 0x3730ADC5093211C1ull }, { 2064, 0x6B2520CF8291BC08ull },
  { 2090, 0x076A4626448F309Cull }, { 2116, 0xC525021058098E49ull },
  { 2137, 0x903C80A0805A42C4ull }, { 2156, 0x0C518403E1146428ull },
  { 2176, 0x7E47C008A48AA32Eull }, { 2203, 0x04002A54CD032BD3ull },
  { 2226, 0xC1834C29426C92B3ull }, { 2252, 0x38DB21462D1CCC92ull },
  { 2280, 0xE0BB5812248C8459ull }, { 2305, 0x912809C930700D06ull },
  { 2326, 0xC280308CF9324441ull }, { 2348, 0x1483817D0C62A472ull },
  { 2373, 0x14780A82150EAAE1ull }, { 2397, 0xB2F02E6F10089770ull },
  { 2425, 0x866253324449301Dull }, { 2449, 0xD9364B110A844565ull },
  { 2475, 0x197C9080613A698Cull }, { 2500, 0x2D050C8B409530C0ull },
  { 2521, 0x1A8596B4A171C00Eull }, { 2547, 0xB7484C511415C016ull },
  { 2571, 0xA1022E8A89109579ull }, { 2595, 0x220891108190D51Cull },
  { 2614, 0x5C2033C078E91EB4ull }, { 2642, 0x471023AAE20EC48Bull },
  { 2668, 0xA851A1197B5528E3ull }, { 2697, 0x6061D12C82900406ull },
  { 2716, 0x23548144410652A1ull }, { 2736, 0x4872222704A91888ull },
  { 2757, 0x29CA1712421C40B6ull }, { 2781, 0x898452E13C015AA0ull },
  { 2804, 0xD2692CF1064001DAull }, { 2829, 0xC88A3421C1634248ull },
  { 2851, 0x442E88092671216Cull }, { 2874, 0xD11286981D9228D5ull },
  { 2900, 0x5014462046A0A352ull }, { 2920, 0x8CA9445083DDDC83ull },
  { 2948, 0x391B8914542E144Dull }, { 2974, 0x02808F2981148042ull },
  { 2991, 0x0C05B08382963203ull }, { 3012, 0x1AECD9F807885114ull },
  { 3040, 0x230686435C314806ull }, { 3062, 0xB2F000A50C4409B3ull },
  { 3085, 0xB618C242E1CA0180ull }, { 3107, 0x612C29522EC79B2Cull },
  { 3136, 0x5E80848B24268A1Aull }, { 3159, 0x2145352A53C10260ull },
  { 3181, 0x04484AC5B842D152ull }, { 3204, 0xC45009C161237016ull },
  { 3226, 0x28221601D9188881ull }, { 3245, 0x09532438EB84908Cull },
  { 3269, 0x30860982146A41A9ull }, { 3290, 0x5A952B004238A29Cull }
}};

PiTable::PiTable(uint64_t max_x, int threads) :
  max_x_(max_x)
{
  // Initialize PiTable from cache
  uint64_t limit = max_x + 1;
  pi_.resize(ceil_div(limit, 240));
  std::size_t n = min(pi_cache_.size(), pi_.size());
  std::copy_n(&pi_cache_[0], n, &pi_[0]);

  uint64_t cache_limit = pi_cache_.size() * 240;
  if (limit > cache_limit)
    init(limit, cache_limit, threads);
}

/// Used if PiTable larger than pi_cache
void PiTable::init(uint64_t limit,
                   uint64_t cache_limit,
                   int threads)
{
  ASSERT(cache_limit < limit);
  uint64_t dist = limit - cache_limit;
  uint64_t thread_threshold = (uint64_t) 1e7;
  threads = ideal_num_threads(dist, threads, thread_threshold);
  uint64_t thread_dist = dist / threads;
  thread_dist = max(thread_threshold, thread_dist);
  thread_dist += 240 - thread_dist % 240;
  counts_.resize(threads);

  #pragma omp parallel num_threads(threads)
  {
    #pragma omp for
    for (int t = 0; t < threads; t++)
    {
      uint64_t low = cache_limit + thread_dist * t;
      uint64_t high = low + thread_dist;
      high = min(high, limit);

      if (low < high)
        init_bits(low, high, t);
    }

    #pragma omp for
    for (int t = 0; t < threads; t++)
    {
      uint64_t low = cache_limit + thread_dist * t;
      uint64_t high = low + thread_dist;
      high = min(high, limit);

      if (low < high)
        init_count(low, high, t);
    }
  }
}

/// Each thread computes PrimePi [low, high[
void PiTable::init_bits(uint64_t low,
                        uint64_t high,
                        uint64_t thread_num)
{
  // Zero initialize pi vector
  uint64_t i = low / 240;
  uint64_t j = ceil_div(high, 240);
  std::fill_n(&pi_[i], j - i, pi_t{0, 0});

  // Iterate over primes >= 7
  low = max(low, 7);
  primesieve::iterator it(low, high);
  uint64_t count = 0;
  uint64_t prime = 0;

  while ((prime = it.next_prime()) < high)
  {
    uint64_t prime_bit = set_bit_[prime % 240];
    pi_[prime / 240].bits |= prime_bit;
    count += 1;
  }

  counts_[thread_num] = count;
}

/// Each thread computes PrimePi [low, high[
void PiTable::init_count(uint64_t low,
                         uint64_t high,
                         uint64_t thread_num)
{
  // First compute PrimePi[low - 1]
  pi_t cache_last = pi_cache_.back();
  uint64_t count = cache_last.count + popcnt64(cache_last.bits);
  for (uint64_t i = 0; i < thread_num; i++)
    count += counts_[i];

  // Convert to array indexes
  uint64_t i = low / 240;
  uint64_t stop_idx = ceil_div(high, 240);

  for (; i < stop_idx; i++)
  {
    pi_[i].count = count;
    count += popcnt64(pi_[i].bits);
  }
}

} // namespace
