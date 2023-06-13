#ifdef _WIN32
    #define _CRT_SECURE_NO_WARNINGS   1
    #define _CRT_NONSTDC_NO_DEPRECATE 1 //  error C4996: '_stricmp': The POSIX name for this item is deprecated. Instead, use the ISO C and C++ conformant name: _stricmp.

    //#define stricmp  _stricmp
    //#pragma warning(suppress : 4996)
#endif

#define DEBUG_MAIN 0

    #include <stdio.h>    // printf()
    #include <stdint.h>   // uint8_t
    #include <sys/stat.h> // stat
    #include <stdlib.h>   // exit()
    #include <string.h>   // memcpy()
    #include <time.h>     // time() localtime()

    #include "itoa.comma.h"
    #include "string.utils.cpp"
    #include "generic.disk.cpp"
    #include "prodos.utils.cpp"
    #include "prodos.tools.cpp"

/*
TODO:
    -meta <filename>   Use these file attributes instead of default <file>._META
    -nometa            Do not generate <file>._META when extracting via 'get'

Is this still needed?
    -path /
*/

    enum DISK_COMMANDS_e
    {
         DISK_COMMAND_CAT_SHORT = 0 // cat
        ,DISK_COMMAND_CAT_LONG      // catalog
        ,DISK_COMMAND_FILE_ADD      // cp  - Copy host filesystem to virtual DSK
        ,DISK_COMMAND_CAT_LONG2     // dir - alias for catalog
        ,DISK_COMMAND_FILE_GET      // get - Copy virtual DSK to host file system
        ,DISK_COMMAND_VOL_INIT      // init
        ,DISK_COMMAND_CAT_NAMES     // ls
        ,DISK_COMMAND_DIR_CREATE    // mkdir
        ,DISK_COMMAND_DIR_CREATE2   // md
        ,DISK_COMMAND_FILE_DELETE   // rm
        ,DISK_COMMAND_DIR_DELETE    // rmdir
        ,DISK_COMMAND_DIR_DELETE2   // rd
        ,DISK_COMMAND_INVALID
        ,NUM_DISK_COMMANDS
    };

    const char *gaCommands[ NUM_DISK_COMMANDS ] =
    {
         "cat"     // CAT__SHORT
        ,"catalog" // CAT__LONG
        ,"cp"      // FILE_ADD
        ,"dir"     // CAT__LONG2
        ,"get"     // FILE_GET
        ,"init"    // VOL__INIT
        ,"ls"      // CAT__NAMES
        ,"mkdir"   // DIR__CREATE
        ,"md"      // DIR__CREATE2
        ,"rm"      // FILE_DELETE
        ,"rmdir"   // DIR__DELETE
        ,"rd"      // DIR__DELETE2
        ,""
    };

    const char *gaCommandDescriptions[ NUM_DISK_COMMANDS ] =
    {
         "Catalog (short form)"      // CAT__SHORT
        ,"Catalog (long form)"       // CAT__LONG
        ,"Add file(s) to volume"     // FILE_ADD
        ,"Catalog (long form)"       // CAT__LONG2
        ,"Extract file from volume"  // FILE_GET
        ,"Format disk"               // VOL__INIT
        ,"Catalog (file names only)" // CAT__NAMES
        ,"Create a sub-directory"    // DIR__CREATE
        ,"Create a sub-directory"    // DIR__CREATE2
        ,"Delete file from volume"   // FILE_DELETE
        ,"Remove a sub-directory"    // DIR__DELETE
        ,"Remove a sub-directory"    // DIR__DELETE2
        ,""                          // NUM_DISK_COMMANDS
    };

    const char *gaOptionsDescriptions[ NUM_DISK_COMMANDS ] =
    {
         // CAT__SHORT
"                 [<path>]            Path of virtual sub-directory to view\n"
"                                     Defaults to: /\n"
        ,// CAT_LONG
"                 [<path>]            Path of virtual sub-directory to view\n"
"                                     Defaults to: /\n"
        ,// FILE_ADD
"                                     File type will auto-detected based on filename extension.\n"
"                                     i.e. BAS, BIN, FNT, TXT,SYS, etc.\n"
"                 -access=$##         Set access flags\n"
"                                     NOTE: Defaults to $C3\n"
"                                         $80 Volume/file can be destroyed\n"
"                                         $40 Volume/file can be renamed\n"
"                                         $20 Volume/file changed since last backup\n"
"                                         $04 Volume/file is invisible\n"
"                                         $02 Volume/file can be read\n"
"                                         $01 Volume/file can be written\n"
"                 -aux=$####          Set the aux address\n"
"                 -date=MM/DD/YY      Set create date to specified date\n"
"                 -date=DD-MON-YY     Set create date to specified date\n"
"                                     MON is one of:\n"
"                                         JAN, FEB, MAR, APR, MAY, JUN,\n"
"                                         JUL, AUG, SEP, OCT, NOV, DEC\n"
"                 -time=HH:MMa        Set create time to specified 12-hour AM\n"
"                 -time=HH:MMp        Set create time to specified 12-hour PM\n"
"                 -time=HH:MM         Set create time to specified 24-hour time\n"
"                 -type=<type>        Set the file type to a 3 character code\n"
"                                     i.e. BIN, SYS\n"
"                 -type=$##           Force file type to one of the 256 types\n"
"                                     The file type is auto-detected via extension\n"
"                 -moddate=MM/DD/YY   Set last modified date to specified date\n"
"                 -modtime=HH:MM      Set last modified date to specified time\n"
"                 <path>              Destination virutal sub-directory to add to\n"
"                                     There is no default -- it must be specified\n"
"                                     NOTE: Options must come first\n"
        , //CAT_LONG2
"                                     This is an alias for 'catalog'\n"
        ,// FILE_GET
"                 [-boot=<file>]      Optional: extract boot sector to file\n"
"                 <path>              Path of virtual file to extract\n"
"                                     NOTES:\n"
"                                         The file remains on the virtual volume\n"
"                                         To delete a file see ........: rm\n"
"                                         To delete a sub-directory see: rmdir\n"
        , // VOL__INIT
"                 [-boot=<file>]      Optional: replace boot sector with file\n"
"                 -size=140           Format 140 KB (5 1/4\")\n"
"                 -size=800           Format 800 KB (3 1/2\")\n"
"                 -size=32            Format 32  MB (Hard Disk)\n"
"                 <path>              Name of virtual volume.\n"
        , // CAT__NAMES
"                 [<path>]            Path to sub-directory to view\n"
"                                     Defaults to: /\n"
        , // DIR__CREATE
"                                     NOTE: Not implemented yet!\n"
"                 <path>              Destination virutal sub-directory to create\n"
"                                     There is no default -- it must be specified\n"
        , // DIR__CREATE2
"                                     Alias for mkdir\n"
        , // FILE_DELETE
"                 <path>              Path of virtual file to delete\n"
"                                     There is no default -- it must be specified\n"
        , // DIR__DELETE
"                                     NOTE: Not implemented yet!\n"
"                 <path>              Path of virtual sub-directory to delete\n"
"                                     There is no default -- it must be specified\n"
"                                     NOTE:\n"
"                                         You can't delete the root directory: /\n"
"                 -f                  Force removal of sub-directory\n"
"                                     (Normally a sub-directory must be empty)\n"
        , // DIR__DELETE2
"                                     Alias for rmdir\n"
        , NULL
    };

    char        sPath[ 256 ] = "/"; // TODO: What is max ProDOS path? 128 characters?
    const char *gpPath = NULL;

    int gnDay   = 0;
    int gnMonth = 0;
    int gnYear  = 0;

    int giOptions = 0; // index of arg that is 1st command
