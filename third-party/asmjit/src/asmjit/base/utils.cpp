// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Dependencies - AsmJit]
#include "../base/utils.h"

// [Dependencies - Posix]
#if ASMJIT_OS_POSIX
# include <time.h>
# include <unistd.h>
#endif // ASMJIT_OS_POSIX

// [Dependencies - Mac]
#if ASMJIT_OS_MAC
# include <mach/mach_time.h>
#endif // ASMJIT_OS_MAC

// [Dependencies - Windows]
#if ASMJIT_OS_WINDOWS
# if defined(_MSC_VER) && _MSC_VER >= 1400
#  include <intrin.h>
# else
#  define _InterlockedCompareExchange InterlockedCompareExchange
# endif // _MSC_VER
#endif // ASMJIT_OS_WINDOWS

// [Api-Begin]
#include "../apibegin.h"

namespace asmjit {

// ============================================================================
// [asmjit::CpuTicks - Windows]
// ============================================================================

#if ASMJIT_OS_WINDOWS
static volatile uint32_t Utils_hiResTicks;
static volatile double Utils_hiResFreq;

uint32_t Utils::getTickCount() noexcept {
  do {
    uint32_t hiResOk = Utils_hiResTicks;

    if (hiResOk == 1) {
      LARGE_INTEGER now;
      if (!::QueryPerformanceCounter(&now))
        break;
      return (int64_t)(double(now.QuadPart) / Utils_hiResFreq);
    }

    if (hiResOk == 0) {
      LARGE_INTEGER qpf;
      if (!::QueryPerformanceFrequency(&qpf)) {
        _InterlockedCompareExchange((LONG*)&Utils_hiResTicks, 0xFFFFFFFF, 0);
        break;
      }

      LARGE_INTEGER now;
      if (!::QueryPerformanceCounter(&now)) {
        _InterlockedCompareExchange((LONG*)&Utils_hiResTicks, 0xFFFFFFFF, 0);
        break;
      }

      double freqDouble = double(qpf.QuadPart) / 1000.0;
      Utils_hiResFreq = freqDouble;
      _InterlockedCompareExchange((LONG*)&Utils_hiResTicks, 1, 0);

      return static_cast<uint32_t>(
        static_cast<int64_t>(double(now.QuadPart) / freqDouble) & 0xFFFFFFFF);
    }
  } while (0);

  // Bail to a less precise GetTickCount().
  return ::GetTickCount();
}

// ============================================================================
// [asmjit::CpuTicks - Mac]
// ============================================================================

#elif ASMJIT_OS_MAC
static mach_timebase_info_data_t CpuTicks_machTime;

uint32_t Utils::getTickCount() noexcept {
  // Initialize the first time CpuTicks::now() is called (See Apple's QA1398).
  if (CpuTicks_machTime.denom == 0) {
    if (mach_timebase_info(&CpuTicks_machTime) != KERN_SUCCESS)
      return 0;
  }

  // mach_absolute_time() returns nanoseconds, we need just milliseconds.
  uint64_t t = mach_absolute_time() / 1000000;

  t = t * CpuTicks_machTime.numer / CpuTicks_machTime.denom;
  return static_cast<uint32_t>(t & 0xFFFFFFFFU);
}

// ============================================================================
// [asmjit::CpuTicks - Posix]
// ============================================================================

#else
uint32_t Utils::getTickCount() noexcept {
#if defined(_POSIX_MONOTONIC_CLOCK) && _POSIX_MONOTONIC_CLOCK >= 0
  struct timespec ts;

  if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
    return 0;

  uint64_t t = (uint64_t(ts.tv_sec ) * 1000) + (uint64_t(ts.tv_nsec) / 1000000);
  return static_cast<uint32_t>(t & 0xFFFFFFFFU);
#else  // _POSIX_MONOTONIC_CLOCK
#error "[asmjit] Utils::getTickCount() is not implemented for your target OS."
  return 0;
#endif  // _POSIX_MONOTONIC_CLOCK
}
#endif // ASMJIT_OS

// ============================================================================
// [asmjit::Utils - Unit]
// ============================================================================

#if defined(ASMJIT_TEST)
UNIT(base_utils) {
  uint32_t i;

  INFO("IntTraits<>.");
  EXPECT(IntTraits<signed char>::kIsSigned,"IntTraits<signed char> should report signed.");
  EXPECT(IntTraits<short>::kIsSigned, "IntTraits<signed short> should report signed.");
  EXPECT(IntTraits<int>::kIsSigned, "IntTraits<int> should report signed.");
  EXPECT(IntTraits<long>::kIsSigned, "IntTraits<long> should report signed.");

  EXPECT(IntTraits<unsigned char>::kIsUnsigned, "IntTraits<unsigned char> should report unsigned.");
  EXPECT(IntTraits<unsigned short>::kIsUnsigned, "IntTraits<unsigned short> should report unsigned.");
  EXPECT(IntTraits<unsigned int>::kIsUnsigned, "IntTraits<unsigned int> should report unsigned.");
  EXPECT(IntTraits<unsigned long>::kIsUnsigned, "IntTraits<unsigned long> should report unsigned.");

  EXPECT(IntTraits<intptr_t>::kIsSigned, "IntTraits<intptr_t> should report signed.");
  EXPECT(IntTraits<uintptr_t>::kIsUnsigned, "IntTraits<uintptr_t> should report unsigned.");

  EXPECT(IntTraits<intptr_t>::kIsIntPtr, "IntTraits<intptr_t> should report intptr_t type.");
  EXPECT(IntTraits<uintptr_t>::kIsIntPtr, "IntTraits<uintptr_t> should report intptr_t type.");

  INFO("Utils::iMin()/iMax().");
  EXPECT(Utils::iMin<int>( 0, -1) == -1, "Utils::iMin<int> should return a minimum value.");
  EXPECT(Utils::iMin<int>(-1, -2) == -2, "Utils::iMin<int> should return a minimum value.");
  EXPECT(Utils::iMin<int>( 1,  2) ==  1, "Utils::iMin<int> should return a minimum value.");

  EXPECT(Utils::iMax<int>( 0, -1) ==  0, "Utils::iMax<int> should return a maximum value.");
  EXPECT(Utils::iMax<int>(-1, -2) == -1, "Utils::iMax<int> should return a maximum value.");
  EXPECT(Utils::iMax<int>( 1,  2) ==  2, "Utils::iMax<int> should return a maximum value.");

  INFO("Utils::inInterval().");
  EXPECT(Utils::inInterval<int>(11 , 10, 20) == true , "Utils::inInterval<int> should return true if inside.");
  EXPECT(Utils::inInterval<int>(101, 10, 20) == false, "Utils::inInterval<int> should return false if outside.");

  INFO("Utils::isInt8().");
  EXPECT(Utils::isInt8(-128) == true , "Utils::isInt8<> should return true if inside.");
  EXPECT(Utils::isInt8( 127) == true , "Utils::isInt8<> should return true if inside.");
  EXPECT(Utils::isInt8(-129) == false, "Utils::isInt8<> should return false if outside.");
  EXPECT(Utils::isInt8( 128) == false, "Utils::isInt8<> should return false if outside.");

  INFO("Utils::isInt16().");
  EXPECT(Utils::isInt16(-32768) == true , "Utils::isInt16<> should return true if inside.");
  EXPECT(Utils::isInt16( 32767) == true , "Utils::isInt16<> should return true if inside.");
  EXPECT(Utils::isInt16(-32769) == false, "Utils::isInt16<> should return false if outside.");
  EXPECT(Utils::isInt16( 32768) == false, "Utils::isInt16<> should return false if outside.");

  INFO("Utils::isInt32().");
  EXPECT(Utils::isInt32( 2147483647    ) == true, "Utils::isInt32<int> should return true if inside.");
  EXPECT(Utils::isInt32(-2147483647 - 1) == true, "Utils::isInt32<int> should return true if inside.");
  EXPECT(Utils::isInt32(ASMJIT_UINT64_C(2147483648)) == false, "Utils::isInt32<int> should return false if outside.");
  EXPECT(Utils::isInt32(ASMJIT_UINT64_C(0xFFFFFFFF)) == false, "Utils::isInt32<int> should return false if outside.");
  EXPECT(Utils::isInt32(ASMJIT_UINT64_C(0xFFFFFFFF) + 1) == false, "Utils::isInt32<int> should return false if outside.");

  INFO("Utils::isUInt8().");
  EXPECT(Utils::isUInt8(0)   == true , "Utils::isUInt8<> should return true if inside.");
  EXPECT(Utils::isUInt8(255) == true , "Utils::isUInt8<> should return true if inside.");
  EXPECT(Utils::isUInt8(256) == false, "Utils::isUInt8<> should return false if outside.");
  EXPECT(Utils::isUInt8(-1)  == false, "Utils::isUInt8<> should return false if negative.");

  INFO("Utils::isUInt12().");
  EXPECT(Utils::isUInt12(0)    == true , "Utils::isUInt12<> should return true if inside.");
  EXPECT(Utils::isUInt12(4095) == true , "Utils::isUInt12<> should return true if inside.");
  EXPECT(Utils::isUInt12(4096) == false, "Utils::isUInt12<> should return false if outside.");
  EXPECT(Utils::isUInt12(-1)   == false, "Utils::isUInt12<> should return false if negative.");

  INFO("Utils::isUInt16().");
  EXPECT(Utils::isUInt16(0)     == true , "Utils::isUInt16<> should return true if inside.");
  EXPECT(Utils::isUInt16(65535) == true , "Utils::isUInt16<> should return true if inside.");
  EXPECT(Utils::isUInt16(65536) == false, "Utils::isUInt16<> should return false if outside.");
  EXPECT(Utils::isUInt16(-1)    == false, "Utils::isUInt16<> should return false if negative.");

  INFO("Utils::isUInt32().");
  EXPECT(Utils::isUInt32(ASMJIT_UINT64_C(0xFFFFFFFF)) == true, "Utils::isUInt32<uint64_t> should return true if inside.");
  EXPECT(Utils::isUInt32(ASMJIT_UINT64_C(0xFFFFFFFF) + 1) == false, "Utils::isUInt32<uint64_t> should return false if outside.");
  EXPECT(Utils::isUInt32(-1) == false, "Utils::isUInt32<int> should return false if negative.");

  INFO("Utils::isPower2().");
  for (i = 0; i < 64; i++) {
    EXPECT(Utils::isPowerOf2(static_cast<uint64_t>(1) << i) == true,
      "Utils::isPower2() didn't report power of 2.");
    EXPECT(Utils::isPowerOf2((static_cast<uint64_t>(1) << i) ^ 0x001101) == false,
      "Utils::isPower2() didn't report not power of 2.");
  }

  INFO("Utils::mask().");
  for (i = 0; i < 32; i++) {
    EXPECT(Utils::mask(i) == (1 << i),
      "Utils::mask(%u) should return %X.", i, (1 << i));
  }

  INFO("Utils::bits().");
  for (i = 0; i < 32; i++) {
    uint32_t expectedBits = 0;

    for (uint32_t b = 0; b < i; b++)
      expectedBits |= static_cast<uint32_t>(1) << b;

    EXPECT(Utils::bits(i) == expectedBits,
      "Utils::bits(%u) should return %X.", i, expectedBits);
  }

  INFO("Utils::hasBit().");
  for (i = 0; i < 32; i++) {
    EXPECT(Utils::hasBit((1 << i), i) == true,
      "Utils::hasBit(%X, %u) should return true.", (1 << i), i);
  }

  INFO("Utils::bitCount().");
  for (i = 0; i < 32; i++) {
    EXPECT(Utils::bitCount((1 << i)) == 1,
      "Utils::bitCount(%X) should return true.", (1 << i));
  }
  EXPECT(Utils::bitCount(0x000000F0) ==  4, "");
  EXPECT(Utils::bitCount(0x10101010) ==  4, "");
  EXPECT(Utils::bitCount(0xFF000000) ==  8, "");
  EXPECT(Utils::bitCount(0xFFFFFFF7) == 31, "");
  EXPECT(Utils::bitCount(0x7FFFFFFF) == 31, "");

  INFO("Utils::findFirstBit().");
  for (i = 0; i < 32; i++) {
    EXPECT(Utils::findFirstBit((1 << i)) == i,
      "Utils::findFirstBit(%X) should return %u.", (1 << i), i);
  }

  INFO("Utils::keepNOnesFromRight().");
  EXPECT(Utils::keepNOnesFromRight(0xF, 1) == 0x1, "");
  EXPECT(Utils::keepNOnesFromRight(0xF, 2) == 0x3, "");
  EXPECT(Utils::keepNOnesFromRight(0xF, 3) == 0x7, "");
  EXPECT(Utils::keepNOnesFromRight(0x5, 2) == 0x5, "");
  EXPECT(Utils::keepNOnesFromRight(0xD, 2) == 0x5, "");

  INFO("Utils::isAligned().");
  EXPECT(Utils::isAligned<size_t>(0xFFFF,  4) == false, "");
  EXPECT(Utils::isAligned<size_t>(0xFFF4,  4) == true , "");
  EXPECT(Utils::isAligned<size_t>(0xFFF8,  8) == true , "");
  EXPECT(Utils::isAligned<size_t>(0xFFF0, 16) == true , "");

  INFO("Utils::alignTo().");
  EXPECT(Utils::alignTo<size_t>(0xFFFF,  4) == 0x10000, "");
  EXPECT(Utils::alignTo<size_t>(0xFFF4,  4) == 0x0FFF4, "");
  EXPECT(Utils::alignTo<size_t>(0xFFF8,  8) == 0x0FFF8, "");
  EXPECT(Utils::alignTo<size_t>(0xFFF0, 16) == 0x0FFF0, "");
  EXPECT(Utils::alignTo<size_t>(0xFFF0, 32) == 0x10000, "");

  INFO("Utils::alignToPowerOf2().");
  EXPECT(Utils::alignToPowerOf2<size_t>(0xFFFF) == 0x10000, "");
  EXPECT(Utils::alignToPowerOf2<size_t>(0xF123) == 0x10000, "");
  EXPECT(Utils::alignToPowerOf2<size_t>(0x0F00) == 0x01000, "");
  EXPECT(Utils::alignToPowerOf2<size_t>(0x0100) == 0x00100, "");
  EXPECT(Utils::alignToPowerOf2<size_t>(0x1001) == 0x02000, "");

  INFO("Utils::alignDiff().");
  EXPECT(Utils::alignDiff<size_t>(0xFFFF,  4) ==  1, "");
  EXPECT(Utils::alignDiff<size_t>(0xFFF4,  4) ==  0, "");
  EXPECT(Utils::alignDiff<size_t>(0xFFF8,  8) ==  0, "");
  EXPECT(Utils::alignDiff<size_t>(0xFFF0, 16) ==  0, "");
  EXPECT(Utils::alignDiff<size_t>(0xFFF0, 32) == 16, "");
}
#endif // ASMJIT_TEST

} // asmjit namespace

// [Api-End]
#include "../apiend.h"
