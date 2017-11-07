#define DEBUG_ADD       0
#define DEBUG_BITMAP    0
#define DEBUG_DATE      0
#define DEBUG_DIR       0
#define DEBUG_FILE      0 // File Entry
#define DEBUG_FIND_FILE 0
#define DEBUG_FREE      0 // Free Blocks
#define DEBUG_INIT      0
#define DEBUG_META      0 // Bitmap Pages
#define DEBUG_PATH      0
#define DEBUG_TIME      0
#define DEBUG_VOLUME    0
/*
    prodos_*    Private functions
    ProDOS_*    Public  functions
*/



// --- ProDOS crap ---

    struct SubDirInfo_t
    {
        uint8_t res75  ; // $75 - magic number
        uint8_t pad7[7];
    };

    struct VolumeExtra_t
    {
        uint16_t bitmap_block;
        uint16_t total_blocks;
    };

    struct SubDirExtra_t
    {
        uint16_t parent_block;
        uint8_t  parent_entry_num;
        uint8_t  parent_entry_len;
    };

    // ============================================================

    // Due to packing/aignment this may NOT be size identical
    // But it is in the correct order
    // See: ProDOS_GetVolumeHeader;
    struct ProDOS_VolumeHeader_t
    {                           ; //Rel Size Abs
        uint8_t  kind           ; // +0    1 $04  \ Hi nibble  Storage Type
        uint8_t  len            ; // +0           / Lo nibble
        char     name[ 16 ]     ; // +1   15 $05  15 on disk but we NULL terminate for convenience
// --- diff from file ---
        union
        {
            uint8_t  pad8[  8 ] ; //+16    8 $14 only in root
            SubDirInfo_t  info  ; //             only in sub-dir
        }                       ;
// --- same as file ---
        uint16_t date           ; //+24    2 $1C
        uint16_t time           ; //+26    2 $1E
        uint8_t  cur_ver        ; //+28    1 $20
        uint8_t  min_ver        ; //+29    1 $21
        uint8_t  access         ; //+30    1 $22
        uint8_t  entry_len      ; //+31    1 $23 Size of each directory entry in bytes
        uint8_t  entry_num      ; //+32    1 $24 Number of directory entries per block
        uint16_t file_count     ; //+33    2 $25 Active entries including directories, excludes volume header
        union                     //          --
        {                         //          --
            VolumeExtra_t meta  ; //+34    2 $27
            SubDirExtra_t subdir; //+36    2 $29
        }                       ; //============
    }                           ; // 38      $2B

    void prodos_GetVolumeHeader( ProDOS_VolumeHeader_t *meta_, int block )
    {
        int base = block*PRODOS_BLOCK_SIZE + 4; // skip prev/next dir block double linked list
        ProDOS_VolumeHeader_t info;

        info.kind       = (gaDsk  [ base +  0 ] >> 4) & 0xF;
        info.len        = (gaDsk  [ base +  0 ] >> 0) & 0xF;

    for( int i = 0; i < 16; i++ )
        info.name[ i ] = 0;

    for( int i = 0; i < info.len; i++ )
        info.name[ i ]  = gaDsk   [ base+i+ 1 ];

    for( int i = 0; i < 8; i++ )
        info.pad8[ i ]  = gaDsk   [ base+i+16 ];

        info.date       = DskGet16( base + 24 );
        info.time       = DskGet16( base + 26 );
        info.cur_ver    = gaDsk   [ base + 28 ];
        info.min_ver    = gaDsk   [ base + 29 ];
        info.access     = gaDsk   [ base + 30 ];
        info.entry_len  = gaDsk   [ base + 31 ]; // @ 0x423
        info.entry_num  = gaDsk   [ base + 32 ];
        info.file_count = DskGet16( base + 33 );

        info.meta.bitmap_block = DskGet16( base + 35 );
        info.meta.total_blocks = DskGet16( base + 37 );

#if DEBUG_VOLUME
    printf( "DEBUG: VOLUME: name: %s\n",   info.name              );
    printf( "DEBUG: VOLUME: date: %04X\n", info.date              );
    printf( "DEBUG: VOLUME: time: %04X\n", info.time              );
    printf( "DEBUG: VOLUME: cVer: %02X\n", info.cur_ver           );
    printf( "DEBUG: VOLUME: mVer: %02X\n", info.min_ver           );
    printf( "DEBUG: VOLUME:acces: %02X\n", info.access            );
    printf( "DEBUG: VOLUME:e.len: %02X\n", info.entry_len         );
    printf( "DEBUG: VOLUME:e.num: %02X\n", info.entry_num         );
    printf( "DEBUG: VOLUME:files: %04X  ", info.file_count        );
    printf( "(%d)\n", info.file_count );

if( block == PRODOS_ROOT_BLOCK )
{
    printf( "DEBUG: ROOT: bitmap: %04X\n", info.meta.bitmap_block );
    printf( "DEBUG: ROOT: blocks: %04X\n", info.meta.total_blocks );
} else {
    printf( "DEBUG: SUBDIR: parent: %04X\n", info.subdir.parent_block     );
    printf( "DEBUG: SUBDIR: pa.len: %02X\n", info.subdir.parent_entry_num );
    printf( "DEBUG: SUBDIR: pa.num: %02X\n", info.subdir.parent_entry_len );
}
#endif

        if(  meta_ )
            *meta_ = info;
    };


    void prodos_SetVolumeHeader( ProDOS_VolumeHeader_t *volume, int block )
    {
        if( !volume )
            return;

        int base = block*PRODOS_BLOCK_SIZE + 4; // skip prev/next dir block double linked list

        uint8_t KindLen = 0
            | ((volume->kind & 0xF) << 4)
            | ((volume->len  & 0xF) << 0)
            ;

        gaDsk[ base + 0 ] = KindLen;

    for( int i = 0; i < volume->len; i++ )
        gaDsk   [ base+ 1+i ] = volume->name[ i ];

    for( int i = 0; i < 8; i++ )
        gaDsk   [ base+16+i ] = volume->pad8[ i ];

        DskPut16( base + 24 ,   volume->date              );
        DskPut16( base + 26 ,   volume->time              );
        gaDsk   [ base + 28 ] = volume->cur_ver  ;
        gaDsk   [ base + 29 ] = volume->min_ver  ;
        gaDsk   [ base + 30 ] = volume->access   ;
        gaDsk   [ base + 31 ] = volume->entry_len;
        gaDsk   [ base + 32 ] = volume->entry_num; 
        DskPut16( base + 33 ,   volume->file_count        );
        DskPut16( base + 35 ,   volume->meta.bitmap_block );
        DskPut16( base + 37 ,   volume->meta.total_blocks );
    }

    // ============================================================
    struct ProDOS_FileHeader_t
    {                           ; //Rel Size Hex
        uint8_t  kind           ; // +0    1 $00  \ Hi nibble Storage Type
        uint8_t  len            ; // +0           / Lo nibble
        char     name[ 16 ]     ; // +1   15 $05  15 on disk but we NULL terminate for convenience
// --- diff from volume ---
        uint8_t  type           ; //+16    1 $10              User Type
        uint16_t inode          ; //+17    2 $11
        uint16_t blocks         ; //+19    2 $13
        uint32_t size           ; //+21    3 $15 EOF address - on disk is 3 bytes, but 32-bit for convenience
// --- same as volume ---
        uint16_t date           ; //+24    2 $18
        uint16_t time           ; //+26    2 $1A
        uint8_t  cur_ver        ; //+28    1 $1C
        uint8_t  min_ver        ; //+29    1 $1D // 0 = ProDOS 1.0
        uint8_t  access         ; //+30    1 $1E
// --- diff from subdir
        uint16_t aux            ; //+31    2 $1F Load Address for Binary
// --- diff from volume ---
        uint16_t mod_date       ; //+33    2 $21
        uint16_t mod_time       ; //+35    2 $23
        uint16_t dir_block      ; //+37    2 $25 pointer to directory block
                                ; //============
    };                          ; // 39      $27

    // ------------------------------------------------------------
    void prodos_GetFileHeader( int offset, ProDOS_FileHeader_t *file_ )
    {
        ProDOS_FileHeader_t info;

        info.kind       = (gaDsk  [ offset + 0  ] >> 4) & 0xF;
        info.len        = (gaDsk  [ offset + 0  ] >> 0) & 0xF;

    for( int i = 0; i < 16; i++ )
        info.name[ i ] = 0;

    for( int i = 0; i < info.len; i++ )
        info.name[ i ]  = gaDsk   [ offset + 1+i];

        info.type       = gaDsk   [ offset + 16 ];
        info.inode      = DskGet16( offset + 17 );
        info.blocks     = DskGet16( offset + 19 );
        info.size       = DskGet24( offset + 21 );
        info.date       = DskGet16( offset + 24 );
        info.time       = DskGet16( offset + 26 );
        info.cur_ver    = gaDsk   [ offset + 28 ];
        info.min_ver    = gaDsk   [ offset + 29 ];
        info.access     = gaDsk   [ offset + 30 ];
        info.aux        = DskGet16( offset + 31 );
        info.mod_date   = DskGet16( offset + 33 );
        info.mod_time   = DskGet16( offset + 35 );
        info.dir_block  = DskGet16( offset + 37 );

#if DEBUG_FILE
    printf( "DEBUG: FILE: name: %s\n"   , info.name      );
    printf( "DEBUG: FILE: kind: $%02X\n", info.kind      );
    printf( "DEBUG: FILE: type: $%02X\n", info.type      );
    printf( "DEBUG: FILE:inode: $%04X\n", info.inode     );
    printf( "DEBUG: FILE:block: $%04X\n", info.blocks    );
    printf( "DEBUG: FILE: size: $%06X\n", info.size      );
    printf( "DEBUG: FILE: date: $%02X\n", info.date      );
    printf( "DEBUG: FILE: time: $%02X\n", info.time      );
    printf( "DEBUG: FILE: cVer: $%02X\n", info.cur_ver   );
    printf( "DEBUG: FILE: mVer: $%02X\n", info.min_ver   );
    printf( "DEBUG: FILE:  aux: $%04X\n", info.aux       );
    printf( "DEBUG: FILE:mDate: $%02X\n", info.mod_date  );
    printf( "DEBUG: FILE:mTime: $%02X\n", info.mod_time  );
    printf( "DEBUG: FILE:dBloc: $%04X\n", info.dir_block );
    printf( "-----\n" );
#endif

        if(  file_ )
            *file_ = info;
    }

    void prodos_PutFileHeader( int offset, ProDOS_FileHeader_t *file )
    {
        int base = offset;

        uint8_t KindLen = 0
            | ((file->kind & 0xF) << 4)
            | ((file->len  & 0xF) << 0)
            ;

        gaDsk   [ base + 0 ] = KindLen;

    for( int i = 0; i < file->len; i++ )
        gaDsk   [ base + 1+i] = file->name[ i ];

        gaDsk   [ base + 16 ] = file->type;
        DskPut16( base + 17 ,   file->inode             );
        DskPut16( base + 19 ,   file->blocks            );
        DskPut24( base + 21 ,   file->size              );

        DskPut16( base + 24 ,   file->date              );
        DskPut16( base + 26 ,   file->time              );
        gaDsk   [ base + 28 ] = file->cur_ver  ;
        gaDsk   [ base + 29 ] = file->min_ver  ;
        gaDsk   [ base + 30 ] = file->access   ;

        gaDsk   [ base + 31 ] = file->aux      ;
        DskPut16( base + 33 ,   file->mod_date          );
        DskPut16( base + 35 ,   file->mod_time          );
        DskPut16( base + 37 ,   file->dir_block         );
    }

    void prodos_InitFileHeader( ProDOS_FileHeader_t *entry )
    {
        memset( entry, 0, sizeof( ProDOS_FileHeader_t ) );
    }

