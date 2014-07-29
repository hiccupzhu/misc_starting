#include <stdio.h>
#include <memory.h>
#include <malloc.h>
#include <string.h>
#include "put_bits.h"
#include "get_bits.h"
#define SAFE_FREE(x)    if((x)){\
                            free((x));\
                            (x) = NULL;\
                        }
#define array_size(x)   (sizeof((x)) / sizeof((x)[0]))
typedef struct{
    int weigth;
    int parent;
    int lchild;
    int rchild;
}HUFFERTREE,*PHUFFERTREE;

int select(PHUFFERTREE ht,int len,int &s1,int &s2)
{
    s1 = s2 = -1;
    for(int i = 1; i <= len ; i++ ){
        if(ht[i].parent == 0){
            if(s1 == -1 ) s1 = i;
            if(s2 == s1 ) s2 = i;
            if(ht[s1].weigth >= ht[i].weigth){
                s2 = s1;
                s1 = i;
            }else if(ht[s2].weigth >= ht[i].weigth){
                s2 = i;
            }
        }
    }
    return 0;
}

#define SWITCH(x,y) {int t=x;x=y;y=t;}

int huffman_coding(PHUFFERTREE &ht,char** hc,int *w,int n)
{
    int m = 2 * n - 1;
    ht = (PHUFFERTREE)calloc( m + 1, sizeof(HUFFERTREE));
    PHUFFERTREE p = NULL;
    int i = 0;
    for(p = ht + 1, i = 1;i <= n;i++,p++,w++){
        p->weigth = *w;
    }
    int s1 = 0;
    int s2 = 0;
    for(i = n + 1; i <= m; i++){
        select(ht,i - 1,s1,s2);
        if(s1 < 1 || s1 > n)  SWITCH(s1,s2);
        ht[s1].parent = i;  ht[s2].parent = i;
        ht[i].lchild = s1;  ht[i].rchild = s2;
        ht[i].weigth = ht[s1].weigth + ht[s2].weigth;
    }
    
    for(int index = 1; index <= m; index ++){
        printf("ht[%d] parent=%d weight=%d lchild=%d rchild=%d\n",
        index,ht[index].parent,ht[index].weigth,ht[index].lchild,ht[index].rchild);
    }
    
    hc = (char**)calloc(n+1,sizeof(char*));
    char* cd = (char*)calloc(n,sizeof(char));
    
    int f = 0;
    int start = 0;
    for(i = 1;i <= n; i ++){
        
        start = n - 1;
        for(int c = i,f = ht[c].parent; f != 0; f = ht[f].parent){
            if(ht[f].lchild == c)   cd[-- start] = '0';
            else    cd[-- start] = '1';
            c = f;
        }
        
        hc[i] = (char*)calloc(n - start,sizeof(char));
        strcpy(hc[i],&cd[start]);
        printf("%s\n",hc[i]);
    }
    return 0;
}



int main(int argc,char* argv[])
{
//     PHUFFERTREE ht = NULL;
//     char** huffman_code = NULL;
//     int w[] = {12,21,44,2,3,55,-1,8};
//     //int w[] = {1, 2, 3, 4};
//     int n = array_size(w);
//     int res = huffman_coding(ht,huffman_code,w,n);   

    unsigned char tmp[1024] = {0};
    FILE* fp = fopen("test.bin","rb");
    fread(tmp,1024,1,fp);
    fclose(fp);
    
    
    PutBitContext* pbit = (PutBitContext*)calloc(1,sizeof(PutBitContext));
    uint8_t* pdata = (uint8_t*)calloc(1024,1);
    init_put_bits(pbit,pdata,1024);
    put_bits(pbit,5,11);
//    flush_put_bits(pbit);
    put_bits(pbit,29,11);
    put_bits(pbit,5,11);
    put_bits(pbit,5,11);
    put_bits(pbit,5,11);
    put_bits(pbit,5,11);
    put_bits(pbit,5,11);
    put_bits(pbit,5,11);
    
    
    
    SAFE_FREE(pbit);
    return 0;
}