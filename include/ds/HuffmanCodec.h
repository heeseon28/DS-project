#ifndef HUFFMAN_CODEC_H
#define HUFFMAN_CODEC_H

#include <string>

struct HuffmanFrequency {
    unsigned char symbol;
    int frequency;
};

struct HuffmanEncodedSprite {
    const unsigned char* bytes;
    int byteCount;
    int bitCount;
    int originalByteCount;
    const HuffmanFrequency* frequencies;
    int frequencyCount;
};

// ASCII sprite text를 Huffman coding으로 압축한 뒤 다시 복원하는 유틸리티.
// 스프라이트가 반복 문자가 많다는 점을 이용해 자료구조/압축 알고리즘을 보여준다.
class HuffmanCodec {
public:
    struct Result {
        std::string decodedText;
        int originalByteCount;
        int compressedBitCount;
        int uniqueSymbolCount;

        Result();
    };

    static Result compressAndDecode(const std::string& text);
    static std::string decodeSprite(const char* sprite);
    static std::string decodeSprite(const std::string& sprite);
    static std::string decodeSprite(const HuffmanEncodedSprite* sprite);
};

#endif