// Globals ________________________________________________________________

    int  gnLastDirBlock = 0;
    char gpLastDirName[ 16 ];

    int  gnLastDirMaxFiles = 0;

    ProDOS_VolumeHeader_t gVolume;
    ProDOS_FileHeader_t   gEntry; // Default fields for cp

// @return total blocks allocated for directory
// ------------------------------------------------------------------------
int prodos_BlockGetDirectoryCount( int offset )
{
    int blocks     = 0;
    int next_block = 0;

    do
    {
        blocks++;
        next_block = DskGet16( offset + 2 );
        offset     = next_block * PRODOS_BLOCK_SIZE;
    } while( next_block );

    return blocks;
}


// @returns 0 if no free block
// ------------------------------------------------------------------------
int prodos_BlockGetFirstFree( ProDOS_VolumeHeader_t *volume )
{
    if( !volume )
    {
        printf( "ERROR: Volume never read\n" );
        return 0;
    }

    int bitmap = volume->meta.bitmap_block;
    int offset = bitmap * PRODOS_BLOCK_SIZE;
    int size   = (gnDskSize + 7) / 8;
    int block  = 0;

#if DEBUG_FREE
    printf( "DskSize: %06x\n", gnDskSize );
    printf( "Bitmap @ %04X\n", bitmap    );
    printf( "Offset : %06X\n", offset    );
    printf( "Bytes  : %06X\n", size      );
#endif

    for( int byte = 0; byte < size; byte++ )
    {
        int mask = 0x80;
        do
        {
            if( gaDsk[ offset + byte ] & mask )
                return block;

            mask >>= 1;
            block++;
        }
        while( mask );
    }

#if DEBUG_FREE
    printf( "ZERO free block\n" );
#endif

    return 0;
}


