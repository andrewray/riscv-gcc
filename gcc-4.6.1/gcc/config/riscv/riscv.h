/* Definitions of target machine for GNU compiler.  MIPS version.
   Copyright (C) 1989, 1990, 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998
   1999, 2000, 2001, 2002, 2003, 2004, 2005, 2007, 2008, 2009, 2010, 2011
   Free Software Foundation, Inc.
   Contributed by A. Lichnewsky (lich@inria.inria.fr).
   Changed by Michael Meissner	(meissner@osf.org).
   64-bit r4000 support by Ian Lance Taylor (ian@cygnus.com) and
   Brendan Eich (brendan@microunity.com).

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */


#include "config/vxworks-dummy.h"

#ifdef GENERATOR_FILE
/* This is used in some insn conditions, so needs to be declared, but
   does not need to be defined.  */
extern int target_flags_explicit;
#endif

/* MIPS external variables defined in mips.c.  */

/* Which ABI to use. */

#define ABI_32  1
#define ABI_64  3

/* Information about one recognized processor.  Defined here for the
   benefit of TARGET_CPU_CPP_BUILTINS.  */
struct mips_cpu_info {
  /* The 'canonical' name of the processor as far as GCC is concerned.
     It's typically a manufacturer's prefix followed by a numerical
     designation.  It should be lowercase.  */
  const char *name;

  /* The internal processor number that most closely matches this
     entry.  Several processors can have the same value, if there's no
     difference between them from GCC's point of view.  */
  enum processor cpu;

  /* A mask of PTF_* values.  */
  unsigned int tune_flags;
};

/* Macros to silence warnings about numbers being signed in traditional
   C and unsigned in ISO C when compiled on 32-bit hosts.  */

#define BITMASK_HIGH	(((unsigned long)1) << 31)	/* 0x80000000 */
#define BITMASK_UPPER16	((unsigned long)0xffff << 16)	/* 0xffff0000 */
#define BITMASK_LOWER16	((unsigned long)0xffff)		/* 0x0000ffff */

/* True if we need to use a global offset table to access some symbols.  */
#define TARGET_USE_GOT TARGET_ABICALLS

/* True if .gpword or .gpdword should be used for switch tables. */
#define TARGET_GPWORD (TARGET_ABICALLS && flag_pic)

/* True if the output must have a writable .eh_frame.
   See ASM_PREFERRED_EH_DATA_FORMAT for details.  */
#ifdef HAVE_LD_PERSONALITY_RELAXATION
#define TARGET_WRITABLE_EH_FRAME 0
#else
#define TARGET_WRITABLE_EH_FRAME flag_pic
#endif

/* TARGET_HARD_FLOAT and TARGET_SOFT_FLOAT reflect whether the FPU is
   directly accessible, while the command-line options select
   TARGET_HARD_FLOAT_ABI and TARGET_SOFT_FLOAT_ABI to reflect the ABI
   in use.  */
#define TARGET_HARD_FLOAT TARGET_HARD_FLOAT_ABI
#define TARGET_SOFT_FLOAT TARGET_SOFT_FLOAT_ABI

/* Target CPU builtins.  */
#define TARGET_CPU_CPP_BUILTINS()					\
  do									\
    {									\
      builtin_assert ("machine=riscv");                        	        \
									\
      builtin_assert ("cpu=riscv");					\
      builtin_define ("__riscv__");     				\
      builtin_define ("__riscv");     					\
      builtin_define ("_riscv");					\
									\
      if (TARGET_64BIT)							\
	builtin_define ("__riscv64");					\
									\
      builtin_define ("_ABI32=1");					\
      builtin_define ("_ABI64=3");					\
									\
      switch (mips_abi)							\
	{								\
	case ABI_32:							\
	  builtin_define ("_RISCV_SIM=_ABI32");			        \
	  break;							\
									\
	case ABI_64:							\
	  builtin_define ("_RISCV_SIM=_ABI64");				\
	  break;							\
	}								\
									\
      builtin_define_with_int_value ("_RISCV_SZINT", INT_TYPE_SIZE);	\
      builtin_define_with_int_value ("_RISCV_SZLONG", LONG_TYPE_SIZE);	\
      builtin_define_with_int_value ("_RISCV_SZPTR", POINTER_SIZE);	\
      builtin_define_with_int_value ("_RISCV_FPSET", 32);		\
									\
      /* These defines reflect the ABI in use, not whether the  	\
	 FPU is directly accessible.  */				\
      if (TARGET_HARD_FLOAT_ABI)					\
	builtin_define ("__riscv_hard_float");				\
      else								\
	builtin_define ("__riscv_soft_float");				\
									\
      if (TARGET_BIG_ENDIAN)						\
	{								\
	  builtin_define_std ("RISCVEB");				\
	  builtin_define ("_RISCVEB");					\
	}								\
      else								\
	{								\
	  builtin_define_std ("RISCVEL");				\
	  builtin_define ("_RISCVEL");					\
	}								\
                                                                        \
      /* Whether calls should go through $25.  The separate __PIC__	\
	 macro indicates whether abicalls code might use a GOT.  */	\
      if (TARGET_ABICALLS)						\
	builtin_define ("__mips_abicalls");				\
									\
      /* Macros dependent on the C dialect.  */				\
      if (preprocessing_asm_p ())					\
	{								\
	  builtin_define_std ("LANGUAGE_ASSEMBLY");			\
	  builtin_define ("_LANGUAGE_ASSEMBLY");			\
	}								\
      else if (c_dialect_cxx ())					\
	{								\
	  builtin_define ("_LANGUAGE_C_PLUS_PLUS");			\
	  builtin_define ("__LANGUAGE_C_PLUS_PLUS");			\
	  builtin_define ("__LANGUAGE_C_PLUS_PLUS__");			\
	}								\
      else								\
	{								\
	  builtin_define_std ("LANGUAGE_C");				\
	  builtin_define ("_LANGUAGE_C");				\
	}								\
      if (c_dialect_objc ())						\
	{								\
	  builtin_define ("_LANGUAGE_OBJECTIVE_C");			\
	  builtin_define ("__LANGUAGE_OBJECTIVE_C");			\
	  /* Bizarre, but needed at least for Irix.  */			\
	  builtin_define_std ("LANGUAGE_C");				\
	  builtin_define ("_LANGUAGE_C");				\
	}								\
    }									\
  while (0)

/* Default target_flags if no switches are specified  */

#ifndef TARGET_DEFAULT
#define TARGET_DEFAULT 0
#endif

#ifndef TARGET_CPU_DEFAULT
#define TARGET_CPU_DEFAULT 0
#endif

#ifndef TARGET_ENDIAN_DEFAULT
#define TARGET_ENDIAN_DEFAULT 0
#endif

#ifndef MIPS_CPU_STRING_DEFAULT
#define MIPS_CPU_STRING_DEFAULT "rocket"
#endif

#define TARGET_LIBGCC_SDATA_SECTION ".sdata"

#ifndef MULTILIB_ENDIAN_DEFAULT
#if TARGET_ENDIAN_DEFAULT == 0
#define MULTILIB_ENDIAN_DEFAULT "EL"
#else
#define MULTILIB_ENDIAN_DEFAULT "EB"
#endif
#endif

#ifndef MIPS_ABI_DEFAULT
#define MIPS_ABI_DEFAULT ABI_64
#endif

#if MIPS_ABI_DEFAULT == ABI_32
#define MULTILIB_ABI_DEFAULT "mabi=32"
#define MULTILIB_ISA_DEFAULT "rv32"
#endif

#if MIPS_ABI_DEFAULT == ABI_64
#define MULTILIB_ABI_DEFAULT "mabi=64"
#define MULTILIB_ISA_DEFAULT "rv64"
#endif

#ifndef MULTILIB_DEFAULTS
#define MULTILIB_DEFAULTS \
    { MULTILIB_ENDIAN_DEFAULT, MULTILIB_ISA_DEFAULT, MULTILIB_ABI_DEFAULT }
#endif

/* We must pass -EL to the linker by default for little endian embedded
   targets using linker scripts with a OUTPUT_FORMAT line.  Otherwise, the
   linker will default to using big-endian output files.  The OUTPUT_FORMAT
   line must be in the linker script, otherwise -EB/-EL will not work.  */

#ifndef ENDIAN_SPEC
#if TARGET_ENDIAN_DEFAULT == 0
#define ENDIAN_SPEC "%{!EB:%{!meb:-EL}} %{EB|meb:-EB}"
#else
#define ENDIAN_SPEC "%{!EL:%{!mel:-EB}} %{EL|mel:-EL}"
#endif
#endif

/* A spec condition that matches all non-mips16 -mips arguments.  */

#define MIPS_ISA_LEVEL_OPTION_SPEC \
  "mips1|mips2|mips3|mips4|mips32*|mips64*"

/* A spec condition that matches all non-mips16 architecture arguments.  */

#define MIPS_ARCH_OPTION_SPEC \
  MIPS_ISA_LEVEL_OPTION_SPEC "|march=*"

/* A spec that infers a -mips argument from an -march argument,
   or injects the default if no architecture is specified.  */

