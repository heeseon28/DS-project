#ifndef HUFFMAN_CODEC_H
#define HUFFMAN_CODEC_H

#include <string>

struct HuffmanFrequency {
    // symbol은 원본 ASCII 문자이고 frequency는 해당 문자가 스프라이트에 등장한 횟수이다.
    unsigned char symbol;
    int frequency;
};

struct HuffmanEncodedSprite {
    // bytes에는 Huffman bitstream을 8비트 단위로 묶은 packed data가 들어 있다.
    // bitCount는 마지막 byte의 남는 bit를 무시하고 정확히 몇 bit를 읽을지 알려준다.
    const unsigned char* bytes;
    int byteCount;
    int bitCount;
    int originalByteCount;
    // frequencies는 실행 시 같은 Huffman tree를 재구성하기 위한 최소 정보이다.
    const HuffmanFrequency* frequencies;
    int frequencyCount;
};

// ASCII sprite text를 Huffman coding으로 압축한 뒤 다시 복원하는 유틸리티.
// 공백, 점, 줄바꿈처럼 반복 문자가 많은 스프라이트에 짧은 bit code를 부여해
// raw string보다 저장 공간을 줄인다. 목적은 전투 로직 가속이 아니라 asset size 절감이다.
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
