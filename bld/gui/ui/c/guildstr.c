/****************************************************************************
*
*                            Open Watcom Project
*
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


// a lot of the contents of this file were liberally lifted from
// wlmsg.c in the WLIB project -- Wes

#include "guiwind.h"
#include "guiextnm.h"
#include <string.h>
#include <sys/types.h>
#include <stdio.h>
#include "wio.h"
#include "watcom.h"
#include "wresdefn.h"
#include "wressetr.h"
#include "wresset2.h"
#include "filefmt.h"
#include "resdiag.h"
#include "resmenu.h"
#include "seekres.h"
#include "guildstr.h"

#include "clibext.h"


#if defined( __QNX__ ) || defined( __UNIX__ )
    #define _newline "\n"
#else
    #define _newline "\r\n"
#endif

#define NO_RES_MESSAGE_PREFIX "Error: could not open message resource file ("
#define NO_RES_MESSAGE_SUFFIX ")." _newline

static  HANDLE_INFO     hInstance = { 0 };

static WResFileOffset res_seek( WResFileID handle, WResFileOffset position, int where )
/* fool the resource compiler into thinking that the resource information
 * starts at offset 0 */
{
    if( where == SEEK_SET ) {
        return( lseek( handle, position + WResFileShift, where ) - WResFileShift );
    } else {
        return( lseek( handle, position, where ) );
    }
}

WResSetRtns( open, close, posix_read, posix_write, res_seek, tell, GUIMemAlloc, GUIMemFree );

bool GUIIsLoadStrInitialized( void )
{
    return( hInstance.status != 0 );
}

bool GUILoadStrInit( const char *fname )
{
    hInstance.status = 0;
    if( OpenResFileX( &hInstance, fname, GUIGetExtName() != NULL ) ) {
        // if we are using an external resource file then we don't have to search
        return( true );
    }
    CloseResFile( &hInstance );
    posix_write( fileno(stdout), NO_RES_MESSAGE_PREFIX, sizeof( NO_RES_MESSAGE_PREFIX ) - 1 );
    posix_write( fileno(stdout), fname,                 strlen( fname ) );
    posix_write( fileno(stdout), NO_RES_MESSAGE_SUFFIX, sizeof( NO_RES_MESSAGE_SUFFIX ) - 1 );
    return( false );
}

bool GUILoadStrFini( void )
{
    return( CloseResFile( &hInstance ) );
}

bool GUILoadString( gui_res_id id, char *buffer, int buffer_length )
{
    if( hInstance.status && buffer != NULL && buffer_length != 0 ) {
        if( WResLoadString( &hInstance, id, (lpstr)buffer, buffer_length ) > 0 ) {
            return( true );
        } else {
            buffer[0] = '\0';
        }
    }

    return( false );
}

bool GUISeekDialogTemplate( res_name_or_id dlg_id )
{
    bool                ok;

    ok = ( hInstance.status != 0 );

    if( ok ) {
        ok = WResSeekResourceX( &hInstance, MAKEINTRESOURCE( RT_DIALOG ), dlg_id );
    }

    return( ok );
}

bool GUISeekMenuTemplate( res_name_or_id menu_id )
{
    bool                ok;

    ok = ( hInstance.status != 0 );

    if( ok ) {
        ok = WResSeekResourceX( &hInstance, MAKEINTRESOURCE( RT_MENU ), menu_id );
    }

    return( ok );
}

bool GUIResReadDialogBoxHeader( DialogBoxHeader *hdr )
{
    return( ResReadDialogBoxHeader( hdr, hInstance.handle ) );
}

bool GUIResReadDialogBoxControl( DialogBoxControl *ctl )
{
    return( ResReadDialogBoxControl( ctl, hInstance.handle ) );
}

bool GUIResReadMenuHeader( MenuHeader *hdr )
{
    return( ResReadMenuHeader( hdr, hInstance.handle ) );
}

bool GUIResReadMenuItem( MenuItem *new )
{
    return( ResReadMenuItem( new, hInstance.handle ) );
}