#define MIPS_ISA_LEVEL_SPEC \
  "%{" MIPS_ISA_LEVEL_OPTION_SPEC ":;: \
     %{march=mips1|march=r2000|march=r3000|march=r3900:-mips1} \
     %{march=mips2|march=r6000:-mips2} \
     %{march=mips3|march=r4*|march=vr4*|march=orion|march=loongson2*:-mips3} \
     %{march=mips4|march=r8000|march=vr5*|march=rm7000|march=rm9000 \
       |march=r10000|march=r12000|march=r14000|march=r16000:-mips4} \
     %{march=mips32|march=4kc|march=4km|march=4kp|march=4ksc:-mips32} \
     %{march=mips32r2|march=m4k|march=4ke*|march=4ksd|march=24k* \
       |march=34k*|march=74k*|march=1004k*: -mips32r2} \
     %{march=mips64|march=5k*|march=20k*|march=sb1*|march=sr71000 \
       |march=xlr|march=loongson3a: -mips64} \
     %{march=mips64r2|march=octeon: -mips64r2} \
     %{!march=*: -" MULTILIB_ISA_DEFAULT "}}"

/* A spec that infers a -mhard-float or -msoft-float setting from an
   -march argument.  Note that soft-float and hard-float code are not
   link-compatible.  */

#define MIPS_ARCH_FLOAT_SPEC \
  "%{mhard-float|msoft-float|march=mips*:; \
     march=vr41*|march=m4k|march=4k*|march=24kc|march=24kec \
     |march=34kc|march=74kc|march=1004kc|march=5kc \
     |march=octeon|march=xlr: -msoft-float;		  \
     march=*: -mhard-float}"

#define OPT_ARCH64 "mabi=32:;"
#define OPT_ARCH32 "mabi=32"

/* Support for a compile-time default CPU, et cetera.  The rules are:
   --with-arch is ignored if -march is specified or a -mips is specified
     (other than -mips16); likewise --with-arch-32 and --with-arch-64.
   --with-tune is ignored if -mtune is specified; likewise
     --with-tune-32 and --with-tune-64.
   --with-abi is ignored if -mabi is specified.
   --with-float is ignored if -mhard-float or -msoft-float are
     specified.
   --with-divide is ignored if -mdivide-traps or -mdivide-breaks are
     specified. */
#define OPTION_DEFAULT_SPECS \
  {"arch", "%{" MIPS_ARCH_OPTION_SPEC ":;: -march=%(VALUE)}" }, \
  {"arch_32", "%{" OPT_ARCH32 ":%{" MIPS_ARCH_OPTION_SPEC ":;: -march=%(VALUE)}}" }, \
  {"arch_64", "%{" OPT_ARCH64 ":%{" MIPS_ARCH_OPTION_SPEC ":;: -march=%(VALUE)}}" }, \
  {"tune", "%{!mtune=*:-mtune=%(VALUE)}" }, \
  {"tune_32", "%{" OPT_ARCH32 ":%{!mtune=*:-mtune=%(VALUE)}}" }, \
  {"tune_64", "%{" OPT_ARCH64 ":%{!mtune=*:-mtune=%(VALUE)}}" }, \
  {"abi", "%{!mabi=*:-mabi=%(VALUE)}" }, \
  {"float", "%{!msoft-float:%{!mhard-float:-m%(VALUE)-float}}" }, \

#define DRIVER_SELF_SPECS ""

#define TARGET_64BIT	(mips_abi == ABI_64)

#ifdef IN_LIBGCC2
#undef TARGET_64BIT
/* Make this compile time constant for libgcc2 */
#ifdef __riscv64
#define TARGET_64BIT		1
#else
#define TARGET_64BIT		0
#endif
#endif /* IN_LIBGCC2 */

/* Tell collect what flags to pass to nm.  */
#ifndef NM_FLAGS
#define NM_FLAGS "-Bn"
#endif

/* SUBTARGET_ASM_DEBUGGING_SPEC handles passing debugging options to
   the assembler.  It may be overridden by subtargets.

   Beginning with gas 2.13, -mdebug must be passed to correctly handle
   COFF debugging info.  */

#ifndef SUBTARGET_ASM_DEBUGGING_SPEC
#define SUBTARGET_ASM_DEBUGGING_SPEC "\
%{g} %{g0} %{g1} %{g2} %{g3} \
%{ggdb:-g} %{ggdb0:-g0} %{ggdb1:-g1} %{ggdb2:-g2} %{ggdb3:-g3} \
%{gstabs:-g} %{gstabs0:-g0} %{gstabs1:-g1} %{gstabs2:-g2} %{gstabs3:-g3} \
%{gstabs+:-g} %{gstabs+0:-g0} %{gstabs+1:-g1} %{gstabs+2:-g2} %{gstabs+3:-g3}"
#endif

/* SUBTARGET_ASM_SPEC is always passed to the assembler.  It may be
   overridden by subtargets.  */

#ifndef SUBTARGET_ASM_SPEC
#define SUBTARGET_ASM_SPEC ""
#endif

#undef ASM_SPEC
#define ASM_SPEC "\
%{G*} %(endian_spec) \
%(subtarget_asm_optimizing_spec) \
%(subtarget_asm_debugging_spec) \
%{mabi=*} %{!mabi=*: %(asm_abi_default_spec)} \
%{march=*} \
%{mshared} %{mno-shared} \
%{mtune=*} \
%(subtarget_asm_spec)"

/* Extra switches sometimes passed to the linker.  */

#ifndef LINK_SPEC
#define LINK_SPEC "\
%{!T:-dT riscv.ld} \
%(endian_spec) \
%{mabi=64:-melf64%{EB:b}%{!EB:l}riscv} \
%{mabi=32:-melf32%{EB:b}%{!EB:l}riscv} \
%{G*} %{mips1} %{mips2} %{mips3} %{mips4} %{mips32*} %{mips64*} \
%{shared}"
#endif  /* LINK_SPEC defined */


/* Specs for the compiler proper */

/* SUBTARGET_CC1_SPEC is passed to the compiler proper.  It may be
   overridden by subtargets.  */
#ifndef SUBTARGET_CC1_SPEC
#define SUBTARGET_CC1_SPEC ""
#endif

/* CC1_SPEC is the set of arguments to pass to the compiler proper.  */

#undef CC1_SPEC
#define CC1_SPEC "\
%{G*} %{EB:-meb} %{EL:-mel} %{EB:%{EL:%emay not use both -EB and -EL}} \
%(subtarget_cc1_spec)"

/* Preprocessor specs.  */

/* SUBTARGET_CPP_SPEC is passed to the preprocessor.  It may be
   overridden by subtargets.  */
#ifndef SUBTARGET_CPP_SPEC
#define SUBTARGET_CPP_SPEC ""
#endif

#define CPP_SPEC "%(subtarget_cpp_spec)"

/* This macro defines names of additional specifications to put in the specs
   that can be used in various specifications like CC1_SPEC.  Its definition
   is an initializer with a subgrouping for each command option.

   Each subgrouping contains a string constant, that defines the
   specification name, and a string constant that used by the GCC driver
   program.

   Do not define this macro if it does not need to do anything.  */

#define EXTRA_SPECS							\
  { "subtarget_cc1_spec", SUBTARGET_CC1_SPEC },				\
  { "subtarget_cpp_spec", SUBTARGET_CPP_SPEC },				\
  { "subtarget_asm_debugging_spec", SUBTARGET_ASM_DEBUGGING_SPEC },	\
  { "subtarget_asm_spec", SUBTARGET_ASM_SPEC },				\
  { "asm_abi_default_spec", "-" MULTILIB_ABI_DEFAULT },			\
  { "endian_spec", ENDIAN_SPEC },					\
  SUBTARGET_EXTRA_SPECS

#ifndef SUBTARGET_EXTRA_SPECS
#define SUBTARGET_EXTRA_SPECS
#endif

#define DBX_DEBUGGING_INFO 1		/* generate stabs (OSF/rose) */
#define DWARF2_DEBUGGING_INFO 1         /* dwarf2 debugging info */

#ifndef PREFERRED_DEBUGGING_TYPE
#define PREFERRED_DEBUGGING_TYPE DWARF2_DEBUG
#endif

#define DWARF2_ADDR_SIZE UNITS_PER_WORD

/* By default, turn on GDB extensions.  */
#define DEFAULT_GDB_EXTENSIONS 1

#define LOCAL_LABEL_PREFIX	"."
#define USER_LABEL_PREFIX	""

/* On Sun 4, this limit is 2048.  We use 1500 to be safe,
   since the length can run past this up to a continuation point.  */
#undef DBX_CONTIN_LENGTH
#define DBX_CONTIN_LENGTH 1500

/* How to renumber registers for dbx and gdb.  */
#define DBX_REGISTER_NUMBER(REGNO) mips_dbx_regno[REGNO]

/* The mapping from gcc register number to DWARF 2 CFA column number.  */
#define DWARF_FRAME_REGNUM(REGNO) mips_dwarf_regno[REGNO]

/* The DWARF 2 CFA column which tracks the return address.  */
#define DWARF_FRAME_RETURN_COLUMN RETURN_ADDR_REGNUM

/* Don't emit .cfi_sections, as it does not work */
#undef HAVE_GAS_CFI_SECTIONS_DIRECTIVE
#define HAVE_GAS_CFI_SECTIONS_DIRECTIVE 0

/* Before the prologue, RA lives in r31.  */
#define INCOMING_RETURN_ADDR_RTX gen_rtx_REG (VOIDmode, RETURN_ADDR_REGNUM)