//  int gnOptions = 0; // number of args minus config options

// ========================================================================
int usage()
{
    printf(
"Usage: <dsk> <command> [<options>] [<path>]\n"
"\n"
    );

    for( int iCommand = 0; iCommand < NUM_DISK_COMMANDS-1; iCommand++ )
    {
        /**/printf( "    %-7s  %s\n"
                , gaCommands           [ iCommand ]
                , gaCommandDescriptions[ iCommand ]
            );
        if( gaOptionsDescriptions[ iCommand ] )
            printf( "%s"
                , gaOptionsDescriptions[ iCommand ]
        );
    }
    printf(
"\n"
"Where <dsk> is a virtual disk image with an extension of:\n"
"\n"
"    .dsk  (Assumes DOS3.3 sector order)\n"
"    .do   (DOS3.3 sector order)\n"
"    .po   (ProDOS sector order)\n"
"\n"
"NOTE: To skip always having to specify the <.dsk> name set the environment variable:\n"
"\n"
"           PRODOS_VOLUME\n"
"e.g.\n"
"    export PRODOS_VOLUME=path/to/volume.po\n"
"    set    PRODOS_VOLUME=disk.dsk\n"
"\n"
"Three different disk sizes are accepted for init\n"
"\n"
"   prodosfs test.dsk init -size=140 /TEST514   # 5 1/4\"  (140 KB)\n"
"   prodosfs test.dsk init -size=800 /TEST312   # 3 1/2\"  (800 KB)\n"
"   prodosfs test.dsk init -size=32  /TEST32M   #HardDisk ( 32 MB)\n"
"\n"
"To put a (512) boot sector file use -boot with the init command:\n"
"   prodosfs test.dsk init -boot=bootsector.bin -size=140 /TEST514\n"
"\n"
"To get a (512) boot sector file use -boot with the cp command:\n"
"   prodosfs test.dsk cp   -boot=bootsector.bin           /TEST514\n"
"\n"
"Examples:\n"
"\n"
"    prodosfs test.dsk ls\n"
"    prodosfs test.dsk cat\n"
"    prodosfs test.dsk cp    foo1 foo2 /\n"
"    prodosfs test.dsk mkdir bar\n"
"    prodosfs test.dsk cp    foo2      /bar\n"
"    prodosfs test.dsk get   /PRODOS\n"
"    prodosfs test.dsk rm    /bar/foo2\n"
"    prodosfs test.dsk rmdir /bar\n"
"    prodosfs test.dsk init  /TEST\n"
"    prodosfs b140.dsk init  -size=140 /BLANK140\n"
"    prodosfs b800.dsk init  -size=800 /BLANK800\n"
"    prodosfs b032.dsk init  -size=32  /BLANK32\n"
"\n"
    );

    return 1;
}


