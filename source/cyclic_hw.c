#include <cyclic_hw.h>

#if defined(__x86_64__)
#include <nmmintrin.h>

u32 cyclic32_checksum(const void* data, u64 length)
{
    u32 crc = 0xFFFFFFFF;

    const u8* curr_cursor = (const u8*)data;

    while (length >= 16) {
        const __m128i data_block128
            = _mm_loadu_si128((const __m128i*)curr_cursor);
        crc = _mm_crc32_u16(crc, _mm_extract_epi16(data_block128, 0));
        crc = _mm_crc32_u16(crc, _mm_extract_epi16(data_block128, 1));
        crc = _mm_crc32_u16(crc, _mm_extract_epi16(data_block128, 2));
        crc = _mm_crc32_u16(crc, _mm_extract_epi16(data_block128, 3));
        crc = _mm_crc32_u16(crc, _mm_extract_epi16(data_block128, 4));
        crc = _mm_crc32_u16(crc, _mm_extract_epi16(data_block128, 5));
        crc = _mm_crc32_u16(crc, _mm_extract_epi16(data_block128, 6));
        crc = _mm_crc32_u16(crc, _mm_extract_epi16(data_block128, 7));

        curr_cursor += 16;
        length -= 16;
    }
    while (length--)
        crc = _mm_crc32_u8(crc, *curr_cursor++);

    return ~crc;
}

#elif defined(__aarch64__)
#include <arm_acle.h>
#include <arm_neon.h>

u32 cyclic32_checksum(const void* data, u64 length)
{
    u32 crc = 0xFFFFFFFF;

    const u8* curr_ptr = (const u8*)data;
    while (length >= 16) {
        const uint32x4_t data32x4 = vld1q_u32((const u32*)curr_ptr);
        crc = __crc32cw(crc, vgetq_lane_u32(data32x4, 0));
        crc = __crc32cw(crc, vgetq_lane_u32(data32x4, 1));
        crc = __crc32cw(crc, vgetq_lane_u32(data32x4, 2));
        crc = __crc32cw(crc, vgetq_lane_u32(data32x4, 3));

        curr_ptr += 16;
        length -= 16;
    }
    // Calculating the remaining bytes
    while (length--)
        crc = __crc32cw(crc, *curr_ptr++);
    return ~crc;
}

#endif