/* Describe how we implement __builtin_eh_return.  */
#define EH_RETURN_DATA_REGNO(N) \
  ((N) < 4 ? (N) + GP_ARG_FIRST : INVALID_REGNUM)

#define EH_RETURN_STACKADJ_RTX  gen_rtx_REG (Pmode, GP_REG_FIRST + 3)

/* Offsets recorded in opcodes are a multiple of this alignment factor.
   The default for this in 64-bit mode is 8, which causes problems with
   SFmode register saves.  */
#define DWARF_CIE_DATA_ALIGNMENT -4

/* Correct the offset of automatic variables and arguments.  Note that
   the MIPS debug format wants all automatic variables and arguments
   to be in terms of the virtual frame pointer (stack pointer before
   any adjustment in the function), while the MIPS 3.0 linker wants
   the frame pointer to be the stack pointer after the initial
   adjustment.  */

#define DEBUGGER_AUTO_OFFSET(X)				\
  mips_debugger_offset (X, (HOST_WIDE_INT) 0)
#define DEBUGGER_ARG_OFFSET(OFFSET, X)			\
  mips_debugger_offset (X, (HOST_WIDE_INT) OFFSET)

/* Target machine storage layout */

#define BITS_BIG_ENDIAN 0
#define BYTES_BIG_ENDIAN (TARGET_BIG_ENDIAN != 0)
#define WORDS_BIG_ENDIAN (TARGET_BIG_ENDIAN != 0)

#define MAX_BITS_PER_WORD 64

/* Width of a word, in units (bytes).  */
#define UNITS_PER_WORD (TARGET_64BIT ? 8 : 4)
#ifndef IN_LIBGCC2
#define MIN_UNITS_PER_WORD 4
#endif

/* For MIPS, width of a floating point register.  */
#define UNITS_PER_FPREG 8

/* The number of consecutive floating-point registers needed to store the
   smallest format supported by the FPU.  */
#define MIN_FPRS_PER_FMT 1

/* The largest size of value that can be held in floating-point
   registers and moved with a single instruction.  */
#define UNITS_PER_HWFPVALUE \
  (TARGET_SOFT_FLOAT_ABI ? 0 : UNITS_PER_FPREG)

/* The largest size of value that can be held in floating-point
   registers.  */
#define UNITS_PER_FPVALUE			\
  (TARGET_SOFT_FLOAT_ABI ? 0			\
   : LONG_DOUBLE_TYPE_SIZE / BITS_PER_UNIT)

/* The number of bytes in a double.  */
#define UNITS_PER_DOUBLE (TYPE_PRECISION (double_type_node) / BITS_PER_UNIT)

/* Set the sizes of the core types.  */
#define SHORT_TYPE_SIZE 16
#define INT_TYPE_SIZE 32
#define LONG_TYPE_SIZE (TARGET_64BIT ? 64 : 32)
#define LONG_LONG_TYPE_SIZE 64

#define FLOAT_TYPE_SIZE 32
#define DOUBLE_TYPE_SIZE 64
#define LONG_DOUBLE_TYPE_SIZE 64

/* Define the sizes of fixed-point types.  */
#define SHORT_FRACT_TYPE_SIZE 8
#define FRACT_TYPE_SIZE 16
#define LONG_FRACT_TYPE_SIZE 32
#define LONG_LONG_FRACT_TYPE_SIZE 64

#define SHORT_ACCUM_TYPE_SIZE 16
#define ACCUM_TYPE_SIZE 32
#define LONG_ACCUM_TYPE_SIZE 64
/* FIXME.  LONG_LONG_ACCUM_TYPE_SIZE should be 128 bits, but GCC
   doesn't support 128-bit integers for MIPS32 currently.  */
#define LONG_LONG_ACCUM_TYPE_SIZE (TARGET_64BIT ? 128 : 64)

/* long double is not a fixed mode, but the idea is that, if we
   support long double, we also want a 128-bit integer type.  */
#define MAX_FIXED_MODE_SIZE LONG_DOUBLE_TYPE_SIZE

#ifdef IN_LIBGCC2
# define LIBGCC2_LONG_DOUBLE_TYPE_SIZE LONG_DOUBLE_TYPE_SIZE
#endif

/* Width in bits of a pointer.  */
#ifndef POINTER_SIZE
#define POINTER_SIZE (TARGET_64BIT ? 64 : 32)
#endif

/* Allocation boundary (in *bits*) for storing arguments in argument list.  */
#define PARM_BOUNDARY BITS_PER_WORD

/* Allocation boundary (in *bits*) for the code of a function.  */
#define FUNCTION_BOUNDARY 32

/* Alignment of field after `int : 0' in a structure.  */
#define EMPTY_FIELD_BOUNDARY 32

/* Every structure's size must be a multiple of this.  */
/* 8 is observed right on a DECstation and on riscos 4.02.  */
#define STRUCTURE_SIZE_BOUNDARY 8

/* There is no point aligning anything to a rounder boundary than this.  */
#define BIGGEST_ALIGNMENT LONG_DOUBLE_TYPE_SIZE

/* All accesses must be aligned.  */
#define STRICT_ALIGNMENT 1

/* Define this if you wish to imitate the way many other C compilers
   handle alignment of bitfields and the structures that contain
   them.

   The behavior is that the type written for a bit-field (`int',
   `short', or other integer type) imposes an alignment for the
   entire structure, as if the structure really did contain an
   ordinary field of that type.  In addition, the bit-field is placed
   within the structure so that it would fit within such a field,
   not crossing a boundary for it.

   Thus, on most machines, a bit-field whose type is written as `int'
   would not cross a four-byte boundary, and would force four-byte
   alignment for the whole structure.  (The alignment used may not
   be four bytes; it is controlled by the other alignment
   parameters.)

   If the macro is defined, its definition should be a C expression;
   a nonzero value for the expression enables this behavior.  */

#define PCC_BITFIELD_TYPE_MATTERS 1

/* If defined, a C expression to compute the alignment given to a
   constant that is being placed in memory.  CONSTANT is the constant
   and ALIGN is the alignment that the object would ordinarily have.
   The value of this macro is used instead of that alignment to align
   the object.

   If this macro is not defined, then ALIGN is used.

   The typical use of this macro is to increase alignment for string
   constants to be word aligned so that `strcpy' calls that copy
   constants can be done inline.  */

#define CONSTANT_ALIGNMENT(EXP, ALIGN)					\
  ((TREE_CODE (EXP) == STRING_CST  || TREE_CODE (EXP) == CONSTRUCTOR)	\
   && (ALIGN) < BITS_PER_WORD ? BITS_PER_WORD : (ALIGN))

/* If defined, a C expression to compute the alignment for a static
   variable.  TYPE is the data type, and ALIGN is the alignment that
   the object would ordinarily have.  The value of this macro is used
   instead of that alignment to align the object.

   If this macro is not defined, then ALIGN is used.

   One use of this macro is to increase alignment of medium-size
   data to make it all fit in fewer cache lines.  Another is to
   cause character arrays to be word-aligned so that `strcpy' calls
   that copy constants to character arrays can be done inline.  */

#undef DATA_ALIGNMENT
#define DATA_ALIGNMENT(TYPE, ALIGN)					\
  ((((ALIGN) < BITS_PER_WORD)						\
    && (TREE_CODE (TYPE) == ARRAY_TYPE					\
	|| TREE_CODE (TYPE) == UNION_TYPE				\
	|| TREE_CODE (TYPE) == RECORD_TYPE)) ? BITS_PER_WORD : (ALIGN))

/* We need this for the same reason as DATA_ALIGNMENT, namely to cause
   character arrays to be word-aligned so that `strcpy' calls that copy
   constants to character arrays can be done inline, and 'strcmp' can be
   optimised to use word loads. */
#define LOCAL_ALIGNMENT(TYPE, ALIGN) \
  DATA_ALIGNMENT (TYPE, ALIGN)
  
#define PAD_VARARGS_DOWN \
  (FUNCTION_ARG_PADDING (TYPE_MODE (type), type) == downward)

/* Define if operations between registers always perform the operation
   on the full register even if a narrower mode is specified.  */
#define WORD_REGISTER_OPERATIONS

/* When in 64-bit mode, move insns will sign extend SImode and CCmode
   moves.  All other references are zero extended.  */
#define LOAD_EXTEND_OP(MODE) \
  (TARGET_64BIT && ((MODE) == SImode || (MODE) == CCmode) \
   ? SIGN_EXTEND : ZERO_EXTEND)

/* Define this macro if it is advisable to hold scalars in registers
   in a wider mode than that declared by the program.  In such cases,
   the value is constrained to be within the bounds of the declared
   type, but kept valid in the wider mode.  The signedness of the
   extension may differ from that of the type.  */

#define PROMOTE_MODE(MODE, UNSIGNEDP, TYPE)	\
  if (GET_MODE_CLASS (MODE) == MODE_INT		\
      && GET_MODE_SIZE (MODE) < UNITS_PER_WORD) \
    {                                           \
      if ((MODE) == SImode)                     \
        (UNSIGNEDP) = 0;                        \
      (MODE) = Pmode;                           \
    }

/* Pmode is always the same as ptr_mode, but not always the same as word_mode.
   Extensions of pointers to word_mode must be signed.  */
#define POINTERS_EXTEND_UNSIGNED false