// ========================================================================
void setTimeNow( ProDOS_FileHeader_t *entry )
{
    time_t     now  = time( NULL );
    struct tm *time = localtime( &now );

    // http://www.manpages.info/macosx/ctime.3.html
    gnMonth = time->tm_mon + 1; // 0-11
    gnDay   = time->tm_mday;    // 1-31
    gnYear  = time->tm_year % 100;

    entry->date = ProDOS_DateToInt( gnMonth, gnDay, gnYear );
}

/*
    TODO: Sync these up with <file>._META ProDOS_FileExtract() and getCopyConfig()

    -access=$##
    -aux=$####
    -date=MM/DD/YY
    -date=DD-MON-YY
    -moddate=MM/DD/YY
    -moddate=DD-MON-YY
    -modtime=HR:MNa
    -modtime=HR:MNp
    -modtime=HH:MM
    -modtime=$####
    -time=HR:MNa
    -time=HR:MNp
    -time=$####
    -time=HH:MM
    -type=$##
    -type=BIN

    -version=$##
    -minver=$##
*/
// @return false if fatel error
// ========================================================================
bool getCopyConfig( ProDOS_FileHeader_t *entry, const char *arg )
{
    size_t nLenPrefix = strlen( arg ); // Total length of: option=val
    size_t nLenSuffix = 0;             // Length of value

    const char *pVal  = 0;
    int val = 0;

    if( strncmp( arg, "date=", 5 ) == 0 )
    {
        nLenSuffix = nLenPrefix - 5;
        pVal       = arg        + 5;

        // Default to current date
        int mon = gnMonth;
        int day = gnDay;
        int yar = gnYear;

        //-date=MM/DD/YY
        //-date=DD-MON-YR
        if( (nLenSuffix != 8) || nLenSuffix != 9 )
        {
            printf( "ERROR: Invalid date. Format is MM/DD/YY or DD-MON-YR\n" );
            return false;
        }

        if( nLenSuffix == 8 )
        {   // 01234567
            // MM/DD/YY
            if( (pVal[ 2 ] != '/') || (pVal[ 5 ] != '/') )
            {
                printf( "ERROR: Invalid date: Format is MM/DD/YY. e.g. 12/31/17\n" );
                return false;
            }

            mon = atoi( pVal + 0 );
            day = atoi( pVal + 3 );
            yar = atoi( pVal + 6 );
        }

        if( nLenSuffix == 9 )
        {   // 012345678
            // DD-MON-YY
            if( (pVal[ 2 ] != '-') || (pVal[ 6 ] != '-') )
            {
                printf( "ERROR: Invalid date: Format is DD-MON-YR. e.g. 01-JAN-17\n" );
                return false;
            }

            day = atoi( pVal + 0 );
            mon = prodos_DateMonthToInt( pVal + 3 );
            yar = atoi( pVal + 7 );
        }

        entry->date = ProDOS_DateToInt( mon, day, yar );

    }
    else
    if( strncmp( arg, "moddate=", 8 ) == 0 )
    {
        nLenSuffix = nLenPrefix - 8;
        pVal       = arg        + 8;

printf( "ERROR: Modified Time not yet implemented\n" );

    }
    else
    if( strncmp( arg, "modtime=", 8 ) == 0 )
    {
        nLenSuffix = nLenPrefix - 8;
        pVal       = arg        + 8;

printf( "ERROR: Modified Time not yet implemented\n" );

    }
    else
    if( strncmp( arg, "time=", 5 ) == 0 )
    {
        nLenSuffix = nLenPrefix - 5;
        pVal       = arg        + 5;

printf( "ERROR: Create Time not yet implemented\n" );

    }
    else
    if( strncmp( arg, "type=", 5 ) == 0 )
    {
        nLenSuffix = nLenPrefix - 5;
        pVal       = arg        + 5;

        if( pVal[0] == '$' )
        {
            val = getHexVal( pVal ); // safely ignores leading $
            if( val < 0x00 ) val = 0x00;
            if( val > 0xFF ) val = 0xFF;
            entry->type = val;
        }
        else
        {
            char sExt[ 4 ];
            int  nLen = string_CopyUpper( sExt, pVal, 3 );

            for( int iType = 0; iType < 256; iType++ )
            {
                if( stricmp( pVal, gaProDOS_FileTypes[ iType ] ) == 0 )
                {
                    entry->type = iType;
                    break;
                }
            }
        }
    }
    else
    if( strncmp( arg, "aux=", 4 ) == 0 )
    {
        nLenSuffix = nLenPrefix - 4;
        pVal       = arg        + 4;

        if( pVal[0] == '$' )
        {
            val = getHexVal( pVal ); // safely ignores leading $
            if( val < 0x0000 ) val = 0x0000;
            if( val > 0xFFFF ) val = 0xFFFF;
            entry->aux = val;
        }
    }
    else
    if( strncmp( arg, "access=", 7 ) == 0 )
    {
        nLenSuffix = nLenPrefix - 7;
        pVal       = arg        + 7;

        val = getHexVal( pVal );
        if( val < 0x00 ) val = 0x00;
        if( val > 0xFF ) val = 0xFF;
        entry->access = val;
    }

    return true;
}

