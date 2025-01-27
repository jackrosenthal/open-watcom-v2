/****************************************************************************
*
*                            Open Watcom Project
*
* Copyright (c) 2002-2022 The Open Watcom Contributors. All Rights Reserved.
*    Portions Copyright (c) 1983-2002 Sybase, Inc. All Rights Reserved.
*
*  ========================================================================
*
*    This file contains Original Code and/or Modifications of Original
*    Code as defined in and that are subject to the Sybase Open Watcom
*    Public License version 1.0 (the 'License'). You may not use this file
*    except in compliance with the License. BY USING THIS FILE YOU AGREE TO
*    ALL TERMS AND CONDITIONS OF THE LICENSE. A copy of the License is
*    provided with the Original Code and Modifications, and is also
*    available at www.sybase.com/developer/opensource.
*
*    The Original Code and all software distributed under the License are
*    distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
*    EXPRESS OR IMPLIED, AND SYBASE AND ALL CONTRIBUTORS HEREBY DISCLAIM
*    ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR
*    NON-INFRINGEMENT. Please see the License for the specific language
*    governing rights and limitations under the License.
*
*  ========================================================================
*
* Description:  WHEN YOU FIGURE OUT WHAT THIS FILE DOES, PLEASE
*               DESCRIBE IT HERE!
*
****************************************************************************/


#include <stdlib.h>
#include <i86.h>
#include "tinyio.h"
#include "digtypes.h"
#include "dospath.h"
#if !defined( SERVER )
#include "dbgpsp.h"
#endif


char *StrCopyDst( const char *src, char *dst )
{
    while( (*dst = *src++) != '\0' ) {
        ++dst;
    }
    return( dst );
}

const char *StrCopySrc( const char *src, char *dst )
{
    while( (*dst++ = *src) != '\0' ) {
        src++;
    }
    return( src );
}

#if defined( SERVER )
const char *DOSEnvFind( const char *src )
{
    return( getenv( src ) );
}
#else
const char __far *DOSEnvFind( const char *src )
{
    const char  __far *env;
    const char  *p;
    char        c1;
    char        c2;

    env = _MK_FP( *(unsigned __far *)_MK_FP( DbgPSP(), 0x2c ), 0 );
    do {
        p = src;
        do {
            c1 = *p++;
            c2 = *env++;
        } while( c1 == c2 && c1 != '\0' && c2 != '=' );
        if( c1 == '\0' && c2 == '=' )
            return( env );
        while( c2 != '\0' ) {
            c2 = *env++;
        }
    } while( *env != '\0' );
    return( NULL );
}
#endif

tiny_ret_t TryPath( const char *name, char *end, const char *ext_list )
{
    tiny_ret_t  rc;
    int         mode;

    mode = 0; //IsDOS3 ? 0x40 : 0;
    do {
        ext_list = StrCopySrc( ext_list, end ) + 1;
        rc = TinyOpen( name, mode );
        if( TINY_OK( rc ) ) {
            TinyClose( TINY_INFO( rc ) );
            return( rc );
        }
    } while( *ext_list != '\0' );
    return( rc );
}

unsigned long FindFilePath( dig_filetype file_type, const char *pgm, char *buffer )
{
    const char  __far *path;
    char        *p2;
    const char  *p3;
    tiny_ret_t  rc;
    int         have_ext;
    int         have_path;
    const char  *ext_list;

    have_ext = 0;
    have_path = 0;
    for( p3 = pgm, p2 = buffer; (*p2 = *p3) != '\0'; ++p3, ++p2 ) {
        switch( *p3 ) {
        case '\\':
        case '/':
        case ':':
            have_path = 1;
            have_ext = 0;
            break;
        case '.':
            have_ext = 1;
            break;
        }
    }
    ext_list = "\0";
    if( have_ext == 0 && file_type == DIG_FILETYPE_EXE ) {
#if defined( DOSXTRAP )
  #if defined( DOS4G )
        ext_list = ".exe\0";
  #else
        ext_list = ".exp\0.rex\0.exe\0";
  #endif
#else
        ext_list = ".com\0.exe\0";
#endif
    }
    rc = TryPath( buffer, p2, ext_list );
    if( TINY_OK( rc ) || have_path )
        return( rc );
    path = DOSEnvFind( "PATH" );
    if( path == NULL )
        return( rc );
    for( ; *path != '\0'; path++ ) {
        p2 = buffer;
        while( *path != '\0' && *path != ';' ) {
            *p2++ = *path++;
        }
        if( p2 != buffer && p2[-1] != '\\' && p2[-1] != '/' ) {
            *p2++ = '\\';
        }
        p2 = StrCopyDst( pgm, p2 );
        rc = TryPath( buffer, p2, ext_list );
        if( TINY_OK( rc ) )
            break;
        if( *path == '\0' ) {
            break;
        }
    }
    return( rc );
}
