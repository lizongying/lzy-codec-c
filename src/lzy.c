#include "lzy.h"
#include <string.h>

// Unicode 有效性校验实现
bool valid_unicode(uint32_t r) {
    return (r >= 0 && r < SURROGATE_MIN) || (r > SURROGATE_MAX && r <= UNICODE_MAX);
}

// 核心编码实现
int encode(const uint32_t* input_runes, size_t rune_count,
           uint8_t* output_bytes, size_t* output_len) {
    if (input_runes == NULL || output_bytes == NULL || output_len == NULL || rune_count == 0) {
        return LZY_ERROR;
    }

    size_t idx = 0;
    for (size_t i = 0; i < rune_count; i++) {
        uint32_t r = input_runes[i];
        if (!valid_unicode(r)) {
            return LZY_ERROR;
        }

        if (r < 0x80) {
            // 单字节编码
            if (idx >= *output_len) { // 防止内存越界
                return LZY_ERROR;
            }
            output_bytes[idx++] = (uint8_t)(r & 0xFF);
        } else if (r < 0x4000) {
            // 双字节编码
            if (idx + 1 >= *output_len) {
                return LZY_ERROR;
            }
            output_bytes[idx++] = (uint8_t)((r >> 7) & 0xFF);
            output_bytes[idx++] = (uint8_t)((0x80 | (r & 0x7F)) & 0xFF);
        } else {
            // 三字节编码
            if (idx + 2 >= *output_len) {
                return LZY_ERROR;
            }
            output_bytes[idx++] = (uint8_t)((r >> 14) & 0xFF);
            output_bytes[idx++] = (uint8_t)((0x80 | ((r >> 7) & 0x7F)) & 0xFF);
            output_bytes[idx++] = (uint8_t)((0x80 | (r & 0x7F)) & 0xFF);
        }
    }

    *output_len = idx;
    return 0;
}

// UTF-8 字符串转 Unicode 码点（内部辅助函数，不对外暴露）
static int utf8_to_runes(const uint8_t* input_utf8, size_t utf8_len,
                         uint32_t* output_runes, size_t* rune_count) {
    if (input_utf8 == NULL || output_runes == NULL || rune_count == NULL) {
        return LZY_ERROR;
    }

    size_t rune_idx = 0;
    size_t utf8_idx = 0;
    while (utf8_idx < utf8_len) {
        uint8_t b = input_utf8[utf8_idx];
        uint32_t rune = 0;
        int bytes_needed = 0;

        // 判断 UTF-8 字节长度
        if ((b & 0x80) == 0) {
            rune = b;
            bytes_needed = 1;
        } else if ((b & 0xE0) == 0xC0) {
            rune = b & 0x1F;
            bytes_needed = 2;
        } else if ((b & 0xF0) == 0xE0) {
            rune = b & 0x0F;
            bytes_needed = 3;
        } else if ((b & 0xF8) == 0xF0) {
            rune = b & 0x07;
            bytes_needed = 4;
        } else {
            return LZY_ERROR; // 无效 UTF-8 字符
        }

        // 检查剩余字节是否足够
        if (utf8_idx + bytes_needed > utf8_len) {
            return LZY_ERROR;
        }

        // 解析后续字节
        for (int i = 1; i < bytes_needed; i++) {
            utf8_idx++;
            uint8_t next_b = input_utf8[utf8_idx];
            if ((next_b & 0xC0) != 0x80) {
                return LZY_ERROR; // 无效 UTF-8 后续字节
            }
            rune = (rune << 6) | (next_b & 0x3F);
        }

        output_runes[rune_idx++] = rune;
        utf8_idx++;
    }

    *rune_count = rune_idx;
    return 0;
}

// 辅助编码：UTF-8 字符串 → LZY 字节序列
int encode_from_string(const char* input_str, uint8_t* output_bytes, size_t* output_len) {
    if (input_str == NULL || output_bytes == NULL || output_len == NULL) {
        return LZY_ERROR;
    }
    size_t utf8_len = strlen(input_str);
    return encode_from_utf8((const uint8_t*)input_str, utf8_len, output_bytes, output_len);
}

// 辅助编码：UTF-8 字节序列 → LZY 字节序列
int encode_from_utf8(const uint8_t* input_utf8, size_t utf8_len,
                     uint8_t* output_bytes, size_t* output_len) {
    if (input_utf8 == NULL || output_bytes == NULL || output_len == NULL || utf8_len == 0) {
        return LZY_ERROR;
    }

    // 第一步：UTF-8 转 Unicode 码点
    uint32_t runes[1024] = {0}; // 临时缓冲区，可根据需求调整大小
    size_t rune_count = 0;
    if (utf8_to_runes(input_utf8, utf8_len, runes, &rune_count) != 0) {
        return LZY_ERROR;
    }

    // 第二步：Unicode 码点转 LZY 字节序列
    size_t max_output_len = *output_len; // 保存最大输出长度
    *output_len = max_output_len; // 传入 encode 函数，用于内存越界判断
    return encode(runes, rune_count, output_bytes, output_len);
}

