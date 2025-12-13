#include "pch.h"
#include "Bitknit2Decompressor.h"

static constexpr auto BITKNIT2_MAGIC = 0x75B1;

template <typename T>
T Peek(const uint8_t* ptr)
{
    T v;
    memcpy(&v, ptr, sizeof(T));
    return v;
}

template <typename T>
T Read(uint8_t*& ptr)
{
    T v;
    memcpy(&v, ptr, sizeof(T));
    ptr += sizeof(T);
    return v;
}

Bitknit2Decompressor::Bitknit2Decompressor()
{
}

Bitknit2Decompressor::~Bitknit2Decompressor()
{
}

BOOL Bitknit2Decompressor::Decompress(uint32_t compressedSize, void* compressedData, uint32_t decompressedSize,
                                      void* decompressedData)
{
    if (!compressedData || !decompressedData || compressedSize < 2 || decompressedSize == 0) {
        return FALSE;
    }

    // Initialize input buffer pointers
    m_srcCur = static_cast<uint8_t*>(compressedData);
    m_srcEnd = m_srcCur + compressedSize;

    // Initialize output buffer pointers
    m_dstCur = m_dstStart = static_cast<uint8_t*>(decompressedData);
    m_dstEnd = m_dstStart + decompressedSize;

    // Reset rANS states
    m_state1 = m_state2 = 0;

    // Reset LZ copy offset state
    m_lastCopyOffset = 1;
    for (auto& m_recentOffset : m_recentOffsets) {
        m_recentOffset = 1;
    }

    // Packed permutation: [0,1,2,3,4,5,6,7]
    m_recentDistMask =
        (7u << 21) | (6u << 18) | (5u << 15) |
        (4u << 12) | (3u << 9) | (2u << 6) |
        (1u << 3) | (0u << 0);

    // Reset first-literal flag
    m_emittedFirstLiteral = FALSE;

    // Initialize entropy models
    InitializeLiteralModels();
    InitializeOffsetModels();
    InitializeBitLengthModel();

    // Verify BitKnit2 magic number
    auto magic = *reinterpret_cast<uint16_t*>(m_srcCur);
    if (magic != BITKNIT2_MAGIC) {
        return FALSE;
    }

    m_srcCur += 2; // advance past magic number

    while (m_dstCur < m_dstEnd) {
        // Compute quantum boundary
        auto remaining = static_cast<uint32_t>(m_dstEnd - m_dstCur);
        auto quantumSize = std::min<uint32_t>(remaining, 65536);
        m_dstQuantumEnd = m_dstCur + quantumSize;

        if (!CanRead(sizeof(uint16_t))) {
            return FALSE;
        }
        auto marker = Peek<uint16_t>(m_srcCur);
        if (marker == 0) { // Raw quantum marker
            m_srcCur += 2;
            if (!CopyRawQuantum()) {
                return FALSE;
            }
        } else {
            if (!DecodeQuantum()) {
                return FALSE;
            }
        }
    }

    return TRUE;
}

BOOL Bitknit2Decompressor::CanRead(size_t size) const
{
    return (m_srcCur + size) <= m_srcEnd;
}

BOOL Bitknit2Decompressor::CanWrite(size_t size) const
{
    return (m_dstCur + size) <= m_dstEnd;
}

uint32_t Bitknit2Decompressor::DecodeLiteral(LiteralModel& model, uint32_t& state)
{
    // Extract the 15-bit residue from the current rANS state.
    //
    // This determines where we land inside the CDF table.
    auto residue = state & 0x7FFFu; // 0..32767

    // Use the fast lookup table to get an initial guess for the symbol.
    //
    // Literals use a 512-entry lookup table (residue >> 6).
    auto sym = model.lookup[residue >> 6];

    // Fast correction: if the residue exceeds the upper bound of this symbol,
    // increment once. This handles the majority of overshoot cases.
    if (residue > model.cdf[sym + 1]) {
        ++sym;
    }

    // Slow correction
    // increment sym until the correct interval is found.
    while (residue >= model.cdf[sym + 1]) {
        ++sym;
    }

    // rANS decode state update:
    //
    //    state = freq * (state >> 15) + (residue - cdf_low)
    //
    // where:
    //    freq     = cdf[sym+1] - cdf[sym]
    //    cdf_low  = cdf[sym]
    //
    auto lo = model.cdf[sym];
    auto freq = model.cdf[sym + 1] - lo;

    state = (state >> 15) * freq + (residue - lo);

    // Adaptive update: each decoded symbol increases its frequency by 31.
    model.freq[sym] += 31;

    // Every 1024 decoded symbols, the model must be rebuilt
    // (CDF + lookup tables). We will implement this soon.
    if (--model.adaptCounter == 0) {
        RebuildLiteralModel(model, sym);
    }

    return sym;
}