// @return number of total disk blocks
// ------------------------------------------------------------------------
int prodos_BlockInitFree( ProDOS_VolumeHeader_t *volume )
{
    int bitmap = volume->meta.bitmap_block;
    int offset = bitmap * PRODOS_BLOCK_SIZE;
    int blocks = (gnDskSize + PRODOS_BLOCK_SIZE - 1) / PRODOS_BLOCK_SIZE;
    int size   = (blocks + 7) / 8;

#if DEBUG_INIT
    printf( "offset: %06X\n", offset );
    printf( "Total  Blocks: %d\n", blocks );
    printf( "Bitmap Blocks: %d\n", (size + PRODOS_BLOCK_SIZE - 1) / PRODOS_BLOCK_SIZE );
#endif

    memset( &gaDsk[ offset ], 0xFF, size );

#if DEBUG_INIT
    printf( "Bitmap after init\n" );
    DskDump( offset, offset + 64 );
    printf( "\n" );
#endif

    volume->meta.total_blocks = blocks;
    return (size + PRODOS_BLOCK_SIZE - 1) / PRODOS_BLOCK_SIZE;
}


// ------------------------------------------------------------------------
bool prodos_BlockSetFree( ProDOS_VolumeHeader_t *volume, int block )
{
    if( !volume )
    {
        printf( "ERROR: Volume never read\n" );
        return false;
    }

    int bitmap = volume->meta.bitmap_block;
    int offset = bitmap * PRODOS_BLOCK_SIZE;

    int byte = block / 8;
    int bits = block % 8;
    int mask = 0x80 >> bits;

    gaDsk[ offset + byte ] |= mask;

    return true;
}

// ------------------------------------------------------------------------
bool prodos_BlockSetUsed( ProDOS_VolumeHeader_t *volume, int block )
{
    if( !volume )
    {
        printf( "ERROR: Volume never read\n" );
        return false;
    }

    int bitmap = volume->meta.bitmap_block;
    int offset = bitmap * PRODOS_BLOCK_SIZE;

    int byte = block / 8;
    int bits = block % 8;
    int mask = 0x80 >> bits;

    gaDsk[ offset + byte ] |= mask;
    gaDsk[ offset + byte ] ^= mask;

    return true;
}


// @return 0 if couldn't find a free directory entry, else offset
// ------------------------------------------------------------------------
int prodos_DirGetFirstFree( ProDOS_VolumeHeader_t *volume, int base )
{
    int next_block;
    int prev_block;

    int offset = base + 4;

    // Try to find the file in this directory
    do
    {
        prev_block = DskGet16( base + 0 );
        next_block = DskGet16( base + 2 );

        for( int iFile = 0; iFile < volume->entry_num; iFile++ )
        {
            ProDOS_FileHeader_t file;
            prodos_GetFileHeader( offset, &file );

#if DEBUG_DIR
    printf( "DEBUG: DIR: " );
    printf( "#%2d %s\n", iFile, file.name );
    printf( "... Type: $%1X, %s\n", file.kind, prodos_KindToString( file.kind ) );
#endif

            if( file.kind == ProDOS_KIND_DEL)
                return offset;

            if( file.len == 0 )
                return offset;

            offset += volume->entry_len;
        }

        base   = next_block * PRODOS_BLOCK_SIZE;
        offset = base + 4;

    } while( next_block );

    return 0;
}