// 核心解码实现
int decode(const uint8_t* input_bytes, size_t input_len,
           uint32_t* output_runes, size_t* rune_count) {
    if (input_bytes == NULL || output_runes == NULL || rune_count == NULL || input_len == 0) {
        return LZY_ERROR;
    }

    // 寻找第一个最高位为 0 的字节（有效起始索引）
    size_t start_idx = (size_t)-1;
    for (size_t i = 0; i < input_len; i++) {
        if ((input_bytes[i] & 0x80) == 0) {
            start_idx = i;
            break;
        }
    }
    if (start_idx == (size_t)-1) {
        return LZY_ERROR; // 无有效起始字节
    }

    size_t rune_idx = 0;
    uint32_t r = 0;
    for (size_t i = start_idx; i < input_len; i++) {
        uint32_t b = (uint32_t)input_bytes[i] & 0xFF;

        if ((b >> 7) == 0) {
            // 遇到单字节标记，处理上一个累积的码点
            if (i > start_idx) {
                if (!valid_unicode(r)) {
                    return LZY_ERROR;
                }
                output_runes[rune_idx++] = r;
            }
            r = b;
        } else {
            // 累积码点计算
            if (r > (UNICODE_MAX >> 7)) {
                return LZY_ERROR;
            }
            r = (r << 7) | (b & 0x7F);
        }
    }

    // 处理最后一个码点
    if (!valid_unicode(r)) {
        return LZY_ERROR;
    }
    output_runes[rune_idx++] = r;

    *rune_count = rune_idx;
    return 0;
}

// Unicode 码点转 UTF-8 字节（内部辅助函数，不对外暴露）
static int runes_to_utf8(const uint32_t* input_runes, size_t rune_count,
                         uint8_t* output_utf8, size_t* utf8_len) {
    if (input_runes == NULL || output_utf8 == NULL || utf8_len == NULL) {
        return LZY_ERROR;
    }

    size_t utf8_idx = 0;
    for (size_t i = 0; i < rune_count; i++) {
        uint32_t r = input_runes[i];
        if (!valid_unicode(r)) {
            return LZY_ERROR;
        }

        // 计算 UTF-8 字节长度并编码
        if (r < 0x80) {
            if (utf8_idx >= *utf8_len) {
                return LZY_ERROR;
            }
            output_utf8[utf8_idx++] = (uint8_t)r;
        } else if (r < 0x800) {
            if (utf8_idx + 1 >= *utf8_len) {
                return LZY_ERROR;
            }
            output_utf8[utf8_idx++] = (uint8_t)(0xC0 | ((r >> 6) & 0x1F));
            output_utf8[utf8_idx++] = (uint8_t)(0x80 | (r & 0x3F));
        } else if (r < 0x10000) {
            if (utf8_idx + 2 >= *utf8_len) {
                return LZY_ERROR;
            }
            output_utf8[utf8_idx++] = (uint8_t)(0xE0 | ((r >> 12) & 0x0F));
            output_utf8[utf8_idx++] = (uint8_t)(0x80 | ((r >> 6) & 0x3F));
            output_utf8[utf8_idx++] = (uint8_t)(0x80 | (r & 0x3F));
        } else if (r <= 0x10FFFF) {
            if (utf8_idx + 3 >= *utf8_len) {
                return LZY_ERROR;
            }
            output_utf8[utf8_idx++] = (uint8_t)(0xF0 | ((r >> 18) & 0x07));
            output_utf8[utf8_idx++] = (uint8_t)(0x80 | ((r >> 12) & 0x3F));
            output_utf8[utf8_idx++] = (uint8_t)(0x80 | ((r >> 6) & 0x3F));
            output_utf8[utf8_idx++] = (uint8_t)(0x80 | (r & 0x3F));
        } else {
            return LZY_ERROR;
        }
    }

    *utf8_len = utf8_idx;
    return 0;
}

// 辅助解码：LZY 字节序列 → UTF-8 字符串
int decode_to_utf8(const uint8_t* input_bytes, size_t input_len,
                   uint8_t* output_utf8, size_t* utf8_len) {
    if (input_bytes == NULL || output_utf8 == NULL || utf8_len == NULL || input_len == 0) {
        return LZY_ERROR;
    }

    // 第一步：LZY 字节序列转 Unicode 码点
    uint32_t runes[1024] = {0}; // 临时缓冲区，可根据需求调整大小
    size_t rune_count = 0;
    if (decode(input_bytes, input_len, runes, &rune_count) != 0) {
        return LZY_ERROR;
    }

    // 第二步：Unicode 码点转 UTF-8 字节序列
    size_t max_utf8_len = *utf8_len;
    *utf8_len = max_utf8_len;
    return runes_to_utf8(runes, rune_count, output_utf8, utf8_len);
}