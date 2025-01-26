#define DEBUG_DSK            0
#define DEBUG_DSK_LOAD       0
#define DEBUG_DSK_SAVE       0
#define DEBUG_DSK_INTERLEAVE 0

#ifdef _WIN32
#else
    // OSX / BSD / Linux
    #define stricmp strcasecmp
#endif

// --- Buffer ---

    // 5 1/4" 140K (single sided)
    const size_t DSK_SIZE_514 = 256 * 16 * 35; // 143,360

    // 3 1/2" = 800K (double sided)
    const size_t DSK_SIZE_312 = 2 * 400 * 1024; // 819,200

    // 32 MB Hard Disk
    const size_t DSK_SIZE_32M = 32 * 1024 * 1024;

    const int DSK_SECTOR_SIZE = 256;


    size_t      gnDskSize = 0;
    uint8_t     gaDsk[ DSK_SIZE_32M ];
    uint8_t     gaTmp[ DSK_SECTOR_SIZE * 2 ];

    uint16_t DskGet16( int offset )
    {
        uint16_t n = 0
        |   (((uint16_t)gaDsk[ offset + 0 ]) << 0)
        |   (((uint16_t)gaDsk[ offset + 1 ]) << 8)
        ;
        return n;
    }

    uint32_t DskGet24( int offset )
    {
        uint32_t n = 0
        |   (((uint32_t)gaDsk[ offset + 0 ]) <<  0)
        |   (((uint32_t)gaDsk[ offset + 1 ]) <<  8)
        |   (((uint32_t)gaDsk[ offset + 2 ]) << 16)
        ;
        return n;
    }

    void DskPut16( int offset, int val )
    {
        gaDsk[ offset + 0 ] = (val >> 0) & 0xFF;
        gaDsk[ offset + 1 ] = (val >> 8) & 0xFF;
    }

    void DskPut24( int offset, int val )
    {
        gaDsk[ offset + 0 ] = (val >> 0) & 0xFF;
        gaDsk[ offset + 1 ] = (val >> 8) & 0xFF;
        gaDsk[ offset + 2 ] = (val >>16) & 0xFF;
    }

    int DskGetIndexBlock( int offset, int index )
    {
        int block = 0
        | (gaDsk[ offset + index + 0   ] << 0)
        | (gaDsk[ offset + index + 256 ] << 8)
        ;
        return block;
    }

    /*
        000:lo0 lo1 lo2 ...
        100:hi0 hi1 hi2 ...
    */
    void DskPutIndexBlock( int offset, int index, int block )
    {
        gaDsk[ offset + index + 0   ] = (block >> 0) & 0xFF;
        gaDsk[ offset + index + 256 ] = (block >> 8) & 0xFF;
    }


// --- Name ---

    const char *gpDskName = NULL;


// --- Sector Interleave ---

    enum SectorOrder_e
    {
        INTERLEAVE_AUTO_DETECT = 0,
        INTERLEAVE_DOS33_ORDER,
        INTERLEAVE_PRODOS_ORDER
    };
    int giInterleaveLastUsed = INTERLEAVE_AUTO_DETECT;

    // Map Physical <-> Logical
    const uint8_t gaInterleave_DSK[ 16 ] =
    {
        //0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F // logical order
        0x0,0xE,0xD,0xC,0xB,0xA,0x9,0x8,0x7,0x6,0x5,0x4,0x3,0x2,0x1,0xF // physical order
    };

    void Interleave_Forward( int iSector, size_t *src_, size_t *dst_ )
    {
        *src_ = gaInterleave_DSK[ iSector ]*DSK_SECTOR_SIZE;
        *dst_ =                   iSector  *DSK_SECTOR_SIZE; // linearize
    };
    void Interleave_Reverse( int iSector, size_t *src_, size_t *dst_ )
    {
        *src_ =                   iSector  *DSK_SECTOR_SIZE; // un-linearize
        *dst_ = gaInterleave_DSK[ iSector ]*DSK_SECTOR_SIZE;
    }


// --- Size --- 

    int File_Size( FILE *pFile )
    {
        fseek( pFile, 0L, SEEK_END );
        size_t size = ftell( pFile );
        fseek( pFile, 0L, SEEK_SET );

        return (int)size;
    }

    int DSK_GetNumTracks()
    {
        size_t tracks = gnDskSize / (16 * DSK_SECTOR_SIZE);
        return (int)tracks;
    }


// @returns true if able to determine interleave
// ========================================================================
bool DskGetInterleave( const char *dsk_filename )
{
    if( !dsk_filename )
        return false;

    // .do  = DOS
    // .po  = PRO
    // .dsk = DOS
    const size_t nLen    = strlen( dsk_filename );
    giInterleaveLastUsed = INTERLEAVE_AUTO_DETECT;

    if( nLen < 1 )
        return false;

    if( giInterleaveLastUsed == INTERLEAVE_AUTO_DETECT )
    {
        const char *pExt = file_GetExtension( dsk_filename );
        if( pExt )
        {
            if( stricmp( pExt, ".do" ) == 0 ) giInterleaveLastUsed = INTERLEAVE_DOS33_ORDER ;
            if( stricmp( pExt, ".po" ) == 0 ) giInterleaveLastUsed = INTERLEAVE_PRODOS_ORDER;
            if( stricmp( pExt, ".dsk") == 0 ) giInterleaveLastUsed = INTERLEAVE_DOS33_ORDER ;
        }

#if DEBUG_DSK_INTERLEAVE
    printf( "DskLoad() auto-detect interleave: " );
    if( giInterleaveLastUsed == INTERLEAVE_DOS33_ORDER ) printf( "DOS3.3 \n" );
    if( giInterleaveLastUsed == INTERLEAVE_PRODOS_ORDER) printf( "ProDOS \n" );
    if( giInterleaveLastUsed == INTERLEAVE_AUTO_DETECT ) printf( "unknown\n" );
#endif
    }

#if 0
    if( giInterleaveLastUsed == INTERLEAVE_AUTO_DETECT )
    {
        printf( "WARNING: Defaulting to DOS 3.3 sector interleave\n" );
        giInterleaveLastUsed = INTERLEAVE_DOS33_ORDER;
    }
#endif

    return giInterleaveLastUsed != INTERLEAVE_AUTO_DETECT;
}


