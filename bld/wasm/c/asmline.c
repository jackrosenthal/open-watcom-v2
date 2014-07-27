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
* Description:  processing input line data and line queueing for macros
*
****************************************************************************/


#include "asmglob.h"
#include <ctype.h>

#if defined( _STANDALONE_ )

#include "directiv.h"
#include "asmalloc.h"
#include "condasm.h"
#include "asmexpnd.h"
#include "macro.h"
#include "asminput.h"
#include "pathgrp.h"
#include "iopath.h"
#include "pathlist.h"

extern int              in_prologue;

typedef struct line_list {
    struct line_list    *next;
    char                *line;
} line_list;

/* NOTE: the line queue is now a STACK of line queues
 *       when a macro is being expanded, we push a new line queue on the stack
 *       thus there is 1 queue on the stack for every level of nesting in macros
 */
typedef struct input_queue {
    struct line_list    *head;
    struct line_list    *tail;
    struct input_queue  *next;
} input_queue;

typedef struct file_list {
    struct file_list    *next;
    union {
        FILE            *file;
        struct input_queue  *lines;
    } u;
    const FNAME         *srcfile;   /* name of include file */
    unsigned long       line_num;   /* current line in parent file */
    char                is_a_file;
    bool                hidden;
} file_list;

extern void             heap( char * );

extern char             write_to_file;
extern bool             CheckSeg;
extern bool             DefineProc;             // TRUE if the definition of procedure
                                                // has not ended
extern unsigned long    PassTotal;

unsigned                BufSize;                // size of CodeBuffer

static input_queue      *line_queue  = NULL;    // line queue
static file_list        *file_stack  = NULL;    // top of included file stack
static char             *IncludePath = NULL;

#else

uint_32                 AsmCodeAddress;     // program counter

#endif

token_idx               Token_Count;        // number of tokens on line
unsigned char           *AsmCodeBuffer;     // code buffer for generated bytes

#if defined( _STANDALONE_ )

static bool get_asmline( char *ptr, unsigned max, FILE *fp )
/**********************************************************/
{
    int         quote = '\0';
    bool        skip;
    int         c;
    bool        got_something;

    /* blow away any comments -- look for ;'s */
    /* note that ;'s are ok in quoted strings */

    skip = FALSE;
    got_something = FALSE;
    for( ;; ) {
        c = getc( fp );
        /* copy the line until we hit a NULL, newline, or ; not in a quote */
        switch( c ) {
        case '\t':
        case ' ':
            if( (Options.mode & MODE_TASM) && ( quote == '\0' )
                && ( got_something == TRUE ) && ( *(ptr - 1) == '\\' ) ) {
                skip = TRUE;
            }
            break;
        case '\'':
        case '"':
        case '`':
            if( quote == '\0' ) {
                quote = c;
            } else if( c == quote ) {
                quote = '\0';
            }
            break;
        case '<':
            if( quote == '\0' ) {
                quote = '>';
            }
            break;
        case '>':
            if( c == quote ) {
                quote = '\0';
            }
            break;
        case ';':
            if( quote != '\0' ) {
                break;
            }
            skip = TRUE;
            break;
        case '\r':
            continue; /* don't store character in string */
        case '\n':
            /* if continuation character found, pass over newline */
            if( ( got_something == TRUE ) && ( *(ptr - 1) == '\\' ) ) {
                ptr--;
                max++;
                LineNumber++;
                skip = FALSE;
                continue; /* don't store character in string */
            }
            *ptr = '\0';
            // fall through
        case '\0':
            /* we have found the end of the line */
            return( TRUE );
        case EOF:
            *ptr = '\0';
            return( got_something );
        }
        if( skip == FALSE ) {
            *ptr++ = (char)c;
            if( --max <= 1 )
                skip = TRUE;
            got_something = TRUE;
        }
    }
}

