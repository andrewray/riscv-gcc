/* Copyright (C) 1992, 1995, 1997, 1999, 2000, 2002, 2003, 2004
   Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Brendan Kehoe (brendan@zen.org).

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <sysdeps/unix/sysdep.h>

#ifdef __ASSEMBLER__

#include <sys/asm.h>

#define ENTRY(name) LEAF(name)

#undef END
#define	END(function)                                   \
		.end	function;		        \
		.size	function,.-function

#define ret	ret

#undef PSEUDO_END
#define PSEUDO_END(sym) .end sym; .size sym,.-sym

#define PSEUDO_NOERRNO(name, syscall_name, args)	\
  .align 2;						\
  ENTRY(name)						\
  li v0, SYS_ify(syscall_name);				\
  syscall

#undef PSEUDO_END_NOERRNO
#define PSEUDO_END_NOERRNO(sym) .end sym; .size sym,.-sym

#define ret_NOERRNO ret

#define PSEUDO_ERRVAL(name, syscall_name, args) \
  PSEUDO_NOERRNO(name, syscall_name, args)

#undef PSEUDO_END_ERRVAL
#define PSEUDO_END_ERRVAL(sym) PSEUDO_END_NOERRNO(sym)

#define ret_ERRVAL ret

#define r0	v0
#define r1	v1
#define MOVE(x,y)	move y , x

#define L(label) .L ## label

#define PSEUDO(name, syscall_name, args) \
  .align 2;							\
  99: PIC_J(__syscall_error);					\
  ENTRY(name)							\
  li v0, SYS_ify(syscall_name);					\
  syscall;							\
  bne a3, zero, 99b;						\
L(syse1):

#endif
