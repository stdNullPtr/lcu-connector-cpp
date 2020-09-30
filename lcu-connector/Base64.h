#pragma once
#include <string>
#include <cassert>
#include <limits>
#include <stdexcept>
#include <cctype>

// macros are evil, kids
#pragma push_macro("max")
#undef max 

static const char b64_table[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const char reverse_table[128] = {
   64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
   64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
   64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
   52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
   64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
   15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
   64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
   41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64
};

std::string base64_encode(const std::string& binData)
{
    using std::string;
    using std::numeric_limits;

    if (binData.size() > (numeric_limits<string::size_type>::max() / 4u) * 3u) 
    {
        throw std::length_error("Converting too large a string to base64.");
    }

    const size_t binLen = binData.size();
    // Use = signs so the end is properly padded.
    string retVal((((binLen + 2) / 3) * 4), '=');
    size_t outPos = 0;
    int bits_collected = 0;
    unsigned int accumulator = 0;
    const string::const_iterator binEnd = binData.end();

    for (string::const_iterator i = binData.begin(); i != binEnd; ++i) {
        accumulator = (accumulator << 8) | (*i & 0xffu);
        bits_collected += 8;
        while (bits_collected >= 6) {
            bits_collected -= 6;
            retVal[outPos++] = b64_table[(accumulator >> bits_collected) & 0x3fu];
        }
    }
    if (bits_collected > 0) { // Any trailing bits that are missing.
        assert(bits_collected < 6);
        accumulator <<= 6 - bits_collected;
        retVal[outPos++] = b64_table[accumulator & 0x3fu];
    }
    assert(outPos >= (retVal.size() - 2));
    assert(outPos <= retVal.size());
    return retVal;
}

std::string base64_decode(const std::string& ascData)
{
    using std::string;
    string retVal;
    const string::const_iterator last = ascData.end();
    int bits_collected = 0;
    unsigned int accumulator = 0;

    for (string::const_iterator i = ascData.begin(); i != last; ++i) {
        const int c = *i;
        if (isspace(c) || c == '=') {
            // Skip whitespace and padding. Be liberal in what you accept.
            continue;
        }
        if ((c > 127) || (c < 0) || (reverse_table[c] > 63)) {
            throw std::invalid_argument("This contains characters not legal in a base64 encoded string.");
        }
        accumulator = (accumulator << 6) | reverse_table[c];
        bits_collected += 6;
        if (bits_collected >= 8) {
            bits_collected -= 8;
            retVal += static_cast<char>((accumulator >> bits_collected) & 0xffu);
        }
    }
    return retVal;
}

#pragma pop_macro("max")