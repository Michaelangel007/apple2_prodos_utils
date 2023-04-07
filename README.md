# Apple 2 ProDOS (Virtual Disk) Utilities

`ProdosFS` is a command-line utility to create and manipulate virtual ProDOS volume images for emulators.

**NOTE:** This is still a work-in-progress.

Commands that work:

* [x] cat
* [x] catalog
* [x] cp  (Only Seed (file size <= 512 bytes), and Sapling files (file size <= 128 KB) are currently supported)
* [x] dir
* [ ] get
* [x] init
* [x] ls
* [ ] mkdir
* [ ] rm
* [ ] rmdir


```
Usage: <dsk> <command> [<options>] [<path>]

    cat      Catalog (short form)
                 [<path>]            Path of virtual sub-directory to view
                                     Defaults to: /
    catalog  Catalog (long form)
                 [<path>]            Path of virtual sub-directory to view
                                     NOTE: Defaults to: /
    cp       Add file(s) to volume   File type will auto-detected based on filename extension.
                                     i.e. BAS, BIN, FNT, TXT,SYS, etc.
                 -access=$##         Set access flags
                                     NOTE: Defaults to $C3
                                         $80 Volume/file can be destroyed
                                         $40 Volume/file can be renamed
                                         $20 Volume/file changed since last backup
                                         $04 Volume/file is invisible
                                         $02 Volume/file can be read
                                         $01 Volume/file can be written
                 -aux=$####          Set the aux address
                 -date=MM/DD/YY      Set create date to specified date
                 -date=DD-MON-YY     Set create date to specified date
                                     MON is one of:
                                         JAN, FEB, MAR, APR, MAY, JUN,
                                         JUL, AUG, SEP, OCT, NOV, DEC
                 -time=HH:MMa        Set create time to specified 12-hour AM
                 -time=HH:MMp        Set create time to specified 12-hour PM
                 -time=HH:MM         Set create time to specified 24-hour time
                 -type=$##           Force file type to one of the 256 types
                 -moddate=MM/DD/YY   Set last modified date to specified date
                 -modtime=HH:MM      Set last modified date to specified time
                 <path>              Destination virutal sub-directory to add to
                                     There is no default -- it must be specified
                                     NOTE: Options must come first
    dir      Catalog (long form)
                                     This is an alias for 'catalog'
    get      Extract file from volume
                 <path>              Path of virtual file to extract
                                     NOTES:
                                         The file remains on the virtual volume
                                         To delete a file see ........: rm
                                         To delete a sub-directory see: rmdir
    init     Format disk
                 <path>              Name of virtual volume.
                 -size=140           Format 140 KB (5 1/4")
                 -size=800           Format 800 KB (3 1/2")
                 -size=32            Format 32  MB (Hard Disk)
    ls       Catalog (file names only)
                 [<path>]            Path to sub-directory to view
                                     Defaults to: /
    mkdir    Create a sub-directory
                 <path>              Destination virutal sub-directory to create
                                     There is no default -- it must be specified
    rm       Delete file from volume
                 <path>              Path of virtual file to delete
                                     There is no default -- it must be specified
    rmdir    Remove a sub-directory
                 <path>              Path of virtual sub-directory to delete
                                     There is no default -- it must be specified
                                     NOTE:
                                         You can't delete the root directory: /
                 -f                  Force removal of sub-directory
                                     (Normally a sub-directory must be empty)

Where <dsk> is a virtual disk image with an extension of:

    .dsk  (Assumes DOS3.3 sector order)
    .do   (DOS3.3 sector order)
    .po   (ProDOS sector order)

NOTE: To skip always having to specify the <.dsk> name set the environment variable:

           PRODOS_VOLUME
e.g.
    export PRODOS_VOLUME=path/to/volume.po
    set    PRODOS_VOLUME=disk.dsk

Three different disk sizes are accepted for init

   prodos test.dsk init -size=140   # 5 1/4"  (140 KB)
   prodos test.dsk init -size=800   # 3 1/2"  (800 KB)
   prodos test.dsk init -size=32    #HardDisk ( 32 MB)

Examples:

    prodos test.dsk ls
    prodos test.dsk cat
    prodos test.dsk cp    foo1 foo2 /
    prodos test.dsk mkdir bar
    prodos test.dsk cp    foo2      /bar
    prodos test.dsk get   /bar/foo2
    prodos test.dsk rm    /bar/foo2
    prodos test.dsk rmdir /bar
    prodos test.dsk init  /TEST
    prodos b140.dsk init  -size=140 /BLANK140
    prodos b800.dsk init  -size=800 /BLANK800
    prodos b032.dsk init  -size=32  /BLANK32
```

Example output:

```
dnb??iwr /TEST            Blocks Size    Type    Aux   Kind  iNode Dir   Ver Min  Create    Time    Modified  Time  
-------- ---------------- ------ ------- ------- ----- ----- ----- ----- --- ---  --------- ------  --------- ------
-------- *FOO1.TXT             1 $000002 TXT $04 $0000 sed 1 @0007 @0002 0.0 v00   6-NOV-17         <NO DATE>        
-------- *FOO2.TXT             1 $000002 TXT $04 $0000 sed 1 @0008 @0002 0.0 v00   6-NOV-17         <NO DATE>        
-------- *FOO3.TXT             1 $000002 TXT $04 $0000 sed 1 @0009 @0002 0.0 v00   6-NOV-17         <NO DATE>        
-------- *TEXT.BIN             3 $000264 BIN $06 $0000 sap 2 @000A @0002 0.0 v00   6-NOV-17         <NO DATE>        
========
Files:        4 / 52 ( 7.69%)
Blocks: 
   Free:    267 (95.36%), 1st: @ $000D = 13
   Used:     13 ( 4.64%)
  Total:    280
```

# Building / Compiling

`make clean; make`

