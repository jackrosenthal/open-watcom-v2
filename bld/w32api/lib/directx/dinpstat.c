/***************************************************************************
*
*                            Open Watcom Project
*
* Copyright (c) 2004-2013 The Open Watcom Contributors. All Rights Reserved.
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
*  Description: Implementation of the statically linked portion of the
*               dinput.lib library.
*
**************************************************************************/

#include <windows.h>
#include <dinput.h>

static DIOBJECTDATAFORMAT keyboard_data[256] = {
    { &GUID_Key, 0L, 0x8000000CL, 0L },
    { &GUID_Key, 1L, 0x8000010CL, 0L },
    { &GUID_Key, 2L, 0x8000020CL, 0L },
    { &GUID_Key, 3L, 0x8000030CL, 0L },
    { &GUID_Key, 4L, 0x8000040CL, 0L },
    { &GUID_Key, 5L, 0x8000050CL, 0L },
    { &GUID_Key, 6L, 0x8000060CL, 0L },
    { &GUID_Key, 7L, 0x8000070CL, 0L },
    { &GUID_Key, 8L, 0x8000080CL, 0L },
    { &GUID_Key, 9L, 0x8000090CL, 0L },
    { &GUID_Key, 10L, 0x80000A0CL, 0L },
    { &GUID_Key, 11L, 0x80000B0CL, 0L },
    { &GUID_Key, 12L, 0x80000C0CL, 0L },
    { &GUID_Key, 13L, 0x80000D0CL, 0L },
    { &GUID_Key, 14L, 0x80000E0CL, 0L },
    { &GUID_Key, 15L, 0x80000F0CL, 0L },
    { &GUID_Key, 16L, 0x8000100CL, 0L },
    { &GUID_Key, 17L, 0x8000110CL, 0L },
    { &GUID_Key, 18L, 0x8000120CL, 0L },
    { &GUID_Key, 19L, 0x8000130CL, 0L },
    { &GUID_Key, 20L, 0x8000140CL, 0L },
    { &GUID_Key, 21L, 0x8000150CL, 0L },
    { &GUID_Key, 22L, 0x8000160CL, 0L },
    { &GUID_Key, 23L, 0x8000170CL, 0L },
    { &GUID_Key, 24L, 0x8000180CL, 0L },
    { &GUID_Key, 25L, 0x8000190CL, 0L },
    { &GUID_Key, 26L, 0x80001A0CL, 0L },
    { &GUID_Key, 27L, 0x80001B0CL, 0L },
    { &GUID_Key, 28L, 0x80001C0CL, 0L },
    { &GUID_Key, 29L, 0x80001D0CL, 0L },
    { &GUID_Key, 30L, 0x80001E0CL, 0L },
    { &GUID_Key, 31L, 0x80001F0CL, 0L },
    { &GUID_Key, 32L, 0x8000200CL, 0L },
    { &GUID_Key, 33L, 0x8000210CL, 0L },
    { &GUID_Key, 34L, 0x8000220CL, 0L },
    { &GUID_Key, 35L, 0x8000230CL, 0L },
    { &GUID_Key, 36L, 0x8000240CL, 0L },
    { &GUID_Key, 37L, 0x8000250CL, 0L },
    { &GUID_Key, 38L, 0x8000260CL, 0L },
    { &GUID_Key, 39L, 0x8000270CL, 0L },
    { &GUID_Key, 40L, 0x8000280CL, 0L },
    { &GUID_Key, 41L, 0x8000290CL, 0L },
    { &GUID_Key, 42L, 0x80002A0CL, 0L },
    { &GUID_Key, 43L, 0x80002B0CL, 0L },
    { &GUID_Key, 44L, 0x80002C0CL, 0L },
    { &GUID_Key, 45L, 0x80002D0CL, 0L },
    { &GUID_Key, 46L, 0x80002E0CL, 0L },
    { &GUID_Key, 47L, 0x80002F0CL, 0L },
    { &GUID_Key, 48L, 0x8000300CL, 0L },
    { &GUID_Key, 49L, 0x8000310CL, 0L },
    { &GUID_Key, 50L, 0x8000320CL, 0L },
    { &GUID_Key, 51L, 0x8000330CL, 0L },
    { &GUID_Key, 52L, 0x8000340CL, 0L },
    { &GUID_Key, 53L, 0x8000350CL, 0L },
    { &GUID_Key, 54L, 0x8000360CL, 0L },
    { &GUID_Key, 55L, 0x8000370CL, 0L },
    { &GUID_Key, 56L, 0x8000380CL, 0L },
    { &GUID_Key, 57L, 0x8000390CL, 0L },
    { &GUID_Key, 58L, 0x80003A0CL, 0L },
    { &GUID_Key, 59L, 0x80003B0CL, 0L },
    { &GUID_Key, 60L, 0x80003C0CL, 0L },
    { &GUID_Key, 61L, 0x80003D0CL, 0L },
    { &GUID_Key, 62L, 0x80003E0CL, 0L },
    { &GUID_Key, 63L, 0x80003F0CL, 0L },
    { &GUID_Key, 64L, 0x8000400CL, 0L },
    { &GUID_Key, 65L, 0x8000410CL, 0L },
    { &GUID_Key, 66L, 0x8000420CL, 0L },
    { &GUID_Key, 67L, 0x8000430CL, 0L },
    { &GUID_Key, 68L, 0x8000440CL, 0L },
    { &GUID_Key, 69L, 0x8000450CL, 0L },
    { &GUID_Key, 70L, 0x8000460CL, 0L },
    { &GUID_Key, 71L, 0x8000470CL, 0L },
    { &GUID_Key, 72L, 0x8000480CL, 0L },
    { &GUID_Key, 73L, 0x8000490CL, 0L },
    { &GUID_Key, 74L, 0x80004A0CL, 0L },
    { &GUID_Key, 75L, 0x80004B0CL, 0L },
    { &GUID_Key, 76L, 0x80004C0CL, 0L },
    { &GUID_Key, 77L, 0x80004D0CL, 0L },
    { &GUID_Key, 78L, 0x80004E0CL, 0L },
    { &GUID_Key, 79L, 0x80004F0CL, 0L },
    { &GUID_Key, 80L, 0x8000500CL, 0L },
    { &GUID_Key, 81L, 0x8000510CL, 0L },
    { &GUID_Key, 82L, 0x8000520CL, 0L },
    { &GUID_Key, 83L, 0x8000530CL, 0L },
    { &GUID_Key, 84L, 0x8000540CL, 0L },
    { &GUID_Key, 85L, 0x8000550CL, 0L },
    { &GUID_Key, 86L, 0x8000560CL, 0L },
    { &GUID_Key, 87L, 0x8000570CL, 0L },
    { &GUID_Key, 88L, 0x8000580CL, 0L },
    { &GUID_Key, 89L, 0x8000590CL, 0L },
    { &GUID_Key, 90L, 0x80005A0CL, 0L },
    { &GUID_Key, 91L, 0x80005B0CL, 0L },
    { &GUID_Key, 92L, 0x80005C0CL, 0L },
    { &GUID_Key, 93L, 0x80005D0CL, 0L },
    { &GUID_Key, 94L, 0x80005E0CL, 0L },
    { &GUID_Key, 95L, 0x80005F0CL, 0L },
    { &GUID_Key, 96L, 0x8000600CL, 0L },
    { &GUID_Key, 97L, 0x8000610CL, 0L },
    { &GUID_Key, 98L, 0x8000620CL, 0L },
    { &GUID_Key, 99L, 0x8000630CL, 0L },
    { &GUID_Key, 100L, 0x8000640CL, 0L },
    { &GUID_Key, 101L, 0x8000650CL, 0L },
    { &GUID_Key, 102L, 0x8000660CL, 0L },
    { &GUID_Key, 103L, 0x8000670CL, 0L },
    { &GUID_Key, 104L, 0x8000680CL, 0L },
    { &GUID_Key, 105L, 0x8000690CL, 0L },
    { &GUID_Key, 106L, 0x80006A0CL, 0L },
    { &GUID_Key, 107L, 0x80006B0CL, 0L },
    { &GUID_Key, 108L, 0x80006C0CL, 0L },
    { &GUID_Key, 109L, 0x80006D0CL, 0L },
    { &GUID_Key, 110L, 0x80006E0CL, 0L },
    { &GUID_Key, 111L, 0x80006F0CL, 0L },
    { &GUID_Key, 112L, 0x8000700CL, 0L },
    { &GUID_Key, 113L, 0x8000710CL, 0L },
    { &GUID_Key, 114L, 0x8000720CL, 0L },
    { &GUID_Key, 115L, 0x8000730CL, 0L },
    { &GUID_Key, 116L, 0x8000740CL, 0L },
    { &GUID_Key, 117L, 0x8000750CL, 0L },
    { &GUID_Key, 118L, 0x8000760CL, 0L },
    { &GUID_Key, 119L, 0x8000770CL, 0L },
    { &GUID_Key, 120L, 0x8000780CL, 0L },
    { &GUID_Key, 121L, 0x8000790CL, 0L },
    { &GUID_Key, 122L, 0x80007A0CL, 0L },
    { &GUID_Key, 123L, 0x80007B0CL, 0L },
    { &GUID_Key, 124L, 0x80007C0CL, 0L },
    { &GUID_Key, 125L, 0x80007D0CL, 0L },
    { &GUID_Key, 126L, 0x80007E0CL, 0L },
    { &GUID_Key, 127L, 0x80007F0CL, 0L },
    { &GUID_Key, 128L, 0x8000800CL, 0L },
    { &GUID_Key, 129L, 0x8000810CL, 0L },
    { &GUID_Key, 130L, 0x8000820CL, 0L },
    { &GUID_Key, 131L, 0x8000830CL, 0L },
    { &GUID_Key, 132L, 0x8000840CL, 0L },
    { &GUID_Key, 133L, 0x8000850CL, 0L },
    { &GUID_Key, 134L, 0x8000860CL, 0L },
    { &GUID_Key, 135L, 0x8000870CL, 0L },
    { &GUID_Key, 136L, 0x8000880CL, 0L },
    { &GUID_Key, 137L, 0x8000890CL, 0L },
    { &GUID_Key, 138L, 0x80008A0CL, 0L },
    { &GUID_Key, 139L, 0x80008B0CL, 0L },
    { &GUID_Key, 140L, 0x80008C0CL, 0L },
    { &GUID_Key, 141L, 0x80008D0CL, 0L },
    { &GUID_Key, 142L, 0x80008E0CL, 0L },
    { &GUID_Key, 143L, 0x80008F0CL, 0L },
    { &GUID_Key, 144L, 0x8000900CL, 0L },
    { &GUID_Key, 145L, 0x8000910CL, 0L },
    { &GUID_Key, 146L, 0x8000920CL, 0L },
    { &GUID_Key, 147L, 0x8000930CL, 0L },
    { &GUID_Key, 148L, 0x8000940CL, 0L },
    { &GUID_Key, 149L, 0x8000950CL, 0L },
    { &GUID_Key, 150L, 0x8000960CL, 0L },
    { &GUID_Key, 151L, 0x8000970CL, 0L },
    { &GUID_Key, 152L, 0x8000980CL, 0L },
    { &GUID_Key, 153L, 0x8000990CL, 0L },
    { &GUID_Key, 154L, 0x80009A0CL, 0L },
    { &GUID_Key, 155L, 0x80009B0CL, 0L },
    { &GUID_Key, 156L, 0x80009C0CL, 0L },
    { &GUID_Key, 157L, 0x80009D0CL, 0L },
    { &GUID_Key, 158L, 0x80009E0CL, 0L },
    { &GUID_Key, 159L, 0x80009F0CL, 0L },
    { &GUID_Key, 160L, 0x8000A00CL, 0L },
    { &GUID_Key, 161L, 0x8000A10CL, 0L },
    { &GUID_Key, 162L, 0x8000A20CL, 0L },
    { &GUID_Key, 163L, 0x8000A30CL, 0L },
    { &GUID_Key, 164L, 0x8000A40CL, 0L },
    { &GUID_Key, 165L, 0x8000A50CL, 0L },
    { &GUID_Key, 166L, 0x8000A60CL, 0L },
    { &GUID_Key, 167L, 0x8000A70CL, 0L },
    { &GUID_Key, 168L, 0x8000A80CL, 0L },
    { &GUID_Key, 169L, 0x8000A90CL, 0L },
    { &GUID_Key, 170L, 0x8000AA0CL, 0L },
    { &GUID_Key, 171L, 0x8000AB0CL, 0L },
    { &GUID_Key, 172L, 0x8000AC0CL, 0L },
    { &GUID_Key, 173L, 0x8000AD0CL, 0L },
    { &GUID_Key, 174L, 0x8000AE0CL, 0L },
    { &GUID_Key, 175L, 0x8000AF0CL, 0L },
    { &GUID_Key, 176L, 0x8000B00CL, 0L },
    { &GUID_Key, 177L, 0x8000B10CL, 0L },
    { &GUID_Key, 178L, 0x8000B20CL, 0L },
    { &GUID_Key, 179L, 0x8000B30CL, 0L },
    { &GUID_Key, 180L, 0x8000B40CL, 0L },
    { &GUID_Key, 181L, 0x8000B50CL, 0L },
    { &GUID_Key, 182L, 0x8000B60CL, 0L },
    { &GUID_Key, 183L, 0x8000B70CL, 0L },
    { &GUID_Key, 184L, 0x8000B80CL, 0L },
    { &GUID_Key, 185L, 0x8000B90CL, 0L },
    { &GUID_Key, 186L, 0x8000BA0CL, 0L },
    { &GUID_Key, 187L, 0x8000BB0CL, 0L },
    { &GUID_Key, 188L, 0x8000BC0CL, 0L },
    { &GUID_Key, 189L, 0x8000BD0CL, 0L },
    { &GUID_Key, 190L, 0x8000BE0CL, 0L },
    { &GUID_Key, 191L, 0x8000BF0CL, 0L },
    { &GUID_Key, 192L, 0x8000C00CL, 0L },
    { &GUID_Key, 193L, 0x8000C10CL, 0L },
    { &GUID_Key, 194L, 0x8000C20CL, 0L },
    { &GUID_Key, 195L, 0x8000C30CL, 0L },
    { &GUID_Key, 196L, 0x8000C40CL, 0L },
    { &GUID_Key, 197L, 0x8000C50CL, 0L },
    { &GUID_Key, 198L, 0x8000C60CL, 0L },
    { &GUID_Key, 199L, 0x8000C70CL, 0L },
    { &GUID_Key, 200L, 0x8000C80CL, 0L },
    { &GUID_Key, 201L, 0x8000C90CL, 0L },
    { &GUID_Key, 202L, 0x8000CA0CL, 0L },
    { &GUID_Key, 203L, 0x8000CB0CL, 0L },
    { &GUID_Key, 204L, 0x8000CC0CL, 0L },
    { &GUID_Key, 205L, 0x8000CD0CL, 0L },
    { &GUID_Key, 206L, 0x8000CE0CL, 0L },
    { &GUID_Key, 207L, 0x8000CF0CL, 0L },
    { &GUID_Key, 208L, 0x8000D00CL, 0L },
    { &GUID_Key, 209L, 0x8000D10CL, 0L },
    { &GUID_Key, 210L, 0x8000D20CL, 0L },
    { &GUID_Key, 211L, 0x8000D30CL, 0L },
    { &GUID_Key, 212L, 0x8000D40CL, 0L },
    { &GUID_Key, 213L, 0x8000D50CL, 0L },
    { &GUID_Key, 214L, 0x8000D60CL, 0L },
    { &GUID_Key, 215L, 0x8000D70CL, 0L },
    { &GUID_Key, 216L, 0x8000D80CL, 0L },
    { &GUID_Key, 217L, 0x8000D90CL, 0L },
    { &GUID_Key, 218L, 0x8000DA0CL, 0L },
    { &GUID_Key, 219L, 0x8000DB0CL, 0L },
    { &GUID_Key, 220L, 0x8000DC0CL, 0L },
    { &GUID_Key, 221L, 0x8000DD0CL, 0L },
    { &GUID_Key, 222L, 0x8000DE0CL, 0L },
    { &GUID_Key, 223L, 0x8000DF0CL, 0L },
    { &GUID_Key, 224L, 0x8000E00CL, 0L },
    { &GUID_Key, 225L, 0x8000E10CL, 0L },
    { &GUID_Key, 226L, 0x8000E20CL, 0L },
    { &GUID_Key, 227L, 0x8000E30CL, 0L },
    { &GUID_Key, 228L, 0x8000E40CL, 0L },
    { &GUID_Key, 229L, 0x8000E50CL, 0L },
    { &GUID_Key, 230L, 0x8000E60CL, 0L },
    { &GUID_Key, 231L, 0x8000E70CL, 0L },
    { &GUID_Key, 232L, 0x8000E80CL, 0L },
    { &GUID_Key, 233L, 0x8000E90CL, 0L },
    { &GUID_Key, 234L, 0x8000EA0CL, 0L },
    { &GUID_Key, 235L, 0x8000EB0CL, 0L },
    { &GUID_Key, 236L, 0x8000EC0CL, 0L },
    { &GUID_Key, 237L, 0x8000ED0CL, 0L },
    { &GUID_Key, 238L, 0x8000EE0CL, 0L },
    { &GUID_Key, 239L, 0x8000EF0CL, 0L },
    { &GUID_Key, 240L, 0x8000F00CL, 0L },
    { &GUID_Key, 241L, 0x8000F10CL, 0L },
    { &GUID_Key, 242L, 0x8000F20CL, 0L },
    { &GUID_Key, 243L, 0x8000F30CL, 0L },
    { &GUID_Key, 244L, 0x8000F40CL, 0L },
    { &GUID_Key, 245L, 0x8000F50CL, 0L },
    { &GUID_Key, 246L, 0x8000F60CL, 0L },
    { &GUID_Key, 247L, 0x8000F70CL, 0L },
    { &GUID_Key, 248L, 0x8000F80CL, 0L },
    { &GUID_Key, 249L, 0x8000F90CL, 0L },
    { &GUID_Key, 250L, 0x8000FA0CL, 0L },
    { &GUID_Key, 251L, 0x8000FB0CL, 0L },
    { &GUID_Key, 252L, 0x8000FC0CL, 0L },
    { &GUID_Key, 253L, 0x8000FD0CL, 0L },
    { &GUID_Key, 254L, 0x8000FE0CL, 0L },
    { &GUID_Key, 255L, 0x8000FF0CL, 0L }
};