uint32_t Bitknit2Decompressor::DecodeOffsetLSB(OffsetLsbModel& model, uint32_t& state)
{
    // Extract lower 15 bits of rANS state
    auto residue = state & 0x7FFFu;

    // Initial guess from lookup table (block = residue >> 9)
    auto sym = static_cast<uint32_t>(model.lookup[residue >> 9]);

    // One-step fast correction
    if (residue > model.cdf[sym + 1]) {
        ++sym;
    }

    // Full correction loop
    while (residue >= model.cdf[sym + 1]) {
        ++sym;
    }

    // Update ANS state:
    //     state = residue + (state >> 15) * width - lo
    auto lo = model.cdf[sym];
    auto hi = model.cdf[sym + 1];
    auto width = hi - lo;

    state = residue + ((state >> 15) * width) - lo;

    // BitKnit LSB frequency update
    model.freq[sym] += 31;

    // Adaptive rebuild?
    if (--model.adaptCounter == 0) {
        RebuildOffsetLSBModel(model, sym);
    }

    return sym;
}

BOOL Bitknit2Decompressor::CopyRawQuantum()
{
    auto quantumSize = m_dstQuantumEnd - m_dstCur;
    if (!CanRead(quantumSize)) {
        return FALSE;
    }

    if (!CanWrite(quantumSize)) {
        return FALSE;
    }

    memcpy(m_dstCur, m_srcCur, quantumSize);

    m_dstCur += quantumSize;
    m_srcCur += quantumSize;

    return TRUE;
}

BOOL Bitknit2Decompressor::Renormalize()
{
    // If state1 is below the ANS threshold, pull 16 bits from input.
    if (m_state1 < 0x10000) {
        // Need 2 bytes from the stream.
        if (!CanRead(2)) {
            return FALSE; // Input truncated
        }

        // Shift state1 and append next 16 bits.
        m_state1 = (m_state1 << 16) | Read<uint16_t>(m_srcCur);
    }

    // Swap the two interleaved rANS states.
    std::swap(m_state1, m_state2);

    return TRUE;
}

