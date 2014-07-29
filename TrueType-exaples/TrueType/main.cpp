#include <stdio.h>
#include <stdint.h>
#include <malloc.h>
#include <memory.h>
#include <Windows.h>
#include "TrueTye_def.h"
#include "TrueType.h"
#include "Iobuf.h"

#define FILE_NAME "E:\\win-xp\\stxingka.ttf"

typedef struct{
    char   tag[4];
    uint32_t   checkSum;
    uint32_t   offset;
    uint32_t   length;
}TableEntry;

typedef   struct
{
    Fixed   sfntversion;   //0x10000   for   version   1.0
    uint16_t   numTables;
    uint16_t   searchRange;
    uint16_t   entrySelector;
    uint16_t   rangeShift;
    TableEntry   entries[1];//variable   number   of   TableEntry
}TableDirectory;
typedef   sturct
{
    Fixed   Table;//x00010000   ro   version   1.0
    Fixed   fontRevision;//Set   by   font   manufacturer.
    ULONG   checkSumAdjustment;
    ULONG   magicNumer;   //Set   to   0x5f0f3cf5
    USHORT   flags;
    USHORT   unitsPerEm;   //Valid   range   is   from   16   to   16384
    longDT   created;   //International   date   (8-byte   field).
    longDT   modified;   //International   date   (8-byte   field).
    FWord   xMin;   //For   all   glyph   bounding   boxes.
    FWord   yMin;   //For   all   glyph   bounding   boxes.
    FWord   xMax;   //For   all   glyph   bounding   boxes.
    FWord   xMax;   //For   all   glyph   bounding   boxes.
    USHORT   macStyle;
    USHORT   lowestRecPPEM;   //Smallest   readable   size   in   pixels.
    SHORT   fontDirctionHint;
    SHORT   indexToLocFormat;   //0   for   short   offsets   ,1   for   long.
    SHORT   glyphDataFormat;     //0   for   current   format.
}Table_head;


int main(int argc,char* argv[])
{
    CIobuf iobuf;
    FILE* fp = fopen(FILE_NAME,"rb");
    iobuf.init(fp);
    
    TableDirectory dirc;
    dirc.sfntversion = iobuf.get_be32();   //0x10000   for   version   1.0
    dirc.numTables = iobuf.get_be16();
    dirc.searchRange = iobuf.get_be16();
    dirc.entrySelector = iobuf.get_be16();
    dirc.rangeShift = iobuf.get_be16();
    
    TableEntry tabel_e;
    for(int i=0;i < array_size(tabel_e.tag);i++){
        tabel_e.tag[i] = iobuf.get_byte();
    }
    tabel_e.checkSum = iobuf.get_be32();
    tabel_e.offset = iobuf.get_be32();
    tabel_e.length = iobuf.get_be32();
    
    
    
    fclose(fp);
    return 0;
}