static DIOBJECTDATAFORMAT mouse_data[7] = {
    { &GUID_XAxis, 0L, 0x00FFFF03L, 0L },
    { &GUID_YAxis, 4L, 0x00FFFF03L, 0L },
    { &GUID_ZAxis, 8L, 0x80FFFF03L, 0L },
    { NULL, 12L, 0x00FFFF0CL, 0L },
    { NULL, 13L, 0x00FFFF0CL, 0L },
    { NULL, 14L, 0x80FFFF0CL, 0L },
    { NULL, 15L, 0x80FFFF0CL, 0L }
};

static DIOBJECTDATAFORMAT joystick_data[44] = {
    { &GUID_XAxis, 0L, 0x80FFFF03L, 256L },
    { &GUID_YAxis, 4L, 0x80FFFF03L, 256L },
    { &GUID_ZAxis, 8L, 0x80FFFF03L, 256L },
    { &GUID_RxAxis, 12L, 0x80FFFF03L, 256L },
    { &GUID_RyAxis, 16L, 0x80FFFF03L, 256L },
    { &GUID_RzAxis, 20L, 0x80FFFF03L, 256L },
    { &GUID_Slider, 24L, 0x80FFFF03L, 256L },
    { &GUID_Slider, 28L, 0x80FFFF03L, 256L },
    { &GUID_POV, 32L, 0x80FFFF10L, 0L },
    { &GUID_POV, 36L, 0x80FFFF10L, 0L },
    { &GUID_POV, 40L, 0x80FFFF10L, 0L },
    { &GUID_POV, 44L, 0x80FFFF10L, 0L },
    { NULL, 48L, 0x80FFFF0CL, 0L },
    { NULL, 49L, 0x80FFFF0CL, 0L },
    { NULL, 50L, 0x80FFFF0CL, 0L },
    { NULL, 51L, 0x80FFFF0CL, 0L },
    { NULL, 52L, 0x80FFFF0CL, 0L },
    { NULL, 53L, 0x80FFFF0CL, 0L },
    { NULL, 54L, 0x80FFFF0CL, 0L },
    { NULL, 55L, 0x80FFFF0CL, 0L },
    { NULL, 56L, 0x80FFFF0CL, 0L },
    { NULL, 57L, 0x80FFFF0CL, 0L },
    { NULL, 58L, 0x80FFFF0CL, 0L },
    { NULL, 59L, 0x80FFFF0CL, 0L },
    { NULL, 60L, 0x80FFFF0CL, 0L },
    { NULL, 61L, 0x80FFFF0CL, 0L },
    { NULL, 62L, 0x80FFFF0CL, 0L },
    { NULL, 63L, 0x80FFFF0CL, 0L },
    { NULL, 64L, 0x80FFFF0CL, 0L },
    { NULL, 65L, 0x80FFFF0CL, 0L },
    { NULL, 66L, 0x80FFFF0CL, 0L },
    { NULL, 67L, 0x80FFFF0CL, 0L },
    { NULL, 68L, 0x80FFFF0CL, 0L },
    { NULL, 69L, 0x80FFFF0CL, 0L },
    { NULL, 70L, 0x80FFFF0CL, 0L },
    { NULL, 71L, 0x80FFFF0CL, 0L },
    { NULL, 72L, 0x80FFFF0CL, 0L },
    { NULL, 73L, 0x80FFFF0CL, 0L },
    { NULL, 74L, 0x80FFFF0CL, 0L },
    { NULL, 75L, 0x80FFFF0CL, 0L },
    { NULL, 76L, 0x80FFFF0CL, 0L },
    { NULL, 77L, 0x80FFFF0CL, 0L },
    { NULL, 78L, 0x80FFFF0CL, 0L },
    { NULL, 79L, 0x80FFFF0CL, 0L },
};