// ========================================================================
bool doCopy( ProDOS_FileHeader_t *entry, const char *filename )
{
  const char  *pSrcFileName = filename;
        size_t nSrcLen      = strlen( pSrcFileName );

        char  *pExt         = const_cast<char*>( file_GetExtension( pSrcFileName ) );
        char   sExt[ 5 ]    = ".???";
        size_t nExtLen      = 0;
        bool   bCopiedName  = false;


    // Chop extension down to leading '.' plus max 3 chars
    if( pExt )
    {
        size_t nLen = string_CopyUpper( sExt, pExt, 4 ); // 3 char extension

        pExt    = sExt;
        nExtLen = nLen;
    }

    const char *pBaseName = filename + nSrcLen;
    size_t      nBaseLen  = 0;

    // Chop off preceeding directory if there is one
    if( pBaseName )
    {
        while( pBaseName > filename )
        {
            if( *pBaseName == '/' )
                break;

            pBaseName--;
            nBaseLen ++;
        }
    }

#if DEBUG_MAIN
    printf( "----------\n" );
    printf( "Extension: %d\n", pExt != NULL );
    printf( "+ %s\n"     , pSrcFileName );
    printf( "  Len: %u\n", nSrcLen      );
    printf( " Base: %s\n", pBaseName    );
    printf( "  Len: %u\n", nBaseLen     );
    printf( "----------\n" );
#endif

    if( nSrcLen > 15 )
    {
        nBaseLen  = nSrcLen - nExtLen;

        // Chop off part of name until it fits
        {
            pBaseName++;
            nBaseLen --;
        }

        // If we have an extension, chop the prefix preserving extension
        // If no extension, chop the prefix
        if( pExt )
        {
            int end  = nSrcLen - 4;
            int len1 = string_CopyUpper( gEntry.name +   0, pBaseName, nBaseLen );
            int len2 = string_CopyUpper( gEntry.name + end, pExt                );

            gEntry.len = len1 + len2;
            bCopiedName = true;
        }
    }

    if( !bCopiedName )
    {
        gEntry.len = (uint8_t) string_CopyUpper( gEntry.name, pSrcFileName );
    }

#if DEBUG_MAIN
    printf( "Entry.name: %s\n", gEntry.name );
#endif

    if( pExt )
    {
        for( int iExt = 0; iExt < 256; iExt++ )
        {
            const char *tExt = gaProDOS_FileTypes[ iExt ];
            if( tExt[0] == '?' )
                continue;

            int nExtLen = 3;
            if( tExt[2] == ' ' )
                nExtLen = 2;

            bool bFoundExt = true;
            for( int i = 0; i < nExtLen; i++ )
                if( sExt[ 1 + i ] != tExt[ i ] )
                    bFoundExt = false;

            if( bFoundExt )
            {
#if DEBUG_MAIN
    printf( "Auto-detect file type: $%02X %s\n", iExt, tExt );
#endif
                gEntry.type = iExt;
                break;
            }
        }
    }

    prodos_MetaLoad( &gEntry );

#if DEBUG_MAIN
    printf( "File Access: $%02X\n", gEntry.access );
#endif

    bool bStatus = ProDOS_FileAdd( gpPath, pSrcFileName, &gEntry );
    return bStatus;
}


