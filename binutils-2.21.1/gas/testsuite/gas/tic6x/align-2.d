#objdump: -dr --prefix-addresses --show-raw-insn
#name: C6X code alignment 2
#as: -mlittle-endian

.*: *file format elf32-tic6x-le


Disassembly of section \.text:
0+ <[^>]*> 00002001[ \t]+nop 2
0+4 <[^>]*> 00000001[ \t]+\|\| nop 1
0+8 <[^>]*> 00000001[ \t]+\|\| nop 1
0+c <[^>]*> 00000001[ \t]+\|\| nop 1
0+10 <[^>]*> 00000001[ \t]+\|\| nop 1
0+14 <[^>]*> 00000001[ \t]+\|\| nop 1
0+18 <[^>]*> 00000001[ \t]+\|\| nop 1
0+1c <[^>]*> 00000000[ \t]+\|\| nop 1
0+20 <[^>]*> 00004001[ \t]+nop 3
0+24 <[^>]*> 00000001[ \t]+\|\| nop 1
0+28 <[^>]*> 00000001[ \t]+\|\| nop 1
0+2c <[^>]*> 00000001[ \t]+\|\| nop 1
0+30 <[^>]*> 00000001[ \t]+\|\| nop 1
0+34 <[^>]*> 00000001[ \t]+\|\| nop 1
0+38 <[^>]*> 00000001[ \t]+\|\| nop 1
0+3c <[^>]*> 00000000[ \t]+\|\| nop 1
0+40 <[^>]*> 00006001[ \t]+nop 4
0+44 <[^>]*> 00000001[ \t]+\|\| nop 1
0+48 <[^>]*> 00000001[ \t]+\|\| nop 1
0+4c <[^>]*> 00000000[ \t]+\|\| nop 1
0+50 <[^>]*> 00008001[ \t]+nop 5
0+54 <[^>]*> 00000001[ \t]+\|\| nop 1
0+58 <[^>]*> 00000001[ \t]+\|\| nop 1
0+5c <[^>]*> 00000001[ \t]+\|\| nop 1
0+60 <[^>]*> 00000000[ \t]+\|\| nop 1
0+64 <[^>]*> 0000a001[ \t]+nop 6
0+68 <[^>]*> 00000001[ \t]+\|\| nop 1
0+6c <[^>]*> 00000001[ \t]+\|\| nop 1
0+70 <[^>]*> 00000001[ \t]+\|\| nop 1
0+74 <[^>]*> 00000001[ \t]+\|\| nop 1
0+78 <[^>]*> 00000001[ \t]+\|\| nop 1
0+7c <[^>]*> 00000000[ \t]+\|\| nop 1
0+80 <[^>]*> 00006001[ \t]+nop 4
0+84 <[^>]*> 00000001[ \t]+\|\| nop 1
0+88 <[^>]*> 00000001[ \t]+\|\| nop 1
0+8c <[^>]*> 00000000[ \t]+\|\| nop 1
0+90 <[^>]*> 00008001[ \t]+nop 5
0+94 <[^>]*> 00000001[ \t]+\|\| nop 1
0+98 <[^>]*> 00000001[ \t]+\|\| nop 1
0+9c <[^>]*> 00000001[ \t]+\|\| nop 1
0+a0 <[^>]*> 00000000[ \t]+\|\| nop 1
0+a4 <[^>]*> 0000a001[ \t]+nop 6
0+a8 <[^>]*> 00000001[ \t]+\|\| nop 1
0+ac <[^>]*> 00000001[ \t]+\|\| nop 1
0+b0 <[^>]*> 00000001[ \t]+\|\| nop 1
0+b4 <[^>]*> 00000001[ \t]+\|\| nop 1
0+b8 <[^>]*> 00000001[ \t]+\|\| nop 1
0+bc <[^>]*> 00000000[ \t]+\|\| nop 1
0+c0 <[^>]*> 00006001[ \t]+nop 4
0+c4 <[^>]*> 00000001[ \t]+\|\| nop 1
0+c8 <[^>]*> 00000001[ \t]+\|\| nop 1
0+cc <[^>]*> 00000000[ \t]+\|\| nop 1
0+d0 <[^>]*> 00008001[ \t]+nop 5
0+d4 <[^>]*> 00000001[ \t]+\|\| nop 1
0+d8 <[^>]*> 00000001[ \t]+\|\| nop 1
0+dc <[^>]*> 00000001[ \t]+\|\| nop 1
0+e0 <[^>]*> 00000000[ \t]+\|\| nop 1
0+e4 <[^>]*> 0000a001[ \t]+nop 6
0+e8 <[^>]*> 00000001[ \t]+\|\| nop 1
0+ec <[^>]*> 00000001[ \t]+\|\| nop 1
0+f0 <[^>]*> 00000001[ \t]+\|\| nop 1
0+f4 <[^>]*> 00000001[ \t]+\|\| nop 1
0+f8 <[^>]*> 00000001[ \t]+\|\| nop 1
0+fc <[^>]*> 00000000[ \t]+\|\| nop 1
0+100 <[^>]*> 00006001[ \t]+nop 4
0+104 <[^>]*> 00000001[ \t]+\|\| nop 1
0+108 <[^>]*> 00000001[ \t]+\|\| nop 1
0+10c <[^>]*> 00000000[ \t]+\|\| nop 1
0+110 <[^>]*> 00008001[ \t]+nop 5
0+114 <[^>]*> 00000001[ \t]+\|\| nop 1
0+118 <[^>]*> 00000001[ \t]+\|\| nop 1
0+11c <[^>]*> 00000001[ \t]+\|\| nop 1
0+120 <[^>]*> 00000000[ \t]+\|\| nop 1
0+124 <[^>]*> 0000a001[ \t]+nop 6
0+128 <[^>]*> 00000001[ \t]+\|\| nop 1
0+12c <[^>]*> 00000001[ \t]+\|\| nop 1
0+130 <[^>]*> 00000001[ \t]+\|\| nop 1
0+134 <[^>]*> 00000001[ \t]+\|\| nop 1
0+138 <[^>]*> 00000001[ \t]+\|\| nop 1
0+13c <[^>]*> 00000000[ \t]+\|\| nop 1
0+140 <[^>]*> 00002001[ \t]+nop 2
0+144 <[^>]*> 00000001[ \t]+\|\| nop 1
0+148 <[^>]*> 00000001[ \t]+\|\| nop 1
0+14c <[^>]*> 00000001[ \t]+\|\| nop 1
0+150 <[^>]*> 00000001[ \t]+\|\| nop 1
0+154 <[^>]*> 00000001[ \t]+\|\| nop 1
0+158 <[^>]*> 00000001[ \t]+\|\| nop 1
0+15c <[^>]*> 00000000[ \t]+\|\| nop 1
0+160 <[^>]*> 00004001[ \t]+nop 3
0+164 <[^>]*> 00000001[ \t]+\|\| nop 1
0+168 <[^>]*> 00000001[ \t]+\|\| nop 1
0+16c <[^>]*> 00000001[ \t]+\|\| nop 1
0+170 <[^>]*> 00000001[ \t]+\|\| nop 1
0+174 <[^>]*> 00000001[ \t]+\|\| nop 1
0+178 <[^>]*> 00000001[ \t]+\|\| nop 1
0+17c <[^>]*> 00000000[ \t]+\|\| nop 1