static DIOBJECTDATAFORMAT joystick2_data[164] = {
    { &GUID_XAxis, 0L, 0x80FFFF03L, 256L },
    { &GUID_YAxis, 4L, 0x80FFFF03L, 256L },
    { &GUID_ZAxis, 8L, 0x80FFFF03L, 256L },
    { &GUID_RxAxis, 12L, 0x80FFFF03L, 256L },
    { &GUID_RyAxis, 16L, 0x80FFFF03L, 256L },
    { &GUID_RzAxis, 20L, 0x80FFFF03L, 256L },
    { &GUID_Slider, 24L, 0x80FFFF03L, 256L },
    { &GUID_Slider, 28L, 0x80FFFF03L, 256L },
    { &GUID_POV, 32L, 0x80FFFF10L, 0L },
    { &GUID_POV, 36L, 0x80FFFF10L, 0L },
    { &GUID_POV, 40L, 0x80FFFF10L, 0L },
    { &GUID_POV, 44L, 0x80FFFF10L, 0L },
    { NULL, 48L, 0x80FFFF0CL, 0L },
    { NULL, 49L, 0x80FFFF0CL, 0L },
    { NULL, 50L, 0x80FFFF0CL, 0L },
    { NULL, 51L, 0x80FFFF0CL, 0L },
    { NULL, 52L, 0x80FFFF0CL, 0L },
    { NULL, 53L, 0x80FFFF0CL, 0L },
    { NULL, 54L, 0x80FFFF0CL, 0L },
    { NULL, 55L, 0x80FFFF0CL, 0L },
    { NULL, 56L, 0x80FFFF0CL, 0L },
    { NULL, 57L, 0x80FFFF0CL, 0L },
    { NULL, 58L, 0x80FFFF0CL, 0L },
    { NULL, 59L, 0x80FFFF0CL, 0L },
    { NULL, 60L, 0x80FFFF0CL, 0L },
    { NULL, 61L, 0x80FFFF0CL, 0L },
    { NULL, 62L, 0x80FFFF0CL, 0L },
    { NULL, 63L, 0x80FFFF0CL, 0L },
    { NULL, 64L, 0x80FFFF0CL, 0L },
    { NULL, 65L, 0x80FFFF0CL, 0L },
    { NULL, 66L, 0x80FFFF0CL, 0L },
    { NULL, 67L, 0x80FFFF0CL, 0L },
    { NULL, 68L, 0x80FFFF0CL, 0L },
    { NULL, 69L, 0x80FFFF0CL, 0L },
    { NULL, 70L, 0x80FFFF0CL, 0L },
    { NULL, 71L, 0x80FFFF0CL, 0L },
    { NULL, 72L, 0x80FFFF0CL, 0L },
    { NULL, 73L, 0x80FFFF0CL, 0L },
    { NULL, 74L, 0x80FFFF0CL, 0L },
    { NULL, 75L, 0x80FFFF0CL, 0L },
    { NULL, 76L, 0x80FFFF0CL, 0L },
    { NULL, 77L, 0x80FFFF0CL, 0L },
    { NULL, 78L, 0x80FFFF0CL, 0L },
    { NULL, 79L, 0x80FFFF0CL, 0L },
    { NULL, 80L, 0x80FFFF0CL, 0L },
    { NULL, 81L, 0x80FFFF0CL, 0L },
    { NULL, 82L, 0x80FFFF0CL, 0L },
    { NULL, 83L, 0x80FFFF0CL, 0L },
    { NULL, 84L, 0x80FFFF0CL, 0L },
    { NULL, 85L, 0x80FFFF0CL, 0L },
    { NULL, 86L, 0x80FFFF0CL, 0L },
    { NULL, 87L, 0x80FFFF0CL, 0L },
    { NULL, 88L, 0x80FFFF0CL, 0L },
    { NULL, 89L, 0x80FFFF0CL, 0L },
    { NULL, 90L, 0x80FFFF0CL, 0L },
    { NULL, 91L, 0x80FFFF0CL, 0L },
    { NULL, 92L, 0x80FFFF0CL, 0L },
    { NULL, 93L, 0x80FFFF0CL, 0L },
    { NULL, 94L, 0x80FFFF0CL, 0L },
    { NULL, 95L, 0x80FFFF0CL, 0L },
    { NULL, 96L, 0x80FFFF0CL, 0L },
    { NULL, 97L, 0x80FFFF0CL, 0L },
    { NULL, 98L, 0x80FFFF0CL, 0L },
    { NULL, 99L, 0x80FFFF0CL, 0L },
    { NULL, 100L, 0x80FFFF0CL, 0L },
    { NULL, 101L, 0x80FFFF0CL, 0L },
    { NULL, 102L, 0x80FFFF0CL, 0L },
    { NULL, 103L, 0x80FFFF0CL, 0L },
    { NULL, 104L, 0x80FFFF0CL, 0L },
    { NULL, 105L, 0x80FFFF0CL, 0L },
    { NULL, 106L, 0x80FFFF0CL, 0L },
    { NULL, 107L, 0x80FFFF0CL, 0L },
    { NULL, 108L, 0x80FFFF0CL, 0L },
    { NULL, 109L, 0x80FFFF0CL, 0L },
    { NULL, 110L, 0x80FFFF0CL, 0L },
    { NULL, 111L, 0x80FFFF0CL, 0L },
    { NULL, 112L, 0x80FFFF0CL, 0L },
    { NULL, 113L, 0x80FFFF0CL, 0L },
    { NULL, 114L, 0x80FFFF0CL, 0L },
    { NULL, 115L, 0x80FFFF0CL, 0L },
    { NULL, 116L, 0x80FFFF0CL, 0L },
    { NULL, 117L, 0x80FFFF0CL, 0L },
    { NULL, 118L, 0x80FFFF0CL, 0L },
    { NULL, 119L, 0x80FFFF0CL, 0L },
    { NULL, 120L, 0x80FFFF0CL, 0L },
    { NULL, 121L, 0x80FFFF0CL, 0L },
    { NULL, 122L, 0x80FFFF0CL, 0L },
    { NULL, 123L, 0x80FFFF0CL, 0L },
    { NULL, 124L, 0x80FFFF0CL, 0L },
    { NULL, 125L, 0x80FFFF0CL, 0L },
    { NULL, 126L, 0x80FFFF0CL, 0L },
    { NULL, 127L, 0x80FFFF0CL, 0L },
    { NULL, 128L, 0x80FFFF0CL, 0L },
    { NULL, 129L, 0x80FFFF0CL, 0L },
    { NULL, 130L, 0x80FFFF0CL, 0L },
    { NULL, 131L, 0x80FFFF0CL, 0L },
    { NULL, 132L, 0x80FFFF0CL, 0L },
    { NULL, 133L, 0x80FFFF0CL, 0L },
    { NULL, 134L, 0x80FFFF0CL, 0L },
    { NULL, 135L, 0x80FFFF0CL, 0L },
    { NULL, 136L, 0x80FFFF0CL, 0L },
    { NULL, 137L, 0x80FFFF0CL, 0L },
    { NULL, 138L, 0x80FFFF0CL, 0L },
    { NULL, 139L, 0x80FFFF0CL, 0L },
    { NULL, 140L, 0x80FFFF0CL, 0L },
    { NULL, 141L, 0x80FFFF0CL, 0L },
    { NULL, 142L, 0x80FFFF0CL, 0L },
    { NULL, 143L, 0x80FFFF0CL, 0L },
    { NULL, 144L, 0x80FFFF0CL, 0L },
    { NULL, 145L, 0x80FFFF0CL, 0L },
    { NULL, 146L, 0x80FFFF0CL, 0L },
    { NULL, 147L, 0x80FFFF0CL, 0L },
    { NULL, 148L, 0x80FFFF0CL, 0L },
    { NULL, 149L, 0x80FFFF0CL, 0L },
    { NULL, 150L, 0x80FFFF0CL, 0L },
    { NULL, 151L, 0x80FFFF0CL, 0L },
    { NULL, 152L, 0x80FFFF0CL, 0L },
    { NULL, 153L, 0x80FFFF0CL, 0L },
    { NULL, 154L, 0x80FFFF0CL, 0L },
    { NULL, 155L, 0x80FFFF0CL, 0L },
    { NULL, 156L, 0x80FFFF0CL, 0L },
    { NULL, 157L, 0x80FFFF0CL, 0L },
    { NULL, 158L, 0x80FFFF0CL, 0L },
    { NULL, 159L, 0x80FFFF0CL, 0L },
    { NULL, 160L, 0x80FFFF0CL, 0L },
    { NULL, 161L, 0x80FFFF0CL, 0L },
    { NULL, 162L, 0x80FFFF0CL, 0L },
    { NULL, 163L, 0x80FFFF0CL, 0L },
    { NULL, 164L, 0x80FFFF0CL, 0L },
    { NULL, 165L, 0x80FFFF0CL, 0L },
    { NULL, 166L, 0x80FFFF0CL, 0L },
    { NULL, 167L, 0x80FFFF0CL, 0L },
    { NULL, 168L, 0x80FFFF0CL, 0L },
    { NULL, 169L, 0x80FFFF0CL, 0L },
    { NULL, 170L, 0x80FFFF0CL, 0L },
    { NULL, 171L, 0x80FFFF0CL, 0L },
    { NULL, 172L, 0x80FFFF0CL, 0L },
    { NULL, 173L, 0x80FFFF0CL, 0L },
    { NULL, 174L, 0x80FFFF0CL, 0L },
    { NULL, 175L, 0x80FFFF0CL, 0L },
    { &GUID_XAxis, 176L, 0x80FFFF03L, 512L },
    { &GUID_YAxis, 180L, 0x80FFFF03L, 512L },
    { &GUID_ZAxis, 184L, 0x80FFFF03L, 512L },
    { &GUID_RxAxis, 188L, 0x80FFFF03L, 512L },
    { &GUID_RyAxis, 192L, 0x80FFFF03L, 512L },
    { &GUID_RzAxis, 196L, 0x80FFFF03L, 512L },
    { &GUID_Slider, 24L, 0x80FFFF03L, 512L },
    { &GUID_Slider, 28L, 0x80FFFF03L, 512L },
    { &GUID_XAxis, 208L, 0x80FFFF03L, 768L },
    { &GUID_YAxis, 212L, 0x80FFFF03L, 768L },
    { &GUID_ZAxis, 216L, 0x80FFFF03L, 768L },
    { &GUID_RxAxis, 220L, 0x80FFFF03L, 768L },
    { &GUID_RyAxis, 224L, 0x80FFFF03L, 768L },
    { &GUID_RzAxis, 228L, 0x80FFFF03L, 768L },
    { &GUID_Slider, 24L, 0x80FFFF03L, 768L },
    { &GUID_Slider, 28L, 0x80FFFF03L, 768L },
    { &GUID_XAxis, 240L, 0x80FFFF03L, 1024L },
    { &GUID_YAxis, 244L, 0x80FFFF03L, 1024L },
    { &GUID_ZAxis, 248L, 0x80FFFF03L, 1024L },
    { &GUID_RxAxis, 252L, 0x80FFFF03L, 1024L },
    { &GUID_RyAxis, 256L, 0x80FFFF03L, 1024L },
    { &GUID_RzAxis, 260L, 0x80FFFF03L, 1024L },
    { &GUID_Slider, 24L, 0x80FFFF03L, 1024L },
    { &GUID_Slider, 28L, 0x80FFFF03L, 1024L }
};

