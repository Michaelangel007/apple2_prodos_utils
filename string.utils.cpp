
// Copy string, converting to uppercase. Will NULL terminate destination
// @param nLen - copy up to N chars. If zero will calculate source string length
// @return Length of string not including NULL terminator
// ========================================================================
size_t string_CopyUpper( char *pDst, const char *pSrc, int nLen = 0 )
{
    if( !nLen )
        nLen = strlen( pSrc );

    char *pBeg = pDst;

    for( int i = 0; i < nLen; i++ )
    {
        char c = *pSrc++;

        if( c == 0 ) // Don't copy past end-of-string
            break;

        if((c >= 'a') && (c <= 'z' ))
            c -= ('a' - 'A');

        *pDst++ = c;               
    }

    *pDst = 0;

    return (pDst - pBeg); 
}


// @return NULL if no extension found, else return pointer to ".<ext>"
// ========================================================================
const char* file_GetExtension( const char *pFileName )
{
    int nLen = strlen( pFileName );

    const char *pSrc = pFileName + nLen - 1;
    const char *pBeg = pFileName;

    while( pSrc >= pBeg )
    {
        if( *pSrc == '.' )
            return pSrc;

        pSrc--;
    }

    return NULL;
}



// Convert text to integer in base 16
// Optional prefix of $
// ========================================================================
int getHexVal( const char *text )
{
    int n = 0;
    const char *pSrc = text;

    if( !text )
        return n;

    if( pSrc[0] == '$' )
        pSrc++;

    n = strtoul( pSrc, NULL, 16 );
    return n;
}
