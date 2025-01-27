/****************************************************************************
*
*                            Open Watcom Project
*
* Copyright (c) 2015-2022 The Open Watcom Contributors. All Rights Reserved.
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

#include <stdio.h>
#include <stdlib.h>
#include <direct.h>
#include "stdnt.h"
#if defined( MD_x86 ) || defined( MD_x64 )
  #include "dbg386.h"
#endif


#define MAX_WATCHES     32

typedef struct {
    uint_64         value;
    dword           linear;
    addr48_ptr      addr;
    word            size;
    word            dregs;
}           watch_point;

typedef struct break_point {
    struct break_point  *next;
    addr48_ptr          addr;
    opcode_type         old_opcode;
}           break_point;

static watch_point  WatchPoints[MAX_WATCHES];
static int          WatchCount = 0;
static break_point  *Breaks = NULL;

static opcode_type place_breakpoint( addr48_ptr *addr )
{
    opcode_type old_opcode;

    if( ReadMemory( addr, &old_opcode, sizeof( old_opcode ) ) == sizeof( old_opcode ) ) {
        WriteMemory( addr, &BreakOpcode, sizeof( BreakOpcode ) );
        return( old_opcode );
    }
    return( 0 );
}

static int remove_breakpoint( addr48_ptr *addr, opcode_type old_opcode )
{
    return( WriteMemory( addr, &old_opcode, sizeof( old_opcode ) ) != sizeof( old_opcode ) );
}

trap_retval TRAP_CORE( Set_break )( void )
{
    set_break_req   *acc;
    set_break_ret   *ret;
    break_point     *new;

    acc = GetInPtr( 0 );
    new = LocalAlloc( LMEM_FIXED, sizeof( *new ) );
    new->old_opcode = place_breakpoint( &acc->break_addr );
    new->addr = acc->break_addr;
    new->next = Breaks;
    Breaks = new;
    ret = GetOutPtr( 0 );
    ret->old = new->old_opcode;
    return( sizeof( *ret ) );
}

trap_retval TRAP_CORE( Clear_break )( void )
{
    clear_break_req *acc;
    break_point     *brk;
    break_point     *next;

    // we can assume all breaks are cleared at once

    for( brk = Breaks; brk != NULL; brk = next ) {
        next = brk->next;
        LocalFree( brk );
    }
    Breaks = NULL;
    acc = GetInPtr( 0 );
    remove_breakpoint( &acc->break_addr, acc->old );
    return( 0 );
}

bool FindBreak( WORD segment, DWORD offset, opcode_type *old_opcode )
{
    break_point *brk;

    for( brk = Breaks; brk != NULL; brk = brk->next ) {
        if( brk->addr.segment != segment ) {
            continue;
        }
        if( brk->addr.offset != offset ) {
            continue;
        }
        *old_opcode = brk->old_opcode;
        return( true );
    }
    return( false );
}

#if defined( MD_x86 ) || defined( MD_x64 )
/*
 * setDR6 - set value of debug register 6
 */
static void setDR6( DWORD tmp )
{
    MYCONTEXT   con;
    thread_info *ti;

    ti = FindThread( DebugeeTid );
    MyGetThreadContext( ti, &con );
    con.Dr6 = tmp;
    MySetThreadContext( ti, &con );
}

/*
 * SetDR7 - set value of debug register 7
 */
void SetDR7( DWORD tmp )
{
    MYCONTEXT   con;
    thread_info *ti;
    //char buff[256];

    ti = FindThread( DebugeeTid );
    if( ti == NULL ) {
        return;
    }
    MyGetThreadContext( ti, &con );
    con.Dr7 = tmp;
    //sprintf( buff, "tid=%8.8x, dr7=%8.8x", DebugeeTid, tmp );
    //MessageBox( NULL, buff, "Dr7", MB_APPLMODAL+MB_OK );
    MySetThreadContext( ti, &con );
}

/*
 * GetDR6 - get value of debug register 6
 */
