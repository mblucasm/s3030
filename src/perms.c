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

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

static int float_compar(const void *a, const void *b) {
    float diff = *(float*)a - *(float*)b;
    return (diff > 0) - (diff < 0);
}

static void __perms_unique(const float * const nums, float *path, size_t path_len, bool *used, const size_t used_len, float *perms, size_t *offset) {

    if(path_len == used_len) {
        memcpy(perms + *offset, path, sizeof(float) * path_len);
        *offset += path_len;
        return;
    }

    for(size_t i = 0; i < used_len; ++i) {

        if(used[i]) continue;
        if(i > 0 && nums[i] == nums[i - 1] && !used[i - 1]) continue;

        used[i] = true;
        path[path_len] = nums[i];
        __perms_unique(nums, path, path_len + 1, used, used_len, perms, offset);
        used[i] = false;
    }
}

int perms_unique(float *nums, const size_t nums_len, float *perms) {
    
    qsort(nums, nums_len, sizeof(float), float_compar);
    
    bool *used = calloc(nums_len, sizeof(bool));
    if(used == NULL) goto defer1;

    float *path = malloc(nums_len * sizeof(float));
    if(path == NULL) goto defer2;
    
    size_t offset = 0;
    __perms_unique(nums, path, 0, used, nums_len, perms, &offset);

    free(used);
    free(path);
    return 0;

defer2:
    free(used);
defer1:
    return 1;
}