static DIOBJECTDATAFORMAT mouse2_data[11] = {
    { &GUID_XAxis, 0L, 0x00FFFF03L, 0L },
    { &GUID_YAxis, 4L, 0x00FFFF03L, 0L },
    { &GUID_ZAxis, 8L, 0x80FFFF03L, 0L },
    { NULL, 12L, 0x00FFFF0CL, 0L },
    { NULL, 13L, 0x00FFFF0CL, 0L },
    { NULL, 14L, 0x80FFFF0CL, 0L },
    { NULL, 15L, 0x80FFFF0CL, 0L },
    { NULL, 16L, 0x80FFFF0CL, 0L },
    { NULL, 17L, 0x80FFFF0CL, 0L },
    { NULL, 18L, 0x80FFFF0CL, 0L },
    { NULL, 19L, 0x80FFFF0CL, 0L },
};

const DIDATAFORMAT c_dfDIKeyboard = { 24L, 16L, 2L, 256L, 256L, &keyboard_data };
const DIDATAFORMAT c_dfDIMouse = { 24L, 16L, 2L, 16L, 7L, &mouse_data };
const DIDATAFORMAT c_dfDIJoystick = { 24L, 16L, 1L, 80L, 44L, &joystick_data };
const DIDATAFORMAT c_dfDIJoystick2 = { 24L, 16L, 1L, 272L, 164L, &joystick2_data };
const DIDATAFORMAT c_dfDIMouse2 = { 24L, 16L, 2L, 20L, 11L, &mouse2_data };