/* RV32 double-precision FP <-> integer moves go through memory */
#define SECONDARY_MEMORY_NEEDED(CLASS1,CLASS2,MODE) \
 (!TARGET_64BIT && GET_MODE_SIZE (MODE) == 8 && \
   (((CLASS1) == FP_REGS && (CLASS2) != FP_REGS) \
   || ((CLASS2) == FP_REGS && (CLASS1) != FP_REGS)))

/* Define if loading short immediate values into registers sign extends.  */
#define SHORT_IMMEDIATES_SIGN_EXTEND

/* The [d]clz instructions have the natural values at 0.  */

#define CLZ_DEFINED_VALUE_AT_ZERO(MODE, VALUE) \
  ((VALUE) = GET_MODE_BITSIZE (MODE), 2)

/* Standard register usage.  */

/* Number of hardware registers.  We have:

   - 32 integer registers
   - 32 floating point registers
   - 32 vector integer registers
   - 32 vector floating point registers
   - 3 fake registers:
	- ARG_POINTER_REGNUM
	- FRAME_POINTER_REGNUM
	- GOT_VERSION_REGNUM (see the comment above load_call<mode> for details)
   - 1 dummy entry that were used at various times in the past. */

#define FIRST_PSEUDO_REGISTER 132

/* By default, fix the global pointer ($28), the stack pointer ($30),
   and the thread pointer ($31). */

#define FIXED_REGISTERS							\
{ /* General registers.  */                                             \
  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,			\
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1,			\
  /* Floating-point registers.  */                                      \
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,			\
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,			\
  /* Vector General registers.  */                                      \
  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,			\
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,			\
  /* Vector Floating-point registers.  */                               \
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,			\
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,			\
  /* Others.  */                                                        \
  1, 1, 1, 1 \
}


/* Set up this array for o32 by default.

   Note that we don't mark $1/$ra as a call-clobbered register.  The idea is
   that it's really the call instructions themselves which clobber $ra.
   We don't care what the called function does with it afterwards.

   This approach makes it easier to implement sibcalls.  Unlike normal
   calls, sibcalls don't clobber $ra, so the register reaches the
   called function in tact.  EPILOGUE_USES says that $ra is useful
   to the called function.  */

#define CALL_USED_REGISTERS						\
{ /* General registers.  */                                             \
  1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,			\
  1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1,			\
  /* Floating-point registers.  */                                      \
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,			\
  1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,			\
  /* Vector General registers.  */                                      \
  1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,			\
  1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,			\
  /* Vector Floating-point registers.  */                               \
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,			\
  1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,			\
  /* Others.  */                                                        \
  1, 1, 1, 1 \
}

#define CALL_REALLY_USED_REGISTERS                                      \
{ /* General registers.  */                                             \
  1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,                       \
  1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,                       \
  /* Floating-point registers.  */                                      \
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,			\
  1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,			\
  /* Vector General registers.  */                                      \
  1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,                       \
  1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,                       \
  /* Vector Floating-point registers.  */                               \
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,			\
  1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,			\
  /* Others.  */                                                        \
  1, 1, 0, 0 \
}

/* Internal macros to classify a register number as to whether it's a
   general purpose register, a floating point register, a
   multiply/divide register, or a status register.  */

#define GP_REG_FIRST 0
#define GP_REG_LAST  31
#define GP_REG_NUM   (GP_REG_LAST - GP_REG_FIRST + 1)
#define GP_DBX_FIRST 0

#define FP_REG_FIRST 32
#define FP_REG_LAST  63
#define FP_REG_NUM   (FP_REG_LAST - FP_REG_FIRST + 1)
#define FP_DBX_FIRST ((write_symbols == DBX_DEBUG) ? 38 : 32)

#define CALLEE_SAVED_GP_REG_FIRST 20
#define CALLEE_SAVED_GP_REG_LAST (CALLEE_SAVED_GP_REG_FIRST + 10 - 1)

#define CALLEE_SAVED_FP_REG_FIRST 52
#define CALLEE_SAVED_FP_REG_LAST (CALLEE_SAVED_FP_REG_FIRST + 12 - 1)

#define VEC_GP_REG_FIRST 64
#define VEC_GP_REG_LAST  95
#define VEC_GP_REG_NUM   (VEC_GP_REG_LAST - VEC_GP_REG_FIRST + 1)

#define VEC_FP_REG_FIRST 96
#define VEC_FP_REG_LAST  127
#define VEC_FP_REG_NUM   (VEC_FP_REG_LAST - VEC_FP_REG_FIRST + 1)

/* The DWARF 2 CFA column which tracks the return address from a
   signal handler context.  This means that to maintain backwards
   compatibility, no hard register can be assigned this column if it
   would need to be handled by the DWARF unwinder.  */
#define DWARF_ALT_FRAME_RETURN_COLUMN 66

#define GP_REG_P(REGNO)	\
  ((unsigned int) ((int) (REGNO) - GP_REG_FIRST) < GP_REG_NUM)
#define FP_REG_P(REGNO)  \
  ((unsigned int) ((int) (REGNO) - FP_REG_FIRST) < FP_REG_NUM)
#define VEC_GP_REG_P(REGNO)	\
  ((unsigned int) ((int) (REGNO) - VEC_GP_REG_FIRST) < VEC_GP_REG_NUM)
#define VEC_FP_REG_P(REGNO)  \
  ((unsigned int) ((int) (REGNO) - VEC_FP_REG_FIRST) < VEC_FP_REG_NUM)

#define FP_REG_RTX_P(X) (REG_P (X) && FP_REG_P (REGNO (X)))

/* Return coprocessor number from register number.  */

#define COPNUM_AS_CHAR_FROM_REGNUM(REGNO) 				\
  (COP0_REG_P (REGNO) ? '0' : COP2_REG_P (REGNO) ? '2'			\
   : COP3_REG_P (REGNO) ? '3' : '?')


#define HARD_REGNO_NREGS(REGNO, MODE) mips_hard_regno_nregs (REGNO, MODE)

#define HARD_REGNO_MODE_OK(REGNO, MODE)					\
  mips_hard_regno_mode_ok[ (int)(MODE) ][ (REGNO) ]

#define MODES_TIEABLE_P mips_modes_tieable_p

/* Register to use for pushing function arguments.  */
#define STACK_POINTER_REGNUM (GP_REG_FIRST + 30)
#define HARD_FRAME_POINTER_REGNUM (GP_REG_FIRST + 29)

#define THREAD_POINTER_REGNUM (GP_REG_FIRST + 31)

#define LINK_REGNUM (GP_REG_FIRST + 1) /* Return address, $ra */

/* These two registers don't really exist: they get eliminated to either
   the stack or hard frame pointer.  */
#define ARG_POINTER_REGNUM 128
#define FRAME_POINTER_REGNUM 129

#define HARD_FRAME_POINTER_IS_FRAME_POINTER 0
#define HARD_FRAME_POINTER_IS_ARG_POINTER 0

/* Register in which static-chain is passed to a function.  */
#define STATIC_CHAIN_REGNUM (GP_REG_FIRST + 15)

/* Registers used as temporaries in prologue/epilogue code:

   - The prologue can use MIPS_PROLOGUE_TEMP as a general temporary
     register.

   - The epilogue can use MIPS_EPILOGUE_TEMP as a general temporary
     register.

   The prologue registers mustn't conflict with any
   incoming arguments, the static chain pointer, or the frame pointer.
   The epilogue temporary mustn't conflict with the return registers,
   the PIC call register ($25), the frame pointer, the EH stack adjustment,
   or the EH data registers.

   If we're generating interrupt handlers, we use K0 as a temporary register
   in prologue/epilogue code.  */

#define MIPS_PROLOGUE_TEMP_REGNUM (GP_REG_FIRST + 3)
#define MIPS_EPILOGUE_TEMP_REGNUM (GP_REG_FIRST + 8)

#define MIPS_PROLOGUE_TEMP(MODE) gen_rtx_REG (MODE, MIPS_PROLOGUE_TEMP_REGNUM)
#define MIPS_EPILOGUE_TEMP(MODE) gen_rtx_REG (MODE, MIPS_EPILOGUE_TEMP_REGNUM)

#define FUNCTION_PROFILER(STREAM, LABELNO)	\
{						\
    sorry ("profiler support for RISC-V");	\
}

/* Define this macro if it is as good or better to call a constant
   function address than to call an address kept in a register.  */
#define NO_FUNCTION_CSE 1

/* The ABI-defined global pointer.  Sometimes we use a different
   register in leaf functions: see PIC_OFFSET_TABLE_REGNUM.  */
#define GLOBAL_POINTER_REGNUM (GP_REG_FIRST + 18)
#define GLOBAL_POINTER_REGNUM_NONLEAF (GP_REG_FIRST + 28)

#define PIC_OFFSET_TABLE_REGNUM GLOBAL_POINTER_REGNUM

#define PIC_FUNCTION_ADDR_REGNUM (GP_REG_FIRST + 19)

/* Define the classes of registers for register constraints in the
   machine description.  Also define ranges of constants.

   One of the classes must always be named ALL_REGS and include all hard regs.
   If there is more than one class, another class must be named NO_REGS
   and contain no registers.

   The name GENERAL_REGS must be the name of a class (or an alias for
   another name such as ALL_REGS).  This is the class of registers
   that is allowed by "g" or "r" in a register constraint.
   Also, registers outside this class are allocated only when
   instructions express preferences for them.

   The classes must be numbered in nondecreasing order; that is,
   a larger-numbered class must never be contained completely
   in a smaller-numbered class.

   For any two classes, it is very desirable that there be another
   class that represents their union.  */