void PushLineQueue( void )
/************************/
{
    input_queue *new;

    DebugMsg(( "PUSHLINEQUEUE\n" ));

    new = AsmAlloc( sizeof( input_queue ) );
    new->next = line_queue;
    new->head = new->tail = NULL;
    line_queue = new;
}

bool PopLineQueue( void )
/***********************/
/* remove a line queue from the top of the stack & throw it away */
{
    input_queue *tmp;

    /* pop the line_queue stack */
    tmp = line_queue;
    in_prologue = FALSE;
    if( tmp == NULL )
        return( FALSE );
    line_queue = line_queue->next;
    AsmFree( tmp );
    return( TRUE );
}

bool GetQueueMacroHidden( void )
/******************************/
{
    if(( file_stack != NULL ) && !file_stack->is_a_file ) {
        return( file_stack->hidden );
    } else {
        return( FALSE );
    }
}

static line_list *enqueue( void )
/*******************************/
{
    line_list   *new;

    new = AsmAlloc( sizeof( line_list ) );
    new->next = NULL;

    if( line_queue == NULL ) {
        line_queue = AsmAlloc( sizeof( input_queue ) );
        line_queue->next = NULL;
        line_queue->tail = NULL;
        line_queue->head = NULL;
    }

    if( line_queue->tail == NULL ) {
        line_queue->head = new;
        line_queue->tail = new;
    } else {
        /* insert at the tail */
        line_queue->tail->next = new;
        line_queue->tail = new;
    }
    return( new );
}

static file_list *push_flist( const char *name, bool is_a_file )
/**************************************************************/
{
    file_list   *new;

    new = AsmAlloc( sizeof( file_list ) );
    new->next = file_stack;
    file_stack = new;
    new->line_num = LineNumber;
    new->is_a_file = is_a_file;
    new->hidden = 0;
    if( !is_a_file ) {
        dir_node *dir;

        dir = (dir_node *)AsmGetSymbol( name );
        LineNumber = dir->line_num;
        new->srcfile = dir->e.macroinfo->srcfile;
    } else {
        new->srcfile = AddFlist( name );
        LineNumber = 0;
    }
    return( new );
}

const FNAME *get_curr_srcfile( void )
/***********************************/
{
    return( file_stack == NULL ? ModuleInfo.srcfile : file_stack->srcfile );
}

void print_include_file_nesting_structure( void )
/***********************************************/
{
    file_list       *tmp;
    unsigned        tab = 1;

    tmp = file_stack;
    if( tmp == NULL )
        return;

    for( ; tmp != NULL ; ) {
        if( tmp->next == NULL )
            break;
        if( !tmp->hidden ) {
            if( tmp->is_a_file ) {
                AsmNote( 0, NOTE_INCLUDED_BY, tmp->next->srcfile->name, tmp->line_num );
            } else {
                AsmNote( 0, NOTE_MACRO_CALLED_FROM, tmp->next->srcfile->name, tmp->line_num );
            }
        }
        tmp = tmp->next;
        tab++;
    }

    if( !tmp->hidden ) {
        if( tmp->is_a_file ) {
            AsmNote( 0, NOTE_INCLUDED_BY, ModuleInfo.srcfile->name, tmp->line_num );
        } else {
            AsmNote( 0, NOTE_MACRO_CALLED_FROM, ModuleInfo.srcfile->name, tmp->line_num );
        }
    }
}

void InputQueueLine( const char *line )
/*************************************/
{
    line_list   *new;

    DebugMsg(( "QUEUELINE: %s  ( line %lu ) \n", line, LineNumber ));
    new = enqueue();
    new->line = AsmStrDup( line );
}

