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

#include <catalan.h>
#include <stdlib.h>

#define GET_BIT(b, i) ((((b)[(i) / 8]) >> ((i) % 8)) & 1)
#define SET_BIT(b, i) (((b)[(i) / 8]) |= (1 << ((i) % 8)))
#define CLR_BIT(b, i) (((b)[(i) / 8]) &= ~(1 << ((i) % 8)))

static const uint64_t catalan_table[] = {1, 1, 2, 5, 14, 42, 132, 429, 1430, 4862, 16796, 58786, 208012, 742900, 2674440, 9694845, 35357670, 129644790, 477638700, 1767263190, 6564120420, 24466267020, 91482563640, 343059613650, 1289904147324, 4861946401452, 18367353072152, 69533550916004, 263747951750360, 1002242216651368, 3814986502092304, 14544636039226909, 55534064877048198, 212336130412243110, 812944042149730764, 3116285494907301262, 11959798385860453492ULL};

uint64_t catalan_number_for(int n) {
    if(n < 0 || n > CATALAN_CNF_NUB) return 0;
    return catalan_table[n];
}

void __catalan_paths(int n, uint8_t *buf, uint64_t *bufoff, uint8_t *pth, int pth_bitsz, int pthoff, int rises, int lvl) {

    if(rises == n) {
        for(int i = 0; i < pth_bitsz; ++i) if(GET_BIT(pth, i)) SET_BIT(buf, *bufoff + i);    
        *bufoff += pth_bitsz;
        return;
    }

    if(lvl > 0) __catalan_paths(n, buf, bufoff, pth, pth_bitsz, pthoff + 1, rises, lvl - 1);
    if(lvl < n) {
        SET_BIT(pth, pthoff);
        __catalan_paths(n, buf, bufoff, pth, pth_bitsz, pthoff + 1, rises + 1, lvl + 1);
        CLR_BIT(pth, pthoff);
    }
}

uint64_t catalan_path_len_bits(int n) {
    return 2*(uint64_t)n - 2;
}

uint8_t *catalan_paths_for(int n) {

    if(n < 1 || n > CATALAN_CPF_NUB) return NULL;

    uint64_t pth_bitsz = catalan_path_len_bits(n);
    uint64_t pth_bytsz = (pth_bitsz + 7) / 8;
    uint64_t buf_bitsz = catalan_number_for(n) * pth_bitsz;
    uint64_t buf_bytsz = (buf_bitsz + 7) / 8;

    uint8_t *buf = calloc(buf_bytsz, 1);
    if(!buf) goto defer1;

    uint8_t *pth = calloc(pth_bytsz, 1);
    if(!pth) goto defer2;

    uint64_t off = 0;
    __catalan_paths(n, buf, &off, pth, pth_bitsz, 0, 1, 1);

    free(pth);
    return buf;

defer2:
    free(buf);
defer1:
    return NULL;
}