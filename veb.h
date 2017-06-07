#ifndef VEB_H
#define VEB_H
typedef struct Veb Veb;

struct Veb
{
    int k,m,n;
    int *T,*B,*D;
};

Veb vebnew(int);
void vebfree(Veb);
int vebpos(Veb,int);
int vebsop(Veb,int);

#endif

