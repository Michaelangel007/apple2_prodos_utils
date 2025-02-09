// Access _________________________________________________________________

    const size_t PRODOS_BLOCK_SIZE   = 0x200; // 512 bytes/block
    const size_t PRODOS_ROOT_BLOCK   = 2;
    const int    PRODOS_ROOT_OFFSET  = PRODOS_ROOT_BLOCK * PRODOS_BLOCK_SIZE;

    const int    PRODOS_MAX_FILENAME = 15;
    const int    PRODOS_MAX_PATH     = 64; // TODO: Verify

    const uint8_t ACCESS_D = 0x80; // Can destroy
    const uint8_t ACCESS_N = 0x40; // Can rename
    const uint8_t ACCESS_B = 0x20; // Can backup
    const uint8_t ACCESS_Z4= 0x10; // not used - must be zero?
    const uint8_t ACCESS_Z3= 0x08; // not used - must be zero?
    const uint8_t ACCESS_I = 0x04; // invisible
    const uint8_t ACCESS_W = 0x02; // Can write
    const uint8_t ACCESS_R = 0x01; // Can read

    enum ProDOS_Kind_e
    {
        ProDOS_KIND_DEL  = 0x0, // Deleted file or unused
        ProDOS_KIND_SEED = 0x1, // Single Block
        ProDOS_KIND_SAPL = 0x2, // 1st Block is allocated blocks
        ProDOS_KIND_TREE = 0x3, // Multiple meta blocks
        ProDOS_KIND_PAS  = 0x4, // http://www.1000bit.it/support/manuali/apple/technotes/pdos/tn.pdos.25.html
        ProDOS_KIND_GSOS = 0x5, // http://www.1000bit.it/support/manuali/apple/technotes/pdos/tn.pdos.25.html
        ProDOS_KIND_DIR  = 0xD, // parent block entry for sub-directory
        ProDOS_KIND_SUB  = 0xE, // subdir header entry to find parent directory; meta to reach parent
        ProDOS_KIND_ROOT = 0xF, // volume header entry for root directory
        NUM_PRODOS_KIND  = 16
    };

// --- ProDOS Structs ---

    struct ProDOS_SubDirInfo_t
    {
        uint8_t res75  ; // $75 - magic number
        uint8_t pad7[7];
    };

    struct ProDOS_VolumeExtra_t
    {
        uint16_t bitmap_block;
        uint16_t total_blocks;
    };

    struct ProDOS_SubDirExtra_t
    {
        uint16_t parent_block;
        uint8_t  parent_entry_num;
        uint8_t  parent_entry_len;
    };

    // ============================================================
    // Due to default compiler packing/alignment this may NOT be byte-size identical on disk but it is in the correct order.
    // NOTE: This does NOT need to be packed/aligned since fields are read/written on a per entry basis.
    // See: ProDOS_GetVolumeHeader;
    struct ProDOS_VolumeHeader_t
    {                                   ; //Rel Size Abs
        uint8_t  kind                   ; // +0    1 $04  \ Hi nibble  Storage Type
        uint8_t  len                    ; // +0           / Lo nibble
        char     name[ 16 ]             ; // +1   15 $05  15 on disk but we NULL terminate for convenience
// --- diff from file ---
        union
        {
            uint8_t              pad8[8]; //+16    8 $14 only in root
            ProDOS_SubDirInfo_t  info   ; //             only in sub-dir
        }                               ;
// --- same as file ---
        uint16_t date                   ; //+24    2 $1C
        uint16_t time                   ; //+26    2 $1E
        uint8_t  cur_ver                ; //+28    1 $20
        uint8_t  min_ver                ; //+29    1 $21
        uint8_t  access                 ; //+30    1 $22
        uint8_t  entry_len              ; //+31    1 $23 Size of each directory entry in bytes
        uint8_t  entry_num              ; //+32    1 $24 Number of directory entries per block
        uint16_t file_count             ; //+33    2 $25 Active entries including directories, excludes volume header
        union                             //          --
        {                                 //          --
            ProDOS_VolumeExtra_t meta   ; //+34    2 $27
            ProDOS_SubDirExtra_t subdir ; //+36    2 $29
        }                               ; //============
    }                                   ; // 38      $2B

    // ============================================================
    // Due to default compiler packing/alignment this may NOT be byte-size identical on disk but it is in the correct order.
    // NOTE: This does NOT need to be packed/aligned since fields are read/written on a per entry basis.
    struct ProDOS_FileHeader_t
    {                           ; //Rel Size Hex
        uint8_t  kind           ; // +0    1 $00  \ Hi nibble Storage Type
        uint8_t  len            ; // +0           / Lo nibble Filename Length
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