// @return 0 if couldn't find file, else offset
// Sets:
//    gtLastDirFile
//    gnLastDirBlock
//    gnLastDirMaxFiles
// ------------------------------------------------------------------------
int prodos_FindFile( ProDOS_VolumeHeader_t *volume, const char *path, int base = PRODOS_ROOT_OFFSET )
{
    if( !path )
        return base;

    size_t nPathLen = strlen( path );
    if( nPathLen == 0 )
        return base;

#if DEBUG_FIND_FILE
    printf( "DEBUG: PATH: %s\n", path );
    printf( "DEBUG: .len: %d\n", nPathLen );
#endif

    if( path[0] == '/' )
        path++;

    // Get path head
    int  iDirName = nPathLen;
    char sDirName[16];
    int  i;

    for( i = 0; i < nPathLen; i++ )
        if( path[i] == '/' )
        {
            iDirName = i - 1;
            break;
        }

    memcpy( sDirName, path, iDirName );
    sDirName[ iDirName ] = 0;

    strcpy( gpLastDirName, sDirName );

#if DEBUG_FIND_FILE
    printf( "DEBUG: FIND: [%d]\n", iDirName );
    printf( "DEBUG: SUBD: %s\n"  , sDirName );
    printf( "DEBUG: base: %04X\n", base );
#endif

    int next_block;
    int prev_block;

    int offset  = base + 4; // skip prev,next block pointers

    // Try to find the file in this directory
    do
    {
        prev_block = DskGet16( base + 0 );
        next_block = DskGet16( base + 2 );

        for( int iFile = 0; iFile < volume->entry_num; iFile++ )
        {
            ProDOS_FileHeader_t file;
            prodos_GetFileHeader( offset, &file );

#if DEBUG_FIND_FILE
    printf( "... %s\n", file.name );
    printf( "... Type: $%1X, %s\n", file.kind, prodos_KindToString( file.kind ) );
#endif

            if((file.len     == 0)
            || (file.name[0] == 0)
            || (file.kind    == ProDOS_KIND_DEL)
            || (file.kind    == ProDOS_KIND_ROOT)
            )
                goto next_file;

/*
            if( file.kind != ProDOS_KIND_DIR )
                goto next_file;
*/

            if( strcmp( file.name, sDirName ) == 0 )
            {
                const char *next = path + iDirName;
                int         addr = file.inode * PRODOS_BLOCK_SIZE;

                gnLastDirBlock    = file.inode;
                gnLastDirMaxFiles = file.size / volume->entry_num;

#if DEBUG_FIND_FILE
    printf( "***\n" );
    printf( "  DIR  -> %04X\n", addr );
    printf( "  path -> %s\n"  , next );
#endif

                return prodos_FindFile( volume, next, addr );
            }

next_file:
            offset += volume->entry_len;
            ;
        }

        base   = next_block * PRODOS_BLOCK_SIZE;
        offset = base + 4;

    } while( next_block );

#if DEBUG_FIND_FILE
    printf( "*** File Not Found\n" );
#endif

    return 0;
}


// ------------------------------------------------------------------------
int prodos_BlockGetPath( const char *path )
{
    int offset = PRODOS_ROOT_OFFSET; // Block 2 * 0x200 Bytes/Block = 0x400 abs offset

    // Scan Directory ...
    strcpy( gpLastDirName, gVolume.name );

    gnLastDirBlock    = offset;
    int nDirBlocks    = prodos_BlockGetDirectoryCount( offset );
    gnLastDirMaxFiles = nDirBlocks * gVolume.entry_num;

#if DEBUG_PATH
    printf( "Directory Entry Bytes  : $%04X\n", gVolume.entry_len );
    printf( "Directory Entries/Block: %d\n"   , gVolume.entry_num );
    printf( "VOLUME Directory Blocks: %d\n"   , nDirBlocks );
    printf( "...Alt. max dir files  : %d\n"   ,(nDirBlocks * PRODOS_BLOCK_SIZE) / volume.entry_len ); // 2nd way to verify
#endif

    if( path == NULL )
        return offset;

    if( strcmp( path, "/" ) == 0 )
        return offset;

    return prodos_FindFile( &gVolume, path, offset );
}


// @returns Total Free Blocks
// ------------------------------------------------------------------------
int prodos_BlockGetFreeTotal( ProDOS_VolumeHeader_t *volume )
{
    int blocks   = volume->meta.total_blocks;
    int ALIGN    = 8 * PRODOS_BLOCK_SIZE;
    int pages    = (blocks + ALIGN) / ALIGN;
    int inode    = volume->meta.bitmap_block;
    int base     = inode * PRODOS_BLOCK_SIZE;

    int free = 0;
    int byte;
    int bits;

#if DEBUG_BITMAP
    printf( "DEBUG: BITMAP: Blocks: %d\n"   , blocks );
    printf( "DEBUG: BITMAP: iNode : @%04X\n", inode  );
    printf( "DEBUG: BITMAP: Pages : %d\n"   , pages  );
    printf( "DEBUG: BITMAP: offset: $%06X\n", base   );
#endif

    while( pages --> 0 )
    {
        for( byte = 0; byte < PRODOS_BLOCK_SIZE; byte++ )
        {
            uint8_t bitmap = gaDsk[ base + byte ];

#if DEBUG_META
if( bitmap )
    printf( "free @ base: %8X, offset: %04X\n", base, byte );
#endif

            for( bits = 0; bits < 8; bits++, bitmap >>= 1 )
                if( bitmap & 1 )
                    free++;
        }

        base += PRODOS_BLOCK_SIZE;
    }

    return free;
}


// ------------------------------------------------------------------------
void prodos_Summary( ProDOS_VolumeHeader_t *volume, int files, int iFirstFree )
{
    if( !volume )
        return;

    int sum  = volume->meta.total_blocks;
    int free = prodos_BlockGetFreeTotal( volume );
    int used = sum - free;

    int max  = gnLastDirMaxFiles;
    if( !max )
        max = 1;

    if( !sum )
        sum = 1;

    // Max files in (65,535-2) blocks * 13 files/block = 851,929 directory entries

    printf( "========\n" );
    printf( "Files:  %7s / %s", itoa_comma( files ), itoa_comma( gnLastDirMaxFiles ) );
    printf( " (%5.2f%%)\n"    , (100. * files) / max );
    printf( "Blocks: \n" );
    printf( "   Free: %6s (%5.2f%%)"  , itoa_comma( free ), (100. * free) / sum );
    if( iFirstFree )
                           printf( ", 1st: @ $%04X = %s", iFirstFree, itoa_comma( iFirstFree ) );
    printf( "\n" );
    printf( "   Used: %6s (%5.2f%%)\n", itoa_comma( used ), (100. * used) / sum );
    printf( "  Total: %6s\n", itoa_comma( sum  ) );
}


