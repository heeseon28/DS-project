#include "ds/HuffmanCodec.h"
#include <cstring>

namespace
{
    const int SYMBOL_LIMIT = 256;
    const int NODE_LIMIT = SYMBOL_LIMIT * 2 - 1;

    struct HuffmanNode
    {
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

    std::string codes[SYMBOL_LIMIT];
    buildCodes(nodes, root, "", codes);

    for (int symbol = 0; symbol < SYMBOL_LIMIT; ++symbol)
    {
        if (frequencies[symbol] > 0)
        {
            result.compressedBitCount += frequencies[symbol] * static_cast<int>(codes[symbol].size());
        }
    }

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
