/* MIPS-specific support for 32-bit ELF
   Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
   2003, 2004, 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.

   Most of the information added by Ian Lance Taylor, Cygnus Support,
   <ian@cygnus.com>.
   N32/64 ABI support added by Mark Mitchell, CodeSourcery, LLC.
   <mark@codesourcery.com>
   Traditional MIPS targets support added by Koundinya.K, Dansk Data
   Elektronik & Operations Research Group. <kk@ddeorg.soft.net>

   This file is part of BFD, the Binary File Descriptor library.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */


/* This file handles MIPS ELF targets.  SGI Irix 5 uses a slightly
   different MIPS ELF from other targets.  This matters when linking.
   This file supports both, switching at runtime.  */

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "bfdlink.h"
#include "genlink.h"
#include "elf-bfd.h"
#include "elfxx-riscv.h"
#include "elf/riscv.h"
#include "opcode/riscv.h"

#include "opcode/riscv.h"

static bfd_boolean mips_elf_assign_gp
  (bfd *, bfd_vma *);
static bfd_reloc_status_type mips_elf_final_gp
  (bfd *, asymbol *, bfd_boolean, char **, bfd_vma *);
static bfd_reloc_status_type mips_elf_gprel16_reloc
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
static bfd_reloc_status_type mips_elf_literal_reloc
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
static bfd_reloc_status_type mips_elf_gprel32_reloc
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
static bfd_reloc_status_type gprel32_with_gp
  (bfd *, asymbol *, arelent *, asection *, bfd_boolean, void *, bfd_vma);
static reloc_howto_type *bfd_elf32_bfd_reloc_type_lookup
  (bfd *, bfd_reloc_code_real_type);
static reloc_howto_type *mips_elf_n32_rtype_to_howto
  (unsigned int, bfd_boolean);
static void mips_info_to_howto_rel
  (bfd *, arelent *, Elf_Internal_Rela *);
static void mips_info_to_howto_rela
  (bfd *, arelent *, Elf_Internal_Rela *);
static bfd_boolean mips_elf_sym_is_global
  (bfd *, asymbol *);
static bfd_boolean mips_elf_n32_object_p
  (bfd *);
static bfd_boolean elf32_mips_grok_prstatus
  (bfd *, Elf_Internal_Note *);
static bfd_boolean elf32_mips_grok_psinfo
  (bfd *, Elf_Internal_Note *);

extern const bfd_target bfd_elf32_nbigmips_vec;
extern const bfd_target bfd_elf32_nlittlemips_vec;

/* The number of local .got entries we reserve.  */
#define MIPS_RESERVED_GOTNO (2)

/* In case we're on a 32-bit machine, construct a 64-bit "-1" value
   from smaller values.  Start with zero, widen, *then* decrement.  */
#define MINUS_ONE	(((bfd_vma)0) - 1)

/* The relocation table used for SHT_REL sections.  */