BOOL Bitknit2Decompressor::DecodeQuantum()
{
    if (!ParseQuantumHeader()) {
        return FALSE;
    }

    if (!m_emittedFirstLiteral) {
        *m_dstCur++ = m_state1 & 0xFF;
        m_state1 >>= 8;
        if (!Renormalize()) {
            return FALSE;
        }
        m_emittedFirstLiteral = TRUE;
    }

    while (m_dstCur < m_dstQuantumEnd) {
        auto& lit = m_literalModels[(m_dstCur - m_dstStart) & 3];
        auto sym = DecodeLiteral(lit, m_state1);
        if (!Renormalize()) {
            return FALSE;
        }
        if (sym < 256) { // literal
            auto literal = static_cast<uint8_t>(sym + m_dstCur[-static_cast<int>(m_lastCopyOffset)]);
            *m_dstCur++ = literal;
            if (m_dstCur >= m_dstQuantumEnd) {
                break;
            }
            auto& lit2 = m_literalModels[(m_dstCur - m_dstStart) & 3];
            sym = DecodeLiteral(lit2, m_state1);
            if (!Renormalize()) {
                return FALSE;
            }
            if (sym < 256) {
                literal = static_cast<uint8_t>(sym + m_dstCur[-static_cast<int>(m_lastCopyOffset)]);
                *m_dstCur++ = literal;
                continue;
            }
        }
        if (sym >= 288) { // extended match length
            auto nb = sym - 287; // number of bits to extract
            // Extract 'nb' bits from STATE1, forming a larger sym
            auto extra = m_state1 & ((1u << nb) - 1u);
            sym = extra + (1u << nb) + 286u;
            m_state1 >>= nb;
            if (!Renormalize()) {
                return FALSE;
            }
        }
        auto copyLength = sym - 254;
        auto& distLsb = m_offsetLsbModels[(m_dstCur - m_dstStart) & 3];
        sym = DecodeOffsetLSB(distLsb, m_state1);
        if (!Renormalize()) {
            return FALSE;
        }

        auto matchDist = 0u;
        if (sym >= 8) { // extended match distance
            auto nb = DecodeOffsetBitLength(m_offsetBitLengthModel, m_state1);
            if (!Renormalize()) {
                return FALSE;
            }
            auto nbBits = nb & 0x0F;
            matchDist = m_state1 & ((1u << nbBits) - 1u);
            m_state1 >>= nbBits;
            if (!Renormalize()) {
                return FALSE;
            }
            if (nb >= 0x10) {
                if (!CanRead(2)) {
                    return FALSE;
                }
                matchDist = (matchDist << 16) | Read<uint16_t>(m_srcCur);
            }
            matchDist = (32u << nb) + (matchDist << 5) + (sym - 39u);
            auto idxA = (m_recentDistMask >> 21) & 7;
            auto idxB = (m_recentDistMask >> 18) & 7;
            m_recentOffsets[idxA] = m_recentOffsets[idxB];
            m_recentOffsets[idxB] = matchDist;
        } else { // cache hit
            auto shift = 3 * sym;
            auto idx = (m_recentDistMask >> shift) & 7u;

            // Extract the match distance
            matchDist = m_recentOffsets[idx];

            // Rotate this index to the front of the permutation list
            auto mask = ~7u << shift;

            m_recentDistMask = (m_recentDistMask & mask) | ((idx + 8 * m_recentDistMask) & ~mask);
        }

        if (matchDist == 0 || matchDist > (m_dstCur - m_dstStart)) {
            return FALSE;
        }

        if (!CanWrite(copyLength)) {
            return FALSE;
        }

        uint8_t* read = m_dstCur - matchDist;
        for (auto i = 0u; i < copyLength; ++i) {
            m_dstCur[i] = read[i];
        }

        m_dstCur += copyLength;
        m_lastCopyOffset = matchDist;
    }

    // End-of-quantum validation
    if (m_state1 != 0x10000 || m_state2 != 0x10000) {
        return FALSE;
    }

    return TRUE;
}

BOOL Bitknit2Decompressor::ParseQuantumHeader()
{
    //
    // The 32-bit header word 'v' contains the packed information needed to
    // reconstruct BOTH rANS decoder states for this quantum.
    //
    // After rotating halves, the layout of 'v' is:
    //
    //      [ a (upper 28 bits) ][ n (lower 4 bits) ]
    //
    // Meaning:
    //
    //   - 'n' (the lowest 4 bits) tells the decoder how many of the low bits
    //     of 'a' belong to STATE2. The encoder uses this to control how many
    //     high bits STATE2 will have (n + 16).
    //
    //   - 'a' (upper 28 bits) contains the shared bitfield from which BOTH
    //     rANS states are reconstructed. STATE1 takes the upper portion of
    //     this field (after shifting by 'n'). STATE2 takes the lower portion
    //     together with a required leading 1 bit. Additional 16-bit chunks
    //     are appended to 'a' as needed until the states reach valid size.
    //
    // In summary:
    //
    //      v = packed header word
    //      a = shared bitfield that contains pieces of both ANS states
    //      n = shift/count parameter that explains how to split and align 'a'
    //
    // These fields must be extracted before reconstructing STATE1 and STATE2.
    //

    // Read first 32-bit word
    if (!CanRead(sizeof(uint32_t))) {
        return FALSE;
    }

    auto v = Read<uint32_t>(m_srcCur);

    // The 32-bit header was stored as: [low16][high16] in the file,
    // but the fields inside the header are arranged as if it were
    // [high16][low16].
    //
    // This rotate restores the intended order.
    v = (v << 16) | (v >> 16);
    if (v < 0x10000) {
        return FALSE; // Invalid compressed data
    }

    auto a = v >> 4;
    auto n = v & 0x0F;

    if (a < 0x10000) {
        if (!CanRead(sizeof(uint16_t))) {
            return FALSE;
        }
        a = (a << 16) | Read<uint16_t>(m_srcCur);
    }

    // Build STATE1
    auto bits = a >> n;
    if (bits < 0x10000) {
        if (!CanRead(2)) {
            return FALSE;
        }
        bits = (bits << 16) | Read<uint16_t>(m_srcCur);
    }

    // Complete construction of 'a'
    if (!CanRead(2)) {
        return FALSE;
    }
    a = (a << 16) | Read<uint16_t>(m_srcCur);

    // Build STATE2
    auto highCount = n + 16;
    auto mask = (1u << highCount) - 1u;
    auto bits2 = (1u << highCount) | (a & mask);

    m_state1 = bits;
    m_state2 = bits2;

    // Mark the "first literal not yet emitted" for this quantum
    m_emittedFirstLiteral = FALSE;

    return TRUE;
}