// @return true = OK, false = ERROR
// ========================================================================
bool DskLoad( const char *dsk_filename, SectorOrder_e sector_order )
{
    if( !dsk_filename )
        return false;

    int valid = 0;

    FILE *pFile = fopen( dsk_filename, "rb" );
    if( pFile )
    {
        gnDskSize = File_Size( pFile );

#if DEBUG_DSK_LOAD
    printf( "==========\n" );
    printf( "gnDskSize: %06X\n", gnDskSize );
    printf( "==========\n" );
#endif

        if( gnDskSize > DSK_SIZE_32M )
        {
            printf( "ERROR: Disk size %s > %s !\n", itoa_comma( gnDskSize ), itoa_comma( DSK_SIZE_32M ) );
            gnDskSize = 0;
            valid = -1;
        }
        else
        {
            if( giInterleaveLastUsed == INTERLEAVE_PRODOS_ORDER )
                fread( gaDsk, 1, gnDskSize, pFile );
            else
            if( giInterleaveLastUsed == INTERLEAVE_DOS33_ORDER )
            {
                size_t nTracks = DSK_GetNumTracks();
                size_t offset = 0;

#if DEBUG_DSK_LOAD
    printf( "Tracks: %d\n", nTracks );
#endif

                for( int iTracks = 0; iTracks < nTracks; iTracks++ )
                {
                    for( int iSector = 0; iSector < 16; iSector++ )
                    {
                        size_t src;
                        size_t dst;
                        Interleave_Forward( iSector, &src, &dst );

#if DEBUG_DSK_LOAD
    if( iTracks == 0 )
        printf( "T0S%X -> %06X\n", iSector, src );
#endif
                        fseek( pFile, (long)(offset + src), SEEK_SET );
                        fread( &gaDsk[ offset + dst ], 1, DSK_SECTOR_SIZE, pFile );
                    }

                    offset += 16 * DSK_SECTOR_SIZE;
                }
            }
            valid = 1;
        }

        fclose( pFile );
    };

#if DEBUG_DSK_LOAD
    printf( "DEBUG: DISK: Read Valid: %d\n", valid );
#endif

    return (valid == 1);
}


// ========================================================================
bool DskSave()
{
    FILE *pFile = fopen( gpDskName, "w+b" );

    if( !pFile )
    {
        printf( "ERROR: Couldn't write to: %s\n", gpDskName );
    }

    fseek( pFile, 0L, SEEK_SET );

    if( giInterleaveLastUsed == INTERLEAVE_PRODOS_ORDER )
    {
        size_t wrote = fwrite( gaDsk, 1, gnDskSize, pFile );
        (void) wrote;
#if DEBUG_DSK_SAVE
        printf( "DskSave() wrote .po: %d\n", wrote );   
#endif
    }
    else
    if( giInterleaveLastUsed == INTERLEAVE_DOS33_ORDER )
    {
        int    nTracks = DSK_GetNumTracks();
        size_t offset = 0;

#if DEBUG_DSK_SAVE
    printf( "Tracks: %d\n", nTracks );
#endif

        for( int iTracks = 0; iTracks < nTracks; iTracks++ )
        {
            size_t dst = offset;

            for( int iSector = 0; iSector < 16; iSector++ )
            {
                size_t src;
                size_t dst;
                Interleave_Reverse( iSector, &src, &dst );

#if DEBUG_DSK_SAVE
    if( iTracks == 0 )
        printf( "%06X -> T0S%X\n", dst, iSector );
#endif

                fseek( pFile, (long)(offset + dst), SEEK_SET );
                fwrite( &gaDsk[ offset + src ], 1, DSK_SECTOR_SIZE, pFile );
            }

            offset += 16 * DSK_SECTOR_SIZE;
        }
    }

    fclose( pFile );

    return true;
}



// ========================================================================
void DskDump( int start, int end )
{
    printf( "----:  0  1  2  3  4  5  6  7 -  8  9  A  B  C  D  E  F |0123456789ABCDEF|\n" );

    while( start < end )
    {
        printf( "%04X: ", start );

        for( int byte = 0; byte < 16; byte++ )
        {
            if( byte == 8 )
                printf("- " );
            printf( "%02X ", gaDsk[ start + byte ] );
        }

        printf( "|" );

        for( int byte = 0; byte < 16; byte++ )
        {
            uint8_t c  = gaDsk[ start + byte ];
                    c &= 0x7F;

            if( c < 0x20 )
                c = '.';
            if( c == 0x7F )
                c = '.';


            printf( "%c", c );
        }

        printf( "|\n" );

        start += 16;
    }

    printf( "\n" );
}

