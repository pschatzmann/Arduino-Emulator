/*
    MD5Builder - Simple MD5 hash calculations

    Updated for the Pico by Earle F. Philhower, III

    Modified from the ESP8266 version which is
    Copyright (c) 2015 Hristo Gochkov. All rights reserved.
    This file is part of the esp8266 core for Arduino environment.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <Arduino.h>
#include <MD5Builder.h>
#include <memory>

/*
 * Constants defined by the MD5 algorithm
 */
#define A 0x67452301
#define B 0xefcdab89
#define C 0x98badcfe
#define D 0x10325476

static uint32_t S[] = {7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
                       5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
                       4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
                       6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21};

static uint32_t K[] = {0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
                       0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
                       0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
                       0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
                       0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
                       0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
                       0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
                       0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
                       0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
                       0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
                       0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
                       0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
                       0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
                       0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
                       0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
                       0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391};

/*
 * Padding used to make the size (in bits) of the input congruent to 448 mod 512
 */
static uint8_t PADDING[] = {0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

/*
 * Bit-manipulation functions defined by the MD5 algorithm
 */
#ifdef F
#undef F
#endif
#define F(X, Y, Z) ((X & Y) | (~X & Z))
#define G(X, Y, Z) ((X & Z) | (Y & ~Z))
#define H(X, Y, Z) (X ^ Y ^ Z)
#define I(X, Y, Z) (Y ^ (X | ~Z))

/*
 * Rotates a 32-bit word left by n bits
 */
static inline uint32_t rotateLeft(uint32_t x, uint32_t n){
    return (x << n) | (x >> (32 - n));
}

/*
 * Step on 512 bits of input with the main MD5 algorithm.
 */
static void md5Step(uint32_t *buffer, uint32_t *input){
    uint32_t AA = buffer[0];
    uint32_t BB = buffer[1];
    uint32_t CC = buffer[2];
    uint32_t DD = buffer[3];

    uint32_t E;

    unsigned int j;

    for(unsigned int i = 0; i < 64; ++i){
        switch(i / 16){
            case 0:
                E = F(BB, CC, DD);
                j = i;
                break;
            case 1:
                E = G(BB, CC, DD);
                j = ((i * 5) + 1) % 16;
                break;
            case 2:
                E = H(BB, CC, DD);
                j = ((i * 3) + 5) % 16;
                break;
            default:
                E = I(BB, CC, DD);
                j = (i * 7) % 16;
                break;
        }

        uint32_t temp = DD;
        DD = CC;
        CC = BB;
        BB = BB + rotateLeft(AA + E + K[i] + input[j], S[i]);
        AA = temp;
    }

    buffer[0] += AA;
    buffer[1] += BB;
    buffer[2] += CC;
    buffer[3] += DD;
}

/*
 * Add some amount of input to the context
 *
 * If the input fills out a block of 512 bits, apply the algorithm (md5Step)
 * and save the result in the buffer. Also updates the overall size.
 */
void MD5Builder::add(const uint8_t* input_buffer, const size_t input_len) {
    uint32_t input[16];
    size_t offset = _size % 64;
    _size += input_len;

    // Copy each byte in input_buffer into the next space in our context input
    for(size_t i = 0; i < input_len; ++i){
        _input[offset++] = (uint8_t)*(input_buffer + i);

        // If we've filled our context input, copy it into our local array input
        // then reset the offset to 0 and fill in a new buffer.
        // Every time we fill out a chunk, we run it through the algorithm
        // to enable some back and forth between cpu and i/o
        if(offset % 64 == 0){
            for(unsigned int j = 0; j < 16; ++j){
                // Convert to little-endian
                // The local variable `input` our 512-bit chunk separated into 32-bit words
                // we can use in calculations
                input[j] = (uint32_t)(_input[(j * 4) + 3]) << 24 |
                           (uint32_t)(_input[(j * 4) + 2]) << 16 |
                           (uint32_t)(_input[(j * 4) + 1]) <<  8 |
                           (uint32_t)(_input[(j * 4)]);
            }
            md5Step(_buffer, input);
            offset = 0;
        }
    }
}

static bool hex_char_to_nibble(uint8_t c, uint8_t& nibble) {
    if (c >= 'a' && c <= 'f') {
        nibble = c - ((uint8_t)'a' - 0xA);
        return true;
    }
    if (c >= 'A' && c <= 'F') {
        nibble = c - ((uint8_t)'A' - 0xA);
        return true;
    }
    if (c >= '0' && c <= '9') {
        nibble = c - (uint8_t)'0';
        return true;
    }
    return false;
}

void MD5Builder::begin(void) {
    _size = 0;

    _buffer[0] = (uint32_t)A;
    _buffer[1] = (uint32_t)B;
    _buffer[2] = (uint32_t)C;
    _buffer[3] = (uint32_t)D;
}

void MD5Builder::addHexString(const char * data) {
    size_t len = strlen(data);

    // Require an even number of hex characters; odd lengths cannot form full bytes.
    if ((len == 0) || (len % 2 != 0)) {
        return;
    }

    constexpr size_t chunk_size = 64;
    uint8_t tmp[chunk_size];
    size_t byte_count = len / 2;

    for (size_t processed = 0; processed < byte_count;) {
        size_t remaining = byte_count - processed;
        size_t this_chunk = (remaining > chunk_size) ? chunk_size : remaining;

        for (size_t i = 0; i < this_chunk; ++i) {
            size_t hex_index = (processed + i) * 2;
            uint8_t high;
            uint8_t low;

            if (!hex_char_to_nibble(static_cast<uint8_t>(data[hex_index]), high) ||
                !hex_char_to_nibble(static_cast<uint8_t>(data[hex_index + 1]), low)) {
                return;
            }

            tmp[i] = static_cast<uint8_t>((high << 4) | low);
        }

        add(tmp, this_chunk);
        processed += this_chunk;
    }
}

bool MD5Builder::addStream(Stream &stream, const size_t maxLen) {
    const int buf_size = 512;
    size_t maxLengthLeft = maxLen;

    auto buf = std::unique_ptr<uint8_t[]> {new (std::nothrow) uint8_t[buf_size]};

    if (!buf) {
        return false;
    }

    size_t bytesAvailable = stream.available();
    while ((bytesAvailable > 0) && (maxLengthLeft > 0)) {

        // determine number of bytes to read
        size_t readBytes = bytesAvailable;
        if (readBytes > maxLengthLeft) {
            readBytes = maxLengthLeft;    // read only until max_len
        }
        if (readBytes > buf_size) {
            readBytes = buf_size;    // not read more the buffer can handle
        }

        // read data and check if we got something
        size_t numBytesRead = stream.readBytes(buf.get(), readBytes);
        if (numBytesRead < 1) {
            return false;
        }

        // Update MD5 with buffer payload
        add(buf.get(), numBytesRead);

        // update available number of bytes
        maxLengthLeft -= numBytesRead;
        bytesAvailable = stream.available();
    }

    return true;
}

/*
 * Pad the current input to get to 448 bytes, append the size in bits to the very end,
 * and save the result of the final iteration into digest.
 */
void MD5Builder::calculate(void) {
    uint32_t input[16];
    size_t offset = _size % 64;
    size_t padding_length = offset < 56 ? 56 - offset : (56 + 64) - offset;

    // Fill in the padding and undo the changes to size that resulted from the update
    add(PADDING, padding_length);
    _size -= padding_length;

    // Do a final update (internal to this function)
    // Last two 32-bit words are the two halves of the size (converted from bytes to bits)
    for(unsigned int j = 0; j < 14; ++j){
        input[j] = (uint32_t)(_input[(j * 4) + 3]) << 24 |
                   (uint32_t)(_input[(j * 4) + 2]) << 16 |
                   (uint32_t)(_input[(j * 4) + 1]) <<  8 |
                   (uint32_t)(_input[(j * 4)]);
    }
    uint64_t bit_length = _size * 8ULL;
    input[14] = (uint32_t)(bit_length);
    input[15] = (uint32_t)(bit_length >> 32);

    md5Step(_buffer, input);

    // Move the result into digest (convert from little-endian)
    for(unsigned int i = 0; i < 4; ++i){
        _digest[(i * 4) + 0] = (uint8_t)((_buffer[i] & 0x000000FF));
        _digest[(i * 4) + 1] = (uint8_t)((_buffer[i] & 0x0000FF00) >>  8);
        _digest[(i * 4) + 2] = (uint8_t)((_buffer[i] & 0x00FF0000) >> 16);
        _digest[(i * 4) + 3] = (uint8_t)((_buffer[i] & 0xFF000000) >> 24);
    }
}

void MD5Builder::getBytes(uint8_t * output) const {
    memcpy(output, _digest, 16);
}

void MD5Builder::getChars(char * output) const {
    for (uint8_t i = 0; i < 16; i++) {
        sprintf(output + (i * 2), "%02x", _digest[i]);
    }
}

String MD5Builder::toString(void) const {
    char out[33];
    getChars(out);
    return String(out);
}