uint32_t Bitknit2Decompressor::DecodeOffsetBitLength(OffsetBitLengthModel& model, uint32_t& state)
{
    // Extract 15-bit residue from rANS state
    auto residue = state & 0x7FFFu;

    // Initial fast guess (shift = 9 -> 64 blocks)
    auto sym = static_cast<uint32_t>(model.lookup[residue >> 9]);

    // One-step correction
    if (residue > model.cdf[sym + 1]) {
        ++sym;
    }

    // Full correction loop (rarely more than 1 iteration)
    while (residue >= model.cdf[sym + 1]) {
        ++sym;
    }

    // rANS state update:
    //
    //   state = residue 
    //         + (state >> 15) * (cdf[s+1] - cdf[s])
    //         - cdf[s]
    //
    auto lo = model.cdf[sym];
    auto hi = model.cdf[sym + 1];
    auto width = hi - lo;

    state = residue + ((state >> 15) * width) - lo;

    // Bit-length model freq update (+31)
    model.freq[sym] += 31;

    // adaptive rebuild?
    if (--model.adaptCounter == 0) {
        RebuildOffsetBitLengthModel(model, sym);
    }

    return sym;
}

void Bitknit2Decompressor::RebuildOffsetBitLengthModel(OffsetBitLengthModel& model, uint32_t sym)
{
    // Reset adaptive interval
    model.adaptCounter = 1024;

    // Apply BitKnit's large symbol weight for the bit-length model
    model.freq[sym] += 1004;

    // Recompute CDF with smoothing
    auto sum = 0u;
    for (auto i = 0u; i < 21; ++i) {
        sum += model.freq[i];

        // Reset raw frequency for next adaptation cycle
        model.freq[i] = 1;

        // Smooth CDF[i+1] toward cumulative sum
        auto oldCdf = model.cdf[i + 1];
        auto newCdf = oldCdf + ((sum - oldCdf) >> 1);

        model.cdf[i + 1] = static_cast<uint16_t>(newCdf);
    }

    // Rebuild lookup table - canonical BitKnit write-4 loop
    constexpr auto SHIFT = 9u; // block = residue >> 9
    auto* p = model.lookup;

    for (auto i = 0u; i < 21; ++i) {
        auto endBlock = (model.cdf[i + 1] - 1) >> SHIFT;
        auto* pEnd = &model.lookup[endBlock];

        do {
            p[0] = p[1] = p[2] = p[3] = static_cast<uint16_t>(i);
            p += 4;
        } while (p <= pEnd);

        p = pEnd + 1;
    }
}

void Bitknit2Decompressor::RebuildOffsetLSBModel(OffsetLsbModel& model, uint32_t sym)
{
    // Reset adaptive interval for next cycle
    model.adaptCounter = 1024;

    // Apply BitKnit's large symbol bias for LSB models
    model.freq[sym] += 985;

    // Recompute the CDF using BitKnit's smoothing formula
    auto sum = 0u;
    for (auto i = 0u; i < 40; ++i) {
        sum += model.freq[i];

        // Reset raw frequencies for next adaptation cycle
        model.freq[i] = 1;

        // Smooth the CDF entry toward the cumulative sum
        auto oldCdf = model.cdf[i + 1];
        auto newCdf = oldCdf + ((sum - oldCdf) >> 1);

        model.cdf[i + 1] = static_cast<uint16_t>(newCdf);
    }

    // Rebuild lookup table (block = residue >> 9)
    constexpr auto SHIFT = 9u;
    auto* p = model.lookup;

    for (auto i = 0u; i < 40; ++i) {
        auto endBlock = (model.cdf[i + 1] - 1) >> SHIFT;
        auto* pEnd = &model.lookup[endBlock];

        do {
            p[0] = p[1] = p[2] = p[3] = static_cast<uint16_t>(i);
            p += 4;
        } while (p <= pEnd);

        p = pEnd + 1;
    }
}


