/* The MIT License (MIT)
 *
 * Copyright (c) 2015 Pontus Karlsson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdlib.h>

#include "crypt.h"

/* Simple, efficient hash function from Daniel J. Bernstein */
unsigned int
DJBHash(uint8_t *data, int len)
{
    unsigned int hash = 5381;
    unsigned int i = 0;

    for(i = 0; i < len; data++, i++)
    {
        hash = ((hash << 5) + hash) + (*data);
    }

    return hash;
}

char*
base64_encode(const unsigned char *data, size_t ilen, size_t *olen)
{
    static const char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/";

    unsigned char a,b,c;
    unsigned int idx;
    unsigned int i , j, s;
    char *obuff;
    size_t osize;

    if ( !data  || !ilen )
        return 0;

    osize = 4 *((ilen + 2) / 3);
    obuff = malloc(osize);

    if ( !obuff )
        return 0;

    s = ilen - (ilen % 3);

    for (i = 0, j = 0; i < s;)
    {
        a = data[i++];
        b = data[i++];
        c = data[i++];

        idx = (a << 16) + (b << 8) + c;
        obuff[j++] = table[(idx >> 3 * 6) & 0x3F];
        obuff[j++] = table[(idx >> 2 * 6) & 0x3F];
        obuff[j++] = table[(idx >> 6) & 0x3F];
        obuff[j++] = table[idx & 0x3F];
    }
    if (s < ilen)
    {
        a = data[i++];
        b = i < ilen ? data[i++] : 0;
        c = 0;

        idx = (a << 16) + (b << 8) + c;
        obuff[j++] = table[(idx >> 3 * 6) & 0x3F];
        obuff[j++] = table[(idx >> 2 * 6) & 0x3F];
        obuff[j++] = table[(idx >> 6) & 0x3F];
        obuff[j++] = table[idx & 0x3F];

        obuff[osize -1] = '=';

        if ( (ilen % 3) == 1)
            obuff[osize - 2] = '=';
    }

    *olen = osize;

    return obuff;
}