enum reg_class
{
  NO_REGS,			/* no registers in set */
  PIC_FN_ADDR_REG,		/* SVR4 PIC function address register */
  V1_REG,			/* Register $v1 ($3) used for TLS access.  */
  LEA_REGS,			/* Every GPR except $25 */
  GR_REGS,			/* integer registers */
  FP_REGS,			/* floating point registers */
  VEC_GR_REGS,			/* vector integer registers */
  VEC_FP_REGS,			/* vector floating point registers */
  FRAME_REGS,			/* $arg and $frame */
  ALL_REGS,			/* all registers */
  LIM_REG_CLASSES		/* max value + 1 */
};

#define N_REG_CLASSES (int) LIM_REG_CLASSES

#define GENERAL_REGS GR_REGS

/* An initializer containing the names of the register classes as C
   string constants.  These names are used in writing some of the
   debugging dumps.  */

#define REG_CLASS_NAMES							\
{									\
  "NO_REGS",								\
  "PIC_FN_ADDR_REG",							\
  "V1_REG",								\
  "LEA_REGS",								\
  "GR_REGS",								\
  "FP_REGS",								\
  "VEC_GR_REGS",							\
  "VEC_FP_REGS",							\
  "FRAME_REGS",								\
  "ALL_REGS"								\
}

/* An initializer containing the contents of the register classes,
   as integers which are bit masks.  The Nth integer specifies the
   contents of class N.  The way the integer MASK is interpreted is
   that register R is in the class if `MASK & (1 << R)' is 1.

   When the machine has more than 32 registers, an integer does not
   suffice.  Then the integers are replaced by sub-initializers,
   braced groupings containing several integers.  Each
   sub-initializer must be suitable as an initializer for the type
   `HARD_REG_SET' which is defined in `hard-reg-set.h'.  */

#define REG_CLASS_CONTENTS									\
{												\
  { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 },	/* NO_REGS */		\
  { 0x00080000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 },	/* PIC_FN_ADDR_REG */	\
  { 0x00000008, 0x00000000, 0x00000000, 0x00000000, 0x00000000 },	/* V1_REG */		\
  { 0xfff7ffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000 },	/* LEA_REGS */		\
  { 0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000 },	/* GR_REGS */		\
  { 0x00000000, 0xffffffff, 0x00000000, 0x00000000, 0x00000000 },	/* FP_REGS */		\
  { 0x00000000, 0x00000000, 0xffffffff, 0x00000000, 0x00000000 },	/* VEC_GR_REGS */	\
  { 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0x00000000 },	/* VEC_FP_REGS */	\
  { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000003 },	/* FRAME_REGS */	\
  { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000003 }	/* ALL_REGS */		\
}

/* A C expression whose value is a register class containing hard
   register REGNO.  In general there is more that one such class;
   choose a class which is "minimal", meaning that no smaller class
   also contains the register.  */

#define REGNO_REG_CLASS(REGNO) mips_regno_to_class[ (REGNO) ]

/* A macro whose definition is the name of the class to which a
   valid base register must belong.  A base register is one used in
   an address which is the register value plus a displacement.  */

#define BASE_REG_CLASS GR_REGS

/* A macro whose definition is the name of the class to which a
   valid index register must belong.  An index register is one used
   in an address where its value is either multiplied by a scale
   factor or added to another register (as well as added to a
   displacement).  */

#define INDEX_REG_CLASS NO_REGS

/* We generally want to put call-clobbered registers ahead of
   call-saved ones.  (IRA expects this.)  */

#define REG_ALLOC_ORDER							\
{ \
  /* Call-clobbered GPRs.  */						\
  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,		\
  16, 17, 18, 19, 1,							\
  /* Call-saved GPRs.  */						\
  20, 21, 22, 23, 24, 25, 26, 27, 29,	        			\
  /* The global pointer.  This is call-clobbered for o32 and o64	\
     abicalls, call-saved for n32 and n64 abicalls, and a program	\
     invariant otherwise.  Putting it between the call-clobbered	\
     and call-saved registers should cope with all eventualities.  */	\
  28,									\
  /* GPRs that can never be exposed to the register allocator.  */	\
  0,  30, 31,								\
  /* Call-clobbered FPRs.  */						\
  34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49,	\
  50, 51, 32, 33,							\
  /* FPRs that are usually call-saved.  The odd ones are actually	\
     call-clobbered for n32, but listing them ahead of the even		\
     registers might encourage the register allocator to fragment	\
     the available FPR pairs.  We need paired FPRs to store long	\
     doubles, so it isn't clear that using a different order		\
     for n32 would be a win.  */					\
  52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,			\
  /* Vector GPRs  */							\
  64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,	\
  80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,	\
  /* Vector FPRs  */							\
  96, 97, 98, 99,100,101,102,103,104,105,106,107,108,109,110,111,	\
 112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,	\
  /* None of the remaining classes have defined call-saved		\
     registers.  */							\
 128,129,130,131 \
}

/* True if VALUE is a signed 16-bit number.  */

#include "opcode-riscv.h"
#define SMALL_OPERAND(VALUE) \
  ((unsigned HOST_WIDE_INT) (VALUE) + RISCV_IMM_REACH/2 < RISCV_IMM_REACH)

/* True if VALUE can be loaded into a register using LUI.  */

#define LUI_OPERAND(VALUE)					\
  (((VALUE) | ((1UL<<31) - RISCV_IMM_REACH)) == ((1UL<<31) - RISCV_IMM_REACH) \
   || ((VALUE) | ((1UL<<31) - RISCV_IMM_REACH)) + RISCV_IMM_REACH == 0)

/* Return a value X with the low 16 bits clear, and such that
   VALUE - X is a signed 16-bit value.  */

#define SMALL_INT(X) SMALL_OPERAND (INTVAL (X))
#define LUI_INT(X) LUI_OPERAND (INTVAL (X))

/* The HI and LO registers can only be reloaded via the general
   registers.  Condition code registers can only be loaded to the
   general registers, and from the floating point registers.  */

#define SECONDARY_INPUT_RELOAD_CLASS(CLASS, MODE, X)			\
  mips_secondary_reload_class (CLASS, MODE, X, true)
#define SECONDARY_OUTPUT_RELOAD_CLASS(CLASS, MODE, X)			\
  mips_secondary_reload_class (CLASS, MODE, X, false)

/* Return the maximum number of consecutive registers
   needed to represent mode MODE in a register of class CLASS.  */

#define CLASS_MAX_NREGS(CLASS, MODE) mips_class_max_nregs (CLASS, MODE)

#define CANNOT_CHANGE_MODE_CLASS(FROM, TO, CLASS) \
  mips_cannot_change_mode_class (FROM, TO, CLASS)

/* Stack layout; function entry, exit and calling.  */

#define STACK_GROWS_DOWNWARD

#define FRAME_GROWS_DOWNWARD 1

#define STARTING_FRAME_OFFSET 0

#define RETURN_ADDR_RTX mips_return_addr

/* Similarly, don't use the least-significant bit to tell pointers to
   code from vtable index.  */

#define TARGET_PTRMEMFUNC_VBIT_LOCATION ptrmemfunc_vbit_in_delta

/* The eliminations to $17 are only used for mips16 code.  See the
   definition of HARD_FRAME_POINTER_REGNUM.  */

#define ELIMINABLE_REGS							\
{{ ARG_POINTER_REGNUM,   STACK_POINTER_REGNUM},				\
 { ARG_POINTER_REGNUM,   HARD_FRAME_POINTER_REGNUM},			\
 { FRAME_POINTER_REGNUM, STACK_POINTER_REGNUM},				\
 { FRAME_POINTER_REGNUM, HARD_FRAME_POINTER_REGNUM}}				\

#define INITIAL_ELIMINATION_OFFSET(FROM, TO, OFFSET) \
  (OFFSET) = mips_initial_elimination_offset (FROM, TO)

/* Allocate stack space for arguments at the beginning of each function.  */
#define ACCUMULATE_OUTGOING_ARGS 1

/* The argument pointer always points to the first argument.  */
#define FIRST_PARM_OFFSET(FNDECL) 0

#define REG_PARM_STACK_SPACE(FNDECL) 0

/* Define this if it is the responsibility of the caller to
   allocate the area reserved for arguments passed in registers.
   If `ACCUMULATE_OUTGOING_ARGS' is also defined, the only effect
   of this macro is to determine whether the space is included in
   `crtl->outgoing_args_size'.  */
#define OUTGOING_REG_PARM_STACK_SPACE(FNTYPE) 1

#define STACK_BOUNDARY 128

/* Symbolic macros for the registers used to return integer and floating
   point values.  */

#define GP_RETURN (GP_REG_FIRST + 2)
#define FP_RETURN ((TARGET_SOFT_FLOAT) ? GP_RETURN : (FP_REG_FIRST + 2))

#define MAX_ARGS_IN_REGISTERS 8

/* Symbolic macros for the first/last argument registers.  */

#define GP_ARG_FIRST (GP_REG_FIRST + 4)
#define GP_ARG_LAST  (GP_ARG_FIRST + MAX_ARGS_IN_REGISTERS - 1)
#define FP_ARG_FIRST (FP_REG_FIRST + 12)
#define FP_ARG_LAST  (FP_ARG_FIRST + MAX_ARGS_IN_REGISTERS - 1)

