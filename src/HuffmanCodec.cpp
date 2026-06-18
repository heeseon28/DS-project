#include "ds/HuffmanCodec.h"
#include <cstring>

namespace
{
    const int SYMBOL_LIMIT = 256;
    const int NODE_LIMIT = SYMBOL_LIMIT * 2 - 1;

    struct HuffmanNode
    {
        // leaf 노드는 실제 문자 하나를 나타내고, internal 노드는 두 subtree의
        // 빈도 합을 가진다. left/right는 nodes 배열 안의 인덱스이다.
        int frequency;
        int left;
        int right;
        unsigned char symbol;
        bool active;
        bool leaf;
    };

    void initializeNodes(HuffmanNode nodes[])
    {
        for (int i = 0; i < NODE_LIMIT; ++i)
        {
            nodes[i].frequency = 0;
            nodes[i].left = -1;
            nodes[i].right = -1;
            nodes[i].symbol = 0;
            nodes[i].active = false;
            nodes[i].leaf = false;
        }
    }

    int findSmallestActiveNode(const HuffmanNode nodes[], int nodeCount)
    {
        // priority_queue 대신 active node 전체를 선형 탐색한다. 문자 종류는 최대
        // 256개라서 구현이 단순하고, tree build 비용은 O(n^2)이다.
        int best = -1;
        for (int i = 0; i < nodeCount; ++i)
        {
            if (!nodes[i].active)
            {
                continue;
            }

            if (best == -1 ||
                nodes[i].frequency < nodes[best].frequency ||
                (nodes[i].frequency == nodes[best].frequency && nodes[i].symbol < nodes[best].symbol))
            {
                best = i;
            }
        }

        return best;
    }

    void buildCodes(const HuffmanNode nodes[], int nodeIndex, std::string currentCode, std::string codes[])
    {
        if (nodeIndex < 0)
        {
            return;
        }

        if (nodes[nodeIndex].leaf)
        {
            // root에서 leaf까지 왼쪽은 0, 오른쪽은 1로 기록한 경로가 해당 문자의
            // 가변 길이 code가 된다. 자주 나온 문자는 짧은 code를 받는다.
            // 한 종류의 문자만 있는 입력도 최소 1비트 코드로 표현한다.
            if (currentCode.empty())
            {
                currentCode = "0";
            }
            codes[nodes[nodeIndex].symbol] = currentCode;
            return;
        }

        buildCodes(nodes, nodes[nodeIndex].left, currentCode + "0", codes);
        buildCodes(nodes, nodes[nodeIndex].right, currentCode + "1", codes);
    }

    void writeBit(unsigned char bytes[], int bitIndex, bool value)
    {
        if (!value)
        {
            return;
        }

        int byteIndex = bitIndex / 8;
        int bitOffset = 7 - (bitIndex % 8);
        // 왼쪽 bit부터 채워 SpriteAssets.cpp에 저장된 packed hex byte와
        // decode할 때의 읽기 순서를 일치시킨다.
        bytes[byteIndex] = static_cast<unsigned char>(bytes[byteIndex] | (1 << bitOffset));
    }

    bool readBit(const unsigned char bytes[], int bitIndex)
    {
        int byteIndex = bitIndex / 8;
        int bitOffset = 7 - (bitIndex % 8);
        return (bytes[byteIndex] & (1 << bitOffset)) != 0;
    }

    int buildTreeFromFrequencies(
        const HuffmanFrequency frequencies[],
        int frequencyCount,
        HuffmanNode nodes[])
    {
        initializeNodes(nodes);

        // 저장된 frequency table만으로 압축 당시와 같은 Huffman tree를 다시 만든다.
        // 그래서 SpriteAssets.cpp에는 raw sprite 문자열 대신 bytes + frequencies만 저장할 수 있다.
        int nodeCount = 0;
        for (int i = 0; i < frequencyCount; ++i)
        {
            nodes[nodeCount].frequency = frequencies[i].frequency;
            nodes[nodeCount].symbol = frequencies[i].symbol;
            nodes[nodeCount].active = true;
            nodes[nodeCount].leaf = true;
            ++nodeCount;
        }

        int activeCount = frequencyCount;
        while (activeCount > 1)
        {
            int left = findSmallestActiveNode(nodes, nodeCount);
            nodes[left].active = false;

            int right = findSmallestActiveNode(nodes, nodeCount);
            nodes[right].active = false;

            nodes[nodeCount].frequency = nodes[left].frequency + nodes[right].frequency;
            nodes[nodeCount].left = left;
            nodes[nodeCount].right = right;
            nodes[nodeCount].symbol = nodes[left].symbol < nodes[right].symbol
                                          ? nodes[left].symbol
                                          : nodes[right].symbol;
            nodes[nodeCount].active = true;
            nodes[nodeCount].leaf = false;
            ++nodeCount;
            --activeCount;
        }

        return findSmallestActiveNode(nodes, nodeCount);
    }
}

HuffmanCodec::Result::Result()
    : decodedText(""),
      originalByteCount(0),
      compressedBitCount(0),
      uniqueSymbolCount(0) {}

