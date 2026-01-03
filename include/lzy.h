#ifndef LZY_CODEC_H
#define LZY_CODEC_H

// 防止头文件重复包含（C 语言经典写法）
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// 统一常量定义（与跨语言规范一致）
#define SURROGATE_MIN 0xD800
#define SURROGATE_MAX 0xDFFF
#define UNICODE_MAX   0x10FFFF
#define LZY_ERROR     -1

/**
 * Unicode 码点有效性校验
 * @param r Unicode 码点（32位无符号整数）
 * @return bool：true 有效，false 无效
 */
bool valid_unicode(uint32_t r);

/**
 * 核心编码：Unicode 码点序列 → LZY 字节序列
 * @param input_runes  输入：Unicode 码点数组
 * @param rune_count   输入：Unicode 码点数组长度
 * @param output_bytes 输出：LZY 字节数组（需外部提前分配内存，建议长度 >= rune_count * 3）
 * @param output_len   输出：实际生成的 LZY 字节长度
 * @return int：0 成功，LZY_ERROR 失败
 */
int encode(const uint32_t* input_runes, size_t rune_count,
           uint8_t* output_bytes, size_t* output_len);

/**
 * 辅助编码：C 字符串（UTF-8）→ LZY 字节序列
 * @param input_str    输入：UTF-8 编码的 C 字符串（以 '\0' 结尾）
 * @param output_bytes 输出：LZY 字节数组（外部提前分配内存）
 * @param output_len   输出：实际生成的 LZY 字节长度
 * @return int：0 成功，LZY_ERROR 失败
 */
int encode_from_string(const char* input_str, uint8_t* output_bytes, size_t* output_len);

/**
 * 辅助编码：UTF-8 字节序列 → LZY 字节序列（与 encode_from_string 功能一致，支持非 '\0' 结尾的字节流）
 * @param input_utf8   输入：UTF-8 字节数组
 * @param utf8_len     输入：UTF-8 字节数组长度
 * @param output_bytes 输出：LZY 字节数组
 * @param output_len   输出：实际生成的 LZY 字节长度
 * @return int：0 成功，LZY_ERROR 失败
 */
int encode_from_utf8(const uint8_t* input_utf8, size_t utf8_len,
                     uint8_t* output_bytes, size_t* output_len);

/**
 * 核心解码：LZY 字节序列 → Unicode 码点序列
 * @param input_bytes  输入：LZY 字节数组
 * @param input_len    输入：LZY 字节数组长度
 * @param output_runes 输出：Unicode 码点数组（外部提前分配内存，建议长度 >= input_len）
 * @param rune_count   输出：实际生成的 Unicode 码点长度
 * @return int：0 成功，LZY_ERROR 失败
 */
int decode(const uint8_t* input_bytes, size_t input_len,
           uint32_t* output_runes, size_t* rune_count);

/**
 * 辅助解码：LZY 字节序列 → UTF-8 字符串
 * @param input_bytes  输入：LZY 字节数组
 * @param input_len    输入：LZY 字节数组长度
 * @param output_utf8  输出：UTF-8 字节数组（外部提前分配内存，建议长度 >= input_len * 4）
 * @param utf8_len     输出：实际生成的 UTF-8 字节长度
 * @return int：0 成功，LZY_ERROR 失败
 */
int decode_to_utf8(const uint8_t* input_bytes, size_t input_len,
                   uint8_t* output_utf8, size_t* utf8_len);

#endif // LZY_CODEC_H