DWORD GetDR6( void )
{
    MYCONTEXT   con;
    thread_info *ti;

    ti = FindThread( DebugeeTid );
    MyGetThreadContext( ti, &con );
    return( con.Dr6 );
}

static dword setDRn( int i, dword linear, word type )
{
    MYCONTEXT   con;
    thread_info *ti;

    ti = FindThread( DebugeeTid );
    MyGetThreadContext( ti, &con );
    ( (DWORD *)&con.Dr0 )[i] = linear;
    MySetThreadContext( ti, &con );

    return( ( (dword)type << DR7_RWLSHIFT( i ) )
//        | ( DR7_GEMASK << DR7_GLSHIFT(i) )
          | ( DR7_LEMASK << DR7_GLSHIFT( i ) ) );
}
#endif

/*
 * ClearDebugRegs - set for no watch points
 */
void ClearDebugRegs( void )
{
#if defined( MD_x86 ) || defined( MD_x64 )
    int i;

    for( i = 0; i < 4; i++ ) {
        setDRn( i, 0, 0 );
    }
    setDR6( 0 );
    SetDR7( 0 );
#elif defined( MD_axp ) || defined( MD_ppc )
    /* nothing to do */
#else
    #error ClearDebugRegs not configured
#endif
}

bool IsWatch( void )
{
    return( WatchCount > 0 );
}

static int DRegsCount( void )
{
    int     needed;
    int     i;

    needed = 0;
    for( i = 0; i < WatchCount; i++ ) {
        needed += WatchPoints[i].dregs;
    }
    return( needed );
}

/*
 * SetDebugRegs - set debug registers for watch points
 */