void Bitknit2Decompressor::RebuildLiteralModel(LiteralModel& model, uint32_t sym)
{
    // Reset adaptive interval for next cycle.
    model.adaptCounter = 1024;

    // Boost the just-seen symbol.
    // BitKnit2 uses +725, a large weight that shifts probability toward recent symbols.
    model.freq[sym] += 725;

    // Rebuild the CDF using an exponential moving average.
    // 'sum' accumulates the raw frequencies.
    // For each symbol i:
    //
    //   newCDF[i+1] = oldCDF[i+1] + (sum - oldCDF[i+1]) / 2
    //
    // This smoothly adjusts the distribution without destabilizing it.
    auto sum = 0u;
    for (auto i = 0u; i < 300; ++i) {
        sum += model.freq[i];

        // Reset raw frequency for next cycle
        model.freq[i] = 1;

        // Smoothly pull model.cdf[i+1] toward `sum`
        model.cdf[i + 1] = static_cast<uint16_t>(
            model.cdf[i + 1] + ((sum - model.cdf[i + 1]) >> 1)
        );
    }

    // Rebuild lookup table using canonical BitKnit write-4 logic
    constexpr auto SHIFT = 6u; // literals: block = residue >> 6
    auto* p = model.lookup;

    for (auto i = 0u; i < 300; ++i) {
        auto endBlock = (model.cdf[i + 1] - 1) >> SHIFT;
        auto* pEnd = &model.lookup[endBlock];

        do {
            p[0] = p[1] = p[2] = p[3] = static_cast<uint16_t>(i);
            p += 4;
        } while (p <= pEnd);

        p = pEnd + 1;
    }
}

void Bitknit2Decompressor::InitializeBitLengthModel()
{
    auto& model = m_offsetBitLengthModel;

    // Reset adaptive countdown
    model.adaptCounter = 1024;

    // Initialize raw frequencies (all symbols = 1)
    for (auto& f : model.freq) {
        f = 1;
    }

    // Build the initial CDF table (21 symbols + terminal)
    constexpr uint32_t TOTAL_RANGE = 32768;
    constexpr uint32_t NUM_SYMBOLS = 21;

    model.cdf[0] = 0;

    for (auto i = 1u; i <= NUM_SYMBOLS; ++i) {
        model.cdf[i] = static_cast<uint16_t>((TOTAL_RANGE * i) / NUM_SYMBOLS);
    }

    ATLASSERT(model.cdf[NUM_SYMBOLS] == TOTAL_RANGE);

    // Build the lookup table
    constexpr auto SHIFT = 9u; // block = residue >> 9
    auto* p = model.lookup; // write pointer into lookup[]

    for (auto i = 0u; i < NUM_SYMBOLS; ++i) {
        // Determine the last block index covered by symbol i:
        //
        //   endBlock = floor((cdf[i+1] - 1) / 512)
        //
        // This gives the inclusive block index whose residue range
        // intersects the CDF interval for symbol i.
        //
        auto endBlock = (model.cdf[i + 1] - 1) >> SHIFT;

        // Pointer to last lookup entry filled by symbol i
        auto* pEnd = &model.lookup[endBlock];

        // Fill lookup entries in groups of 4, as BitKnit does
        do {
            p[0] = p[1] = p[2] = p[3] = static_cast<uint16_t>(i);
            p += 4;
        } while (p <= pEnd);

        // Advance to next block
        p = pEnd + 1;
    }
}

