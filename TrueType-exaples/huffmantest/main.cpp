#include <stdio.h> 
#define MAXBIT 10 /*定义哈夫曼编码的最大长度*/ 
#define MAXVALUE 10000 /*定义最大权值*/ 
#define MAXLEAF 30 /*定义哈夫曼树中最多叶子节点个数*/ 
#define MAXNODE MAXLEAF*2-1 /*哈夫曼树最多结点数*/ 
typedef struct { /*哈夫曼编码信息的结构*/ 
    int bit[MAXBIT]; 
    int start;}Hcodetype; 
    typedef struct { /*哈夫曼树结点的结构*/ 
        int weight; 
        int parent; 
        int lchild; 
        int rchild; 
    }Hnodetype; 
    void huffmantree(Hnodetype huffnode[MAXNODE],int n) /*构造哈夫曼树的函数*/ 
    { 
        int i,j,m1,m2,x1,x2; 
        for(i=0;i<2*n-1;i++) /*存放哈夫曼树结点的数组huffnode[]初始化*/ 
        { 
            huffnode[i].weight=0; 
            huffnode[i].parent=-1; 
            huffnode[i].lchild=-1; 
            huffnode[i].rchild=-1; 
        } 
        for(i=0;i<n;i++) /*输入入N个叶子节点的权值*/ 
        { 
            printf("please input %d character's weight\n",i); 
            scanf("%d",&huffnode[i].weight); 
        } 
        for(i=0;i<n-1;i++) /*开始循环构造哈夫曼树*/ 
        { 
            m1=m2=MAXVALUE; 
            x1=x2=0; 
            for(j=0;j<n+i;j++) 
            { 
                if(huffnode[j].weight<m1&&huffnode[j].parent==-1) 
                { 
                    m2=m1;x2=x1;m1=huffnode[j].weight;x1=j; 
                } 
                else if(huffnode[j].weight<m2&&huffnode[j].parent==-1) 
                { 
                    m2=huffnode[j].weight;x2=j; 
                } 
            } 
            huffnode[x1].parent=n+i; 
            huffnode[x2].parent=n+i; 
            huffnode[n+i].weight=huffnode[x1].weight+huffnode[x2].weight; 
            huffnode[n+i].lchild=x1; 
            huffnode[n+i].rchild=x2; 
        } 
    } 
    void main() 
    { 
        Hnodetype huffnode[MAXNODE]; 
        Hcodetype huffcode[MAXLEAF],cd; 
        int i,j,c,p,n; 
        printf("please input n\n"); 
        scanf("%d",&n); /*输入叶子节点个数*/ 
        huffmantree(huffnode,n); /*建立哈夫曼树*/ 
        for(i=0;i<n;i++) /*该循环求每个叶子节点对应字符的哈夫曼编码*/ 
        { 
            cd.start=n-1;c=i; 
            p=huffnode[c].parent; 
            while(p!=-1) 
            { 
                if(huffnode[p].lchild==c) cd.bit[cd.start]=0; 
                else cd.bit[cd.start]=1; 
                cd.start--;c=p; 
                p=huffnode[c].parent; 
            } 
            for(j=cd.start+1;j<n;j++) /*保存求出的每个叶节点的哈夫曼编码和编码的起始位*/ 
                huffcode[i].bit[j]=cd.bit[j]; 
            huffcode[i].start=cd.start; 
        } 
        for(i=0;i<n;i++) /*输出每个叶子节点的哈夫曼编码*/ 
        { 
            printf("%d character is:",i); 
            for(j=huffcode[i].start+1;j<n;j++) 
                printf("%d",huffcode[i].bit[j]); 
            printf("\n"); 
        } 
    }