#define LIBCALL_VALUE(MODE) \
  mips_function_value (NULL_TREE, NULL_TREE, MODE)

#define FUNCTION_VALUE(VALTYPE, FUNC) \
  mips_function_value (VALTYPE, FUNC, VOIDmode)

/* 1 if N is a possible register number for a function value.
   On the MIPS, R2 R3 and F0 F2 are the only register thus used.
   Currently, R2 and F0 are only implemented here (C has no complex type)  */

#define FUNCTION_VALUE_REGNO_P(N) ((N) == GP_RETURN || (N) == FP_RETURN \
  || (LONG_DOUBLE_TYPE_SIZE == 128 && FP_RETURN != GP_RETURN \
      && (N) == FP_RETURN + 2))

/* 1 if N is a possible register number for function argument passing.
   We have no FP argument registers when soft-float.  When FP registers
   are 32 bits, we can't directly reference the odd numbered ones.  */

#define FUNCTION_ARG_REGNO_P(N)					\
  ((IN_RANGE((N), GP_ARG_FIRST, GP_ARG_LAST)			\
    || (IN_RANGE((N), FP_ARG_FIRST, FP_ARG_LAST)))		\
   && !fixed_regs[N])

/* This structure has to cope with two different argument allocation
   schemes.  Most MIPS ABIs view the arguments as a structure, of which
   the first N words go in registers and the rest go on the stack.  If I
   < N, the Ith word might go in Ith integer argument register or in a
   floating-point register.  For these ABIs, we only need to remember
   the offset of the current argument into the structure.

   The EABI instead allocates the integer and floating-point arguments
   separately.  The first N words of FP arguments go in FP registers,
   the rest go on the stack.  Likewise, the first N words of the other
   arguments go in integer registers, and the rest go on the stack.  We
   need to maintain three counts: the number of integer registers used,
   the number of floating-point registers used, and the number of words
   passed on the stack.

   We could keep separate information for the two ABIs (a word count for
   the standard ABIs, and three separate counts for the EABI).  But it
   seems simpler to view the standard ABIs as forms of EABI that do not
   allocate floating-point registers.

   So for the standard ABIs, the first N words are allocated to integer
   registers, and mips_function_arg decides on an argument-by-argument
   basis whether that argument should really go in an integer register,
   or in a floating-point one.  */

typedef struct mips_args {
  /* The number of integer registers used so far.  For all ABIs except
     EABI, this is the number of words that have been added to the
     argument structure, limited to MAX_ARGS_IN_REGISTERS.  */
  unsigned int num_gprs;

  /* The number of words passed on the stack.  */
  unsigned int stack_words;
} CUMULATIVE_ARGS;

/* Initialize a variable CUM of type CUMULATIVE_ARGS
   for a call to a function whose data type is FNTYPE.
   For a library call, FNTYPE is 0.  */

#define INIT_CUMULATIVE_ARGS(CUM, FNTYPE, LIBNAME, INDIRECT, N_NAMED_ARGS) \
  mips_init_cumulative_args (&CUM, FNTYPE)

#define FUNCTION_ARG_PADDING(MODE, TYPE) \
  (mips_pad_arg_upward (MODE, TYPE) ? upward : downward)

#define BLOCK_REG_PADDING(MODE, TYPE, FIRST) \
  (mips_pad_reg_upward (MODE, TYPE) ? upward : downward)


#define EPILOGUE_USES(REGNO)	mips_epilogue_uses (REGNO)

/* Even on RV32, provide 8-byte alignment for 64b floats. */
#define MIPS_STACK_ALIGN(LOC) (((LOC) + 7) & -8)

/* No mips port has ever used the profiler counter word, so don't emit it
   or the label for it.  */

#define NO_PROFILE_COUNTERS 1

/* Define this macro if the code for function profiling should come
   before the function prologue.  Normally, the profiling code comes
   after.  */

/* #define PROFILE_BEFORE_PROLOGUE */

/* EXIT_IGNORE_STACK should be nonzero if, when returning from a function,
   the stack pointer does not matter.  The value is tested only in
   functions that have frame pointers.
   No definition is equivalent to always zero.  */

#define EXIT_IGNORE_STACK 1


/* Trampolines are a block of code followed by two pointers.  */

#define TRAMPOLINE_CODE_SIZE 16
#define TRAMPOLINE_SIZE (TRAMPOLINE_CODE_SIZE + GET_MODE_SIZE (ptr_mode) * 2)

/* Forcing a 64-bit alignment for 32-bit targets allows us to load two
   pointers from a single LUI base.  */

#define TRAMPOLINE_ALIGNMENT 64

/* Addressing modes, and classification of registers for them.  */

#define REGNO_OK_FOR_INDEX_P(REGNO) 0
#define REGNO_MODE_OK_FOR_BASE_P(REGNO, MODE) \
  mips_regno_mode_ok_for_base_p (REGNO, MODE, 1)

/* The macros REG_OK_FOR..._P assume that the arg is a REG rtx
   and check its validity for a certain class.
   We have two alternate definitions for each of them.
   The usual definition accepts all pseudo regs; the other rejects them all.
   The symbol REG_OK_STRICT causes the latter definition to be used.

   Most source files want to accept pseudo regs in the hope that
   they will get allocated to the class that the insn wants them to be in.
   Some source files that are used after register allocation
   need to be strict.  */

#ifndef REG_OK_STRICT
#define REG_MODE_OK_FOR_BASE_P(X, MODE) \
  mips_regno_mode_ok_for_base_p (REGNO (X), MODE, 0)
#else
#define REG_MODE_OK_FOR_BASE_P(X, MODE) \
  mips_regno_mode_ok_for_base_p (REGNO (X), MODE, 1)
#endif

#define REG_OK_FOR_INDEX_P(X) 0


/* Maximum number of registers that can appear in a valid memory address.  */

#define MAX_REGS_PER_ADDRESS 1

/* Check for constness inline but use mips_legitimate_address_p
   to check whether a constant really is an address.  */

#define CONSTANT_ADDRESS_P(X) \
  (CONSTANT_P (X) && memory_address_p (SImode, X))

#define LEGITIMATE_CONSTANT_P(X) (mips_const_insns (X) > 0)

/* This handles the magic '..CURRENT_FUNCTION' symbol, which means
   'the start of the function that this code is output in'.  */

#define ASM_OUTPUT_LABELREF(FILE,NAME)  \
  if (strcmp (NAME, "..CURRENT_FUNCTION") == 0)				\
    asm_fprintf ((FILE), "%U%s",					\
		 XSTR (XEXP (DECL_RTL (current_function_decl), 0), 0));	\
  else									\
    asm_fprintf ((FILE), "%U%s", (NAME))

/* Flag to mark a function decl symbol that requires a long call.  */
#define SYMBOL_FLAG_LONG_CALL	(SYMBOL_FLAG_MACH_DEP << 0)
#define SYMBOL_REF_LONG_CALL_P(X)					\
  ((SYMBOL_REF_FLAGS (X) & SYMBOL_FLAG_LONG_CALL) != 0)

/* This flag marks functions that cannot be lazily bound.  */
#define SYMBOL_FLAG_BIND_NOW (SYMBOL_FLAG_MACH_DEP << 1)
#define SYMBOL_REF_BIND_NOW_P(RTX) \
  ((SYMBOL_REF_FLAGS (RTX) & SYMBOL_FLAG_BIND_NOW) != 0)

#define JUMP_TABLES_IN_TEXT_SECTION 0
#define CASE_VECTOR_MODE ptr_mode
#define CASE_VECTOR_PC_RELATIVE 0

/* Define this as 1 if `char' should by default be signed; else as 0.  */
#ifndef DEFAULT_SIGNED_CHAR
#define DEFAULT_SIGNED_CHAR 1
#endif

/* Although LDC1 and SDC1 provide 64-bit moves on 32-bit targets,
   we generally don't want to use them for copying arbitrary data.
   A single N-word move is usually the same cost as N single-word moves.  */
#define MOVE_MAX UNITS_PER_WORD
#define MAX_MOVE_MAX 8

/* Define this macro as a C expression which is nonzero if
   accessing less than a word of memory (i.e. a `char' or a
   `short') is no faster than accessing a word of memory, i.e., if
   such access require more than one instruction or if there is no
   difference in cost between byte and (aligned) word loads.

   On RISC machines, it tends to generate better code to define
   this as 1, since it avoids making a QI or HI mode register.

   But, generating word accesses for -mips16 is generally bad as shifts
   (often extended) would be needed for byte accesses.  */
#define SLOW_BYTE_ACCESS 1

/* Standard MIPS integer shifts truncate the shift amount to the
   width of the shifted operand.  However, Loongson vector shifts
   do not truncate the shift amount at all.  */
#define SHIFT_COUNT_TRUNCATED 1

/* Value is 1 if truncating an integer of INPREC bits to OUTPREC bits
   is done just by pretending it is already truncated.  */
#define TRULY_NOOP_TRUNCATION(OUTPREC, INPREC) \
  (TARGET_64BIT ? ((INPREC) <= 32 || (OUTPREC) > 32) : 1)


/* Specify the machine mode that pointers have.
   After generation of rtl, the compiler makes no further distinction
   between pointers and any other objects of this machine mode.  */

#ifndef Pmode
#define Pmode (TARGET_64BIT ? DImode : SImode)
#endif