static FILE *open_file_in_include_path( const char *name, char *fullpath )
/************************************************************************/
{
    char            *path_list;
    char            *p;
    FILE            *file = NULL;
    char            c;
    size_t          len;

    while( isspace( *name ) )
        name++;

    path_list = IncludePath;
    if( path_list != NULL ) {
        len = strlen( name ) + 1;
        while( (c = *path_list) != '\0' ) {
            p = fullpath;
            do {
                ++path_list;
                if( IS_PATH_LIST_SEP( c ) ) {
                    break;
                }
                *p++ = c;
            } while( (c = *path_list) != '\0' );
            c = p[-1];
            if( !IS_PATH_SEP( c ) ) {
                *p++ = DIR_SEP;
            }
            memcpy( p, name, len );
            file = fopen( fullpath, "r" );
            if( file != NULL ) {
                break;
            }
        }
    }
    return( file );
}

bool InputQueueFile( const char *path )
/*************************************/
{
    FILE        *file;
    file_list   *new;
    char        fullpath[ _MAX_PATH ];
    PGROUP      pg;

    _splitpath2( path, pg.buffer, &pg.drive, &pg.dir, &pg.fname, &pg.ext );
    _makepath( fullpath, pg.drive, pg.dir, pg.fname, pg.ext );
    file = fopen( fullpath, "r" );
    if( file == NULL ) {
        if( IncludePath != NULL ) {
            file = open_file_in_include_path( path, pg.buffer );
        }
        if( file == NULL ) {
            AsmErr( CANNOT_OPEN_INCLUDE_FILE, fullpath );
            return( RC_ERROR );
        }
        new = push_flist( pg.buffer, TRUE );
    } else {
        new = push_flist( path, TRUE );
    }
    new->u.file = file;
    return( RC_OK );
}

static char *input_get( char *string )
/************************************/
{
    line_list   *inputline;
    file_list   *inputfile;

    /* Check the line_queue first; then the file_stack */

    Globals.expand_count = 0;
    while( line_queue != NULL ) {
        if( line_queue->head != NULL ) {
            inputline = line_queue->head;
            strcpy( string, inputline->line );
            line_queue->head = inputline->next;
            if( line_queue->head == NULL )
                line_queue->tail = NULL;
            AsmFree( inputline->line );
            AsmFree( inputline );
            return( string );
        }
        PopLineQueue();
    }

    while( file_stack != NULL ) {
        inputfile = file_stack;
        if( inputfile->is_a_file ) {
            if( get_asmline( string, MAX_LINE_LEN, inputfile->u.file ) ) {
                LineNumber++;
                return( string );
            }
            /* EOF is reached */
            file_stack = inputfile->next;
            fclose( inputfile->u.file );
            LineNumber = inputfile->line_num;
            AsmFree( inputfile );
        } else {
            /* this "file" is just a line queue for a macro */
            inputline = inputfile->u.lines->head;
            LineNumber++;
            if( inputline != NULL ) {
                strcpy( string, inputline->line );
                inputfile->u.lines->head = inputline->next;
                AsmFree( inputline->line );
                AsmFree( inputline );
                return( string );
            }
            MacroEnd( FALSE );

            file_stack = inputfile->next;
            AsmFree( inputfile->u.lines );
            LineNumber = inputfile->line_num;
            AsmFree( inputfile );
        }
    }
    return( NULL );
}

char *ReadTextLine( char *string )
/********************************/
{
    /* get a new line, first checking the line queue & the file stack,
     * then looking in the assembly file itself
     * put it into string and return a pointer to the front
     */

    char *line;

    string[0] = '\0';
    line = input_get( string );
    if( line != NULL )
        return( line );
    if( !get_asmline( string, MAX_LINE_LEN, AsmFiles.file[ASM] ) ) {
        return( NULL );
    }
    LineNumber++;
    return( string );
}

static char *join_multiline_cmds( char *line, size_t max_len )
/************************************************************/
{
    size_t  linelen;
    size_t  i;

    linelen = strlen( line );

    /* if the last non-whitespace character is a comma, join the next line on */

    for( i = linelen; i; --i ) {
        if( !isspace( line[ i - 1 ] ) ) {
            if( line[ i - 1 ] == ',' )
                ScanLine( line + linelen, max_len - linelen );
            break;
        }
    }
    return( line );
}

