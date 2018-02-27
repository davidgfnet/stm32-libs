
#ifndef __COMMON_FILE__H__
#define __COMMON_FILE__H__

// Endianess detection and endianess conversion related macros
#if !defined(__BYTE_ORDER__) || !defined(__ORDER_LITTLE_ENDIAN__) || !defined(__ORDER_BIG_ENDIAN__)
  #error "Missing some defines here!"
#endif

// Macros to read/write words in different endianess

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  #define read64be(x) __builtin_bswap64(x)
  #define read64le(x)                  (x)
  #define read32be(x) __builtin_bswap32(x)
  #define read32le(x)                  (x)
  #define read16be(x) __builtin_bswap16(x)
  #define read16le(x)                  (x)
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  #define read64le(x) __builtin_bswap64(x)
  #define read64be(x)                  (x)
  #define read32le(x) __builtin_bswap32(x)
  #define read32be(x)                  (x)
  #define read16le(x) __builtin_bswap16(x)
  #define read16be(x)                  (x)
#else
  #error Could not detect platform endianess
#endif

#endif