static reloc_howto_type elf_mips_howto_table_rel[] =
{
  /* No relocation.  */
  HOWTO (R_RISCV_NONE,		/* type */
	 0,			/* rightshift */
	 0,			/* size (0 = byte, 1 = short, 2 = long) */
	 0,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_NONE",		/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 FALSE),		/* pcrel_offset */

  EMPTY_HOWTO (1),

  /* 32 bit relocation.  */
  HOWTO (R_RISCV_32,		/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_32",		/* name */
	 TRUE,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* 32 bit symbol relative relocation.  */
  HOWTO (R_RISCV_REL32,		/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_REL32",	/* name */
	 TRUE,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* 26 bit jump address.  */
  HOWTO (R_RISCV_26,		/* type */
	 RISCV_JUMP_ALIGN_BITS,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_JUMP_BITS,			/* bitsize */
	 TRUE,			/* pc_relative */
	 OP_SH_TARGET,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
				/* This needs complex overflow
				   detection, because the upper 36
				   bits must match the PC + 4.  */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_26",		/* name */
	 TRUE,			/* partial_inplace */
	 ((1<<RISCV_JUMP_BITS)-1) << OP_SH_TARGET,		/* src_mask */
	 ((1<<RISCV_JUMP_BITS)-1) << OP_SH_TARGET,		/* dst_mask */
	 TRUE),		/* pcrel_offset */

  /* R_RISCV_HI16 and R_RISCV_LO16 are unsupported for NewABI REL.
     However, the native IRIX6 tools use them, so we try our best. */

  /* High 16 bits of symbol value.  */
  HOWTO (R_RISCV_HI16,		/* type */
	 RISCV_IMM_BITS,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_BIGIMM_BITS,			/* bitsize */
	 FALSE,			/* pc_relative */
	 OP_SH_BIGIMMEDIATE,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_hi16_reloc, /* special_function */
	 "R_RISCV_HI16",		/* name */
	 TRUE,			/* partial_inplace */
	 ((1<<RISCV_BIGIMM_BITS)-1) << OP_SH_BIGIMMEDIATE,		/* src_mask */
	 ((1<<RISCV_BIGIMM_BITS)-1) << OP_SH_BIGIMMEDIATE,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Low 16 bits of symbol value.  */
  HOWTO (R_RISCV_LO16,		/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_IMM_BITS,			/* bitsize */
	 FALSE,			/* pc_relative */
	 OP_SH_IMMEDIATE,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_lo16_reloc, /* special_function */
	 "R_RISCV_LO16",		/* name */
	 TRUE,			/* partial_inplace */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,		/* src_mask */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* GP relative reference.  */
  HOWTO (R_RISCV_GPREL16,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_IMM_BITS,			/* bitsize */
	 FALSE,			/* pc_relative */
	 OP_SH_IMMEDIATE,	/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 mips_elf_gprel16_reloc, /* special_function */
	 "R_RISCV_GPREL16",	/* name */
	 TRUE,			/* partial_inplace */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,	/* src_mask */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,	/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Reference to literal section.  */
  HOWTO (R_RISCV_LITERAL,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_IMM_BITS,			/* bitsize */
	 FALSE,			/* pc_relative */
	 OP_SH_IMMEDIATE,	/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 mips_elf_literal_reloc, /* special_function */
	 "R_RISCV_LITERAL",	/* name */
	 TRUE,			/* partial_inplace */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,	/* src_mask */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,	/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Reference to global offset table.  */
  HOWTO (R_RISCV_GOT16,		/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_IMM_BITS,	/* bitsize */
	 FALSE,			/* pc_relative */
	 OP_SH_IMMEDIATE,	/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_riscv_elf_got16_reloc, /* special_function */
	 "R_RISCV_GOT16",	/* name */
	 TRUE,			/* partial_inplace */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,		/* src_mask */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* 16 bit PC relative reference.  Note that the ABI document has a typo
     and claims R_RISCV_PC16 to be not rightshifted, rendering it useless.
     We do the right thing here.  */
  HOWTO (R_RISCV_PC16,		/* type */
	 RISCV_BRANCH_ALIGN_BITS,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_IMM_BITS,			/* bitsize */
	 TRUE,			/* pc_relative */
	 OP_SH_IMMEDIATE,	/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_PC16",		/* name */
	 TRUE,			/* partial_inplace */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,	/* src_mask */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,	/* dst_mask */
	 TRUE),			/* pcrel_offset */

  /* 16 bit call through global offset table.  */
  HOWTO (R_RISCV_CALL16,		/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_IMM_BITS,			/* bitsize */
	 FALSE,			/* pc_relative */
	 OP_SH_IMMEDIATE,	/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_CALL16",	/* name */
	 TRUE,			/* partial_inplace */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,	/* src_mask */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,	/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* 32 bit GP relative reference.  */
  HOWTO (R_RISCV_GPREL32,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 mips_elf_gprel32_reloc, /* special_function */
	 "R_RISCV_GPREL32",	/* name */
	 TRUE,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  EMPTY_HOWTO (13),
  EMPTY_HOWTO (14),
  EMPTY_HOWTO (15),
  EMPTY_HOWTO (16),
  EMPTY_HOWTO (17),

  /* 64 bit relocation.  */
  HOWTO (R_RISCV_64,		/* type */
	 0,			/* rightshift */
	 4,			/* size (0 = byte, 1 = short, 2 = long) */
	 64,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_64",		/* name */
	 TRUE,			/* partial_inplace */
	 MINUS_ONE,		/* src_mask */
	 MINUS_ONE,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Displacement in the global offset table.  */
  HOWTO (R_RISCV_GOT_DISP,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_IMM_BITS,			/* bitsize */
	 FALSE,			/* pc_relative */
	 OP_SH_IMMEDIATE,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_GOT_DISP",	/* name */
	 TRUE,			/* partial_inplace */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,		/* src_mask */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  EMPTY_HOWTO (20),
  EMPTY_HOWTO (21),

  /* High 16 bits of displacement in global offset table.  */
  HOWTO (R_RISCV_GOT_HI16,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_BIGIMM_BITS,	/* bitsize */
	 TRUE,			/* pc_relative */
	 OP_SH_BIGIMMEDIATE,	/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_GOT_HI16",	/* name */
	 TRUE,			/* partial_inplace */
	 ((1<<RISCV_BIGIMM_BITS)-1) << OP_SH_BIGIMMEDIATE,		/* src_mask */
	 ((1<<RISCV_BIGIMM_BITS)-1) << OP_SH_BIGIMMEDIATE,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Low 16 bits of displacement in global offset table.  */
  HOWTO (R_RISCV_GOT_LO16,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_IMM_BITS,			/* bitsize */
	 TRUE,			/* pc_relative */
	 OP_SH_IMMEDIATE,	/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_GOT_LO16",	/* name */
	 TRUE,			/* partial_inplace */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,		/* src_mask */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* 64 bit subtraction.  */
  HOWTO (R_RISCV_SUB,		/* type */
	 0,			/* rightshift */
	 4,			/* size (0 = byte, 1 = short, 2 = long) */
	 64,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_SUB",		/* name */
	 TRUE,			/* partial_inplace */
	 MINUS_ONE,		/* src_mask */
	 MINUS_ONE,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Insert the addend as an instruction.  */
  /* FIXME: Not handled correctly.  */
  HOWTO (R_RISCV_INSERT_A,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_INSERT_A",	/* name */
	 TRUE,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Insert the addend as an instruction, and change all relocations
     to refer to the old instruction at the address.  */
  /* FIXME: Not handled correctly.  */
  HOWTO (R_RISCV_INSERT_B,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_INSERT_B",	/* name */
	 TRUE,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Delete a 32 bit instruction.  */
  /* FIXME: Not handled correctly.  */
  HOWTO (R_RISCV_DELETE,		/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_DELETE",	/* name */
	 TRUE,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  EMPTY_HOWTO (28),
  EMPTY_HOWTO (29),

  /* High 16 bits of displacement in global offset table.  */
  HOWTO (R_RISCV_CALL_HI16,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_BIGIMM_BITS,			/* bitsize */
	 FALSE,			/* pc_relative */
	 OP_SH_BIGIMMEDIATE,	/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_CALL_HI16",	/* name */
	 TRUE,			/* partial_inplace */
	 ((1<<RISCV_BIGIMM_BITS)-1) << OP_SH_BIGIMMEDIATE,	/* src_mask */
	 ((1<<RISCV_BIGIMM_BITS)-1) << OP_SH_BIGIMMEDIATE,	/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Low 16 bits of displacement in global offset table.  */
  HOWTO (R_RISCV_CALL_LO16,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_IMM_BITS,			/* bitsize */
	 FALSE,			/* pc_relative */
	 OP_SH_IMMEDIATE,	/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_CALL_LO16",	/* name */
	 TRUE,			/* partial_inplace */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,	/* src_mask */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,	/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Section displacement, used by an associated event location section.  */
  HOWTO (R_RISCV_SCN_DISP,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_SCN_DISP",	/* name */
	 TRUE,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  HOWTO (R_RISCV_REL16,		/* type */
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_IMM_BITS,			/* bitsize */
	 FALSE,			/* pc_relative */
	 OP_SH_IMMEDIATE,	/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_REL16",	/* name */
	 TRUE,			/* partial_inplace */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,	/* src_mask */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,	/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* These two are obsolete.  */
  EMPTY_HOWTO (R_RISCV_ADD_IMMEDIATE),
  EMPTY_HOWTO (R_RISCV_PJUMP),

  /* Similiar to R_RISCV_REL32, but used for relocations in a GOT section.
     It must be used for multigot GOT's (and only there).  */
  HOWTO (R_RISCV_RELGOT,		/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_RELGOT",	/* name */
	 TRUE,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  EMPTY_HOWTO(37),

  /* TLS relocations.  */
  HOWTO (R_RISCV_TLS_DTPMOD32,	/* type */
	 0,			/* rightshift */
	 4,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc, /* special_function */
	 "R_RISCV_TLS_DTPMOD32",	/* name */
	 TRUE,			/* partial_inplace */
	 MINUS_ONE,		/* src_mask */
	 MINUS_ONE,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  HOWTO (R_RISCV_TLS_DTPREL32,	/* type */
	 0,			/* rightshift */
	 4,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc, /* special_function */
	 "R_RISCV_TLS_DTPREL32",	/* name */
	 TRUE,			/* partial_inplace */
	 MINUS_ONE,		/* src_mask */
	 MINUS_ONE,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  EMPTY_HOWTO (R_RISCV_TLS_DTPMOD64),
  EMPTY_HOWTO (R_RISCV_TLS_DTPREL64),

  /* TLS general dynamic variable reference.  */
  HOWTO (R_RISCV_TLS_GD,		/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_IMM_BITS,			/* bitsize */
	 FALSE,			/* pc_relative */
	 OP_SH_IMMEDIATE,	/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc, /* special_function */
	 "R_RISCV_TLS_GD",	/* name */
	 TRUE,			/* partial_inplace */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,	/* src_mask */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,	/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* TLS local dynamic variable reference.  */
  HOWTO (R_RISCV_TLS_LDM,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_IMM_BITS,			/* bitsize */
	 FALSE,			/* pc_relative */
	 OP_SH_IMMEDIATE,	/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc, /* special_function */
	 "R_RISCV_TLS_LDM",	/* name */
	 TRUE,			/* partial_inplace */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,	/* src_mask */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,	/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* TLS local dynamic offset.  */
  HOWTO (R_RISCV_TLS_DTPREL_HI16,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_BIGIMM_BITS,			/* bitsize */
	 FALSE,			/* pc_relative */
	 OP_SH_BIGIMMEDIATE,	/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc, /* special_function */
	 "R_RISCV_TLS_DTPREL_HI16",	/* name */
	 TRUE,			/* partial_inplace */
	 ((1<<RISCV_BIGIMM_BITS)-1) << OP_SH_BIGIMMEDIATE,	/* src_mask */
	 ((1<<RISCV_BIGIMM_BITS)-1) << OP_SH_BIGIMMEDIATE,	/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* TLS local dynamic offset.  */
  HOWTO (R_RISCV_TLS_DTPREL_LO16,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_IMM_BITS,			/* bitsize */
	 FALSE,			/* pc_relative */
	 OP_SH_IMMEDIATE,	/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc, /* special_function */
	 "R_RISCV_TLS_DTPREL_LO16",	/* name */
	 TRUE,			/* partial_inplace */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,	/* src_mask */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,	/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* TLS thread pointer offset.  */
  HOWTO (R_RISCV_TLS_GOTTPREL,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_IMM_BITS,			/* bitsize */
	 FALSE,			/* pc_relative */
	 OP_SH_IMMEDIATE,	/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc, /* special_function */
	 "R_RISCV_TLS_GOTTPREL",	/* name */
	 TRUE,			/* partial_inplace */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,	/* src_mask */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,	/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* TLS IE dynamic relocations.  */
  HOWTO (R_RISCV_TLS_TPREL32,	/* type */
	 0,			/* rightshift */
	 4,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc, /* special_function */
	 "R_RISCV_TLS_TPREL32",	/* name */
	 TRUE,			/* partial_inplace */
	 MINUS_ONE,		/* src_mask */
	 MINUS_ONE,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  EMPTY_HOWTO (R_RISCV_TLS_TPREL64),

  /* TLS thread pointer offset.  */
  HOWTO (R_RISCV_TLS_TPREL_HI16,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_BIGIMM_BITS,			/* bitsize */
	 FALSE,			/* pc_relative */
	 OP_SH_BIGIMMEDIATE,	/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc, /* special_function */
	 "R_RISCV_TLS_TPREL_HI16", /* name */
	 TRUE,			/* partial_inplace */
	 ((1<<RISCV_BIGIMM_BITS)-1) << OP_SH_BIGIMMEDIATE,	/* src_mask */
	 ((1<<RISCV_BIGIMM_BITS)-1) << OP_SH_BIGIMMEDIATE,	/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* TLS thread pointer offset.  */
  HOWTO (R_RISCV_TLS_TPREL_LO16,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_IMM_BITS,			/* bitsize */
	 FALSE,			/* pc_relative */
	 OP_SH_IMMEDIATE,	/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc, /* special_function */
	 "R_RISCV_TLS_TPREL_LO16", /* name */
	 TRUE,			/* partial_inplace */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,	/* src_mask */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,	/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* High 16 bits of displacement in global offset table.  */
  HOWTO (R_RISCV_TLS_GOT_HI16,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_BIGIMM_BITS,	/* bitsize */
	 TRUE,			/* pc_relative */
	 OP_SH_BIGIMMEDIATE,	/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_TLS_GOT_HI16",	/* name */
	 TRUE,			/* partial_inplace */
	 ((1<<RISCV_BIGIMM_BITS)-1) << OP_SH_BIGIMMEDIATE,		/* src_mask */
	 ((1<<RISCV_BIGIMM_BITS)-1) << OP_SH_BIGIMMEDIATE,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* TLS thread pointer offset.  */
  HOWTO (R_RISCV_TLS_GOT_LO16,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_IMM_BITS,			/* bitsize */
	 TRUE,			/* pc_relative */
	 OP_SH_IMMEDIATE,	/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc, /* special_function */
	 "R_RISCV_TLS_GOT_LO16",	/* name */
	 TRUE,			/* partial_inplace */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,	/* src_mask */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,	/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* High 16 bits of displacement in global offset table.  */
  HOWTO (R_RISCV_TLS_GD_HI16,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_BIGIMM_BITS,	/* bitsize */
	 FALSE,			/* pc_relative */
	 OP_SH_BIGIMMEDIATE,	/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_TLS_GD_HI16",	/* name */
	 TRUE,			/* partial_inplace */
	 ((1<<RISCV_BIGIMM_BITS)-1) << OP_SH_BIGIMMEDIATE,		/* src_mask */
	 ((1<<RISCV_BIGIMM_BITS)-1) << OP_SH_BIGIMMEDIATE,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* TLS thread pointer offset.  */
  HOWTO (R_RISCV_TLS_GD_LO16,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_IMM_BITS,			/* bitsize */
	 FALSE,			/* pc_relative */
	 OP_SH_IMMEDIATE,	/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc, /* special_function */
	 "R_RISCV_TLS_GD_LO16",	/* name */
	 TRUE,			/* partial_inplace */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,	/* src_mask */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,	/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* High 16 bits of displacement in global offset table.  */
  HOWTO (R_RISCV_TLS_LDM_HI16,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_BIGIMM_BITS,	/* bitsize */
	 FALSE,			/* pc_relative */
	 OP_SH_BIGIMMEDIATE,	/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_TLS_LDM_HI16",	/* name */
	 TRUE,			/* partial_inplace */
	 ((1<<RISCV_BIGIMM_BITS)-1) << OP_SH_BIGIMMEDIATE,		/* src_mask */
	 ((1<<RISCV_BIGIMM_BITS)-1) << OP_SH_BIGIMMEDIATE,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* TLS thread pointer offset.  */
  HOWTO (R_RISCV_TLS_LDM_LO16,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_IMM_BITS,			/* bitsize */
	 FALSE,			/* pc_relative */
	 OP_SH_IMMEDIATE,	/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc, /* special_function */
	 "R_RISCV_TLS_LDM_LO16",	/* name */
	 TRUE,			/* partial_inplace */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,	/* src_mask */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,	/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* 32 bit relocation with no addend.  */
  HOWTO (R_RISCV_GLOB_DAT,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc, /* special_function */
	 "R_RISCV_GLOB_DAT",	/* name */
	 FALSE,			/* partial_inplace */
	 0x0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */
};

/* The relocation table used for SHT_RELA sections.  */

static reloc_howto_type elf_mips_howto_table_rela[] =
{
  /* No relocation.  */
  HOWTO (R_RISCV_NONE,		/* type */
	 0,			/* rightshift */
	 0,			/* size (0 = byte, 1 = short, 2 = long) */
	 0,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_NONE",		/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 FALSE),		/* pcrel_offset */

  EMPTY_HOWTO (1),

  /* 32 bit relocation.  */
  HOWTO (R_RISCV_32,		/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_32",		/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* 32 bit symbol relative relocation.  */
  HOWTO (R_RISCV_REL32,		/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_REL32",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* 26 bit jump address.  */
  HOWTO (R_RISCV_26,		/* type */
	 RISCV_JUMP_ALIGN_BITS,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_JUMP_BITS,			/* bitsize */
	 TRUE,			/* pc_relative */
	 OP_SH_TARGET,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
				/* This needs complex overflow
				   detection, because the upper 36
				   bits must match the PC + 4.  */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_26",		/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 ((1<<RISCV_JUMP_BITS)-1) << OP_SH_TARGET,		/* dst_mask */
	 TRUE),		/* pcrel_offset */

  /* High 16 bits of symbol value.  */
  HOWTO (R_RISCV_HI16,		/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_BIGIMM_BITS,			/* bitsize */
	 FALSE,			/* pc_relative */
	 OP_SH_BIGIMMEDIATE,	/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_HI16",		/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 ((1<<RISCV_BIGIMM_BITS)-1) << OP_SH_BIGIMMEDIATE,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Low 16 bits of symbol value.  */
  HOWTO (R_RISCV_LO16,		/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_IMM_BITS,			/* bitsize */
	 FALSE,			/* pc_relative */
	 OP_SH_IMMEDIATE,	/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_LO16",		/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* GP relative reference.  */
  HOWTO (R_RISCV_GPREL16,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_IMM_BITS,			/* bitsize */
	 FALSE,			/* pc_relative */
	 OP_SH_IMMEDIATE,	/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 mips_elf_gprel16_reloc, /* special_function */
	 "R_RISCV_GPREL16",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,	/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Reference to literal section.  */
  HOWTO (R_RISCV_LITERAL,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_IMM_BITS,			/* bitsize */
	 FALSE,			/* pc_relative */
	 OP_SH_IMMEDIATE,	/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 mips_elf_literal_reloc, /* special_function */
	 "R_RISCV_LITERAL",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,	/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Reference to global offset table.  */
  HOWTO (R_RISCV_GOT16,		/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_IMM_BITS,			/* bitsize */
	 FALSE,			/* pc_relative */
	 OP_SH_IMMEDIATE,	/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc, /* special_function */
	 "R_RISCV_GOT16",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,	/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* 16 bit PC relative reference.  Note that the ABI document has a typo
     and claims R_RISCV_PC16 to be not rightshifted, rendering it useless.
     We do the right thing here.  */
  HOWTO (R_RISCV_PC16,		/* type */
	 2,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_IMM_BITS,			/* bitsize */
	 TRUE,			/* pc_relative */
	 OP_SH_IMMEDIATE,	/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_PC16",		/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,	/* dst_mask */
	 TRUE),			/* pcrel_offset */

  /* 16 bit call through global offset table.  */
  HOWTO (R_RISCV_CALL16,		/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_IMM_BITS,			/* bitsize */
	 FALSE,			/* pc_relative */
	 OP_SH_IMMEDIATE,	/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_CALL16",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,	/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* 32 bit GP relative reference.  */
  HOWTO (R_RISCV_GPREL32,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 mips_elf_gprel32_reloc, /* special_function */
	 "R_RISCV_GPREL32",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  EMPTY_HOWTO (13),
  EMPTY_HOWTO (14),
  EMPTY_HOWTO (15),
  EMPTY_HOWTO (16),
  EMPTY_HOWTO (17),

  /* 64 bit relocation.  */
  HOWTO (R_RISCV_64,		/* type */
	 0,			/* rightshift */
	 4,			/* size (0 = byte, 1 = short, 2 = long) */
	 64,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_64",		/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 MINUS_ONE,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Displacement in the global offset table.  */
  HOWTO (R_RISCV_GOT_DISP,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_IMM_BITS,			/* bitsize */
	 FALSE,			/* pc_relative */
	 OP_SH_IMMEDIATE,	/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_GOT_DISP",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,	/* dst_mask */
	 FALSE),		/* pcrel_offset */

  EMPTY_HOWTO (20),
  EMPTY_HOWTO (21),

  /* High 16 bits of displacement in global offset table.  */
  HOWTO (R_RISCV_GOT_HI16,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_BIGIMM_BITS,			/* bitsize */
	 TRUE,			/* pc_relative */
	 OP_SH_BIGIMMEDIATE,	/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_GOT_HI16",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 ((1<<RISCV_BIGIMM_BITS)-1) << OP_SH_BIGIMMEDIATE,	/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Low 16 bits of displacement in global offset table.  */
  HOWTO (R_RISCV_GOT_LO16,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_IMM_BITS,			/* bitsize */
	 TRUE,			/* pc_relative */
	 OP_SH_IMMEDIATE,	/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_GOT_LO16",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,	/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* 64 bit subtraction.  */
  HOWTO (R_RISCV_SUB,		/* type */
	 0,			/* rightshift */
	 4,			/* size (0 = byte, 1 = short, 2 = long) */
	 64,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_SUB",		/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 MINUS_ONE,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Insert the addend as an instruction.  */
  /* FIXME: Not handled correctly.  */
  HOWTO (R_RISCV_INSERT_A,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_INSERT_A",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Insert the addend as an instruction, and change all relocations
     to refer to the old instruction at the address.  */
  /* FIXME: Not handled correctly.  */
  HOWTO (R_RISCV_INSERT_B,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_INSERT_B",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Delete a 32 bit instruction.  */
  /* FIXME: Not handled correctly.  */
  HOWTO (R_RISCV_DELETE,		/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_DELETE",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  EMPTY_HOWTO (28),
  EMPTY_HOWTO (29),

  /* High 16 bits of displacement in global offset table.  */
  HOWTO (R_RISCV_CALL_HI16,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_BIGIMM_BITS,			/* bitsize */
	 FALSE,			/* pc_relative */
	 OP_SH_BIGIMMEDIATE,	/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_CALL_HI16",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 ((1<<RISCV_BIGIMM_BITS)-1) << OP_SH_BIGIMMEDIATE,	/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Low 16 bits of displacement in global offset table.  */
  HOWTO (R_RISCV_CALL_LO16,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_IMM_BITS,			/* bitsize */
	 FALSE,			/* pc_relative */
	 OP_SH_IMMEDIATE,	/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_CALL_LO16",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,	/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Section displacement, used by an associated event location section.  */
  HOWTO (R_RISCV_SCN_DISP,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_SCN_DISP",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  HOWTO (R_RISCV_REL16,		/* type */
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_IMM_BITS,			/* bitsize */
	 FALSE,			/* pc_relative */
	 OP_SH_IMMEDIATE,	/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_REL16",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,	/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* These two are obsolete.  */
  EMPTY_HOWTO (R_RISCV_ADD_IMMEDIATE),
  EMPTY_HOWTO (R_RISCV_PJUMP),

  /* Similiar to R_RISCV_REL32, but used for relocations in a GOT section.
     It must be used for multigot GOT's (and only there).  */
  HOWTO (R_RISCV_RELGOT,		/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_RELGOT",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  EMPTY_HOWTO(37),

  /* TLS relocations.  */
  HOWTO (R_RISCV_TLS_DTPMOD32,	/* type */
	 0,			/* rightshift */
	 4,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc, /* special_function */
	 "R_RISCV_TLS_DTPMOD32", /* name */
	 FALSE,			/* partial_inplace */
	 MINUS_ONE,		/* src_mask */
	 MINUS_ONE,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  HOWTO (R_RISCV_TLS_DTPREL32,	/* type */
	 0,			/* rightshift */
	 4,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc, /* special_function */
	 "R_RISCV_TLS_DTPREL32",	/* name */
	 TRUE,			/* partial_inplace */
	 MINUS_ONE,		/* src_mask */
	 MINUS_ONE,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  EMPTY_HOWTO (R_RISCV_TLS_DTPMOD64),
  EMPTY_HOWTO (R_RISCV_TLS_DTPREL64),

  /* TLS general dynamic variable reference.  */
  HOWTO (R_RISCV_TLS_GD,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_IMM_BITS,	/* bitsize */
	 FALSE,			/* pc_relative */
	 OP_SH_IMMEDIATE,	/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc, /* special_function */
	 "R_RISCV_TLS_GD",	/* name */
	 TRUE,			/* partial_inplace */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,	/* src_mask */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,	/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* TLS local dynamic variable reference.  */
  HOWTO (R_RISCV_TLS_LDM,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_IMM_BITS,	/* bitsize */
	 FALSE,			/* pc_relative */
	 OP_SH_IMMEDIATE,	/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc, /* special_function */
	 "R_RISCV_TLS_LDM",	/* name */
	 TRUE,			/* partial_inplace */
	 0,			/* src_mask */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,	/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* TLS local dynamic offset.  */
  HOWTO (R_RISCV_TLS_DTPREL_HI16,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_BIGIMM_BITS,	/* bitsize */
	 FALSE,			/* pc_relative */
	 OP_SH_BIGIMMEDIATE,	/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc, /* special_function */
	 "R_RISCV_TLS_DTPREL_HI16",	/* name */
	 TRUE,			/* partial_inplace */
	 0,			/* src_mask */
	 ((1<<RISCV_BIGIMM_BITS)-1) << OP_SH_BIGIMMEDIATE,	/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* TLS local dynamic offset.  */
  HOWTO (R_RISCV_TLS_DTPREL_LO16,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_IMM_BITS,	/* bitsize */
	 FALSE,			/* pc_relative */
	 OP_SH_IMMEDIATE,	/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc, /* special_function */
	 "R_RISCV_TLS_DTPREL_LO16",	/* name */
	 TRUE,			/* partial_inplace */
	 0,			/* src_mask */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,	/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* TLS thread pointer offset.  */
  HOWTO (R_RISCV_TLS_GOTTPREL,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_IMM_BITS,	/* bitsize */
	 FALSE,			/* pc_relative */
	 OP_SH_IMMEDIATE,	/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc, /* special_function */
	 "R_RISCV_TLS_GOTTPREL",	/* name */
	 TRUE,			/* partial_inplace */
	 0,			/* src_mask */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,	/* dst_mask */
	 FALSE),		/* pcrel_offset */

  HOWTO (R_RISCV_TLS_TPREL32,	/* type */
	 0,			/* rightshift */
	 4,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc, /* special_function */
	 "R_RISCV_TLS_TPREL32",	/* name */
	 FALSE,			/* partial_inplace */
	 MINUS_ONE,		/* src_mask */
	 MINUS_ONE,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  EMPTY_HOWTO (R_RISCV_TLS_TPREL64),

  /* TLS thread pointer offset.  */
  HOWTO (R_RISCV_TLS_TPREL_HI16,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_BIGIMM_BITS,	/* bitsize */
	 FALSE,			/* pc_relative */
	 OP_SH_BIGIMMEDIATE,	/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc, /* special_function */
	 "R_RISCV_TLS_TPREL_HI16", /* name */
	 TRUE,			/* partial_inplace */
	 0,			/* src_mask */
	 ((1<<RISCV_BIGIMM_BITS)-1) << OP_SH_BIGIMMEDIATE,	/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* TLS thread pointer offset.  */
  HOWTO (R_RISCV_TLS_TPREL_LO16,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_IMM_BITS,	/* bitsize */
	 FALSE,			/* pc_relative */
	 OP_SH_IMMEDIATE,	/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc, /* special_function */
	 "R_RISCV_TLS_TPREL_LO16", /* name */
	 TRUE,			/* partial_inplace */
	 0,			/* src_mask */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,	/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* High 16 bits of displacement in global offset table.  */
  HOWTO (R_RISCV_TLS_GOT_HI16,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_BIGIMM_BITS,			/* bitsize */
	 TRUE,			/* pc_relative */
	 OP_SH_BIGIMMEDIATE,	/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_TLS_GOT_HI16",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 ((1<<RISCV_BIGIMM_BITS)-1) << OP_SH_BIGIMMEDIATE,	/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Low 16 bits of displacement in global offset table.  */
  HOWTO (R_RISCV_TLS_GOT_LO16,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_IMM_BITS,			/* bitsize */
	 TRUE,			/* pc_relative */
	 OP_SH_IMMEDIATE,	/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_TLS_GOT_LO16",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,	/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* High 16 bits of displacement in global offset table.  */
  HOWTO (R_RISCV_TLS_GD_HI16,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_BIGIMM_BITS,			/* bitsize */
	 FALSE,			/* pc_relative */
	 OP_SH_BIGIMMEDIATE,	/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_TLS_GD_HI16",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 ((1<<RISCV_BIGIMM_BITS)-1) << OP_SH_BIGIMMEDIATE,	/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Low 16 bits of displacement in global offset table.  */
  HOWTO (R_RISCV_TLS_GD_LO16,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_IMM_BITS,			/* bitsize */
	 FALSE,			/* pc_relative */
	 OP_SH_IMMEDIATE,	/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_TLS_GD_LO16",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,	/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* High 16 bits of displacement in global offset table.  */
  HOWTO (R_RISCV_TLS_LDM_HI16,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_BIGIMM_BITS,			/* bitsize */
	 FALSE,			/* pc_relative */
	 OP_SH_BIGIMMEDIATE,	/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_TLS_LDM_HI16",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 ((1<<RISCV_BIGIMM_BITS)-1) << OP_SH_BIGIMMEDIATE,	/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Low 16 bits of displacement in global offset table.  */
  HOWTO (R_RISCV_TLS_LDM_LO16,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_IMM_BITS,			/* bitsize */
	 FALSE,			/* pc_relative */
	 OP_SH_IMMEDIATE,	/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_TLS_LDM_LO16",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 (RISCV_IMM_REACH-1) << OP_SH_IMMEDIATE,	/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* 32 bit relocation with no addend.  */
  HOWTO (R_RISCV_GLOB_DAT,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc, /* special_function */
	 "R_RISCV_GLOB_DAT",	/* name */
	 FALSE,			/* partial_inplace */
	 0x0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */
};

/* GNU extension to record C++ vtable hierarchy */
static reloc_howto_type elf_mips_gnu_vtinherit_howto =
  HOWTO (R_RISCV_GNU_VTINHERIT,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 0,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 NULL,			/* special_function */
	 "R_RISCV_GNU_VTINHERIT", /* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 FALSE);		/* pcrel_offset */

/* GNU extension to record C++ vtable member usage */
static reloc_howto_type elf_mips_gnu_vtentry_howto =
  HOWTO (R_RISCV_GNU_VTENTRY,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 0,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_elf_rel_vtable_reloc_fn, /* special_function */
	 "R_RISCV_GNU_VTENTRY",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 FALSE);		/* pcrel_offset */

/* 16 bit offset for pc-relative branches.  */
static reloc_howto_type elf_mips_gnu_rel16_s2 =
  HOWTO (R_RISCV_GNU_REL16_S2,	/* type */
	 2,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 TRUE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc, /* special_function */
	 "R_RISCV_GNU_REL16_S2",	/* name */
	 TRUE,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 TRUE);			/* pcrel_offset */

/* 16 bit offset for pc-relative branches.  */
static reloc_howto_type elf_mips_gnu_rela16_s2 =
  HOWTO (R_RISCV_GNU_REL16_S2,	/* type */
	 2,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 TRUE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc, /* special_function */
	 "R_RISCV_GNU_REL16_S2",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 TRUE);			/* pcrel_offset */

/* Originally a VxWorks extension, but now used for other systems too.  */
static reloc_howto_type elf_mips_copy_howto =
  HOWTO (R_RISCV_COPY,		/* type */
	 0,			/* rightshift */
	 0,			/* this one is variable size */
	 0,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_RISCV_COPY",		/* name */
	 FALSE,			/* partial_inplace */
	 0x0,         		/* src_mask */
	 0x0,		        /* dst_mask */
	 FALSE);		/* pcrel_offset */

/* Originally a VxWorks extension, but now used for other systems too.  */
static reloc_howto_type elf_mips_jump_slot_howto =
  HOWTO (R_RISCV_JUMP_SLOT,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_RISCV_JUMP_SLOT",	/* name */
	 FALSE,			/* partial_inplace */
	 0x0,         		/* src_mask */
	 0x0,		        /* dst_mask */
	 FALSE);		/* pcrel_offset */

/* Set the GP value for OUTPUT_BFD.  Returns FALSE if this is a
   dangerous relocation.  */

static bfd_boolean
mips_elf_assign_gp (bfd *output_bfd, bfd_vma *pgp)
{
  unsigned int count;
  asymbol **sym;
  unsigned int i;

  /* If we've already figured out what GP will be, just return it.  */
  *pgp = _bfd_get_gp_value (output_bfd);
  if (*pgp)
    return TRUE;

  count = bfd_get_symcount (output_bfd);
  sym = bfd_get_outsymbols (output_bfd);

  /* The linker script will have created a symbol named `_gp' with the
     appropriate value.  */
  if (sym == NULL)
    i = count;
  else
    {
      for (i = 0; i < count; i++, sym++)
	{
	  register const char *name;

	  name = bfd_asymbol_name (*sym);
	  if (*name == '_' && strcmp (name, "_gp") == 0)
	    {
	      *pgp = bfd_asymbol_value (*sym);
	      _bfd_set_gp_value (output_bfd, *pgp);
	      break;
	    }
	}
    }

  if (i >= count)
    {
      /* Only get the error once.  */
      *pgp = 4;
      _bfd_set_gp_value (output_bfd, *pgp);
      return FALSE;
    }

  return TRUE;
}

/* We have to figure out the gp value, so that we can adjust the
   symbol value correctly.  We look up the symbol _gp in the output
   BFD.  If we can't find it, we're stuck.  We cache it in the ELF
   target data.  We don't need to adjust the symbol value for an
   external symbol if we are producing relocatable output.  */

static bfd_reloc_status_type
mips_elf_final_gp (bfd *output_bfd, asymbol *symbol, bfd_boolean relocatable,
		   char **error_message, bfd_vma *pgp)
{
  if (bfd_is_und_section (symbol->section)
      && ! relocatable)
    {
      *pgp = 0;
      return bfd_reloc_undefined;
    }

  *pgp = _bfd_get_gp_value (output_bfd);
  if (*pgp == 0
      && (! relocatable
	  || (symbol->flags & BSF_SECTION_SYM) != 0))
    {
      if (relocatable)
	{
	  /* Make up a value.  */
	  *pgp = symbol->section->output_section->vma /*+ 0x4000*/;
	  _bfd_set_gp_value (output_bfd, *pgp);
	}
      else if (!mips_elf_assign_gp (output_bfd, pgp))
	{
	  *error_message =
	    (char *) _("GP relative relocation when _gp not defined");
	  return bfd_reloc_dangerous;
	}
    }

  return bfd_reloc_ok;
}

/* Do a R_RISCV_GPREL16 relocation.  This is a 16 bit value which must
   become the offset from the gp register.  */

static bfd_reloc_status_type
mips_elf_gprel16_reloc (bfd *abfd ATTRIBUTE_UNUSED, arelent *reloc_entry,
			asymbol *symbol, void *data ATTRIBUTE_UNUSED,
			asection *input_section, bfd *output_bfd,
			char **error_message ATTRIBUTE_UNUSED)
{
  bfd_boolean relocatable;
  bfd_reloc_status_type ret;
  bfd_vma gp;

  if (output_bfd != NULL)
    relocatable = TRUE;
  else
    {
      relocatable = FALSE;
      output_bfd = symbol->section->output_section->owner;
    }

  ret = mips_elf_final_gp (output_bfd, symbol, relocatable, error_message,
			   &gp);
  if (ret != bfd_reloc_ok)
    return ret;

  return _bfd_riscv_elf_gprel16_with_gp (abfd, symbol, reloc_entry,
					input_section, relocatable,
					data, gp);
}

/* Do a R_RISCV_LITERAL relocation.  */

static bfd_reloc_status_type
mips_elf_literal_reloc (bfd *abfd, arelent *reloc_entry, asymbol *symbol,
			void *data, asection *input_section, bfd *output_bfd,
			char **error_message)
{
  bfd_boolean relocatable;
  bfd_reloc_status_type ret;
  bfd_vma gp;

  /* R_RISCV_LITERAL relocations are defined for local symbols only.  */
  if (output_bfd != NULL
      && (symbol->flags & BSF_SECTION_SYM) == 0
      && (symbol->flags & BSF_LOCAL) != 0)
    {
      *error_message = (char *)
	_("literal relocation occurs for an external symbol");
      return bfd_reloc_outofrange;
    }

  /* FIXME: The entries in the .lit8 and .lit4 sections should be merged.  */
  if (output_bfd != NULL)
    relocatable = TRUE;
  else
    {
      relocatable = FALSE;
      output_bfd = symbol->section->output_section->owner;
    }

  ret = mips_elf_final_gp (output_bfd, symbol, relocatable, error_message,
			   &gp);
  if (ret != bfd_reloc_ok)
    return ret;

  return _bfd_riscv_elf_gprel16_with_gp (abfd, symbol, reloc_entry,
					input_section, relocatable,
					data, gp);
}

/* Do a R_RISCV_GPREL32 relocation.  This is a 32 bit value which must
   become the offset from the gp register.  */

static bfd_reloc_status_type
mips_elf_gprel32_reloc (bfd *abfd, arelent *reloc_entry, asymbol *symbol,
			void *data, asection *input_section, bfd *output_bfd,
			char **error_message)
{
  bfd_boolean relocatable;
  bfd_reloc_status_type ret;
  bfd_vma gp;

  /* R_RISCV_GPREL32 relocations are defined for local symbols only.  */
  if (output_bfd != NULL
      && (symbol->flags & BSF_SECTION_SYM) == 0
      && (symbol->flags & BSF_LOCAL) != 0)
    {
      *error_message = (char *)
	_("32bits gp relative relocation occurs for an external symbol");
      return bfd_reloc_outofrange;
    }

  if (output_bfd != NULL)
    {
      relocatable = TRUE;
      gp = _bfd_get_gp_value (output_bfd);
    }
  else
    {
      relocatable = FALSE;
      output_bfd = symbol->section->output_section->owner;

      ret = mips_elf_final_gp (output_bfd, symbol, relocatable,
			       error_message, &gp);
      if (ret != bfd_reloc_ok)
	return ret;
    }

  return gprel32_with_gp (abfd, symbol, reloc_entry, input_section,
			  relocatable, data, gp);
}

static bfd_reloc_status_type
gprel32_with_gp (bfd *abfd, asymbol *symbol, arelent *reloc_entry,
		 asection *input_section, bfd_boolean relocatable,
		 void *data, bfd_vma gp)
{
  bfd_vma relocation;
  unsigned long val;

  if (bfd_is_com_section (symbol->section))
    relocation = 0;
  else
    relocation = symbol->value;

  relocation += symbol->section->output_section->vma;
  relocation += symbol->section->output_offset;

  if (reloc_entry->address > bfd_get_section_limit (abfd, input_section))
    return bfd_reloc_outofrange;

  if (reloc_entry->howto->src_mask == 0)
    val = 0;
  else
    val = bfd_get_32 (abfd, (bfd_byte *) data + reloc_entry->address);

  /* Set val to the offset into the section or symbol.  */
  val += reloc_entry->addend;

  /* Adjust val for the final section location and GP value.  If we
     are producing relocatable output, we don't want to do this for
     an external symbol.  */
  if (! relocatable
      || (symbol->flags & BSF_SECTION_SYM) != 0)
    val += relocation - gp;

  bfd_put_32 (abfd, val, (bfd_byte *) data + reloc_entry->address);

  if (relocatable)
    reloc_entry->address += input_section->output_offset;

  return bfd_reloc_ok;
}

/* A mapping from BFD reloc types to MIPS ELF reloc types.  */

struct elf_reloc_map {
  bfd_reloc_code_real_type bfd_val;
  enum elf_riscv_reloc_type elf_val;
};

static const struct elf_reloc_map mips_reloc_map[] =
{
  { BFD_RELOC_NONE, R_RISCV_NONE },
  { BFD_RELOC_32, R_RISCV_32 },
  /* There is no BFD reloc for R_RISCV_REL32.  */
  { BFD_RELOC_CTOR, R_RISCV_32 },
  { BFD_RELOC_64, R_RISCV_64 },
  { BFD_RELOC_16_PCREL_S2, R_RISCV_PC16 },
  { BFD_RELOC_HI16_S, R_RISCV_HI16 },
  { BFD_RELOC_LO16, R_RISCV_LO16 },
  { BFD_RELOC_GPREL16, R_RISCV_GPREL16 },
  { BFD_RELOC_GPREL32, R_RISCV_GPREL32 },
  { BFD_RELOC_MIPS_JMP, R_RISCV_26 },
  { BFD_RELOC_MIPS_LITERAL, R_RISCV_LITERAL },
  { BFD_RELOC_MIPS_GOT16, R_RISCV_GOT16 },
  { BFD_RELOC_MIPS_CALL16, R_RISCV_CALL16 },
  { BFD_RELOC_MIPS_GOT_DISP, R_RISCV_GOT_DISP },
  { BFD_RELOC_MIPS_GOT_HI16, R_RISCV_GOT_HI16 },
  { BFD_RELOC_MIPS_GOT_LO16, R_RISCV_GOT_LO16 },
  { BFD_RELOC_MIPS_SUB, R_RISCV_SUB },
  { BFD_RELOC_MIPS_INSERT_A, R_RISCV_INSERT_A },
  { BFD_RELOC_MIPS_INSERT_B, R_RISCV_INSERT_B },
  { BFD_RELOC_MIPS_DELETE, R_RISCV_DELETE },
  { BFD_RELOC_MIPS_CALL_HI16, R_RISCV_CALL_HI16 },
  { BFD_RELOC_MIPS_CALL_LO16, R_RISCV_CALL_LO16 },
  { BFD_RELOC_MIPS_SCN_DISP, R_RISCV_SCN_DISP },
  { BFD_RELOC_MIPS_REL16, R_RISCV_REL16 },
  { BFD_RELOC_MIPS_RELGOT, R_RISCV_RELGOT },
  { BFD_RELOC_MIPS_TLS_DTPMOD32, R_RISCV_TLS_DTPMOD32 },
  { BFD_RELOC_MIPS_TLS_DTPREL32, R_RISCV_TLS_DTPREL32 },
  { BFD_RELOC_MIPS_TLS_DTPMOD64, R_RISCV_TLS_DTPMOD64 },
  { BFD_RELOC_MIPS_TLS_DTPREL64, R_RISCV_TLS_DTPREL64 },
  { BFD_RELOC_MIPS_TLS_GD, R_RISCV_TLS_GD },
  { BFD_RELOC_MIPS_TLS_LDM, R_RISCV_TLS_LDM },
  { BFD_RELOC_MIPS_TLS_DTPREL_HI16, R_RISCV_TLS_DTPREL_HI16 },
  { BFD_RELOC_MIPS_TLS_DTPREL_LO16, R_RISCV_TLS_DTPREL_LO16 },
  { BFD_RELOC_MIPS_TLS_GOTTPREL, R_RISCV_TLS_GOTTPREL },
  { BFD_RELOC_MIPS_TLS_TPREL32, R_RISCV_TLS_TPREL32 },
  { BFD_RELOC_MIPS_TLS_TPREL64, R_RISCV_TLS_TPREL64 },
  { BFD_RELOC_MIPS_TLS_TPREL_HI16, R_RISCV_TLS_TPREL_HI16 },
  { BFD_RELOC_MIPS_TLS_TPREL_LO16, R_RISCV_TLS_TPREL_LO16 }
};

/* Given a BFD reloc type, return a howto structure.  */

static reloc_howto_type *
bfd_elf32_bfd_reloc_type_lookup (bfd *abfd ATTRIBUTE_UNUSED,
				 bfd_reloc_code_real_type code)
{
  unsigned int i;
  /* FIXME: We default to RELA here instead of choosing the right
     relocation variant.  */
  reloc_howto_type *howto_table = elf_mips_howto_table_rela;

  for (i = 0; i < sizeof (mips_reloc_map) / sizeof (struct elf_reloc_map);
       i++)
    {
      if (mips_reloc_map[i].bfd_val == code)
	return &howto_table[(int) mips_reloc_map[i].elf_val];
    }

  switch (code)
    {
    case BFD_RELOC_VTABLE_INHERIT:
      return &elf_mips_gnu_vtinherit_howto;
    case BFD_RELOC_VTABLE_ENTRY:
      return &elf_mips_gnu_vtentry_howto;
    case BFD_RELOC_MIPS_COPY:
      return &elf_mips_copy_howto;
    case BFD_RELOC_MIPS_JUMP_SLOT:
      return &elf_mips_jump_slot_howto;
    default:
      bfd_set_error (bfd_error_bad_value);
      return NULL;
    }
}

static reloc_howto_type *
bfd_elf32_bfd_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED,
				 const char *r_name)
{
  unsigned int i;

  for (i = 0;
       i < (sizeof (elf_mips_howto_table_rela)
	    / sizeof (elf_mips_howto_table_rela[0]));
       i++)
    if (elf_mips_howto_table_rela[i].name != NULL
	&& strcasecmp (elf_mips_howto_table_rela[i].name, r_name) == 0)
      return &elf_mips_howto_table_rela[i];

  if (strcasecmp (elf_mips_gnu_vtinherit_howto.name, r_name) == 0)
    return &elf_mips_gnu_vtinherit_howto;
  if (strcasecmp (elf_mips_gnu_vtentry_howto.name, r_name) == 0)
    return &elf_mips_gnu_vtentry_howto;
  if (strcasecmp (elf_mips_gnu_rel16_s2.name, r_name) == 0)
    return &elf_mips_gnu_rel16_s2;
  if (strcasecmp (elf_mips_gnu_rela16_s2.name, r_name) == 0)
    return &elf_mips_gnu_rela16_s2;
  if (strcasecmp (elf_mips_copy_howto.name, r_name) == 0)
    return &elf_mips_copy_howto;
  if (strcasecmp (elf_mips_jump_slot_howto.name, r_name) == 0)
    return &elf_mips_jump_slot_howto;

  return NULL;
}

/* Given a MIPS Elf_Internal_Rel, fill in an arelent structure.  */

static reloc_howto_type *
mips_elf_n32_rtype_to_howto (unsigned int r_type, bfd_boolean rela_p)
{
  switch (r_type)
    {
    case R_RISCV_GNU_VTINHERIT:
      return &elf_mips_gnu_vtinherit_howto;
    case R_RISCV_GNU_VTENTRY:
      return &elf_mips_gnu_vtentry_howto;
    case R_RISCV_GNU_REL16_S2:
      if (rela_p)
	return &elf_mips_gnu_rela16_s2;
      else
	return &elf_mips_gnu_rel16_s2;
    case R_RISCV_COPY:
      return &elf_mips_copy_howto;
    case R_RISCV_JUMP_SLOT:
      return &elf_mips_jump_slot_howto;
    default:
      BFD_ASSERT (r_type < (unsigned int) R_RISCV_max);
      if (rela_p)
	return &elf_mips_howto_table_rela[r_type];
      else
	return &elf_mips_howto_table_rel[r_type];
      break;
    }
}

/* Given a MIPS Elf_Internal_Rel, fill in an arelent structure.  */

static void
mips_info_to_howto_rel (bfd *abfd, arelent *cache_ptr, Elf_Internal_Rela *dst)
{
  unsigned int r_type;

  r_type = ELF32_R_TYPE (dst->r_info);
  cache_ptr->howto = mips_elf_n32_rtype_to_howto (r_type, FALSE);

  /* The addend for a GPREL16 or LITERAL relocation comes from the GP
     value for the object file.  We get the addend now, rather than
     when we do the relocation, because the symbol manipulations done
     by the linker may cause us to lose track of the input BFD.  */
  if (((*cache_ptr->sym_ptr_ptr)->flags & BSF_SECTION_SYM) != 0
      && (r_type == R_RISCV_GPREL16 || r_type == (unsigned int) R_RISCV_LITERAL))
    cache_ptr->addend = elf_gp (abfd);
}

/* Given a MIPS Elf_Internal_Rela, fill in an arelent structure.  */

static void
mips_info_to_howto_rela (bfd *abfd ATTRIBUTE_UNUSED,
			 arelent *cache_ptr, Elf_Internal_Rela *dst)
{
  unsigned int r_type;

  r_type = ELF32_R_TYPE (dst->r_info);
  cache_ptr->howto = mips_elf_n32_rtype_to_howto (r_type, TRUE);
  cache_ptr->addend = dst->r_addend;
}

/* Determine whether a symbol is global for the purposes of splitting
   the symbol table into global symbols and local symbols.  At least
   on Irix 5, this split must be between section symbols and all other
   symbols.  On most ELF targets the split is between static symbols
   and externally visible symbols.  */

static bfd_boolean
mips_elf_sym_is_global (bfd *abfd ATTRIBUTE_UNUSED, asymbol *sym)
{
  return ((sym->flags & (BSF_GLOBAL | BSF_WEAK | BSF_GNU_UNIQUE)) != 0
	  || bfd_is_und_section (bfd_get_section (sym))
	  || bfd_is_com_section (bfd_get_section (sym)));
}

/* Set the right machine number for a MIPS ELF file.  */

static bfd_boolean
mips_elf_n32_object_p (bfd *abfd)
{
  unsigned long mach;

  mach = _bfd_elf_riscv_mach (elf_elfheader (abfd)->e_flags);
  bfd_default_set_arch_mach (abfd, bfd_arch_riscv, mach);
  return TRUE;
}

/* Support for core dump NOTE sections.  */
static bfd_boolean
elf32_mips_grok_prstatus (bfd *abfd, Elf_Internal_Note *note)
{
  int offset;
  unsigned int size;

  switch (note->descsz)
    {
      default:
	return FALSE;

      case 440:		/* Linux/MIPS N32 */
	/* pr_cursig */
	elf_tdata (abfd)->core_signal = bfd_get_16 (abfd, note->descdata + 12);

	/* pr_pid */
	elf_tdata (abfd)->core_lwpid = bfd_get_32 (abfd, note->descdata + 24);

	/* pr_reg */
	offset = 72;
	size = 360;

	break;
    }

  /* Make a ".reg/999" section.  */
  return _bfd_elfcore_make_pseudosection (abfd, ".reg", size,
					  note->descpos + offset);
}

static bfd_boolean
elf32_mips_grok_psinfo (bfd *abfd, Elf_Internal_Note *note)
{
  switch (note->descsz)
    {
      default:
	return FALSE;

      case 128:		/* Linux/MIPS elf_prpsinfo */
	elf_tdata (abfd)->core_program
	 = _bfd_elfcore_strndup (abfd, note->descdata + 32, 16);
	elf_tdata (abfd)->core_command
	 = _bfd_elfcore_strndup (abfd, note->descdata + 48, 80);
    }

  /* Note that for some reason, a spurious space is tacked
     onto the end of the args in some (at least one anyway)
     implementations, so strip it off if it exists.  */

  {
    char *command = elf_tdata (abfd)->core_command;
    int n = strlen (command);

    if (0 < n && command[n - 1] == ' ')
      command[n - 1] = '\0';
  }

  return TRUE;
}

#define ELF_ARCH			bfd_arch_riscv
#define ELF_TARGET_ID			MIPS_ELF_DATA
#define ELF_MACHINE_CODE		EM_RISCV

#define elf_backend_collect		TRUE
#define elf_backend_type_change_ok	TRUE
#define elf_backend_can_gc_sections	TRUE
#define elf_info_to_howto		mips_info_to_howto_rela
#define elf_info_to_howto_rel		mips_info_to_howto_rel
#define elf_backend_sym_is_global	mips_elf_sym_is_global
#define elf_backend_object_p		mips_elf_n32_object_p
#define elf_backend_symbol_processing	_bfd_riscv_elf_symbol_processing
#define elf_backend_section_from_shdr	_bfd_riscv_elf_section_from_shdr
#define elf_backend_fake_sections	_bfd_riscv_elf_fake_sections
#define elf_backend_section_from_bfd_section \
					_bfd_riscv_elf_section_from_bfd_section
#define elf_backend_add_symbol_hook	_bfd_riscv_elf_add_symbol_hook
#define elf_backend_link_output_symbol_hook \
					_bfd_riscv_elf_link_output_symbol_hook
#define elf_backend_create_dynamic_sections \
					_bfd_riscv_elf_create_dynamic_sections
#define elf_backend_check_relocs	_bfd_riscv_elf_check_relocs
#define elf_backend_merge_symbol_attribute \
					_bfd_riscv_elf_merge_symbol_attribute
#define elf_backend_get_target_dtag	_bfd_riscv_elf_get_target_dtag
#define elf_backend_adjust_dynamic_symbol \
					_bfd_riscv_elf_adjust_dynamic_symbol
#define elf_backend_always_size_sections \
					_bfd_riscv_elf_always_size_sections
#define elf_backend_size_dynamic_sections \
					_bfd_riscv_elf_size_dynamic_sections
#define elf_backend_init_index_section	_bfd_elf_init_1_index_section
#define elf_backend_relocate_section	_bfd_riscv_elf_relocate_section
#define elf_backend_finish_dynamic_symbol \
					_bfd_riscv_elf_finish_dynamic_symbol
#define elf_backend_finish_dynamic_sections \
					_bfd_riscv_elf_finish_dynamic_sections
#define elf_backend_final_write_processing \
					_bfd_riscv_elf_final_write_processing
#define elf_backend_additional_program_headers \
					_bfd_riscv_elf_additional_program_headers
#define elf_backend_modify_segment_map	_bfd_riscv_elf_modify_segment_map
#define elf_backend_gc_mark_hook	_bfd_riscv_elf_gc_mark_hook
#define elf_backend_gc_sweep_hook	_bfd_riscv_elf_gc_sweep_hook
#define elf_backend_copy_indirect_symbol \
					_bfd_riscv_elf_copy_indirect_symbol
#define elf_backend_grok_prstatus	elf32_mips_grok_prstatus
#define elf_backend_grok_psinfo		elf32_mips_grok_psinfo

#define elf_backend_got_header_size	(4 * MIPS_RESERVED_GOTNO)

/* MIPS n32 ELF can use a mixture of REL and RELA, but some Relocations
   work better/work only in RELA, so we default to this.  */
#define elf_backend_may_use_rel_p	1
#define elf_backend_may_use_rela_p	1
#define elf_backend_default_use_rela_p	1
#define elf_backend_rela_plts_and_copies_p 0
#define elf_backend_sign_extend_vma	TRUE
#define elf_backend_plt_readonly	1
#define elf_backend_plt_sym_val		_bfd_riscv_elf_plt_sym_val

#define elf_backend_discard_info	_bfd_riscv_elf_discard_info
#define elf_backend_ignore_discarded_relocs \
					_bfd_riscv_elf_ignore_discarded_relocs
#define elf_backend_write_section	_bfd_riscv_elf_write_section
#define elf_backend_mips_rtype_to_howto	mips_elf_n32_rtype_to_howto
#define bfd_elf32_find_nearest_line	_bfd_riscv_elf_find_nearest_line
#define bfd_elf32_find_inliner_info	_bfd_riscv_elf_find_inliner_info
#define bfd_elf32_new_section_hook	_bfd_riscv_elf_new_section_hook
#define bfd_elf32_set_section_contents	_bfd_riscv_elf_set_section_contents
#define bfd_elf32_bfd_get_relocated_section_contents \
				bfd_generic_get_relocated_section_contents
#define bfd_elf32_bfd_link_hash_table_create \
					_bfd_riscv_elf_link_hash_table_create
#define bfd_elf32_bfd_final_link	_bfd_riscv_elf_final_link
#define bfd_elf32_bfd_merge_private_bfd_data \
					_bfd_riscv_elf_merge_private_bfd_data
#define bfd_elf32_bfd_set_private_flags	_bfd_riscv_elf_set_private_flags
#define bfd_elf32_bfd_print_private_bfd_data \
					_bfd_riscv_elf_print_private_bfd_data
#define bfd_elf32_bfd_relax_section     _bfd_riscv_relax_section

/* Support for SGI-ish mips targets using n32 ABI.  */

#define TARGET_LITTLE_SYM               bfd_elf32_littleriscv_vec
#define TARGET_LITTLE_NAME              "elf32-littleriscv"
#define TARGET_BIG_SYM                  bfd_elf32_bigriscv_vec
#define TARGET_BIG_NAME                 "elf32-bigriscv"

#define ELF_MAXPAGESIZE			0x10000
#define ELF_COMMONPAGESIZE		0x1000

#include "elf32-target.h"