// ========================================================================
const char* getVirtualPath( int nArg, const char *aArg[], int *iArg, bool inc )
{
    if( *iArg < nArg )
    {
        const char *pSrc = aArg[ *iArg ];
        char       *pDst = sPath;
        int         nLen = string_CopyUpper( pDst, pSrc, 255 );

        if( inc )
            *iArg++;

        return sPath;
    }

    return NULL;
}


// ========================================================================
void errorBadInterleave()
{
    printf( "ERROR: Unable to detect sector interleave. e.g. .do, .po, or .dsk\n" );
    exit(1);
}

void errorBadDisk()
{
    printf( "ERROR: Unable to read ProDOS disk: %s\n", gpDskName );
    exit( 1 );
}

// ========================================================================
void readVolume( int nArg, const char *aArg[], int *iArg )
{
    if( gpDskName )
    {
        if( !DskGetInterleave( gpDskName ) )
            errorBadInterleave();

        if( !DskLoad( gpDskName, (SectorOrder_e) giInterleaveLastUsed ) )
            errorBadDisk();
    }

#if DEBUG_MAIN
    printf( "nArg: %d\n",  nArg );
    printf( "iArg: %d\n", *iArg );
#endif
    gpPath = getVirtualPath( nArg, aArg, iArg, true );

#if DEBUG_MAIN
    printf( "virtual path: %s\n", gpPath );
#endif

    prodos_GetVolumeHeader( &gVolume, PRODOS_ROOT_BLOCK );

#if DEBUG_MAIN
    printf( "Loaded...\n" );
#endif
}


