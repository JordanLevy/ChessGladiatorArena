#include "values.h"
#include "bitwise.h"
#include <ctype.h>
#include <stdlib.h>


unsigned long long r_shift(unsigned long long x, int n){
    if(n <= 0){
        return x << abs(n);
    }
    if(x > 0){
        return x >> n;
    }
    unsigned long long y = x >> 1;
    y &= ~square_a8;
    y = (y >> (n - 1));
    return y;
}

unsigned long long l_shift(unsigned long long x, int n){
    if(n <= 0){
        return r_shift(x, abs(n));
    }
    return x << n;
}

unsigned long long generate_bitboard(int squares[], int num_squares){
    unsigned long long a = 0ULL;
    for(int i = 0; i < num_squares; i++){
        a |= 1ULL << squares[i];
    }
    return a;
}

unsigned long long generate_bitboard_from_range(int a, int b){
    unsigned long long ans = ~(0ULL);
    ans = r_shift(ans, 64 - b + a - 1) << a;
    return ans;
}



unsigned long long reverse(unsigned long long i){
    i = ((i & 0x5555555555555555) << 1) | ((i >> 1) & 0x5555555555555555);
    i = ((i & 0x3333333333333333) << 2) | ((i >> 2) & 0x3333333333333333);
    i = ((i & 0x0f0f0f0f0f0f0f0f) << 4) | ((i >> 4) & 0x0f0f0f0f0f0f0f0f);
    i = ((i & 0x00ff00ff00ff00ff) << 8) | ((i >> 8) & 0x00ff00ff00ff00ff);
    i = (i << 48) | ((i & 0xffff0000) << 16) | ((i >> 16) & 0xffff0000) | (i >> 48);
    return i;
}

int leading_zeros(unsigned long long i){
    if(i == 0){
        return 64;
    }
    if(i < 0){
        return 0;
    }
    int n = 1;
    unsigned long long x = i >> 32;
    if(x == 0){
        n += 32;
        x = i;
    }
    if((x >> 16) == 0){
        n += 16;
        x = x << 16;
    }
    if((x >> 24) == 0){
        n += 8;
        x = x << 8;
    }
    if((x >> 28) == 0){
        n += 4;
        x = x << 4;
    }
    if((x >> 30) == 0){
        n += 2;
        x = x << 2;
    }
    n -= x >> 31;
    return n;
}

void print_bitboard(unsigned long long bitboard){
    for(int i = 63; i >= 0; i--){
        printf(" %d ", (bitboard & (1ULL << i)) ? 1 : 0);
        if(i % 8 == 0){
            printf("\n");
        }
    }
    printf("\n");
}

unsigned long long offset_span(unsigned long long span, int origin, int i){
    if(i > origin){
        return l_shift(span, i - origin);
    }
    return r_shift(span, origin - i);
}

unsigned long long remove_span_warps(unsigned long long span, int i){
    if(i % 8 < 4){
        return span & ~file_ab;
    }
    return span & ~file_gh;
}