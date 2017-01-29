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
* Description:  Internal heap free space filling
*               (16-bit code only)
*
****************************************************************************/


#include "dll.h"        // needs to be first
#include "variety.h"
#include <stddef.h>
#include <malloc.h>
#include <string.h>
#include "heap.h"
#include "heapacc.h"


#define FRLBPTR     XBPTR( freelistp, seg )

extern  void    _mymemset(void _WCFAR *,unsigned,unsigned);
#pragma aux     _mymemset = \
        "shr cx,1"              \
        "rep stosw"             \
        "adc cx,cx"             \
        "rep stosb"             \
    parm caller [es di] [ax] [cx] modify exact [di cx]

int __HeapSet( __segment seg, unsigned int fill )
{
    FRLBPTR     curr_frl;

    fill |= fill << 8;
    _AccessFHeap();
    for( ; seg != _NULLSEG; seg = HBPTR( seg )->nextseg ) {
        curr_frl = HBPTR( seg )->freehead.next;
        while( FP_OFF( curr_frl ) != offsetof( heapblk, freehead ) ) {
            _mymemset( curr_frl + 1, fill, curr_frl->len - sizeof( freelistp ) );
            curr_frl = curr_frl->next;
        }
    }
    _ReleaseFHeap();
    return( _HEAPOK );
}