bool SetDebugRegs( void )
{
#if defined( MD_x86 ) || defined( MD_x64 )
    int         i;
    int         dr;
    dword       dr7;
    watch_point *wp;
    dword       linear;
    word        size;


    /*
     *  Carl. I really don't like this code, but the DR count check is done above
     *  so there's not much harm that can happen. We can't get here needing more than 4
     *  debug registers. Just ugly!
     *
     *  the linear address is adjusted by size so a short across a dword boundary will screw
     *  up I think!
     */

    if( DRegsCount() > 4 ) {
        return( false );
    }

    dr  = 0;
    dr7 = /* DR7_GE | */ DR7_LE;

    if( SupportingExactBreakpoints ) {
        /*
         *  With SupportingExactBreakpoints enabled, the linear address will not have been
         *  adjusted, so we may be on an odd boundary. A 4 byte watch of offset 1 will entail
         *  3 registers - 1@1, 2@2 and 1@3
         *
         */

        for( wp = WatchPoints, i = WatchCount; i-- > 0; wp++ ) {
            size = wp->size;
            linear = wp->linear;
            switch( wp->dregs ) {
            case 1:
                /*
                 * If we only need one register, then we are 1 byte anywhere,
                 * 2 bytes on word boundary, or 4 bytes on dword boundary
                 */
                dr7 |= setDRn( dr+0, linear, DRLen( size ) | DR7_BWR );
                dr++;
                break;
            case 2:
                /*
                 * If we need 2 registers, then ...
                 */
                if( size == 2 ) {
                    /*
                     * For a 2 byte write, this is two 1's at any address
                     */
                    dr7 |= setDRn( dr+0, linear+0, DRLen( 1 ) | DR7_BWR );
                    dr7 |= setDRn( dr+1, linear+1, DRLen( 1 ) | DR7_BWR );
                } else if( size == 4 ) {
                    /*
                     * For a 4 byte write, this must be two 2's at a word boundary
                     */
                    dr7 |= setDRn( dr+0, linear+0, DRLen( 2 ) | DR7_BWR );
                    dr7 |= setDRn( dr+1, linear+2, DRLen( 2 ) | DR7_BWR );
                } else if( size == 8 ) {
                    /*
                     * For an 8 byte write, this must be two 4's at a dword boundary
                     */
                    dr7 |= setDRn( dr+0, linear+0, DRLen( 4 ) | DR7_BWR );
                    dr7 |= setDRn( dr+1, linear+4, DRLen( 4 ) | DR7_BWR );
                }
                dr += 2;
                break;
            case 3:
                /*
                 * If we need 3 registers, then must be 4 bytes or larger, and ...
                 */
                if( size == 4 ) {
                    /*
                     * For a 4 byte write, this must be 1,2,1 at a odd boundary
                     */
                    dr7 |= setDRn( dr+0, linear+0, DRLen( 1 ) | DR7_BWR );
                    dr7 |= setDRn( dr+1, linear+1, DRLen( 2 ) | DR7_BWR );
                    dr7 |= setDRn( dr+2, linear+3, DRLen( 1 ) | DR7_BWR );
                } else if( size == 8 ) {
                    /*
                     * For an 8 byte write, this must be 2,4,2 at a word boundary
                     */
                    dr7 |= setDRn( dr+0, linear+0, DRLen( 2 ) | DR7_BWR );
                    dr7 |= setDRn( dr+1, linear+2, DRLen( 4 ) | DR7_BWR );
                    dr7 |= setDRn( dr+2, linear+6, DRLen( 2 ) | DR7_BWR );
                }
                dr += 3;
                break;
            case 4:
                /*
                 * If we need 4 registers, then must be 8 bytes and ...
                 */
                if( (linear & 0x3) == 1 ) {
                    /* Need 1, 2, 4, 1 */
                    dr7 |= setDRn( dr+0, linear+0, DRLen( 1 ) | DR7_BWR );
                    dr7 |= setDRn( dr+1, linear+1, DRLen( 2 ) | DR7_BWR );
                    dr7 |= setDRn( dr+2, linear+3, DRLen( 4 ) | DR7_BWR );
                    dr7 |= setDRn( dr+3, linear+7, DRLen( 1 ) | DR7_BWR );
                } else { /* (linear & 0x3) == 3 */
                    /* Need 1, 4, 2, 1 */
                    dr7 |= setDRn( dr+0, linear+0, DRLen( 1 ) | DR7_BWR );
                    dr7 |= setDRn( dr+1, linear+1, DRLen( 4 ) | DR7_BWR );
                    dr7 |= setDRn( dr+2, linear+5, DRLen( 2 ) | DR7_BWR );
                    dr7 |= setDRn( dr+3, linear+7, DRLen( 1 ) | DR7_BWR );
                }
                dr += 4;
                break;
            default:
                return( false );
            }
        }
    } else {
        word    type;
        int     j;

        for( wp = WatchPoints, i = WatchCount; i-- > 0; wp++ ) {
            linear = wp->linear;
            size = wp->size;
            if( size == 8 )
                size = 4;
            type = DRLen( size ) | DR7_BWR;
            for( j = 0; j < wp->dregs; j++ ) {
                dr7 |= setDRn( dr, linear, type );
                dr++;
                linear += size;
            }
        }
    }

    SetDR7( dr7 );
    return( true );
#elif defined( MD_axp ) || defined( MD_ppc )
    return( false );
#else
    #error SetDebugRegs not configured
#endif
}

/*
 * CheckWatchPoints - check if a watchpoint was hit
 */
bool CheckWatchPoints( void )
{
    watch_point *wp;
    int         i;
    uint_64     value;

    for( wp = WatchPoints, i = WatchCount; i-- > 0; wp++ ) {
        value = 0;
        ReadMemory( &wp->addr, &value, wp->size );
        if( wp->value != value ) {
            return( true );
        }
    }
    return( false );
}