// ========================================================================
bool ProDOS_GetVolumeName( char *pVolume_ )
{
    bool bValid = false;

    int vol_length = gaDsk[ PRODOS_ROOT_OFFSET + 4 ] & 0xF;

    if( vol_length < 16 )
    {
        for( int i = 0; i < vol_length; i++ )
            *pVolume_++ = gaDsk[ PRODOS_ROOT_OFFSET + 5 + i ];
    }

    *pVolume_ = 0;

    return bValid;
}


/*
    Root
                             <-- print blank line
        /VOLUME ... <HEADER> ...
        \_____/
         name

    Sub-dir

        /VOLUME/dir1/subdir1  <-- printed
         subdir1 ... <HEADER> ...
         \_____/
          name
*/
// ========================================================================
void prodos_PathFullyQualified( ProDOS_VolumeHeader_t *volume, const char *path, int base, char *name_ )
{
    // We have a sub-directory
    if( base != PRODOS_ROOT_BLOCK*PRODOS_BLOCK_SIZE )
    {
        printf( "/%s", volume->name );
        printf( "/%s\n", path[0] == '/' ? path+1 : path );

        strcpy( name_, " Name" );
    }
    else
    {
        printf( "\n" );
        sprintf( name_, "/%s", volume->name );
    }
}


// ========================================================================
bool ProDOS_CatalogLong( const char *path = NULL )
{
    int files      = 0;
    int base       = prodos_BlockGetPath( path );
    int offset     = base + 4; // skip prev,next block pointers

#if DEBUG_DIR
    printf( "DEBUG: CATALOG: Dir @ %04X\n", base );
#endif

    if( !base )
    {
        if( path )
            printf( "ERROR: Couldn't find directory: %s\n", path );
        return false;
    }

    int prev_block = 0;
    int next_block = 0;

    char dirname[ 15 + 3 + 1 ];
    prodos_PathFullyQualified( &gVolume, path, base, dirname );

    printf( "Acc dnb??iwr %-16s"
                                         " Blocks Size    Type    Aux   Kind  iNode Dir   Ver Min  Create    Time    Modified  Time  \n", dirname );
    printf( "--- -------- ---------------- ------ ------- ------- ----- ----- ----- ----- --- ---  --------- ------  --------- ------\n" );
//  printf( "dnb00011 0123456789ABCDEF  65536 $123456 BIN $FF $0000 sap F @0000 @0000 v00 v00  11-JAN-01  1:23a  11-JAN-01 12:34p\n" );

    do
    {
        prev_block = DskGet16( base + 0 );
        next_block = DskGet16( base + 2 );

        for( int iFile = 0; iFile < gVolume.entry_num; iFile++ )
        {
            ProDOS_VolumeHeader_t dir;
            ProDOS_FileHeader_t   file;
            prodos_GetFileHeader( offset, &file );


            if( (file.len == 0) || (file.name[0] == 0) )
                goto next_file;

            if( file.kind == ProDOS_KIND_ROOT ) // Skip root Volume name entry
                goto next_file;

            if( file.kind == ProDOS_KIND_SUB )
                prodos_GetVolumeHeader( &dir, base/PRODOS_BLOCK_SIZE );
            else
                files++;

            char sAccess[16];
            prodos_AccessToString( file.access, &sAccess[0] );

            char sCreateDate[ 16 ];
            char sCreateTime[ 16 ];
            prodos_DateToString( file.date, &sCreateDate[0] );
            prodos_TimeToString( file.time, &sCreateTime[0] );

            char sAccessDate[ 16 ];
            char sAccessTime[ 16 ];
            prodos_DateToString( file.mod_date, &sAccessDate[0] );
            prodos_TimeToString( file.mod_time, &sAccessTime[0] );

            printf( "$%02X " , file.access   );
            printf( "%s "    , sAccess       );

            if( file.kind == ProDOS_KIND_DIR )
                printf( "/" );
            else
            {
                if( !(file.access & ACCESS_W) )
                    printf( "*" ); // File Locked -- can't write to it
                else
                    printf( " " );
            }

            printf( "%-15s "   , file.name      );

            // Sub-dir header entry doesn't have these fields
            // type
            // inode
            // blocks
            // size
            // aux
        if( file.kind == ProDOS_KIND_SUB ) {
            printf( "------ "  );
            printf( "$------ " );
            printf( "--- "     );
            printf( "$-- "     );
            printf( "$---- "   );
        } else {
            printf( "%6d "     , file.blocks    ); // * 512 bytes/block = 33,554,432 bytes max ProDOS volume
            printf( "$%06X "   , file.size      );
            printf( "%s "      , gaProDOS_FileTypes[ file.type ] );
            printf( "$%02X "   , file.type      );
            printf( "$%04X "   , file.aux       );
        }

            printf( "%s "      , gaProDOS_KindDescription[ file.kind ] );
            printf( "%1X "     , file.kind      );

        if( file.kind == ProDOS_KIND_SUB )
        {
            printf( "*%04X ", dir.subdir.parent_block );
            printf( "@---- " );
        } else {
            printf( "@%04X "   , file.inode     ); // FFFF = 65536
            printf( "@%04X "   , file.dir_block );
        }
            printf( "%1d.%1d " , file.cur_ver/16, file.cur_ver & 15 );
            printf( "v%02X  "  , file.min_ver   );

            printf( "%s "      , sCreateDate    ); // Created
            printf( "%s  "     , sCreateTime    );
            printf( "%s "      , sAccessDate    ); // Modified
            printf( "%s "      , sAccessTime    );
            printf( "\n" );

        if( file.kind == ProDOS_KIND_SUB )
        {
        }

next_file:
            offset += gVolume.entry_len;
        }

        base   = next_block * PRODOS_BLOCK_SIZE;
        offset = base + 4; // skip prev,next block pointers

    } while( next_block );

    int iFirstFree = prodos_BlockGetFirstFree( &gVolume );
    prodos_Summary( &gVolume, files, iFirstFree );

    return true;
}


