#pragma once

class Bitknit2Decompressor
{
public:
    Bitknit2Decompressor();
    ~Bitknit2Decompressor();

    BOOL Decompress(uint32_t compressedSize, void* compressedData, uint32_t decompressedSize,
                    void* decompressedData);

private:
    struct LiteralModel
    {
        // Fast lookup table used to map 15-bit ANS residues to a symbol.
        // BitKnit2 uses 512 entries + 4 safety entries.
        uint16_t lookup[512 + 4]{};

        // CDF table: size 301 (300 symbols + terminating entry).
        uint16_t cdf[300 + 1]{};

        // Raw frequency counters for adaptation.
        uint16_t freq[300]{};

        // Adaptive countdown (starts at 1024 per the spec).
        uint32_t adaptCounter = 1024;
    };

    struct OffsetLsbModel
    {
        // Fast lookup table for 15-bit ANS residues. 
        // This vocabulary is smaller, so BitKnit2 uses 64 entries + 4 safety slots.
        uint16_t lookup[64 + 4]{};

        // Cumulative distribution function: 40 symbols + 1 terminal.
        uint16_t cdf[40 + 1]{};

        // Raw frequency counters for adaptive updates.
        uint16_t freq[40]{};

        // Adaptive countdown (resets every 1024 symbols).
        uint32_t adaptCounter = 1024;
    };

    struct OffsetBitLengthModel
    {
        // Fast lookup table for 15-bit ANS residue -> initial symbol guess.
        uint16_t lookup[64 + 4]{};

        // Cumulative distribution table: 21 symbols + 1 terminal.
        uint16_t cdf[21 + 1]{};

        // Raw frequency counters used for adaptive probability updates.
        uint16_t freq[21]{};

        // Adaptive update interval countdown (starts at 1024).
        uint32_t adaptCounter = 1024;
    };

    BOOL CanRead(size_t size) const;
    BOOL CanWrite(size_t size) const;
    BOOL CopyRawQuantum();
    BOOL DecodeQuantum();
    BOOL ParseQuantumHeader();
    BOOL Renormalize();
    uint32_t DecodeLiteral(LiteralModel& model, uint32_t& state);
    uint32_t DecodeOffsetBitLength(OffsetBitLengthModel& model, uint32_t& state);
    uint32_t DecodeOffsetLSB(OffsetLsbModel& model, uint32_t& state);
    void InitializeBitLengthModel();
    void InitializeLiteralModels();
    void InitializeOffsetModels();
    void RebuildLiteralModel(LiteralModel& model, uint32_t sym);
    void RebuildOffsetBitLengthModel(OffsetBitLengthModel& model, uint32_t sym);
    void RebuildOffsetLSBModel(OffsetLsbModel& model, uint32_t sym);

    // Four adaptive literal models (300-symbol vocabulary).
    // Selected by (DstOffset & 3). Persistent across quanta.
    LiteralModel m_literalModels[4];

    // Four adaptive offset-LSB models (vocabulary size = 40).
    // Used to decode CR symbols in the range 0..39.
    // Selected by (DstOffset & 3). Persistent across quanta.
    OffsetLsbModel m_offsetLsbModels[4];

    // Single adaptive model for extended match-distance bit lengths.
    // Vocabulary size = 21. Used when CR >= 8 (cache miss cases).
    OffsetBitLengthModel m_offsetBitLengthModel;

    // The most recently used match distance (CO).
    // Used for delta-decoding literals: literal = symbol + output[-m_lastCopyOffset].
    // Initialized to 1 at the start of each BitKnit2 stream.
    uint32_t m_lastCopyOffset = 1;

    // 8-entry LRU cache of recent copy offsets.
    // Used when decoding CR symbols (0..8 = cache hit).
    // All entries initialize to 1 at the start of the stream.
    uint32_t m_recentOffsets[8]{1, 1, 1, 1, 1, 1, 1, 1};

    // 24-bit packed index structure used for LRU updates to m_recentOffsets.
    // Each 3-bit field holds the index of one cache entry.
    // Required by BitKnit's distance cache update rules.

    // Packed 3-bit indices: [0,1,2,3,4,5,6,7] in 24 bits.
    // Required by BitKnit for its LRU distance cache.
    uint32_t m_recentDistMask =
        (7u << 21) | (6u << 18) | (5u << 15) |
        (4u << 12) | (3u << 9) | (2u << 6) |
        (1u << 3) | (0u << 0);

    // The two interleaved rANS decoding states used by BitKnit2.
    // These hold the compressed stream "state integers" that symbols
    // are decoded from. Each DecodeSymbol() operation consumes bits
    // from one state, updates it, then renormalizes as needed.
    //
    // BitKnit alternates between these two states (STATE1, STATE2)
    // on each symbol to increase throughput and reduce dependency
    // chains. Both are re-initialized at the start of every quantum.
    uint32_t m_state1 = 0;
    uint32_t m_state2 = 0;

    // Pointer to the beginning of the decompressed output buffer.
    // Needed because match distances are relative to the entire output,
    // not just within the current quantum.
    uint8_t* m_dstStart = nullptr;

    // Pointer to the end of the decompressed output buffer.
    // Used to prevent overrun during copy commands.
    uint8_t* m_dstEnd = nullptr;

    // Pointer to the current destination position in the output buffer.
    uint8_t* m_dstCur = nullptr;

    // Pointer to the end of the current 64 KB quantum.
    // Each quantum ends at the next 65536-byte boundary from m_dstStart.
    uint8_t* m_dstQuantumEnd = nullptr;

    // Current read position in the compressed input buffer.
    uint8_t* m_srcCur = nullptr;

    // End of compressed input buffer (one past last valid byte).
    uint8_t* m_srcEnd = nullptr;

    // Number of high bits that belong to STATE2 when reconstructing rANS state
    // from the quantum initialization header. Derived from I1 % 16.
    uint32_t m_state2HighBitCount = 0;

    // Flag indicating whether the first literal has been emitted.
    BOOL m_emittedFirstLiteral = FALSE;
};