/* Give call MEMs SImode since it is the "most permissive" mode
   for both 32-bit and 64-bit targets.  */

#define FUNCTION_MODE SImode



/* Define if copies to/from condition code registers should be avoided.

   This is needed for the MIPS because reload_outcc is not complete;
   it needs to handle cases where the source is a general or another
   condition code register.  */
#define AVOID_CCMODE_COPIES

/* A C expression for the cost of a branch instruction.  A value of
   1 is the default; other values are interpreted relative to that.  */

#define BRANCH_COST(speed_p, predictable_p) mips_branch_cost
#define LOGICAL_OP_NON_SHORT_CIRCUIT 0

/* Control the assembler format that we output.  */

/* Output to assembler file text saying following lines
   may contain character constants, extra white space, comments, etc.  */

#ifndef ASM_APP_ON
#define ASM_APP_ON " #APP\n"
#endif

/* Output to assembler file text saying following lines
   no longer contain unusual constructs.  */

#ifndef ASM_APP_OFF
#define ASM_APP_OFF " #NO_APP\n"
#endif

#define REGISTER_NAMES							  \
{ "x0",  "x1",  "x2",  "x3",  "x4",  "x5",  "x6",  "x7",	   \
  "x8",  "x9",  "x10", "x11", "x12", "x13", "x14", "x15",	   \
  "x16", "x17", "x18", "x19", "x20", "x21", "x22", "x23",	   \
  "x24", "x25", "x26", "x27", "x28", "x29", "x30", "x31",	   \
  "f0",  "f1",  "f2",  "f3",  "f4",  "f5",  "f6",  "f7",	   \
  "f8",  "f9",  "f10", "f11", "f12", "f13", "f14", "f15",	   \
  "f16", "f17", "f18", "f19", "f20", "f21", "f22", "f23",	   \
  "f24", "f25", "f26", "f27", "f28", "f29", "f30", "f31",	   \
  "vx0", "vx1", "vx2", "vx3", "vx4", "vx5", "vx6", "vx7",	   \
  "vx8", "vx9", "vx10","vx11","vx12","vx13","vx14","vx15",	   \
  "vx16","vx17","vx18","vx19","vx20","vx21","vx22","vx23",	   \
  "vx24","vx25","vx26","vx27","vx28","vx29","vx30","vx31",	   \
  "vf0", "vf1", "vf2", "vf3", "vf4", "vf5", "vf6", "vf7",	   \
  "vf8", "vf9", "vf10","vf11","vf12","vf13","vf14","vf15",	   \
  "vf16","vf17","vf18","vf19","vf20","vf21","vf22","vf23",	   \
  "vf24","vf25","vf26","vf27","vf28","vf29","vf30","vf31",	   \
  "arg", "frame", "fakec", "", }

/* List the "software" names for each register.  Also list the numerical
   names for $fp and $sp.  */

#define ADDITIONAL_REGISTER_NAMES					\
{									\
  { "zero",	 0 + GP_REG_FIRST },					\
  { "ra",	 1 + GP_REG_FIRST },					\
  { "v0",	 2 + GP_REG_FIRST },					\
  { "v1",	 3 + GP_REG_FIRST },					\
  { "a0",	 4 + GP_REG_FIRST },					\
  { "a1",	 5 + GP_REG_FIRST },					\
  { "a2",	 6 + GP_REG_FIRST },					\
  { "a3",	 7 + GP_REG_FIRST },					\
  { "a4",	 8 + GP_REG_FIRST },					\
  { "a5",	 9 + GP_REG_FIRST },					\
  { "a6",	10 + GP_REG_FIRST },					\
  { "a7",	11 + GP_REG_FIRST },					\
  { "t0",	12 + GP_REG_FIRST },					\
  { "t1",	13 + GP_REG_FIRST },					\
  { "t2",	14 + GP_REG_FIRST },					\
  { "t3",	15 + GP_REG_FIRST },					\
  { "t4",	16 + GP_REG_FIRST },					\
  { "t5",	17 + GP_REG_FIRST },					\
  { "t6",	18 + GP_REG_FIRST },					\
  { "t7",	19 + GP_REG_FIRST },					\
  { "s0",	20 + GP_REG_FIRST },					\
  { "s1",	21 + GP_REG_FIRST },					\
  { "s2",	22 + GP_REG_FIRST },					\
  { "s3",	23 + GP_REG_FIRST },					\
  { "s4",	24 + GP_REG_FIRST },					\
  { "s5",	25 + GP_REG_FIRST },					\
  { "s6",	26 + GP_REG_FIRST },					\
  { "s7",	27 + GP_REG_FIRST },					\
  { "gp",	28 + GP_REG_FIRST },					\
  { "s8",	28 + GP_REG_FIRST },					\
  { "fp",	29 + GP_REG_FIRST },					\
  { "s9",	29 + GP_REG_FIRST },					\
  { "sp",	30 + GP_REG_FIRST },					\
  { "tp",	31 + GP_REG_FIRST },					\
}

/* This is meant to be redefined in the host dependent files.  It is a
   set of alternative names and regnums for mips coprocessors.  */

#define ALL_COP_ADDITIONAL_REGISTER_NAMES

#define DBR_OUTPUT_SEQEND(STREAM)					\
do									\
  {									\
    /* Emit a blank line after the delay slot for emphasis.  */		\
    fputs ("\n", STREAM);						\
  }									\
while (0)

/* mips-tfile does not understand .stabd directives.  */
#define DBX_OUTPUT_SOURCE_LINE(STREAM, LINE, COUNTER) do {	\
  dbxout_begin_stabn_sline (LINE);				\
  dbxout_stab_value_internal_label ("LM", &COUNTER);		\
} while (0)

/* The MIPS implementation uses some labels for its own purpose.  The
   following lists what labels are created, and are all formed by the
   pattern $L[a-z].*.  The machine independent portion of GCC creates
   labels matching:  $L[A-Z][0-9]+ and $L[0-9]+.

	LM[0-9]+	Silicon Graphics/ECOFF stabs label before each stmt.
	$Lb[0-9]+	Begin blocks for MIPS debug support
	$Lc[0-9]+	Label for use in s<xx> operation.
	$Le[0-9]+	End blocks for MIPS debug support  */

#undef ASM_DECLARE_OBJECT_NAME
#define ASM_DECLARE_OBJECT_NAME(STREAM, NAME, DECL) \
  mips_declare_object (STREAM, NAME, "", ":\n")

/* Globalizing directive for a label.  */
#define GLOBAL_ASM_OP "\t.globl\t"

/* This says how to define a global common symbol.  */

#define ASM_OUTPUT_ALIGNED_DECL_COMMON mips_output_aligned_decl_common

/* This says how to define a local common symbol (i.e., not visible to
   linker).  */

#ifndef ASM_OUTPUT_ALIGNED_LOCAL
#define ASM_OUTPUT_ALIGNED_LOCAL(STREAM, NAME, SIZE, ALIGN) \
  mips_declare_common_object (STREAM, NAME, "\n\t.lcomm\t", SIZE, ALIGN, false)
#endif

/* This is how to declare a function name.  The actual work of
   emitting the label is moved to function_prologue, so that we can
   get the line number correctly emitted before the .ent directive,
   and after any .file directives.  Define as empty so that the function
   is not declared before the .ent directive elsewhere.  */

#undef ASM_DECLARE_FUNCTION_NAME
#define ASM_DECLARE_FUNCTION_NAME(STREAM,NAME,DECL)

/* This is how to store into the string LABEL
   the symbol_ref name of an internal numbered label where
   PREFIX is the class of label and NUM is the number within the class.
   This is suitable for output with `assemble_name'.  */

#undef ASM_GENERATE_INTERNAL_LABEL
#define ASM_GENERATE_INTERNAL_LABEL(LABEL,PREFIX,NUM)			\
  sprintf ((LABEL), "*%s%s%ld", (LOCAL_LABEL_PREFIX), (PREFIX), (long)(NUM))

/* Print debug labels as "foo = ." rather than "foo:" because they should
   represent a byte pointer rather than an ISA-encoded address.  This is
   particularly important for code like:

	$LFBxxx = .
		.cfi_startproc
		...
		.section .gcc_except_table,...
		...
		.uleb128 foo-$LFBxxx

   The .uleb128 requies $LFBxxx to match the FDE start address, which is
   likewise a byte pointer rather than an ISA-encoded address.

   At the time of writing, this hook is not used for the function end
   label:

   	$LFExxx:
		.end foo

   But this doesn't matter, because GAS doesn't treat a pre-.end label
   as a MIPS16 one anyway.  */

#define ASM_OUTPUT_DEBUG_LABEL(FILE, PREFIX, NUM)			\
  fprintf (FILE, "%s%s%d = .\n", LOCAL_LABEL_PREFIX, PREFIX, NUM)

/* This is how to output an element of a case-vector that is absolute.  */

#define ASM_OUTPUT_ADDR_VEC_ELT(STREAM, VALUE)				\
  fprintf (STREAM, "\t%s\t%sL%d\n",					\
	   ptr_mode == DImode ? ".dword" : ".word",			\
	   LOCAL_LABEL_PREFIX,						\
	   VALUE)

/* This is how to output an element of a case-vector.  We can make the
   entries PC-relative in MIPS16 code and GP-relative when .gp(d)word
   is supported.  */

