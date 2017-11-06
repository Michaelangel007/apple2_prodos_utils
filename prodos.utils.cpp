// Access _________________________________________________________________

    const size_t PRODOS_BLOCK_SIZE   = 0x200; // 512 bytes/block
    const size_t PRODOS_ROOT_BLOCK   = 2;
    const int    PRODOS_MAX_FILENAME = 15;
//  const int    PRODOS_MAX_PATH     = 64; // TODO: Verify

    const uint8_t ACCESS_D = 0x80; // Can destroy
    const uint8_t ACCESS_N = 0x40; // Can rename 
    const uint8_t ACCESS_B = 0x20; // Can backup
    const uint8_t ACCESS_Z4= 0x10; // not used - must be zero?
    const uint8_t ACCESS_Z3= 0x08;
    const uint8_t ACCESS_I = 0x04; // invisible
    const uint8_t ACCESS_W = 0x02; // Can write
    const uint8_t ACCESS_R = 0x01; // Can read

    void prodos_AccessToString( uint8_t access, char *text_ )
#if 0
    {               // 76543210
        char in [9] = "dnb--iwr";
        char out[9] = "--------";

        char *src = in ;
        char *dst = out;
#endif
    {
        const char *src = "dnb--iwr";
        /* */ char *dst = text_;

        int mask = 0x80;
        while( mask )
        {
            *dst = access & mask
                 ? *src
                 : '-'
                 ;

            dst++;
            src++;
            mask >>= 1;
        }

        *dst = 0;

//      sprintf( text_, out );
    }

// Storage Kind ___________________________________________________________

    enum ProDOS_Kind_e
    {
        ProDOS_KIND_DEL  = 0x0,
        ProDOS_KIND_SEED = 0x1, // Single Block
        ProDOS_KIND_SAPL = 0x2, // 1st Block is allocated blocks
        ProDOS_KIND_TREE = 0x3,
        ProDOS_KIND_PAS  = 0x4, // http://www.1000bit.it/support/manuali/apple/technotes/pdos/tn.pdos.25.html
        ProDOS_KIND_GSOS = 0x5, // http://www.1000bit.it/support/manuali/apple/technotes/pdos/tn.pdos.25.html
        ProDOS_KIND_DIR  = 0xD, // parent block entry for sub-directory
        ProDOS_KIND_SUB  = 0xE, // subdir header entry to find parent directory; meta to reach parent
        ProDOS_KIND_ROOT = 0xF, // volume header entry for root directory
        NUM_PRODOS_KIND  = 16
    };

    const char *gaProDOS_KindDescription[ NUM_PRODOS_KIND ] = // Storage Type
    {
         "del" // 0 Deleted
        ,"sed" // 1 Seedling size <=     512 bytes ==   1*PRODOS_BLOCK_SIZE
        ,"sap" // 2 Sapling  size <= 131,072 bytes == 256*PRODOS_BLOCK_SIZE -- interleaved: lo blocks * 256, hi blocks * 256
        ,"tre" // 3 Tree     size <= 32 MB
        ,"pas" // 4 Pascal Volume
        ,"gs " // 5 GS/OS extended file data+resource fork
        ,"? 6" // 6
        ,"? 7" // 7
        ,"? 8" // 8
        ,"? 9" // 9
        ,"? A" // A
        ,"? B" // B
        ,"? C" // C
        ,"dir" // D Sub-directory
        ,"sub" // E Sub-directory header
        ,"vol" // F Volume directory header
    };
    const char *prodos_KindToString( uint8_t type )
    {
        type &= 0xF;
        return gaProDOS_KindDescription[ type ];
    }

// Date ___________________________________________________________________

    const char *gaMonths[16] =
    {
         "bug" //  0
        ,"JAN" //  1
        ,"FEB" //  2
        ,"MAR" //  3
        ,"APR" //  4
        ,"MAY" //  5
        ,"JUN" //  6
        ,"JUL" //  7
        ,"AUG" //  8
        ,"SEP" //  9
        ,"OCT" // 10
        ,"NOV" // 11
        ,"DEC" // 12
        ,"bug" // 13 Should never happen
        ,"bug" // 14 Should never happen
        ,"bug" // 15 Should never happen
    };

    // Converts date to 9 chars: DD-MON-YR format
    // NULL terminates the string
    // ------------------------------------------------------------
    void prodos_DateToString( uint16_t date, char *text_ )
    {
        if( date == 0 )
            sprintf( text_, "<NO DATE>" );
        else
        {
            // 76543210
            //        3 210
            // FEDCBA98 76543210
            // yyyyyyym mmmddddd
            int day = (date >> 0) & 0x1F;
            int mon = (date >> 5) & 0x0F;
            int yar = (date >> 9) & 0x7F;

#if DEBUG_DATE
    printf( "DEBUG: DATE: day: % 2d\n", day       );
    printf( "DEBUG: DATE: mon: %d\n"  , mon       );
    printf( "DEBUG: DATE: yar: %2d\n" , yar % 100 );
#endif

            sprintf( text_, "%*s%d-%s-%2d", (day < 10), "", day, gaMonths[ mon ], yar % 100 );
        }
    }

    uint16_t ProDOS_DateToInt( int mon, int day, int year )
    {
        if( day  <  1 ) day  =  1;
        if( day  > 31 ) day  = 31;

        if( mon  <  1 ) mon  =  1;
        if( mon  > 12 ) mon  = 12;

        if( year <  0 ) year =  0;
        if( year > 99 ) year = 99;

        // FEDCBA98 76543210
        // yyyyyyym mmmddddd
        int16_t date = 0
            | (day  << 0)
            | (mon  << 5)
            | (year << 9)
            ;

        return date;
    }

    int prodos_DateMonthToInt( const char *month )
    {
        for( int iMonth = 1; iMonth <= 12; iMonth++ )
            if( stricmp( month, gaMonths[ iMonth ] ) == 0 )
                return iMonth;

        return 0;
    }


// Time ___________________________________________________________________

    // Converts time to 6 chars: HH:MMm
    // ------------------------------------------------------------
    void prodos_TimeToString( uint16_t time, char *text_ )
    {
        if( time == 0 )
            sprintf( text_, "      " );
        else
        {
            // FEDCBA98 76543210
            // 000hhhhh 00mmmmmm
            int  min    = (time >> 0) & 0x3F;
            int  hour   = (time >> 8) & 0x1F;

            char median = (hour < 12)
                        ? 'a'
                        : 'p'
                        ;

#if DEBUG_TIME
    printf( "DEBUG: TIME: time: %02X\n", time );
    printf( "DEBUG: TIME: min : %02X\n", min  );
    printf( "DEBUG: TIME: hour: %02X\n", hour );
    printf( "DEBUG: TIME: medn: %c\n"  ,median);
#endif

            if( hour >= 12 )
                hour -= 12;
            else
            if( hour == 0 )
                hour += 12;

            sprintf( text_, "%*s%d:%02d%c", (hour < 10), "", hour, min, median );
        }
    }

// User Type ______________________________________________________________

    // User type
    // http://www.1000bit.it/support/manuali/apple/technotes/ftyp/ft.about.html
    // http://www.kreativekorp.com/miscpages/a2info/filetypes.shtml

    const char *gaProDOS_FileTypes[ 256 ] =
    {
// $0x Types: General
          "UNK" // $00 Unknown
        , "BAD" // $01 Bad Block
        , "PCD" // $02 Pascal Code
        , "PTX" // $03 Pascal Text
        , "TXT" // $04 ASCII Text; aux type = 0 sequential, <> 0 random-acess
        , "PDA" // $05 Pascal Data
        , "BIN" // $06 Binary File; aux type = load address
        , "FNT" // $07 Apple III Font
        , "FOT" // $08 HiRes/Double HiRes Graphics
        , "BA3" // $09 Apple /// BASIC Program
        , "DA3" // $0A Apple /// BASIC Data
        , "WPF" // $0B Generic Word Processing
        , "SOS" // $0C SOS System File
        , "?0D" // $0D ???
        , "?0E" // $0E ???
        , "DIR" // $0F ProDOS Directory
// $1x Ty pes: Productivity
        , "RPD" // $10 RPS Data
        , "RPI" // $11 RPS Index
        , "AFD" // $12 AppleFile Discard
        , "AFM" // $13 AppleFile Model
        , "AFR" // $14 AppleFile Report
        , "SCL" // $15 Screen Library
        , "PFS" // $16 PFS Document
        , "?17" // $17 ???
        , "?18" // $18 ???
        , "ADB" // $19 AppleWorks Database
        , "AWP" // $1A AppleWorks Word Processing
        , "ASP" // $1B AppleWorks Spreadsheet
        , "?1C" // $1C ???
        , "?1D" // $1D ???
        , "?1E" // $1E ???
        , "?1F" // $1F ???
// $2x Ty pes: Code
        , "TDM" // $20 Desktop Manager File
        , "IPS" // $21 Instant Pascal Source
        , "UPV" // $22 UCSD Pascal Volume
        , "?23" // $23 ???
        , "?24" // $24 ???
        , "?25" // $25 ???
        , "?26" // $26 ???
        , "?27" // $27 ???
        , "?28" // $28 ???
        , "3SD" // $29 Apple /// SOS Directory
        , "8SC" // $2A Source Code
        , "8OB" // $2B Object Code
        , "8IC" // $2C Interpreted Code; aux type = $8003 Apex Program File
        , "8LD" // $2D Language Data
        , "P8C" // $2E ProDOS 8 Code Module
        , "?2F" // $2F ???
// $3x unused
        , "?30" // $30 ???
        , "?31" // $31 ???
        , "?32" // $32 ???
        , "?33" // $33 ???
        , "?34" // $34 ???
        , "?35" // $35 ???
        , "?36" // $36 ???
        , "?37" // $37 ???
        , "?38" // $38 ???
        , "?39" // $39 ???
        , "?3A" // $3A ???
        , "?3B" // $3B ???
        , "?3C" // $3C ???
        , "?3D" // $3D ???
        , "?3E" // $3E ???
        , "?3F" // $3F ???
// $4x Types: Miscellaneous
        , "?40" // $40 ???
        , "OCR" // $41 Optical Character Recognition
        , "FTD" // $42 File Type Definitions
        , "PER" // $43 --- missing from Kreative: Peripheral data
        , "?44" // $44 ???
        , "?45" // $45 ???
        , "?46" // $46 ???
        , "?47" // $47 ???
        , "?48" // $48 ???
        , "?49" // $49 ???
        , "?4A" // $4A ???
        , "?4B" // $4B ???
        , "?4C" // $4C ???
        , "?4D" // $4D ???
        , "?4E" // $4E ???
        , "?4F" // $4F ???
// $5x Types: Apple IIgs General
        , "GWP" // $50 Apple IIgs Word Processing
        , "GSS" // $51 Apple IIgs Spreadsheet
        , "GDB" // $52 Apple IIgs Database
        , "DRW" // $53 Object Oriented Graphics
        , "GDP" // $54 Apple IIgs Desktop Publishing
        , "HMD" // $55 HyperMedia
        , "EDU" // $56 Educational Program Data
        , "STN" // $57 Stationery
        , "HLP" // $58 Help File
        , "COM" // $59 Communications
        , "CFG" // $5A Configuration
        , "ANM" // $5B Animation
        , "MUM" // $5C Multimedia
        , "ENT" // $5D Entertainment
        , "DVU" // $5E Development Utility
        , "FIN" // $5F --- missing from Kreative: FIN
// $6x Types: PC Transporter
        , "PRE" // $60 PC Pre-Boot
        , "?61" // $61 ???
        , "?62" // $62 ???
        , "?63" // $63 ???
        , "?64" // $64 ???
        , "?65" // $65 ???
        , "NCF" // $66 ProDOS File Navigator Command File
        , "?67" // $67 ???
        , "?68" // $68 ???
        , "?69" // $69 ???
        , "?6A" // $6A ???
        , "BIO" // $6B PC Transporter BIOS
        , "?6C" // $6C ???
        , "DVR" // $6D PC Transporter Driver
        , "PRE" // $6E PC Transporter Pre-Boot
        , "HDV" // $6F PC Transporter Hard Disk Image
// $7x Types: Kreative Software
        , "SN2" // $70 Sabine's Notebook 2.0
        , "KMT" // $71
        , "DSR" // $72
        , "BAN" // $73
        , "CG7" // $74
        , "TNJ" // $75
        , "SA7" // $76
        , "KES" // $77
        , "JAP" // $78
        , "CSL" // $79
        , "TME" // $7A
        , "TLB" // $7B
        , "MR7" // $7C
        , "MLR" // $7D Mika City
        , "MMM" // $7E
        , "JCP" // $7F
// $8x Types: GEOS
        , "GES" // $80 System File
        , "GEA" // $81 Desk Accessory
        , "GEO" // $82 Application
        , "GED" // $83 Document
        , "GEF" // $84 Font
        , "GEP" // $85 Printer Driver
        , "GEI" // $86 Input Driver
        , "GEX" // $87 Auxiliary Driver
        , "?88" // $88 ???
        , "GEV" // $89 Swap File
        , "?8A" // $8A ???
        , "GEC" // $8B Clock Driver
        , "GEK" // $8C Interface Card Driver
        , "GEW" // $8D Formatting Data
        , "?8E" // $8E ???
        , "?8F" // $8F ???
// $9x unused
        , "?90" // $90 ???
        , "?91" // $91 ???
        , "?92" // $92 ???
        , "?93" // $93 ???
        , "?94" // $94 ???
        , "?95" // $95 ???
        , "?96" // $96 ???
        , "?97" // $97 ???
        , "?98" // $98 ???
        , "?99" // $99 ???
        , "?9A" // $9A ???
        , "?9B" // $9B ???
        , "?9C" // $9C ???
        , "?9D" // $9D ???
        , "?9E" // $9E ???
        , "?9F" // $9F ???
// $Ax Types: Apple IIgs BASIC
        , "WP " // $A0 WordPerfect
        , "?A1" // $A1 ???
        , "?A2" // $A2 ???
        , "?A3" // $A3 ???
        , "?A4" // $A4 ???
        , "?A5" // $A5 ???
        , "?A6" // $A6 ???
        , "?A7" // $A7 ???
        , "?A8" // $A8 ???
        , "?A9" // $A9 ???
        , "?AA" // $AA ???
        , "GSB" // $AB Apple IIgs BASIC Program
        , "TDF" // $AC Apple IIgs BASIC TDF
        , "BDF" // $AD Apple IIgs BASIC Data
        , "?AE" // $AE ???
        , "?AF" // $AF ???
// $Bx Types: Apple IIgs System
        , "SRC" // $B0 Apple IIgs Source Code
        , "OBJ" // $B1 Apple IIgs Object Code
        , "LIB" // $B2 Apple IIgs Library
        , "S16" // $B3 Apple IIgs Application Program
        , "RTL" // $B4 Apple IIgs Runtime Library
        , "EXE" // $B5 Apple IIgs Shell Script
        , "PIF" // $B6 Apple IIgs Permanent INIT
        , "TIF" // $B7 Apple IIgs Temporary INIT
        , "NDA" // $B8 Apple IIgs New Desk Accessory
        , "CDA" // $B9 Apple IIgs Classic Desk Accessory
        , "TOL" // $BA Apple IIgs Tool
        , "DRV" // $BB Apple IIgs Device Driver
        , "LDF" // $BC Apple IIgs Generic Load File
        , "FST" // $BD Apple IIgs File System Translator
        , "?BE" // $BE ???
        , "DOC" // $BF Apple IIgs Document
// $Cx Ty pes: Graphics
        , "PNT" // $C0 Apple IIgs Packed Super HiRes
        , "PIC" // $C1 Apple IIgs Super HiRes; aux_type = $2 = Super HiRes 3200
        , "ANI" // $C2 PaintWorks Animation
        , "PAL" // $C3 PaintWorks Palette
        , "?C4" // $C4 ???
        , "OOG" // $C5 Object-Oriented Graphics
        , "SCR" // $C6 Script
        , "CDV" // $C7 Apple IIgs Control Panel
        , "FON" // $C8 Apple IIgs Font
        , "FND" // $C9 Apple IIgs Finder Data
        , "ICN" // $CA Apple IIgs Icon File
        , "?CB" // $CB ???
        , "?CC" // $CC ???
        , "?CD" // $CD ???
        , "?CE" // $CE ???
        , "?CF" // $CF ???
//$Dx Types: Audio
        , "?D0" // $D0 ???
        , "?D1" // $D1 ???
        , "?D2" // $D2 ???
        , "?D3" // $D3 ???
        , "?D4" // $D4 ???
        , "MUS" // $D5 Music
        , "INS" // $D6 Instrument
        , "MDI" // $D7 MIDI
        , "SND" // $D8 Apple IIgs Audio
        , "?D9" // $D9 ???
        , "?DA" // $DA ???
        , "DBM" // $DB DB Master Document
        , "?DC" // $DC ???
        , "?DD" // $DD ???
        , "?DE" // $DE ???
        , "?DF" // $DF ???
// $Ex Types: Miscellaneous
        , "LBR" // $E0 Archive
        , "?E1" // $E1 ???
        , "ATK" // $E2 AppleTalk Data; aux_type = $FFFF - EasyMount Alias
        , "?E3" // $E3 ???
        , "?E4" // $E4 ???
        , "?E5" // $E5 ???
        , "?E6" // $E6 ???
        , "?E7" // $E7 ???
        , "?E8" // $E8 ???
        , "?E9" // $E9 ???
        , "?EA" // $EA ???
        , "?EB" // $EB ???
        , "?EC" // $EC ???
        , "?ED" // $ED ???
        , "R16" // $EE EDASM 816 Relocatable Code
        , "PAR" // $EFPascal Area
// $Fx Types: System
        , "CMD" // $F0 ProDOS Command File
        , "OVL" // $F1 User Defined 1
        , "UD2" // $F2 User Defined 2
        , "UD3" // $F3 User Defined 3
        , "UD4" // $F4 User Defined 4
        , "BAT" // $F5 User Defined 5
        , "UD6" // $F6 User Defined 6
        , "UD7" // $F7 User Defined 7
        , "PRG" // $F8 User Defined 8
        , "P16" // $F9 ProDOS-16 System File
        , "INT" // $FA Integer BASIC Program
        , "IVR" // $FB Integer BASIC Variables
        , "BAS" // $FC Applesoft BASIC Program; aux type = $0801
        , "VAR" // $FD Applesoft BASIC Variables
        , "REL" // $FE EDASM Relocatable Code
        , "SYS" // $FF ProDOS-8 System File
    };