char *ScanLine( char *string, size_t max_len )
/********************************************/
{
    char        *line;
    char        buffer[MAX_LINE_LEN];
    size_t      len;

    line = ( max_len < MAX_LINE_LEN ? buffer : string );
    line = ReadTextLine( line );
    if( line != NULL ) {

        prep_line_for_conditional_assembly( line );
        if( line != string ) {          /* comparing the pointers */
            len = strlen( line ) + 1;
            if( len <= max_len ) {
                memcpy( string, line, len );
            } else {
                AsmError( ASM_CODE_TOO_LONG );
                return( NULL );
            }
        }
        join_multiline_cmds( string, max_len );
    }
    return( line );
}

void AsmCodeByte( unsigned char byte )
/************************************/
{
    if( CurrSeg != NULL ) {
        if( CurrSeg->seg->e.seginfo->iscode == SEGTYPE_UNDEF ) {
            CurrSeg->seg->e.seginfo->iscode = SEGTYPE_ISCODE;
        }
    }
    OutSelect( FALSE );
    AsmByte( byte );
}

void AsmDataByte( unsigned char byte )
/************************************/
{
    OutSelect( TRUE );
    AsmByte( byte );
}

static bool CheckHaveSeg( void )
/******************************/
{
    if( CurrSeg != NULL )
        return( TRUE );

    if( CheckSeg ) {
        AsmError( DATA_EMITTED_WITH_NO_SEGMENT );
        write_to_file = FALSE;
        CheckSeg = FALSE;
    }
    return( FALSE );
}

void AddItemToIncludePath( const char *path_list, const char *end )
/*****************************************************************/
{
    size_t      len;
    size_t      old_len;
    char        *old_list;
    char        *p;

    if( end == NULL ) {
        len = strlen( path_list );
        end = path_list + len;
    } else {
        len = end - path_list;
    }
    if( len > 0 ) {
        if( IncludePath == NULL ) {
            p = IncludePath = AsmAlloc( len + 1 );
        } else {
            old_list = IncludePath;
            old_len = strlen( old_list );
            IncludePath = AsmAlloc( old_len + 1 + len + 1 );
            memcpy( IncludePath, old_list, old_len );
            AsmFree( old_list );
            p = IncludePath + old_len;
        }
        do {
            if( p != IncludePath )
                *p++ = PATH_LIST_SEP;
            path_list = GetPathElement( path_list, end, &p );
        } while( path_list != end );
        *p = '\0';
    }
}

void FreeIncludePath( void )
/**************************/
{
    AsmFree( IncludePath );
}

void PushMacro( const char *name, bool hidden )
/*********************************************/
{
    file_list *new;

    DebugMsg(( "PUSH_MACRO\n" ));
    new = push_flist( name, FALSE );
    new->u.lines = line_queue;
    new->hidden = hidden;
    line_queue = line_queue->next;
}