// ========================================================================
int main( const int nArg, const char *aArg[] )
{
    int   iArg              = 1; // DSK is 1st arg
    char *pathname_filename = NULL;
    char *auto_dsk_name     = getenv( "PRODOS_VOLUME" );

    gpDskName = auto_dsk_name;

    if( auto_dsk_name )
    {
        printf( "INFO: Using auto ProDOS disk name: %s\n", auto_dsk_name );
    }
    else
    {
        if( iArg < nArg )
        {
            gpDskName = aArg[ iArg ];
            iArg++;
#if DEBUG_MAIN
    printf( "Found disk name from command line: %s\n", gpDskName );
#endif
        }
    }

    const char *pCommand = iArg < nArg
        ? aArg[ iArg ]
        : NULL
        ;
    int iCommand = DISK_COMMAND_INVALID;
    if( pCommand )
        for( iCommand = 0; iCommand < NUM_DISK_COMMANDS-1; iCommand++ )
        {
            if( strcmp( aArg[ iArg ], gaCommands[ iCommand ] ) == 0 )
            {
                iArg++;
                break;
            }
        }

#if DEBUG_MAIN
    printf( "iCommand: %d\n", iCommand );
    printf( "pCommand: %s\n", pCommand );
    printf( "pPathName %s\n", gpPath   );
#endif

    switch( iCommand )
    {
        case DISK_COMMAND_CAT_LONG:
        case DISK_COMMAND_CAT_LONG2:
            readVolume( nArg, aArg, &iArg );
            ProDOS_CatalogLong( gpPath );
            break;

        case DISK_COMMAND_CAT_NAMES:
            readVolume( nArg, aArg, &iArg );
            ProDOS_CatalogNames( gpPath );
            break;

        case DISK_COMMAND_CAT_SHORT:
            readVolume( nArg, aArg, &iArg );
            ProDOS_CatalogShort( gpPath );
            break;

        // prodos <DSK> cp file1 [file2 ...] /
        case DISK_COMMAND_FILE_ADD:
        {
#if DEBUG_MAIN
    printf( "DEBUG: cp\n" );
#endif

            readVolume( nArg, aArg, &iArg );

            // Check for at least 1 destination (path)
            int iDst = nArg - 1;
            gpPath   = getVirtualPath( nArg, aArg, &iDst, false );

            // Check for at least 1 source
            bool bFilesAdded = false;

            prodos_InitFileHeader( &gEntry );
            setTimeNow( &gEntry );

#if DEBUG_MAIN
    printf( "DEBUG: zero'd file entry\n" );
    printf( "iArg: %d\n", iArg );
    printf( "nArg: %d\n", nArg );
#endif

            if( iArg == nArg )
            {
                printf( "ERROR: Need virtual destination path. e.g. /\n" );
                break;
            }
            else
            for( ; iArg < nArg-1; iArg++ )
            {
                const char *pArg = &aArg[iArg][0];

#if DEBUG_MAIN
    printf( "DEBUG: %s\n", pArg );
#endif

                if( pArg[0] == '-' )
                    bFilesAdded = getCopyConfig( &gEntry, pArg+1 );
                else
                {
                    bFilesAdded = doCopy( &gEntry, pArg );

                    prodos_InitFileHeader( &gEntry ); // prep for next file
                    setTimeNow( &gEntry );
                }

                if( !bFilesAdded )
                    break;
            }

            if( bFilesAdded )
                DskSave();

            break;
        }

        case DISK_COMMAND_FILE_DELETE:
        {
            readVolume( nArg, aArg, &iArg );
//          ProDOS_FileDelete( gpPath ); // pathname_filename
            break;
        }

        case DISK_COMMAND_FILE_GET:
        {
#if DEBUG_MAIN
    printf( "DEBUG: get\n" );
#endif

            const char *pBootSectorFileName = NULL;

            for( ; iArg < nArg; iArg++ )
            {
                const char *pArg = &aArg[iArg][0];

                if( pArg[0] == '-' )
                {
                    if( strncmp( pArg+1,"boot=", 5 ) == 0 )
                    {
                        pBootSectorFileName = pArg + 6;

                        size_t nBootSectorNameLength = strlen( pBootSectorFileName );
                        if(   !nBootSectorNameLength )
                        {
                            printf( "ERROR: Need a file name to extract the boot sector to.\n" );
                            return 1;
                        }
                    }
                    else
                        return printf( "ERROR: Unknown option: %s\n", pArg );
                }
                else
                    break;
            }

            readVolume( nArg, aArg, &iArg );

#if DEBUG_MAIN
    printf( "DEBUG: get path: %s\n", gpPath );
#endif

            if( pBootSectorFileName )
                ProDOS_ExtractBootSector( pBootSectorFileName );
            else
                ProDOS_FileExtract( gpPath ); // pathname_filename
            break;
        }

        case DISK_COMMAND_VOL_INIT:
        {
            gnDskSize            = DSK_SIZE_312; // TODO: --size=140 --size=800 --size=32
            if( !DskGetInterleave( gpDskName ) )
                errorBadInterleave();

#if DEBUG_MAIN
    printf( "iArg: %d / %d\n", iArg, nArg );
#endif
            const char *pBootSectorFileName = NULL;

            for( ; iArg < nArg; iArg++ )
            {
                const char *pArg = &aArg[iArg][0];

#if DEBUG_MAIN
    printf( "#%d: %s\n", iArg, pArg );
#endif

                if( pArg[0] == '-' )
                {
                    if( strncmp( pArg+1,"boot=", 5 ) == 0 )
                    {
                        if( pBootSectorFileName )
                            printf( "ERROR: Already have boot sector filename. Skipping.\n" );
                        else
                            pBootSectorFileName = pArg + 6;
                    }
                    else
                    if( strncmp( pArg+1,"size=", 5 ) == 0 )
                    {
                        int size = atoi( pArg + 6 );

                        if( size == 140 ) gnDskSize = DSK_SIZE_514;
                        if( size == 800 ) gnDskSize = DSK_SIZE_312;
                        if( size ==  32 ) gnDskSize = DSK_SIZE_32M;

#if DEBUG_MAIN
    printf( "INIT: size = %s\n", itoa_comma( gnDskSize ) );
#endif
                    }
                    else
                        return printf( "ERROR: Unknown option: %s\n", pArg );
                }
                else
                    break;
            }

            gpPath = getVirtualPath( nArg, aArg, &iArg, false );

#if DEBUG_MAIN
    printf( "INIT: path: %s\n", gpPath );
    printf( "iArg: %d / %d\n", iArg, nArg );
#endif

            if( gpPath )
            {
                ProDOS_Init( gpPath );
                if( pBootSectorFileName )
                {
                    bool bReplaced = ProDOS_ReplaceBootSector( pBootSectorFileName );
                    if( !bReplaced )
                        printf( "ERROR: Couldn't replace boot sector\n" );
                }
                DskSave();
            }
            else
                return printf( "ERROR: Need virtual volume name. e.g. /TEST\n" );

            break;
        }

        default:
            if( (nArg < 2) || !pCommand )
                return usage();
            else
                return printf( "ERROR: Unknown command: %s\n", pCommand );
            break;
    }

    return 0;
}