// ========================================================================
bool ProDOS_CatalogNames( const char *path = NULL )
{
    int files      = 0;
    int base       = prodos_BlockGetPath( path );
    int offset     = base + 4; // skip prev,next block pointers

    int prev_block = 0;
    int next_block = 0;

    if( !base )
    {
        if( path )
            printf( "ERROR: Couldn't find directory: %s\n", path );
        return false;
    }

    do
    {
        prev_block = DskGet16( base + 0 );
        next_block = DskGet16( base + 2 );

        for( int iFile = 0; iFile < gVolume.entry_num; iFile++ )
        {
            ProDOS_FileHeader_t file;
            prodos_GetFileHeader( offset, &file );

            if( (file.len == 0) || (file.name[0] == 0) )
                goto next_file;

            if( file.kind == ProDOS_KIND_ROOT )
                goto next_file;

            // TODO: Skip sub-dir kind: $D or $E ?

            printf( "%s", file.name );

            if( file.kind == ProDOS_KIND_DIR ) // skip root volume name
                printf( "/" );

            printf( "\n" );
next_file:
            offset += gVolume.entry_len;
        }

        base   = next_block * PRODOS_BLOCK_SIZE;
        offset = base + 4; // skip prev,next block pointers

    } while( next_block );

    return true;
}


// ========================================================================
bool ProDOS_CatalogShort( const char *path = NULL )
{
#if DEBUG_DIR
    printf( "ProDOS_CatalogShort()\n" );

    int addr;

printf( "==========\n" );
    addr = 2*PRODOS_BLOCK_SIZE + 5;
    printf( "$%04X: ", addr );

    for( int i = 0; i < 16; i++ )
        printf( "%c", gaDsk[ addr + i ] ); // Block 2a
    printf( "\n" );

    addr = 5*PRODOS_BLOCK_SIZE/2 + 17;
    printf( "$%04X: ", addr );

    for( int i = 0; i < 16; i++ )
        printf( "%c", gaDsk[ addr + i ] ); // Block 2b
    printf( "\n" );

    addr = 3*PRODOS_BLOCK_SIZE + 1;
    printf( "$%04X: ", addr );

    for( int i = 0; i < 16; i++ )
        printf( "%c", gaDsk[ addr + i ] ); // Block 3a
    printf( "\n" );
printf( "==========\n" );
#endif

    int files      = 0;
    int base       = prodos_BlockGetPath( path );
    int offset     = base + 4; // skip prev,next block pointers

    int prev_block = 0;
    int next_block = 0;

    if( !base )
    {
        if( path )
            printf( "ERROR: Couldn't find directory: %s\n", path );
        return false;
    }

    char dirname[ 15 + 3 + 1 ];
    prodos_PathFullyQualified( &gVolume, path, base, dirname );

    printf( "%-16s"
                            " Blocks Type Modified  Time  \n", dirname );
    printf( "---------------- ------ ---- --------- ------\n" );
//  printf( "0123456789ABCDEF  65536 BIN  11-JAN-01 12:34p\n" );

    do
    {
        prev_block = DskGet16( base + 0 );
        next_block = DskGet16( base + 2 );

        for( int iFile = 0; iFile < gVolume.entry_num; iFile++ )
        {
            ProDOS_FileHeader_t file;
            prodos_GetFileHeader( offset, &file );

            if( (file.len == 0) || (file.name[0] == 0) )
                goto next_file;

            if( file.kind == ProDOS_KIND_ROOT )
                goto next_file;

            files++;

            char sAccessDate[ 16 ];
            char sAccessTime[ 16 ];
            prodos_DateToString( file.mod_date, &sAccessDate[0] );
            prodos_TimeToString( file.mod_time, &sAccessTime[0] );

            if( file.kind == ProDOS_KIND_DIR ) // skip root volume name
                printf( "/" );
            else
            {
                if( !(file.access & ACCESS_W) )
                    printf( "*" ); // File Locked -- can't write to it
                else
                    printf( " " );
            }

            printf( "%-15s "   , file.name                       );
            printf( "%6d "     , file.blocks                     ); // * 512 bytes/block = 33,554,432 bytes max ProDOS volume
            printf( "%s  "     , gaProDOS_FileTypes[ file.type ] );
            printf( "%s "      , sAccessDate                     ); // Modified
            printf( "%s "      , sAccessTime                     );
            printf( "\n" );

next_file:
            offset += gVolume.entry_len;
        }

        base   = next_block * PRODOS_BLOCK_SIZE;
        offset = base + 4; // skip prev,next block pointers

    } while( next_block );

    prodos_Summary( &gVolume, files, 0 );

    return true;
}