#ifdef DEBUG_OUT
static void dbg_output( void )
/****************************/
/* For debugging use only; print out a simplied version of the source line
   after it is parsed and the expression is evaluated */
{
    if( Options.debug ) {
        token_idx   i;

        DebugMsg(("Line: %lu ", LineNumber ));
        DebugMsg(("Output :"));
        for( i = 0; i < Token_Count; i++ ) {
            switch( AsmBuffer[i].class ) {
            case TC_NUM:
                DebugMsg(( " %d ", AsmBuffer[i].u.value ));
                break;
            case TC_STRING:
                DebugMsg(( " '%s' ", AsmBuffer[i].string_ptr));
                break;
            case TC_OP_SQ_BRACKET:
                DebugMsg(( " %s ", "[" ));
                break;
            case TC_CL_SQ_BRACKET:
                DebugMsg(( " %s ", "]" ));
                break;
            case TC_COLON:
                DebugMsg(( " %s ", ":" ));
                break;
            case TC_RES_ID:
                switch( AsmBuffer[i].u.token ) {
                case T_PTR:
                    DebugMsg(( " %s ", "Ptr" ));
                    break;
                case T_NEAR:
                    DebugMsg(( " %s ", "Near" ));
                    break;
                case T_FAR:
                    DebugMsg(( " %s ", "Far" ));
                    break;
                case T_PROC:
                    DebugMsg(( " %s ", "Proc" ));
                    break;
                case T_BYTE:
                case T_SBYTE:
                    DebugMsg(( " %s ", "Byte" ));
                    break;
                case T_WORD:
                case T_SWORD:
                    DebugMsg(( " %s ", "Word" ));
                    break;
                case T_DWORD:
                case T_SDWORD:
                    DebugMsg(( " %s ", "DWord" ));
                    break;
                case T_PWORD:
                case T_FWORD:
                    DebugMsg(( " %s ", "FWord" ));
                    break;
                case T_QWORD:
                    DebugMsg(( " %s ", "QWord" ));
                    break;
                case T_TBYTE:
                    DebugMsg(( " %s ", "TByte" ));
                    break;
                case T_OWORD:
                    DebugMsg(( " %s ", "OWord" ));
                    break;
                default:
                    DebugMsg((" %s ", AsmBuffer[i].string_ptr ));
                    break;
                }
                break;
            default:
                DebugMsg((" %s ", AsmBuffer[i].string_ptr ));
                break;
            }
        }
        DebugMsg(("\n"));
    } /* if Options.debug */
}
#endif
#endif

void AsmByte( unsigned char byte )
/********************************/
/* Write a byte to the object file */
{
#if defined( _STANDALONE_ )
    if( CheckHaveSeg() ) {
        (CurrSeg->seg->e.seginfo->current_loc)++;
        if( CurrSeg->seg->e.seginfo->current_loc >= CurrSeg->seg->e.seginfo->length ) {
            CurrSeg->seg->e.seginfo->length = CurrSeg->seg->e.seginfo->current_loc;
        }
        if( Parse_Pass != PASS_1 && write_to_file ) {
            AsmCodeBuffer[BufSize++] = byte;
            if( BufSize >= MAX_LEDATA_THRESHOLD ) {
                FlushCurrSeg();
            }
        }
    }
    PassTotal++;
#else
    AsmCodeBuffer[AsmCodeAddress] = byte;
    ++AsmCodeAddress;
#endif
}

#if defined( _STANDALONE_ )

void AsmLine( const char *string )
/********************************/
{
    token_idx       count;

    // Token_Count is the number of tokens scanned
    Token_Count = AsmScan( string );
    Token_Count = ExpandMacro( Token_Count );

    if( ExpandTheWorld( 1, TRUE, TRUE ) ) {
        write_to_file = FALSE;
        return;
    }

    if( CurrProc != NULL && !DefineProc ) {
        for( count = 0; count < Token_Count; count++ ) {
            bool expanded;
            if( ExpandProcString( count, &expanded ) ) {
                write_to_file = FALSE;
                return;
            }
            if( expanded ) {
                return;
            }
        }
    }

    if( Token_Count > 0 ) {
        if( AsmParse( string ) ) {
            write_to_file = FALSE;
        }
    } else if( Token_Count == INVALID_IDX ) {
        // syntax error
        write_to_file = FALSE;
    }
  #ifdef DEBUG_OUT
    dbg_output();               // for debuggin only
  #endif
}

#else

void AsmLine( const char *string, bool use_emu )
/**********************************************/
{
    enum fpe    old_floating_point;

    old_floating_point = floating_point;
    if( old_floating_point != NO_FP_ALLOWED ) {
        floating_point = ( use_emu ) ? DO_FP_EMULATION : NO_FP_EMULATION;
    }
    // Token_Count is the number of tokens scanned
    Token_Count = AsmScan( string );
    if( Token_Count > 0 ) {
        AsmParse( string );
    }
    floating_point = old_floating_point;
}

#endif