#define ASM_OUTPUT_ADDR_DIFF_ELT(STREAM, BODY, VALUE, REL)		\
do {									\
  if (TARGET_GPWORD)						\
    fprintf (STREAM, "\t%s\t%sL%d\n",					\
	     ptr_mode == DImode ? ".gpdword" : ".gpword",		\
	     LOCAL_LABEL_PREFIX, VALUE);				\
  else									\
    fprintf (STREAM, "\t%s\t%sL%d\n",					\
	     ptr_mode == DImode ? ".dword" : ".word",			\
	     LOCAL_LABEL_PREFIX, VALUE);				\
} while (0)

/* This is how to output an assembler line
   that says to advance the location counter
   to a multiple of 2**LOG bytes.  */

#define ASM_OUTPUT_ALIGN(STREAM,LOG)					\
  fprintf (STREAM, "\t.align\t%d\n", (LOG))

/* This is how to output an assembler line to advance the location
   counter by SIZE bytes.  */

#undef ASM_OUTPUT_SKIP
#define ASM_OUTPUT_SKIP(STREAM,SIZE)					\
  fprintf (STREAM, "\t.space\t"HOST_WIDE_INT_PRINT_UNSIGNED"\n", (SIZE))

/* This is how to output a string.  */
#undef ASM_OUTPUT_ASCII
#define ASM_OUTPUT_ASCII mips_output_ascii

/* Output #ident as a in the read-only data section.  */
#undef  ASM_OUTPUT_IDENT
#define ASM_OUTPUT_IDENT(FILE, STRING)					\
{									\
  const char *p = STRING;						\
  int size = strlen (p) + 1;						\
  switch_to_section (readonly_data_section);				\
  assemble_string (p, size);						\
}

/* Define the strings to put out for each section in the object file.  */
#define TEXT_SECTION_ASM_OP	"\t.text"	/* instructions */
#define DATA_SECTION_ASM_OP	"\t.data"	/* large data */

#define ASM_OUTPUT_REG_PUSH(STREAM,REGNO)				\
do									\
  {									\
    fprintf (STREAM, "\t%s\t%s,%s,-8\n\t%s\t%s,0(%s)\n",		\
	     TARGET_64BIT ? "daddiu" : "addiu",				\
	     reg_names[STACK_POINTER_REGNUM],				\
	     reg_names[STACK_POINTER_REGNUM],				\
	     TARGET_64BIT ? "sd" : "sw",				\
	     reg_names[REGNO],						\
	     reg_names[STACK_POINTER_REGNUM]);				\
  }									\
while (0)

#define ASM_OUTPUT_REG_POP(STREAM,REGNO)				\
do									\
  {									\
    fprintf (STREAM, "\t%s\t%s,0(%s)\n\t%s\t%s,%s,8\n",			\
	     TARGET_64BIT ? "ld" : "lw",				\
	     reg_names[REGNO],						\
	     reg_names[STACK_POINTER_REGNUM],				\
	     TARGET_64BIT ? "daddu" : "addu",				\
	     reg_names[STACK_POINTER_REGNUM],				\
	     reg_names[STACK_POINTER_REGNUM]);				\
  }									\
while (0)

/* How to start an assembler comment.
   The leading space is important (the mips native assembler requires it).  */
#ifndef ASM_COMMENT_START
#define ASM_COMMENT_START " #"
#endif

#undef SIZE_TYPE
#define SIZE_TYPE (POINTER_SIZE == 64 ? "long unsigned int" : "unsigned int")

#undef PTRDIFF_TYPE
#define PTRDIFF_TYPE (POINTER_SIZE == 64 ? "long int" : "int")

/* The maximum number of bytes that can be copied by one iteration of
   a movmemsi loop; see mips_block_move_loop.  */
#define MIPS_MAX_MOVE_BYTES_PER_LOOP_ITER \
  (UNITS_PER_WORD * 4)

/* The maximum number of bytes that can be copied by a straight-line
   implementation of movmemsi; see mips_block_move_straight.  We want
   to make sure that any loop-based implementation will iterate at
   least twice.  */
#define MIPS_MAX_MOVE_BYTES_STRAIGHT \
  (MIPS_MAX_MOVE_BYTES_PER_LOOP_ITER * 2)

/* The base cost of a memcpy call, for MOVE_RATIO and friends.  These
   values were determined experimentally by benchmarking with CSiBE.
   In theory, the call overhead is higher for TARGET_ABICALLS (especially
   for o32 where we have to restore $gp afterwards as well as make an
   indirect call), but in practice, bumping this up higher for
   TARGET_ABICALLS doesn't make much difference to code size.  */

#define MIPS_CALL_RATIO 8

/* Any loop-based implementation of movmemsi will have at least
   MIPS_MAX_MOVE_BYTES_STRAIGHT / UNITS_PER_WORD memory-to-memory
   moves, so allow individual copies of fewer elements.

   When movmemsi is not available, use a value approximating
   the length of a memcpy call sequence, so that move_by_pieces
   will generate inline code if it is shorter than a function call.
   Since move_by_pieces_ninsns counts memory-to-memory moves, but
   we'll have to generate a load/store pair for each, halve the
   value of MIPS_CALL_RATIO to take that into account.  */

#define MOVE_RATIO(speed)				\
  (HAVE_movmemsi					\
   ? MIPS_MAX_MOVE_BYTES_STRAIGHT / MOVE_MAX		\
   : MIPS_CALL_RATIO / 2)

/* movmemsi is meant to generate code that is at least as good as
   move_by_pieces.  However, movmemsi effectively uses a by-pieces
   implementation both for moves smaller than a word and for word-aligned
   moves of no more than MIPS_MAX_MOVE_BYTES_STRAIGHT bytes.  We should
   allow the tree-level optimisers to do such moves by pieces, as it
   often exposes other optimization opportunities.  We might as well
   continue to use movmemsi at the rtl level though, as it produces
   better code when scheduling is disabled (such as at -O).  */

#define MOVE_BY_PIECES_P(SIZE, ALIGN)				\
  (HAVE_movmemsi						\
   ? (!currently_expanding_to_rtl				\
      && ((ALIGN) < BITS_PER_WORD				\
	  ? (SIZE) < UNITS_PER_WORD				\
	  : (SIZE) <= MIPS_MAX_MOVE_BYTES_STRAIGHT))		\
   : (move_by_pieces_ninsns (SIZE, ALIGN, MOVE_MAX_PIECES + 1)	\
      < (unsigned int) MOVE_RATIO (false)))

/* For CLEAR_RATIO, when optimizing for size, give a better estimate
   of the length of a memset call, but use the default otherwise.  */

#define CLEAR_RATIO(speed)\
  ((speed) ? 15 : MIPS_CALL_RATIO)

/* This is similar to CLEAR_RATIO, but for a non-zero constant, so when
   optimizing for size adjust the ratio to account for the overhead of
   loading the constant and replicating it across the word.  */

#define SET_RATIO(speed) \
  ((speed) ? 15 : MIPS_CALL_RATIO - 2)

/* STORE_BY_PIECES_P can be used when copying a constant string, but
   in that case each word takes 3 insns (lui, ori, sw), or more in
   64-bit mode, instead of 2 (lw, sw).  For now we always fail this
   and let the move_by_pieces code copy the string from read-only
   memory.  In the future, this could be tuned further for multi-issue
   CPUs that can issue stores down one pipe and arithmetic instructions
   down another; in that case, the lui/ori/sw combination would be a
   win for long enough strings.  */

#define STORE_BY_PIECES_P(SIZE, ALIGN) 0

#ifndef HAVE_AS_TLS
#define HAVE_AS_TLS 0
#endif

#ifndef USED_FOR_TARGET

extern const enum reg_class mips_regno_to_class[];
extern bool mips_hard_regno_mode_ok[][FIRST_PSEUDO_REGISTER];
extern const char *current_function_file; /* filename current function is in */
extern int num_source_filenames;	/* current .file # */
extern int mips_dbx_regno[];
extern int mips_dwarf_regno[];
extern bool mips_split_p[];
extern enum processor mips_arch;        /* which cpu to codegen for */
extern enum processor mips_tune;        /* which cpu to schedule for */
extern int mips_abi;			/* which ABI to use */
extern GTY(()) struct target_globals *mips16_globals;
#endif

/* As on most targets, we want the .eh_frame section to be read-only where
   possible.  And as on most targets, this means two things:

     (a) Non-locally-binding pointers must have an indirect encoding,
	 so that the addresses in the .eh_frame section itself become
	 locally-binding.

     (b) A shared library's .eh_frame section must encode locally-binding
	 pointers in a relative (relocation-free) form.

   However, MIPS has traditionally not allowed directives like:

	.long	x-.

   in cases where "x" is in a different section, or is not defined in the
   same assembly file.  We are therefore unable to emit the PC-relative
   form required by (b) at assembly time.

   Fortunately, the linker is able to convert absolute addresses into
   PC-relative addresses on our behalf.  Unfortunately, only certain
   versions of the linker know how to do this for indirect pointers,
   and for personality data.  We must fall back on using writable
   .eh_frame sections for shared libraries if the linker does not
   support this feature.  */
#define ASM_PREFERRED_EH_DATA_FORMAT(CODE,GLOBAL) \
  (((GLOBAL) ? DW_EH_PE_indirect : 0) | DW_EH_PE_absptr)

/* For switching between MIPS16 and non-MIPS16 modes.  */
#define SWITCHABLE_TARGET 1