#if defined( MD_x86 )
static word GetDRInfo( word segment, dword offset, word size, dword *plinear )
{
    word        dregs;
    dword       linear;
    LDT_ENTRY   ldt;

    linear = offset;
    if( GetSelectorLDTEntry( segment, &ldt ) ) {
        linear += GET_LDT_BASE( ldt );
    }
    dregs = 1;
    if( size == 8 ) {
        /* QWord always needs 1 more register */
        dregs++;
        size = 4;
    }

    /* Calculate where the breakpoint should be */
    /* 1 byte breakpoint starts at where it says on the tin -   OK */
    /* 2 byte breakpoint starts at previous word offset -       OK */
    /* 4 byte breakpoint starts at previous dword offset -      OK */
    /* 8 byte breakpoint starts at previous dword offset -      OK */

    /* If we are supporting exact break on write, then don't adjust the linear address */
    if( SupportingExactBreakpoints ) {
        if( size == 1 ) {
        } else if ( size == 2 ) {
            if( linear & 1 ) {
                dregs++;        /* Need two 1 byte watches */
            }
        } else if ( size == 4 ) {
            switch( linear & 0x3 ) {
            case 0:
                                /* 0x00-0x03:   4B@0x00 */
                                /* 0x00-0x07:   4B@0x00, 4B@0x04 */
                break;
            case 1:
                dregs += 2;     /* 0x01-0x04:   1B@0x01, 2B@0x02, 1B@0x04 */
                                /* 0x01-0x08:   1B@0x01, 2B@0x02, 4B@0x04, 1B@0x08 */
                break;
            case 2:
                dregs += 1;     /* 0x02-0x05:   2B@0x02, 2B@0x04 */
                                /* 0x02-0x09:   2B@0x02, 4B@0x04, 2B@0x08 */
                break;
            case 3:
                dregs += 2;     /* 0x03-0x06:   1B@0x03, 2B@0x04, 1B@0x06 */
                                /* 0x03-0x0A:   1B@0x03, 4B@0x04, 2B@0x08, 1B@0x0A */
                break;
            }
        } else {
            /* Error */
            return( 0 );
        }
    } else {
        /*
         * This is checking if we are crossing a "size" boundary to use 2 registers.
         * We need to do the same if we are a QWord
         */
        if( linear & ( size - 1 ) ) {
            dregs++;
        }
        linear &= ~( size - 1 );
    }
    if( plinear != NULL ) {
        *plinear = linear;
    }
    return( dregs );
}
#endif

trap_retval TRAP_CORE( Set_watch )( void )
{
    set_watch_req   *acc;
    set_watch_ret   *ret;
    watch_point     *wp;

    acc = GetInPtr( 0 );
    ret = GetOutPtr( 0 );
    ret->multiplier = 100000;
    ret->err = 1;       // failure
    if( WatchCount < MAX_WATCHES && acc->size <= 8 ) {
        wp = WatchPoints + WatchCount;
        wp->addr.segment = acc->watch_addr.segment;
        wp->addr.offset = acc->watch_addr.offset;
        wp->size = acc->size;
        wp->value = 0;
        ReadMemory( &wp->addr, &wp->value, wp->size );

#if defined( MD_x86 )
        wp->dregs = GetDRInfo( wp->addr.segment, wp->addr.offset, wp->size, &wp->linear );
        if( wp->dregs == 0 )
            /* Error */
            return( sizeof( *ret ) );

        WatchCount++;
        if( DRegsCount() <= 4 ) {
            ret->multiplier |= USING_DEBUG_REG;
        }
#else
        WatchCount++;
#endif
        ret->err = 0;   // OK
    }
    return( sizeof( *ret ) );
}

trap_retval TRAP_CORE( Clear_watch )( void )
{
    clear_watch_req *acc;
    watch_point     *dst;
    watch_point     *src;
    int             i;

    acc = GetInPtr( 0 );
    dst = src = WatchPoints;
    for( i = 0; i < WatchCount; i++ ) {
        if( src->addr.segment != acc->watch_addr.segment
          || src->addr.offset != acc->watch_addr.offset ) {
            *dst = *src;
            dst++;
        }
        src++;
    }
    WatchCount--;
    return( 0 );
}