void Bitknit2Decompressor::InitializeLiteralModels()
{
    for (auto& model : m_literalModels) {
        // Reset the adaptive countdown
        // BitKnit2 updates its probabilities every 1024 symbols.
        // Setting this to 1024 is the "fresh model" state.
        model.adaptCounter = 1024;

        // Initialize the raw frequency array.
        // Before any decoding happens, BitKnit2 sets freq[s] = 1
        // for all symbols (0..299).
        for (auto& f : model.freq) {
            f = 1;
        }

        // Build the initial CDF table for 300 symbols.
        // BitKnit2 assigns:
        //   - first 264 symbols: equal-sized intervals
        //   - last  36 symbols:  minimal intervals (one unit each)
        //
        // Total range is 2^15 = 32768.
        constexpr uint32_t TOTAL_RANGE = 32768;
        constexpr uint32_t NUM_SYMBOLS = 300;
        constexpr uint32_t NUM_FULL = 264; // symbols 0..263

        // CDF[0] is always zero
        model.cdf[0] = 0;

        // Fill CDF[1]..CDF[264]
        for (auto i = 1u; i < NUM_FULL; ++i) {
            model.cdf[i] = static_cast<uint16_t>((32732u * i) / 264u);
        }

        // Fill CDF[265]..CDF[300]
        for (auto i = NUM_FULL; i <= NUM_SYMBOLS; ++i) {
            model.cdf[i] = static_cast<uint16_t>(32468u + i);
        }

        // At this point:
        //   model.cdf[300] == 32768
        // This matches BitKnit2's required total.
        //
        ATLASSERT(model.cdf[NUM_SYMBOLS] == TOTAL_RANGE);

        // Build the lookup table.
        constexpr auto SHIFT = 6u; // literals: block = residue >> 6
        uint16_t* p = model.lookup; // write pointer into lookup table

        for (auto i = 0u; i < NUM_SYMBOLS; ++i) {
            // Which lookup block does this symbol extend to?
            //
            // Example:
            //   cdf[i+1] = 2000
            //   (cdf[i+1] - 1) >> 6 = block covering residue 1984..2047
            //
            auto endBlock = (model.cdf[i + 1] - 1) >> SHIFT;
            auto* pEnd = &model.lookup[endBlock];

            // Fill entries in groups of four, as BitKnit does.
            // This is faster and ensures block boundaries match the encoder.
            do {
                p[0] = p[1] = p[2] = p[3] = static_cast<uint16_t>(i);
                p += 4;
            } while (p <= pEnd);

            // Move pointer to the start of the next block range.
            p = pEnd + 1;
        }
    }
}

void Bitknit2Decompressor::InitializeOffsetModels()
{
    for (auto& model : m_offsetLsbModels) {
        // Reset adaptive countdown.
        model.adaptCounter = 1024;

        // Initialize raw frequencies.
        // All 40 symbols start with equal weight = 1.
        for (auto& f : model.freq) {
            f = 1;
        }

        // Build initial CDF table (41 entries).
        // Offsets use a full 15-bit ANS range (0..32767), just like literals.
        //
        // BUT - unlike literals - the lookup table uses a coarser block
        // size (residue >> 9), not (residue >> 6).
        constexpr uint32_t TOTAL_RANGE = 32768; // 2^15 ANS range
        constexpr uint32_t NUM_SYMBOLS = 40; // 40 offset-LSB symbols

        model.cdf[0] = 0;

        // Uniform partition over full ANS range.
        for (auto i = 1u; i <= NUM_SYMBOLS; ++i) {
            model.cdf[i] = static_cast<uint16_t>((TOTAL_RANGE * i) / NUM_SYMBOLS);
        }

        ATLASSERT(model.cdf[NUM_SYMBOLS] == TOTAL_RANGE);

        // Build the lookup table.
        constexpr auto SHIFT = 9u; // Offset-LSB: block = residue >> 9
        auto* p = model.lookup; // write pointer into lookup table

        for (auto i = 0u; i < NUM_SYMBOLS; ++i) {
            // Determine final block where CDF[i+1]-1 resides.
            auto endBlock = (model.cdf[i + 1] - 1) >> SHIFT;

            // Pointer to end of this block range.
            auto* pEnd = &model.lookup[endBlock];

            // Fill blocks with symbol i (4 writes at a time).
            do {
                p[0] = p[1] = p[2] = p[3] = static_cast<uint16_t>(i);
                p += 4;
            } while (p <= pEnd);

            // Move to beginning of next block range.
            p = pEnd + 1;
        }
    }
}
