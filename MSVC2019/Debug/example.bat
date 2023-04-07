    echo.>foo1.txt
    echo.>foo2.txt
    echo.>foo3.txt

    REM Windows 
    echo a020a90220a8fc8d30c0a92420a8fc8d30c088d0ed60 > softbeep.hex
    certutil.exe -f -v -decodehex                       softbeep.hex softbeep.bin
    REM Windows

    prodosfs test.dsk init  -size=140               /TEST
    prodosfs test.dsk cp               foo1.txt     /
    prodosfs test.dsk cp               foo2.txt     /
    prodosfs test.dsk cp               foo3.txt     /
    prodosfs test.dsk cp    -aux=$0300 softbeep.bin / 
    prodosfs test.dsk catalog
