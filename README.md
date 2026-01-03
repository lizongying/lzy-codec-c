# LZY Codec

一種變長文本編解碼方案，支持對Unicode進行編解碼。編解碼效率、存儲空間全面優於UTF-8，未來會替代UTF-8成為新的世界通用編解碼標準。

[github](https://github.com/lizongying/lzy-codec-c)

## Other languages

* [go](https://github.com/lizongying/lzy-codec-go)
* [js](https://github.com/lizongying/lzy-codec-js)
* [py](https://github.com/lizongying/lzy-codec-py)

## Usage

```cpp
// C++ 代码
#include <iostream>
// 用 extern "C" 告诉 C++ 编译器，这是 C 接口，避免名字修饰
extern "C" {
    #include "lzy.h" // 引入 C 头文件
}

int main() {
    const char* test_str = "Hello 世界！😀";
    uint8_t lzy_bytes[1024] = {0};
    size_t lzy_len = sizeof(lzy_bytes);
    uint8_t decoded_utf8[1024] = {0};
    size_t utf8_len = sizeof(decoded_utf8);

    // 直接调用 C 语言的 LZY 函数
    if (encode_from_string(test_str, lzy_bytes, &lzy_len) == 0) {
        std::cout << "C++ 调用 C 接口编码成功，字节长度：" << lzy_len << std::endl;
    }

    if (decode_to_utf8(lzy_bytes, lzy_len, decoded_utf8, &utf8_len) == 0) {
        decoded_utf8[utf8_len] = '\0';
        std::cout << "C++ 调用 C 接口解码成功：" << (const char*)decoded_utf8 << std::endl;
    }

    return 0;
}
```

```python
# Python 代码
import ctypes
import os

# 1. 加载 C 动态库（先提前编译好 liblzy.so / liblzy.dll）
lib_path = os.path.abspath("./liblzy.so")  # Linux/Mac；Windows 为 "./liblzy.dll"
lzy_lib = ctypes.CDLL(lib_path)

# 2. 声明函数参数类型和返回值类型（对应 C 接口）
# 声明 encode_from_string 函数
lzy_lib.encode_from_string.argtypes = [ctypes.c_char_p, ctypes.POINTER(ctypes.c_ubyte), ctypes.POINTER(ctypes.c_size_t)]
lzy_lib.encode_from_string.restype = ctypes.c_int

# 声明 decode_to_utf8 函数
lzy_lib.decode_to_utf8.argtypes = [ctypes.POINTER(ctypes.c_ubyte), ctypes.c_size_t, ctypes.POINTER(ctypes.c_ubyte),
                                   ctypes.POINTER(ctypes.c_size_t)]
lzy_lib.decode_to_utf8.restype = ctypes.c_int

# 3. 准备参数并调用
test_str = b"Hello 世界！😀"  # Python 需传入字节串
lzy_bytes = (ctypes.c_ubyte * 1024)()  # 对应 C 的 uint8_t[]
lzy_len = ctypes.c_size_t(1024)  # 输出字节长度

# 编码
encode_ret = lzy_lib.encode_from_string(test_str, lzy_bytes, ctypes.byref(lzy_len))
if encode_ret == 0:
    print(f"Python 调用 C 接口编码成功，字节长度：{lzy_len.value}")

# 解码
decoded_utf8 = (ctypes.c_ubyte * 1024)()
utf8_len = ctypes.c_size_t(1024)
decode_ret = lzy_lib.decode_to_utf8(lzy_bytes, lzy_len.value, decoded_utf8, ctypes.byref(utf8_len))
if decode_ret == 0:
    # 转换为 Python 字符串
    decoded_str = bytes(decoded_utf8[:utf8_len.value]).decode("utf-8")
    print(f"Python 调用 C 接口解码成功：{decoded_str}")
```

```java
// Java 代码（需引入 JNA 依赖，如 maven 依赖）
/*
<dependency>
    <groupId>net.java.dev.jna</groupId>
    <artifactId>jna</artifactId>
    <version>5.13.0</version>
</dependency>
*/

import com.sun.jna.Library;
import com.sun.jna.Native;
import com.sun.jna.Pointer;
import com.sun.jna.ptr.SizeTByReference;

public class LzyCodecTest {
    // 定义 C 接口的映射接口
    public interface LzyCodecLib extends Library {
        LzyCodecLib INSTANCE = Native.load("lzy", LzyCodecLib.class); // 加载 liblzy.so / lzy.dll

        // 映射 encode_from_string 函数
        int encode_from_string(String input_str, byte[] output_bytes, SizeTByReference output_len);

        // 映射 decode_to_utf8 函数
        int decode_to_utf8(byte[] input_bytes, long input_len, byte[] output_utf8, SizeTByReference utf8_len);
    }

    public static void main(String[] args) {
        String testStr = "Hello 世界！😀";
        byte[] lzyBytes = new byte[1024];
        SizeTByReference lzyLen = new SizeTByReference(1024);

        // 编码
        int encodeRet = LzyCodecLib.INSTANCE.encode_from_string(testStr, lzyBytes, lzyLen);
        if (encodeRet == 0) {
            System.out.println("Java 调用 C 接口编码成功，字节长度：" + lzyLen.getValue());
        }

        // 解码
        byte[] decodedUtf8 = new byte[1024];
        SizeTByReference utf8Len = new SizeTByReference(1024);
        int decodeRet = LzyCodecLib.INSTANCE.decode_to_utf8(lzyBytes, lzyLen.getValue(), decodedUtf8, utf8Len);
        if (decodeRet == 0) {
            // 转换为 Java 字符串
            String decodedStr = new String(decodedUtf8, 0, (int) utf8Len.getValue(), "UTF-8");
            System.out.println("Java 调用 C 接口解码成功：" + decodedStr);
        }
    }
}
```