HuffmanCodec::Result HuffmanCodec::compressAndDecode(const std::string& text)
{
    Result result;
    result.originalByteCount = static_cast<int>(text.size());

    if (text.empty())
    {
        return result;
    }

    // 1단계: 원본 문자열에서 각 ASCII 문자의 등장 횟수를 센다.
    // 빈도 차이가 클수록 Huffman coding의 저장 공간 절감 효과가 커진다.
    int frequencies[SYMBOL_LIMIT] = {0};
    for (int i = 0; i < static_cast<int>(text.size()); ++i)
    {
        unsigned char symbol = static_cast<unsigned char>(text[i]);
        ++frequencies[symbol];
    }

    HuffmanNode nodes[NODE_LIMIT];
    initializeNodes(nodes);

    int nodeCount = 0;
    for (int symbol = 0; symbol < SYMBOL_LIMIT; ++symbol)
    {
        if (frequencies[symbol] == 0)
        {
            continue;
        }

        nodes[nodeCount].frequency = frequencies[symbol];
        nodes[nodeCount].symbol = static_cast<unsigned char>(symbol);
        nodes[nodeCount].active = true;
        nodes[nodeCount].leaf = true;
        ++nodeCount;
        ++result.uniqueSymbolCount;
    }

    // 2단계: 가장 작은 두 active node를 반복해서 묶어 Huffman tree를 만든다.
    int activeCount = result.uniqueSymbolCount;
    while (activeCount > 1)
    {
        int left = findSmallestActiveNode(nodes, nodeCount);
        nodes[left].active = false;

        int right = findSmallestActiveNode(nodes, nodeCount);
        nodes[right].active = false;

        nodes[nodeCount].frequency = nodes[left].frequency + nodes[right].frequency;
        nodes[nodeCount].left = left;
        nodes[nodeCount].right = right;
        nodes[nodeCount].symbol = nodes[left].symbol < nodes[right].symbol
                                      ? nodes[left].symbol
                                      : nodes[right].symbol;
        nodes[nodeCount].active = true;
        nodes[nodeCount].leaf = false;
        ++nodeCount;
        --activeCount;
    }

    int root = findSmallestActiveNode(nodes, nodeCount);

    // 3단계: tree의 root-to-leaf 경로를 문자별 bit code로 변환한다.
    std::string codes[SYMBOL_LIMIT];
    buildCodes(nodes, root, "", codes);

    for (int symbol = 0; symbol < SYMBOL_LIMIT; ++symbol)
    {
        if (frequencies[symbol] > 0)
        {
            result.compressedBitCount += frequencies[symbol] * static_cast<int>(codes[symbol].size());
        }
    }

    // 4단계: 가변 길이 code를 하나의 bitstream으로 이어 붙이고 byte 단위로 packing한다.
    int compressedByteCount = (result.compressedBitCount + 7) / 8;
    unsigned char* compressedBytes = new unsigned char[compressedByteCount];
    std::memset(compressedBytes, 0, compressedByteCount);

    int bitIndex = 0;
    for (int i = 0; i < static_cast<int>(text.size()); ++i)
    {
        unsigned char symbol = static_cast<unsigned char>(text[i]);
        const std::string& code = codes[symbol];
        for (int j = 0; j < static_cast<int>(code.size()); ++j)
        {
            writeBit(compressedBytes, bitIndex, code[j] == '1');
            ++bitIndex;
        }
    }

    // 5단계: 테스트 편의를 위해 곧바로 decode해서 원본과 같은지 확인할 수 있게 한다.
    std::string decoded;
    decoded.reserve(text.size());

    if (nodes[root].leaf)
    {
        for (int i = 0; i < result.originalByteCount; ++i)
        {
            decoded += static_cast<char>(nodes[root].symbol);
        }
    }
    else
    {
        int current = root;
        for (int i = 0;
             i < result.compressedBitCount && static_cast<int>(decoded.size()) < result.originalByteCount;
             ++i)
        {
            current = readBit(compressedBytes, i) ? nodes[current].right : nodes[current].left;
            if (nodes[current].leaf)
            {
                decoded += static_cast<char>(nodes[current].symbol);
                current = root;
            }
        }
    }

    delete[] compressedBytes;

    result.decodedText = decoded;
    return result;
}

std::string HuffmanCodec::decodeSprite(const char* sprite)
{
    if (sprite == nullptr || sprite[0] == '\0')
    {
        return "";
    }

    return compressAndDecode(std::string(sprite)).decodedText;
}

std::string HuffmanCodec::decodeSprite(const std::string& sprite)
{
    return compressAndDecode(sprite).decodedText;
}

std::string HuffmanCodec::decodeSprite(const HuffmanEncodedSprite* sprite)
{
    if (sprite == nullptr ||
        sprite->bytes == nullptr ||
        sprite->frequencies == nullptr ||
        sprite->frequencyCount <= 0 ||
        sprite->originalByteCount <= 0)
    {
        return "";
    }

    // 실행 파일에 저장된 packed sprite를 화면에 출력할 때 사용하는 경로이다.
    // frequency table로 tree를 복원하고 bitstream을 따라가며 leaf마다 문자를 하나씩 만든다.
    HuffmanNode nodes[NODE_LIMIT];
    int root = buildTreeFromFrequencies(sprite->frequencies, sprite->frequencyCount, nodes);
    if (root < 0)
    {
        return "";
    }

    std::string decoded;
    decoded.reserve(sprite->originalByteCount);

    if (nodes[root].leaf)
    {
        for (int i = 0; i < sprite->originalByteCount; ++i)
        {
            decoded += static_cast<char>(nodes[root].symbol);
        }
        return decoded;
    }

    int current = root;
    for (int i = 0;
         i < sprite->bitCount && static_cast<int>(decoded.size()) < sprite->originalByteCount;
         ++i)
    {
        current = readBit(sprite->bytes, i) ? nodes[current].right : nodes[current].left;
        if (nodes[current].leaf)
        {
            decoded += static_cast<char>(nodes[current].symbol);
            current = root;
        }
    }

    return decoded;
}