// dst <- src
// ========================================================================
bool ProDOS_FileAdd( const char *to_path, const char *from_filename, ProDOS_FileHeader_t *attribs )
{
#if DEBUG_ADD
    printf( "Adding:\n" );
    printf( "   ... %s"      , from_filename );
    printf( "   --> %s\n", to_path       );
#endif

    int base = prodos_BlockGetPath( to_path );
    if( !base )
    {
        printf( "ERROR: Couldn't find destination directory: %s\n", to_path );
        return false;
    }

#if DEBUG_ADD
    printf( "DEBUG: ADD: Path @ %06X\n", base );
#endif


//  int nDirBlocks       = prodos_BlockGetDirectoryCount();
//  int nDirEntriesTotal = 0;
//  int nDirEntriesFree  = 0;

    // Do we have room to add it to this directory?
    int iDirBlock = base / PRODOS_BLOCK_SIZE;

    ProDOS_VolumeHeader_t dir;
    prodos_GetVolumeHeader( &dir, iDirBlock );

    if( dir.file_count + 1 >= gnLastDirMaxFiles )
    {
        printf( "ERROR: Not enough room in directory: %d files\n", dir.file_count );
        return false;
    }

    int     iKind    = ProDOS_KIND_DEL;
    FILE   *pSrcFile = fopen( from_filename, "rb" );

    if( !pSrcFile )
    {
        printf( "ERROR: Couldn't open source file: %s\n", from_filename );
        return false;
    }


    // Block Management

    size_t nSrcSize     = File_Size( pSrcFile );
    int    nBlocksData  = (nSrcSize + PRODOS_BLOCK_SIZE - 1) / PRODOS_BLOCK_SIZE;
    int    nBlocksIndex = 0; // Num Blocks needed for meta (Index)
    int    nBlocksTotal = 0;
    int    nBlocksFree  = 0;

    int    iNode        = 0; // Master Index, Single Index, or 0 if none
    int    iIndexBase   = 0; // Single Index
    int    iMasterIndex = 0; // master block points to N IndexBlocks


    if( nSrcSize > gnDskSize )
    {
        printf( "ERROR: Source file (%s bytes) doesn't fit on volume (%s bytes).\n", itoa_comma( nSrcSize ), itoa_comma( gnDskSize ) );
        return false;
    }


    uint8_t *buffer = new uint8_t[ nSrcSize + PRODOS_BLOCK_SIZE ];
    memset( buffer, 0, nSrcSize + PRODOS_BLOCK_SIZE );

    size_t   actual = fread( buffer, 1, nSrcSize, pSrcFile );
    fclose( pSrcFile );

    // Find Free Index Blocks
    if( nSrcSize <=   1*PRODOS_BLOCK_SIZE ) // <= 512, 1 Blocks
        iKind = ProDOS_KIND_SEED;
    else
    if( nSrcSize > 256*PRODOS_BLOCK_SIZE ) // >= 128K, 257-65536 Blocks
    {
        iKind        = ProDOS_KIND_TREE;
        nBlocksIndex = (nBlocksData + (PRODOS_BLOCK_SIZE/2-1)) / (PRODOS_BLOCK_SIZE / 2);
        nBlocksIndex++; // include master index block
    }
    else
    if( nSrcSize > PRODOS_BLOCK_SIZE ) // <= 128K, 2-256 blocks
    {
        iKind        = ProDOS_KIND_SAPL;
        nBlocksIndex = 1; // single index = PRODOS_BLOCK_SIZE/2 = 256 data blocks
    }

    // Verify we have room in the current directory
    int iEntryOffset = prodos_DirGetFirstFree( &gVolume, base );

#if DEBUG_ADD
    printf( "File Entry offset in Dir: %06X\n", iEntryOffset );
#endif

    // Verify we have room for index + blocks
    nBlocksFree  = prodos_BlockGetFreeTotal( &gVolume );
    nBlocksTotal = nBlocksIndex + nBlocksData;

    if( nBlocksFree < nBlocksTotal )
    {
        printf( "ERROR: Not enough free room on volume: (%d < %d)\n", nBlocksFree, nBlocksTotal );
        return false;
    }

#if DEBUG_ADD
    printf( "DEBUG: ADD: Kind        : %d %s\n"  , iKind, gaProDOS_KindDescription[ iKind ] );
    printf( "DEBUG: ADD: Size        : %06X %s\n", nSrcSize, itoa_comma( nSrcSize ) );
    printf( "DEBUG: ADD: Index Blocks: %d\n"     , nBlocksIndex );
    printf( "DEBUG: ADD: Data  Blocks: %d\n"     , nBlocksData  );
    printf( "DEBUG: ADD: First free #  %06X\n"   , iEntryOffset );
    printf( "DEBUG: ADD: Free  Blocks: %d\n"     , nBlocksFree  );
    printf( "DEBUG: ADD: Total Blocks: %d\n"     , nBlocksTotal );
#endif

    for( int iBlock = 0; iBlock < nBlocksIndex; iBlock++ )
    {
        int iMetaBlock = prodos_BlockGetFirstFree( &gVolume );

#if DEBUG_ADD
    printf( "DEBUG: ADD: Found Free Block @ %04X\n", free );
#endif
        if( !iMetaBlock )
        {
            printf( "ERROR: Couldn't allocate index block\n" );
            return false;
        }

        if( iBlock == 0 )
        {
            iNode      = iMetaBlock;
            iIndexBase = iMetaBlock * PRODOS_BLOCK_SIZE;
        }
        else
        {
            if( iKind == ProDOS_KIND_TREE )
            {
                gaDsk[ iMasterIndex + iBlock + 0   ] = (iMetaBlock >> 0) & 0xFF;
                gaDsk[ iMasterIndex + iBlock + 256 ] = (iMetaBlock >> 8) & 0xFF;
            }
        }

        prodos_BlockSetUsed( &gVolume, iMetaBlock );
    }

    // Copy Data
    int src = 0;
    int dst;

    for( int iBlock = 0; iBlock < nBlocksData; iBlock++ )
    {
        int  iDataBlock = prodos_BlockGetFirstFree( &gVolume );
        if( !iDataBlock )
        {
            printf( "ERROR: Couldn't allocate data block\n" );
            return false;
        }

        if( iBlock == 0 )
            if( iKind == ProDOS_KIND_SEED )
                iNode = iDataBlock;

        dst = iDataBlock * PRODOS_BLOCK_SIZE;
        memcpy( &gaDsk[ dst ], &buffer[ src ], PRODOS_BLOCK_SIZE );
        src += PRODOS_BLOCK_SIZE;

        prodos_BlockSetUsed( &gVolume, iDataBlock );

        if( iKind == ProDOS_KIND_SAPL )
        {
            // Update single index block
            if( iIndexBase )
            {
                gaDsk[ iIndexBase + iBlock + 0   ] = (iDataBlock >> 0) & 0xFF;
                gaDsk[ iIndexBase + iBlock + 256 ] = (iDataBlock >> 8) & 0xFF;
            }
        }
        else
        if( iKind == ProDOS_KIND_TREE )
        {
            // Master Index Block already up-to-date

            // Update multiple index blocks
            printf( "TODO: Master Index Block + Data Block: %d\n", iBlock );
        }
    }

    // Update Directory with Entry
    ProDOS_FileHeader_t entry;

    if( attribs )
        entry = *attribs;
    else
        memset( &entry, 0, sizeof( entry ) );

    entry.kind      = iKind;
    entry.inode     = iNode;
    entry.blocks    = nBlocksTotal;
    entry.size      = nSrcSize;
    entry.dir_block = iDirBlock;

#if DEBUG_ADD
    printf( "==========\n" );
    printf( "DEBUG: ADD:   kind: $%X\n"  , iKind );
    printf( "DEBUG: ADD:  inode: @%04X\n", iNode );
    printf( "DEBUG: ADD: blocks: $%04X\n", nBlocksTotal );
    printf( "DEBUG: ADD:dir_blk: @%04X\n", entry.dir_block );
    printf( "==========\n" );
#endif

    prodos_PutFileHeader( iEntryOffset, &entry );

    // Update Directory with Header
    dir.file_count++;
    prodos_SetVolumeHeader( &dir, base );

#if DEBUG_ADD
    int addr;

    addr = 2 * PRODOS_BLOCK_SIZE;
    DskDump( addr, addr + 4 + gVolume.entry_len*3 );

    printf( "\n" );

    addr = gVolume.meta.bitmap_block * PRODOS_BLOCK_SIZE;
    printf( "Volume bitmap \n" );
    DskDump( addr, addr + 64 );

    ProDOS_VolumeHeader_t test;
    prodos_GetVolumeHeader( &test, base );

    printf( "DEBUG: ADD: ----- all done! -----\n" );
    printf( "\n" );
#endif

    return true;
}


