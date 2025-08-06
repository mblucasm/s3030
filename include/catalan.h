// Copyright © 2025 Lucas Martín
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the “Software”), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is furnished to do
// so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef CATALAN_H
#define CATALAN_H

// Catalan paths (Dyck words) are encoded as binary sequences over {1,0},
// where '1' represents an up-step and '0' a down-step. Due to their structure,
// all valid paths of order n start with an up-step and end with a down-step
//
// These outer bits are invariant and therefore omitted from storage
// The remaining (2n − 2) bits per path are stored contiguously in a packed bit array
//
// Total number of paths: C(n) (the n-th Catalan number)
// Total storage required: C(n) * (2n − 2) bits, rounded up to the nearest byte
//
// Example (n = 3):
// Full paths:      101010, 101100, 110010, 110100, 111000  (C(3) = 5 paths)
// Stored bits:      0101,   0110,   1001,   1010,   1100   (4 bits per path)
// Total bits: 5 * 4 = 20 bits (ceil(20 / 8) = 3 bytes)

#include <stdint.h>

#define CATALAN_CNF_NUB (36)
#define CATALAN_CPF_NUB (33)

// Returns the n-th Catalan number C(n) by array lookup
// Returns 0 if n not in [0, 36]
// The upper bound n = 36 is chosen because C(36) fits within 64 bits
uint64_t catalan_number_for(int n);

// Returns the bit-length of a Catalan path of order n,
// excluding the canonical leading '1' and trailing '0'
// Defined as 2n - 2
uint64_t catalan_path_len_bits(int n);

// Allocates and returns a packed bit array containing all Catalan paths of order n
// Each path is stored as a (2n − 2)-bit sequence (with its boundary bits omitted)
// All paths are concatenated back-to-back without delimiters or alignment
//
// Total buffer size: ceil(C(n) * (2n − 2) / 8) bytes
// For n = 33 (the largest supported input), the intermediate bit count 
// C(33) * (2*33 − 2) fits safely within 64 bits (no risk of overflow)
//
// Returns NULL if n not in [1, 33]
uint8_t *catalan_paths_for(int n);


#endif // CATALAN_H