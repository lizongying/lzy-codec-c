#include "../include/lzy.h"
#include <stdio.h>
#include <string.h>

int main() {
    // 测试用例（包含中文、Emoji，验证 Unicode 全字符支持）
    const char* test_str = "Hello 世界！😀";
    printf("原始字符串：%s\n", test_str);

    // 1. 字符串 → LZY 字节序列
    uint8_t lzy_bytes[1024] = {0};
    size_t lzy_len = sizeof(lzy_bytes);
    if (encode_from_string(test_str, lzy_bytes, &lzy_len) != 0) {
        printf("编码失败！\n");
        return -1;
    }
    printf("LZY 编码字节长度：%zu\n", lzy_len);

    // 2. LZY 字节序列 → UTF-8 字符串
    uint8_t decoded_utf8[1024] = {0};
    size_t utf8_len = sizeof(decoded_utf8);
    if (decode_to_utf8(lzy_bytes, lzy_len, decoded_utf8, &utf8_len) != 0) {
        printf("解码失败！\n");
        return -1;
    }
    decoded_utf8[utf8_len] = '\0'; // 添加字符串结束符
    printf("解码后字符串：%s\n", decoded_utf8);

    // 3. 验证一致性
    if (strcmp(test_str, (const char*)decoded_utf8) == 0) {
        printf("✅ 编码解码一致性校验通过！\n");
    } else {
        printf("❌ 编码解码一致性校验失败！\n");
    }

    return 0;
}