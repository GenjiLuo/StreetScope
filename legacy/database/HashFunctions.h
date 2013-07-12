//==============================================================================
// HashFunctions.h
// Created 2012.5.7
//==============================================================================

#ifndef ESTDLIB_HASH_FUNCTIONS
#define ESTDLIB_HASH_FUNCTIONS


//==============================================================================
// Hash Function Declarations
//==============================================================================

const unsigned murmurConst = 0x5bd1e995;

// Makes all bits in a word depend on all other bits.
inline void mix1 (unsigned& a);
inline unsigned hash1 (unsigned a);

// Mixes b into a, but not the other way around.
inline void mix2 (unsigned& a, unsigned b);
inline unsigned hash2 (unsigned a, unsigned b);

// Hashes an array of unsigneds
inline unsigned murmurhash (void* data, unsigned len_u, unsigned seed = 0xceed);


//==============================================================================
// Hash Function Definitions
//==============================================================================

// Makes all bits in a word depend on all other bits.
void mix1 (unsigned& a) {
    //a ^= a >> 13;    // google original, has 12 cycles, biggest is 1.9e9, 0 is a 1 cycle
    a ^= ~a >> 13;    // 26 cycles but mostly 1 big cycle
    a *= murmurConst;
    a ^= a >> 15;
}
unsigned hash1 (unsigned a) { mix1(a); return a; }

// Mixes b into a, but not the other way around.
void mix2 (unsigned& a, unsigned b) {
    b *= murmurConst;
    b ^= b >> 24;
    b *= murmurConst;
    a *= murmurConst;
    a ^= b;
}
unsigned hash2 (unsigned a, unsigned b) { mix2(a, b); return a; }

// Hashes an array of unsigneds
unsigned murmurhash (void* data, unsigned len_u, unsigned seed) {
    unsigned hash = seed ^ len_u;
    unsigned* data_u = reinterpret_cast<unsigned*>(data);
    while (len_u > 0) {
        mix2(hash, *data_u);
        ++data_u;
        --len_u;
    }
    mix1(hash);
    return hash;
}


#endif // ESTDLIB_HASH_FUNCTIONS
