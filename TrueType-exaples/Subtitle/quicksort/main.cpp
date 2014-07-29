#include <stdio.h>
#include <memory.h>
#include <malloc.h>

#define array_size(x) (sizeof((x)) / sizeof((x)[0]))

int size = 0;

int print(int index[])
{
    for(int i = 0; i < size; i++){
        printf("%4d",index[i]);
    }
    printf("\n");
    return 0;
}

int swap(int& a,int &b){
    int t = a;
    a = b;
    b = t;
    return 0;
}

int partition(int *index,int low,int high)
{
    int prove = index[high];
    int mid = high;
    while(low < high){
        while(low < high && index[low] < prove) low ++;
        swap(index[low],index[mid]);
        mid = low;
        print(index);
        while(low < high && index[high] > prove) high --;
        swap(index[mid],index[high]);
        mid = high;
        print(index);
    }
    return mid;
}

typedef struct {
    int low;
    int high;
    int mid;
}SORT;

typedef struct{
    int entry_count;
    SORT* entries;
}SORT_LIST;

SORT_LIST slist;
void add_entry(SORT e)
{
    slist.entry_count ++;
    slist.entries[slist.entry_count] = e;
    return 0;

}

int quick_sort(int *index,int low,int high)
{
    SORT e = {0};
    while(slist.entry_count > 0){
        e = slist[slist.entry_count --];
        int i = partition(index,e.low,e.high);
        e.low = low;
        e.high = i - 1;
        e.mid = i;
        add_entry(e);

        e.low = i + 1;
        e.high = high;
        e.mid = i;
        add_entry(e);

        quick_sort(index,low,i - 1);
        quick_sort(index,i + 1,high);
    }
    return 0;
}

int main(int argc,char*argv[])
{
    int index[] = {1, 22, -2, 4, 65, 55, 7, 23, 58};
    
    size = array_size(index);
    print(index);

    
    slist.entry_count = 0;
    slist.entries = (SORT*)malloc(1000 * sizeof(SORT));

    SORT e = {0};
    e.low = 0;
    e.high = size - 1;
    e.mid = 0;

    quick_sort(index,e.low,e.high);
    
    free(slist.entries);
    return 0;
}