// ========================================================================
void ProDOS_FileDelete( const char *path )
{
}


// TODO: ProDOS_VolumeHeader_t *config
// ========================================================================
void ProDOS_Init( const char *path )
{
    // Zero disk
    memset( gaDsk, 0, gnDskSize );

    // Copy Boot Sector
    // TODO:

    // Init Bitmap

    // Create blocks for root directory
    int nRootDirBlocks = 4;
    int iPrevDirBlock  = 0;
    int iNextDirBlock  = 0;
    int iOffset;

    gVolume.meta.bitmap_block = PRODOS_ROOT_BLOCK + nRootDirBlocks;
    int nBitmapBlocks = prodos_BlockInitFree( &gVolume );

    // Set boot blocks as in-use
    // PRODOS_ROOT_BLOCK == 2
    prodos_BlockSetUsed( &gVolume, 0 );
    prodos_BlockSetUsed( &gVolume, 1 );

#if DEBUG_BITMAP
    int offset = gVolume.meta.bitmap_block * PRODOS_BLOCK_SIZE;
    printf( "Bitmap after flaggings Blocks 0, 1\n" );
    DskDump( offset, offset + 32 );
#endif

    for( int iBlock = 0; iBlock < nRootDirBlocks; iBlock++ )
    {
        iNextDirBlock = prodos_BlockGetFirstFree( &gVolume );
        iOffset       = iNextDirBlock * PRODOS_BLOCK_SIZE;
        prodos_BlockSetUsed( &gVolume, iNextDirBlock );

        // Double Linked List
        // [0] = prev
        // [2] = next -- will be set on next allocation
        DskPut16( iOffset + 0, iPrevDirBlock );
//      DskPut16( iOffset + 2, 0 ); // OPTIMIZATION: disk zeroed above

        if( iBlock )
        {
            // Fixup previous directory block with pointer to this one
            iOffset = iPrevDirBlock * PRODOS_BLOCK_SIZE;
            DskPut16( iOffset + 2, iNextDirBlock );
        }

        iPrevDirBlock = iNextDirBlock;
    }

#if DEBUG_BITMAP
    printf( "Bitmap after root directory\n" );
    DskDump( offset, offset + 64 );
#endif

    // Alloc Bitmap Blocks
    for( int iBlock = 0; iBlock < nBitmapBlocks; iBlock++ )
    {
        int iBitmap = prodos_BlockGetFirstFree( &gVolume );
        prodos_BlockSetUsed( &gVolume, iBitmap );
    }

#if DEBUG_BITMAP
    printf( "Bitmap after flag for bitmaps\n" );
    DskDump( offset, offset + 64 );
#endif

    gVolume.entry_len = 0x27;
    gVolume.entry_num = PRODOS_BLOCK_SIZE / gVolume.entry_len;

    // Note:
    // .file_count = 0, since no files added
    // OPTIMIZATION: disk zeroed above
    if( *path == '/' )
        path++;

    size_t nLen = strlen( path );

    gVolume.kind = ProDOS_KIND_ROOT;
    gVolume.len  = nLen;
    string_CopyUpper( gVolume.name, path, 15 );

    gVolume.access = 0
        | ACCESS_D
        | ACCESS_N
        | ACCESS_B
//      | ACCESS_Z4 -- not used
//      | ACCESS_Z3 -- not used
//      | ACCESS_I  -- no point to making the volume invis
        | ACCESS_W
        | ACCESS_R
        ;

/*
    // TODO:
    if( config ) gVolume.access = config.access;
    if( config ) gVolume.date   = config.date;
    if( config ) gVolume.time   = config.time;
*/

#if DEBUG_BITMAP
    printf( "Bitmap before SetVolume\n" );
    DskDump( offset, offset + 32 );
#endif

    prodos_SetVolumeHeader( &gVolume, PRODOS_ROOT_BLOCK );

#if DEBUG_INIT
    int addr;

    addr = 2 * PRODOS_BLOCK_SIZE;
    DskDump( addr, addr + 64 );

    printf( "\n" );

    addr = 3 * PRODOS_BLOCK_SIZE;
    DskDump( addr, addr + 64 );

    printf( "\n" );

//  addr = gVolume.meta.bitmap_block * PRODOS_BLOCK_SIZE;
    printf( "Volume bitmap after SetVolumeHeader()\n" );
    DskDump( offset, offset + 64 );
#endif
}

