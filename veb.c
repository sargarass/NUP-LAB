#include <stdlib.h>
#include "veb.h"

#define WORD \
    sizeof(int)*8

#define clz(x) \
    (__builtin_clz(x))

#define fls(x) \
    (WORD-clz(x))

#define pow2(n) \
    (1<<(n))

#define size(n) \
    pow2(n)-1

static void init(Veb L, int h, int d)
{
    if(h <= 1)
        return;
    int b = h/2;
    int t = h-b;
    L.T[d+t-1] = size(t);
    L.B[d+t-1] = size(b);
    L.D[d+t-1] = d;
    init(L,t,d);
    init(L,b,d+t);
}

Veb vebnew(int n)
{
    Veb L;
    L.n = n;
    L.k = fls(n);
    L.m = size(L.k);
    L.T = malloc((L.k-1)*sizeof(int));
    L.B = malloc((L.k-1)*sizeof(int));
    L.D = malloc((L.k-1)*sizeof(int));
    init(L,L.k,0);
    return L;
}

static int pos(Veb L, int v)
{
    int k = fls(v);
    int p[fls(k)];
    int i = 0;
    for(int d = --k; d > 0; d = L.D[d]){
        int w = v>>(k-d);
        --d;
        p[i] = L.T[d]+(w&L.T[d])*L.B[d];
        ++i;
    }
    for(p[i] = 1; i > 0; --i)
        p[i-1] += p[i];
    return p[0];
}

int vebpos(Veb L, int v)
{
    if(v <= 0 || v > L.m)
        return 0;
    int w = pos(L,v);
    if(w > L.n)
        return 0;
    return w;
}

static int sop(Veb L, int w)
{
    int h = L.k;
    int d = 0;
    int v = 1;
    while(h > 1){
        int b = h/2;
        int t = h-b;
        int i = t+d-1;
        if(w > L.T[i]){
            w -= L.T[i];
            int x = (w-1)/L.B[i];
            w -= x*L.B[i];
            v <<= t;
            v += x;
            d += t;
            h = b;
        }else
            h = t;
    }
    return v;
}

int vebsop(Veb L, int w)
{
    if(w <= 0 || w > L.n)
        return 0;
    return sop(L,w);
}
