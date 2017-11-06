/**
 @param  {uint64_t}  x        - Number to convert to comma delimited string
 @param  {char*}    [output_] - Optional buffer to copy output to
 @return {char*} Returns pointer to comma delimited NULL terminated string
 @notes          Does NOT NULL terminate the output_ string (to allow for string concatenation)
 @history
    version 9 - Renamed p,e to dst, end
    Version 8 - Cleanup notes
    Version 7 - cleanup 'x > 9' to 'x > 0'
    Version 6 - Add jsdoc
    Verison 5 - add optional output, does NOT null terminate output
    Version 4 - document hard-coded max string length 32
    Version 3 - cleanup '>= 10' to '> 9'
    Verison 2 - add 4 buffers
    Version 1 - initial implementation
*/
const char*
itoa_comma( uint64_t x, char *output_ = NULL )
{
    const  int     MAX_CHARS   = 31; // 2^32 - 1 =              4,294,967,295 = 13 chars
    const  int     NUM_BUFFERS =  4; // 2^64 - 1 = 18,446,744,073,709,551,615 = 26 chars

    static char    aString[ NUM_BUFFERS ][ MAX_CHARS+1 ];
    static uint8_t iString     = 0;
   
    char *dst = &aString[ iString ][ MAX_CHARS ];
    char *end = dst; // save copy of end pointer

    *dst-- = 0;
    while( x > 999 )
    {
        *dst-- = '0' + (x % 10); x /= 10;
        *dst-- = '0' + (x % 10); x /= 10;
        *dst-- = '0' + (x % 10); x /= 10;
        *dst-- = ',';
    }

    /*      */ { *dst-- = '0' + (x % 10); x /= 10; }
    if (x > 0) { *dst-- = '0' + (x % 10); x /= 10; }
    if (x > 0) { *dst-- = '0' + (x % 10); x /= 10; }

    iString++;
    iString &= (NUM_BUFFERS-1);

    if( output_ )
        memcpy( output_, dst+1, end-dst-1 ); // -1 to NOT include NULL terminator

    return ++dst;
}
