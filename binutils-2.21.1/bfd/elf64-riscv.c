/* MIPS-specific support for 64-bit ELF
   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005,
   2006, 2007, 2008, 2009, 2010
   Free Software Foundation, Inc.
   Ian Lance Taylor, Cygnus Support
   Linker support added by Mark Mitchell, CodeSourcery, LLC.
   <mark@codesourcery.com>

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


/* This file supports the 64-bit MIPS ELF ABI.

   The MIPS 64-bit ELF ABI uses an unusual reloc format.  This file
   overrides the usual ELF reloc handling, and handles reading and
   writing the relocations here.  */

/* TODO: Many things are unsupported, even if there is some code for it
 .       (which was mostly stolen from elf32-mips.c and slightly adapted).
 .
 .   - Relocation handling for REL relocs is wrong in many cases and
 .     generally untested.
 .   - Relocation handling for RELA relocs related to GOT support are
 .     also likely to be wrong.
 .   - Support for MIPS16 is untested.
 .   - Combined relocs with RSS_* entries are unsupported.
 .   - The whole GOT handling for NewABI is missing, some parts of
 .     the OldABI version is still lying around and should be removed.
 */

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "aout/ar.h"
#include "bfdlink.h"
#include "genlink.h"
#include "elf-bfd.h"
#include "elfxx-riscv.h"
#include "elf/riscv.h"
#include "opcode/riscv.h"

#include "opcode/riscv.h"

static void mips_elf64_swap_reloc_in
  (bfd *, const Elf64_Mips_External_Rel *, Elf64_Mips_Internal_Rela *);
static void mips_elf64_swap_reloca_in
  (bfd *, const Elf64_Mips_External_Rela *, Elf64_Mips_Internal_Rela *);
static void mips_elf64_swap_reloc_out
  (bfd *, const Elf64_Mips_Internal_Rela *, Elf64_Mips_External_Rel *);
static void mips_elf64_swap_reloca_out
  (bfd *, const Elf64_Mips_Internal_Rela *, Elf64_Mips_External_Rela *);
static void mips_elf64_be_swap_reloc_in
  (bfd *, const bfd_byte *, Elf_Internal_Rela *);
static void mips_elf64_be_swap_reloc_out
  (bfd *, const Elf_Internal_Rela *, bfd_byte *);
static void mips_elf64_be_swap_reloca_in
  (bfd *, const bfd_byte *, Elf_Internal_Rela *);
static void mips_elf64_be_swap_reloca_out
  (bfd *, const Elf_Internal_Rela *, bfd_byte *);
static reloc_howto_type *bfd_elf64_bfd_reloc_type_lookup
  (bfd *, bfd_reloc_code_real_type);
static reloc_howto_type *mips_elf64_rtype_to_howto
  (unsigned int, bfd_boolean);
static void mips_elf64_info_to_howto_rel
  (bfd *, arelent *, Elf_Internal_Rela *);
static void mips_elf64_info_to_howto_rela
  (bfd *, arelent *, Elf_Internal_Rela *);
static long mips_elf64_get_reloc_upper_bound
  (bfd *, asection *);
static long mips_elf64_canonicalize_reloc
  (bfd *, asection *, arelent **, asymbol **);
static long mips_elf64_get_dynamic_reloc_upper_bound
  (bfd *);
static long mips_elf64_canonicalize_dynamic_reloc
  (bfd *, arelent **, asymbol **);
static bfd_boolean mips_elf64_slurp_one_reloc_table
  (bfd *, asection *, Elf_Internal_Shdr *, bfd_size_type, arelent *,
   asymbol **, bfd_boolean);
static bfd_boolean mips_elf64_slurp_reloc_table
  (bfd *, asection *, asymbol **, bfd_boolean);
static void mips_elf64_write_relocs
  (bfd *, asection *, void *);
static void mips_elf64_write_rel
  (bfd *, asection *, Elf_Internal_Shdr *, int *, void *);
static void mips_elf64_write_rela
  (bfd *, asection *, Elf_Internal_Shdr *, int *, void *);
static bfd_boolean mips_elf64_object_p
  (bfd *);
static bfd_boolean elf64_mips_grok_prstatus
  (bfd *, Elf_Internal_Note *);
static bfd_boolean elf64_mips_grok_psinfo
  (bfd *, Elf_Internal_Note *);

/* In case we're on a 32-bit machine, construct a 64-bit "-1" value
   from smaller values.  Start with zero, widen, *then* decrement.  */
#define MINUS_ONE	(((bfd_vma)0) - 1)

/* The number of local .got entries we reserve.  */
#define MIPS_RESERVED_GOTNO (2)

/* The relocation table used for SHT_REL sections.  */

static reloc_howto_type mips_elf64_howto_table_rel[] =
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

  EMPTY_HOWTO (7),
  EMPTY_HOWTO (8),
  EMPTY_HOWTO (9),

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

  EMPTY_HOWTO (11),
  EMPTY_HOWTO (12),
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

  EMPTY_HOWTO (19),
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

  EMPTY_HOWTO (24),
  EMPTY_HOWTO (25),
  EMPTY_HOWTO (26),
  EMPTY_HOWTO (27),
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

  EMPTY_HOWTO (32),

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
  EMPTY_HOWTO (R_RISCV_TLS_DTPMOD32),
  EMPTY_HOWTO (R_RISCV_TLS_DTPREL32),

  HOWTO (R_RISCV_TLS_DTPMOD64,	/* type */
	 0,			/* rightshift */
	 4,			/* size (0 = byte, 1 = short, 2 = long) */
	 64,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc, /* special_function */
	 "R_RISCV_TLS_DTPMOD64",	/* name */
	 TRUE,			/* partial_inplace */
	 MINUS_ONE,		/* src_mask */
	 MINUS_ONE,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  HOWTO (R_RISCV_TLS_DTPREL64,	/* type */
	 0,			/* rightshift */
	 4,			/* size (0 = byte, 1 = short, 2 = long) */
	 64,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc, /* special_function */
	 "R_RISCV_TLS_DTPREL64",	/* name */
	 TRUE,			/* partial_inplace */
	 MINUS_ONE,		/* src_mask */
	 MINUS_ONE,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

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
  EMPTY_HOWTO (R_RISCV_TLS_TPREL32),

  HOWTO (R_RISCV_TLS_TPREL64,	/* type */
	 0,			/* rightshift */
	 4,			/* size (0 = byte, 1 = short, 2 = long) */
	 64,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc, /* special_function */
	 "R_RISCV_TLS_TPREL64",	/* name */
	 TRUE,			/* partial_inplace */
	 MINUS_ONE,		/* src_mask */
	 MINUS_ONE,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

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

static reloc_howto_type mips_elf64_howto_table_rela[] =
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

  EMPTY_HOWTO (7),
  EMPTY_HOWTO (8),
  EMPTY_HOWTO (9),

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

  EMPTY_HOWTO (11),
  EMPTY_HOWTO (12),
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

  EMPTY_HOWTO (19),
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

  EMPTY_HOWTO (24),
  EMPTY_HOWTO (25),
  EMPTY_HOWTO (26),
  EMPTY_HOWTO (27),
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

  EMPTY_HOWTO (32),

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
  EMPTY_HOWTO (R_RISCV_TLS_DTPMOD32),
  EMPTY_HOWTO (R_RISCV_TLS_DTPREL32),

  HOWTO (R_RISCV_TLS_DTPMOD64,	/* type */
	 0,			/* rightshift */
	 4,			/* size (0 = byte, 1 = short, 2 = long) */
	 64,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc, /* special_function */
	 "R_RISCV_TLS_DTPMOD64", /* name */
	 FALSE,			/* partial_inplace */
	 MINUS_ONE,		/* src_mask */
	 MINUS_ONE,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  HOWTO (R_RISCV_TLS_DTPREL64,	/* type */
	 0,			/* rightshift */
	 4,			/* size (0 = byte, 1 = short, 2 = long) */
	 64,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc, /* special_function */
	 "R_RISCV_TLS_DTPREL64",	/* name */
	 TRUE,			/* partial_inplace */
	 MINUS_ONE,		/* src_mask */
	 MINUS_ONE,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

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

  EMPTY_HOWTO (R_RISCV_TLS_TPREL32),

  HOWTO (R_RISCV_TLS_TPREL64,	/* type */
	 0,			/* rightshift */
	 4,			/* size (0 = byte, 1 = short, 2 = long) */
	 64,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc, /* special_function */
	 "R_RISCV_TLS_TPREL64",	/* name */
	 FALSE,			/* partial_inplace */
	 MINUS_ONE,		/* src_mask */
	 MINUS_ONE,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

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
	 RISCV_BRANCH_ALIGN_BITS,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_BRANCH_BITS,			/* bitsize */
	 TRUE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc, /* special_function */
	 "R_RISCV_GNU_REL16_S2",	/* name */
	 TRUE,			/* partial_inplace */
	 RISCV_BRANCH_REACH-1,		/* src_mask */
	 RISCV_BRANCH_REACH-1,		/* dst_mask */
	 TRUE);			/* pcrel_offset */

/* 16 bit offset for pc-relative branches.  */
static reloc_howto_type elf_mips_gnu_rela16_s2 =
  HOWTO (R_RISCV_GNU_REL16_S2,	/* type */
	 RISCV_BRANCH_ALIGN_BITS,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 RISCV_BRANCH_BITS,			/* bitsize */
	 TRUE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_riscv_elf_generic_reloc,	/* special_function */
	 "R_RISCV_GNU_REL16_S2",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 RISCV_BRANCH_REACH-1,		/* dst_mask */
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
	 4,			/* size (0 = byte, 1 = short, 2 = long) */
	 64,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_RISCV_JUMP_SLOT",	/* name */
	 FALSE,			/* partial_inplace */
	 0x0,         		/* src_mask */
	 0x0,		        /* dst_mask */
	 FALSE);		/* pcrel_offset */

/* Swap in a MIPS 64-bit Rel reloc.  */

static void
mips_elf64_swap_reloc_in (bfd *abfd, const Elf64_Mips_External_Rel *src,
			  Elf64_Mips_Internal_Rela *dst)
{
  dst->r_offset = H_GET_64 (abfd, src->r_offset);
  dst->r_sym = H_GET_32 (abfd, src->r_sym);
  dst->r_ssym = H_GET_8 (abfd, src->r_ssym);
  dst->r_type3 = H_GET_8 (abfd, src->r_type3);
  dst->r_type2 = H_GET_8 (abfd, src->r_type2);
  dst->r_type = H_GET_8 (abfd, src->r_type);
  dst->r_addend = 0;
}

/* Swap in a MIPS 64-bit Rela reloc.  */

static void
mips_elf64_swap_reloca_in (bfd *abfd, const Elf64_Mips_External_Rela *src,
			   Elf64_Mips_Internal_Rela *dst)
{
  dst->r_offset = H_GET_64 (abfd, src->r_offset);
  dst->r_sym = H_GET_32 (abfd, src->r_sym);
  dst->r_ssym = H_GET_8 (abfd, src->r_ssym);
  dst->r_type3 = H_GET_8 (abfd, src->r_type3);
  dst->r_type2 = H_GET_8 (abfd, src->r_type2);
  dst->r_type = H_GET_8 (abfd, src->r_type);
  dst->r_addend = H_GET_S64 (abfd, src->r_addend);
}

/* Swap out a MIPS 64-bit Rel reloc.  */

static void
mips_elf64_swap_reloc_out (bfd *abfd, const Elf64_Mips_Internal_Rela *src,
			   Elf64_Mips_External_Rel *dst)
{
  H_PUT_64 (abfd, src->r_offset, dst->r_offset);
  H_PUT_32 (abfd, src->r_sym, dst->r_sym);
  H_PUT_8 (abfd, src->r_ssym, dst->r_ssym);
  H_PUT_8 (abfd, src->r_type3, dst->r_type3);
  H_PUT_8 (abfd, src->r_type2, dst->r_type2);
  H_PUT_8 (abfd, src->r_type, dst->r_type);
}

/* Swap out a MIPS 64-bit Rela reloc.  */

static void
mips_elf64_swap_reloca_out (bfd *abfd, const Elf64_Mips_Internal_Rela *src,
			    Elf64_Mips_External_Rela *dst)
{
  H_PUT_64 (abfd, src->r_offset, dst->r_offset);
  H_PUT_32 (abfd, src->r_sym, dst->r_sym);
  H_PUT_8 (abfd, src->r_ssym, dst->r_ssym);
  H_PUT_8 (abfd, src->r_type3, dst->r_type3);
  H_PUT_8 (abfd, src->r_type2, dst->r_type2);
  H_PUT_8 (abfd, src->r_type, dst->r_type);
  H_PUT_S64 (abfd, src->r_addend, dst->r_addend);
}

/* Swap in a MIPS 64-bit Rel reloc.  */

static void
mips_elf64_be_swap_reloc_in (bfd *abfd, const bfd_byte *src,
			     Elf_Internal_Rela *dst)
{
  Elf64_Mips_Internal_Rela mirel;

  mips_elf64_swap_reloc_in (abfd,
			    (const Elf64_Mips_External_Rel *) src,
			    &mirel);

  dst[0].r_offset = mirel.r_offset;
  dst[0].r_info = ELF64_R_INFO (mirel.r_sym, mirel.r_type);
  dst[0].r_addend = 0;
  dst[1].r_offset = mirel.r_offset;
  dst[1].r_info = ELF64_R_INFO (mirel.r_ssym, mirel.r_type2);
  dst[1].r_addend = 0;
  dst[2].r_offset = mirel.r_offset;
  dst[2].r_info = ELF64_R_INFO (STN_UNDEF, mirel.r_type3);
  dst[2].r_addend = 0;
}

/* Swap in a MIPS 64-bit Rela reloc.  */

static void
mips_elf64_be_swap_reloca_in (bfd *abfd, const bfd_byte *src,
			      Elf_Internal_Rela *dst)
{
  Elf64_Mips_Internal_Rela mirela;

  mips_elf64_swap_reloca_in (abfd,
			     (const Elf64_Mips_External_Rela *) src,
			     &mirela);

  dst[0].r_offset = mirela.r_offset;
  dst[0].r_info = ELF64_R_INFO (mirela.r_sym, mirela.r_type);
  dst[0].r_addend = mirela.r_addend;
  dst[1].r_offset = mirela.r_offset;
  dst[1].r_info = ELF64_R_INFO (mirela.r_ssym, mirela.r_type2);
  dst[1].r_addend = 0;
  dst[2].r_offset = mirela.r_offset;
  dst[2].r_info = ELF64_R_INFO (STN_UNDEF, mirela.r_type3);
  dst[2].r_addend = 0;
}

/* Swap out a MIPS 64-bit Rel reloc.  */

static void
mips_elf64_be_swap_reloc_out (bfd *abfd, const Elf_Internal_Rela *src,
			      bfd_byte *dst)
{
  Elf64_Mips_Internal_Rela mirel;

  mirel.r_offset = src[0].r_offset;
  BFD_ASSERT(src[0].r_offset == src[1].r_offset);

  mirel.r_type = ELF64_MIPS_R_TYPE (src[0].r_info);
  mirel.r_sym = ELF64_R_SYM (src[0].r_info);
  mirel.r_type2 = ELF64_MIPS_R_TYPE (src[1].r_info);
  mirel.r_ssym = ELF64_MIPS_R_SSYM (src[1].r_info);
  mirel.r_type3 = ELF64_MIPS_R_TYPE (src[2].r_info);

  mips_elf64_swap_reloc_out (abfd, &mirel,
			     (Elf64_Mips_External_Rel *) dst);
}

/* Swap out a MIPS 64-bit Rela reloc.  */

static void
mips_elf64_be_swap_reloca_out (bfd *abfd, const Elf_Internal_Rela *src,
			       bfd_byte *dst)
{
  Elf64_Mips_Internal_Rela mirela;

  mirela.r_offset = src[0].r_offset;
  BFD_ASSERT(src[0].r_offset == src[1].r_offset);
  BFD_ASSERT(src[0].r_offset == src[2].r_offset);

  mirela.r_type = ELF64_MIPS_R_TYPE (src[0].r_info);
  mirela.r_sym = ELF64_R_SYM (src[0].r_info);
  mirela.r_addend = src[0].r_addend;
  BFD_ASSERT(src[1].r_addend == 0);
  BFD_ASSERT(src[2].r_addend == 0);

  mirela.r_type2 = ELF64_MIPS_R_TYPE (src[1].r_info);
  mirela.r_ssym = ELF64_MIPS_R_SSYM (src[1].r_info);
  mirela.r_type3 = ELF64_MIPS_R_TYPE (src[2].r_info);

  mips_elf64_swap_reloca_out (abfd, &mirela,
			      (Elf64_Mips_External_Rela *) dst);
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
  { BFD_RELOC_64, R_RISCV_64 },
  { BFD_RELOC_CTOR, R_RISCV_64 },
  { BFD_RELOC_16_PCREL_S2, R_RISCV_PC16 },
  { BFD_RELOC_HI16_S, R_RISCV_HI16 },
  { BFD_RELOC_LO16, R_RISCV_LO16 },
  { BFD_RELOC_MIPS_JMP, R_RISCV_26 },
  { BFD_RELOC_MIPS_GOT_HI16, R_RISCV_GOT_HI16 },
  { BFD_RELOC_MIPS_GOT_LO16, R_RISCV_GOT_LO16 },
  { BFD_RELOC_MIPS_CALL_HI16, R_RISCV_CALL_HI16 },
  { BFD_RELOC_MIPS_CALL_LO16, R_RISCV_CALL_LO16 },
  { BFD_RELOC_MIPS_REL16, R_RISCV_REL16 },
  /* Use of R_RISCV_ADD_IMMEDIATE and R_RISCV_PJUMP is deprecated.  */
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
  { BFD_RELOC_MIPS_TLS_TPREL_LO16, R_RISCV_TLS_TPREL_LO16 },
  { BFD_RELOC_RISCV_TLS_GOT_HI16, R_RISCV_TLS_GOT_HI16 },
  { BFD_RELOC_RISCV_TLS_GOT_LO16, R_RISCV_TLS_GOT_LO16 },
  { BFD_RELOC_RISCV_TLS_GD_HI16, R_RISCV_TLS_GD_HI16 },
  { BFD_RELOC_RISCV_TLS_GD_LO16, R_RISCV_TLS_GD_LO16 },
  { BFD_RELOC_RISCV_TLS_LDM_HI16, R_RISCV_TLS_LDM_HI16 },
  { BFD_RELOC_RISCV_TLS_LDM_LO16, R_RISCV_TLS_LDM_LO16 }
};

/* Given a BFD reloc type, return a howto structure.  */

static reloc_howto_type *
bfd_elf64_bfd_reloc_type_lookup (bfd *abfd ATTRIBUTE_UNUSED,
				 bfd_reloc_code_real_type code)
{
  unsigned int i;
  /* FIXME: We default to RELA here instead of choosing the right
     relocation variant.  */
  reloc_howto_type *howto_table = mips_elf64_howto_table_rela;

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
bfd_elf64_bfd_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED,
				 const char *r_name)
{
  unsigned int i;

  for (i = 0;
       i < (sizeof (mips_elf64_howto_table_rela)
	    / sizeof (mips_elf64_howto_table_rela[0])); i++)
    if (mips_elf64_howto_table_rela[i].name != NULL
	&& strcasecmp (mips_elf64_howto_table_rela[i].name, r_name) == 0)
      return &mips_elf64_howto_table_rela[i];

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
mips_elf64_rtype_to_howto (unsigned int r_type, bfd_boolean rela_p)
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
	return &mips_elf64_howto_table_rela[r_type];
      else
	return &mips_elf64_howto_table_rel[r_type];
      break;
    }
}

/* Prevent relocation handling by bfd for MIPS ELF64.  */

static void
mips_elf64_info_to_howto_rel (bfd *abfd ATTRIBUTE_UNUSED,
			      arelent *cache_ptr ATTRIBUTE_UNUSED,
			      Elf_Internal_Rela *dst ATTRIBUTE_UNUSED)
{
  BFD_ASSERT (0);
}

static void
mips_elf64_info_to_howto_rela (bfd *abfd ATTRIBUTE_UNUSED,
			       arelent *cache_ptr ATTRIBUTE_UNUSED,
			       Elf_Internal_Rela *dst ATTRIBUTE_UNUSED)
{
  BFD_ASSERT (0);
}

/* Since each entry in an SHT_REL or SHT_RELA section can represent up
   to three relocs, we must tell the user to allocate more space.  */

static long
mips_elf64_get_reloc_upper_bound (bfd *abfd ATTRIBUTE_UNUSED, asection *sec)
{
  return (sec->reloc_count * 3 + 1) * sizeof (arelent *);
}

static long
mips_elf64_get_dynamic_reloc_upper_bound (bfd *abfd)
{
  return _bfd_elf_get_dynamic_reloc_upper_bound (abfd) * 3;
}

/* We must also copy more relocations than the corresponding functions
   in elf.c would, so the two following functions are slightly
   modified from elf.c, that multiply the external relocation count by
   3 to obtain the internal relocation count.  */

static long
mips_elf64_canonicalize_reloc (bfd *abfd, sec_ptr section,
			       arelent **relptr, asymbol **symbols)
{
  arelent *tblptr;
  unsigned int i;
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);

  if (! bed->s->slurp_reloc_table (abfd, section, symbols, FALSE))
    return -1;

  tblptr = section->relocation;
  for (i = 0; i < section->reloc_count * 3; i++)
    *relptr++ = tblptr++;

  *relptr = NULL;

  return section->reloc_count * 3;
}

static long
mips_elf64_canonicalize_dynamic_reloc (bfd *abfd, arelent **storage,
				       asymbol **syms)
{
  bfd_boolean (*slurp_relocs) (bfd *, asection *, asymbol **, bfd_boolean);
  asection *s;
  long ret;

  if (elf_dynsymtab (abfd) == 0)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return -1;
    }

  slurp_relocs = get_elf_backend_data (abfd)->s->slurp_reloc_table;
  ret = 0;
  for (s = abfd->sections; s != NULL; s = s->next)
    {
      if (elf_section_data (s)->this_hdr.sh_link == elf_dynsymtab (abfd)
	  && (elf_section_data (s)->this_hdr.sh_type == SHT_REL
	      || elf_section_data (s)->this_hdr.sh_type == SHT_RELA))
	{
	  arelent *p;
	  long count, i;

	  if (! (*slurp_relocs) (abfd, s, syms, TRUE))
	    return -1;
	  count = s->size / elf_section_data (s)->this_hdr.sh_entsize * 3;
	  p = s->relocation;
	  for (i = 0; i < count; i++)
	    *storage++ = p++;
	  ret += count;
	}
    }

  *storage = NULL;

  return ret;
}

/* Read the relocations from one reloc section.  This is mostly copied
   from elfcode.h, except for the changes to expand one external
   relocation to 3 internal ones.  We must unfortunately set
   reloc_count to the number of external relocations, because a lot of
   generic code seems to depend on this.  */

static bfd_boolean
mips_elf64_slurp_one_reloc_table (bfd *abfd, asection *asect,
				  Elf_Internal_Shdr *rel_hdr,
				  bfd_size_type reloc_count,
				  arelent *relents, asymbol **symbols,
				  bfd_boolean dynamic)
{
  void *allocated;
  bfd_byte *native_relocs;
  arelent *relent;
  bfd_vma i;
  int entsize;
  bfd_boolean rela_p;

  allocated = bfd_malloc (rel_hdr->sh_size);
  if (allocated == NULL)
    return FALSE;

  if (bfd_seek (abfd, rel_hdr->sh_offset, SEEK_SET) != 0
      || (bfd_bread (allocated, rel_hdr->sh_size, abfd)
	  != rel_hdr->sh_size))
    goto error_return;

  native_relocs = allocated;

  entsize = rel_hdr->sh_entsize;
  BFD_ASSERT (entsize == sizeof (Elf64_Mips_External_Rel)
	      || entsize == sizeof (Elf64_Mips_External_Rela));

  if (entsize == sizeof (Elf64_Mips_External_Rel))
    rela_p = FALSE;
  else
    rela_p = TRUE;

  for (i = 0, relent = relents;
       i < reloc_count;
       i++, native_relocs += entsize)
    {
      Elf64_Mips_Internal_Rela rela;
      bfd_boolean used_sym, used_ssym;
      int ir;

      if (entsize == sizeof (Elf64_Mips_External_Rela))
	mips_elf64_swap_reloca_in (abfd,
				   (Elf64_Mips_External_Rela *) native_relocs,
				   &rela);
      else
	mips_elf64_swap_reloc_in (abfd,
				  (Elf64_Mips_External_Rel *) native_relocs,
				  &rela);

      /* Each entry represents exactly three actual relocations.  */

      used_sym = FALSE;
      used_ssym = FALSE;
      for (ir = 0; ir < 3; ir++)
	{
	  enum elf_riscv_reloc_type type;

	  switch (ir)
	    {
	    default:
	      abort ();
	    case 0:
	      type = (enum elf_riscv_reloc_type) rela.r_type;
	      break;
	    case 1:
	      type = (enum elf_riscv_reloc_type) rela.r_type2;
	      break;
	    case 2:
	      type = (enum elf_riscv_reloc_type) rela.r_type3;
	      break;
	    }

	  /* Some types require symbols, whereas some do not.  */
	  switch (type)
	    {
	    case R_RISCV_NONE:
	      relent->sym_ptr_ptr = bfd_abs_section_ptr->symbol_ptr_ptr;
	      break;

	    default:
	      if (! used_sym)
		{
		  if (rela.r_sym == STN_UNDEF)
		    relent->sym_ptr_ptr = bfd_abs_section_ptr->symbol_ptr_ptr;
		  else
		    {
		      asymbol **ps, *s;

		      ps = symbols + rela.r_sym - 1;
		      s = *ps;
		      if ((s->flags & BSF_SECTION_SYM) == 0)
			relent->sym_ptr_ptr = ps;
		      else
			relent->sym_ptr_ptr = s->section->symbol_ptr_ptr;
		    }

		  used_sym = TRUE;
		}
	      else if (! used_ssym)
		{
		  switch (rela.r_ssym)
		    {
		    case RSS_UNDEF:
		      relent->sym_ptr_ptr =
			bfd_abs_section_ptr->symbol_ptr_ptr;
		      break;

		    case RSS_GP:
		    case RSS_GP0:
		    case RSS_LOC:
		      /* FIXME: I think these need to be handled using
			 special howto structures.  */
		      BFD_ASSERT (0);
		      break;

		    default:
		      BFD_ASSERT (0);
		      break;
		    }

		  used_ssym = TRUE;
		}
	      else
		relent->sym_ptr_ptr = bfd_abs_section_ptr->symbol_ptr_ptr;

	      break;
	    }

	  /* The address of an ELF reloc is section relative for an
	     object file, and absolute for an executable file or
	     shared library.  The address of a BFD reloc is always
	     section relative.  */
	  if ((abfd->flags & (EXEC_P | DYNAMIC)) == 0 || dynamic)
	    relent->address = rela.r_offset;
	  else
	    relent->address = rela.r_offset - asect->vma;

	  relent->addend = rela.r_addend;

	  relent->howto = mips_elf64_rtype_to_howto (type, rela_p);

	  ++relent;
	}
    }

  asect->reloc_count += (relent - relents) / 3;

  if (allocated != NULL)
    free (allocated);

  return TRUE;

 error_return:
  if (allocated != NULL)
    free (allocated);
  return FALSE;
}

/* Read the relocations.  On Irix 6, there can be two reloc sections
   associated with a single data section.  This is copied from
   elfcode.h as well, with changes as small as accounting for 3
   internal relocs per external reloc and resetting reloc_count to
   zero before processing the relocs of a section.  */

static bfd_boolean
mips_elf64_slurp_reloc_table (bfd *abfd, asection *asect,
			      asymbol **symbols, bfd_boolean dynamic)
{
  struct bfd_elf_section_data * const d = elf_section_data (asect);
  Elf_Internal_Shdr *rel_hdr;
  Elf_Internal_Shdr *rel_hdr2;
  bfd_size_type reloc_count;
  bfd_size_type reloc_count2;
  arelent *relents;
  bfd_size_type amt;

  if (asect->relocation != NULL)
    return TRUE;

  if (! dynamic)
    {
      if ((asect->flags & SEC_RELOC) == 0
	  || asect->reloc_count == 0)
	return TRUE;

      rel_hdr = d->rel.hdr;
      reloc_count = rel_hdr ? NUM_SHDR_ENTRIES (rel_hdr) : 0;
      rel_hdr2 = d->rela.hdr;
      reloc_count2 = (rel_hdr2 ? NUM_SHDR_ENTRIES (rel_hdr2) : 0);

      BFD_ASSERT (asect->reloc_count == reloc_count + reloc_count2);
      BFD_ASSERT ((rel_hdr && asect->rel_filepos == rel_hdr->sh_offset)
		  || (rel_hdr2 && asect->rel_filepos == rel_hdr2->sh_offset));

    }
  else
    {
      /* Note that ASECT->RELOC_COUNT tends not to be accurate in this
	 case because relocations against this section may use the
	 dynamic symbol table, and in that case bfd_section_from_shdr
	 in elf.c does not update the RELOC_COUNT.  */
      if (asect->size == 0)
	return TRUE;

      rel_hdr = &d->this_hdr;
      reloc_count = NUM_SHDR_ENTRIES (rel_hdr);
      rel_hdr2 = NULL;
      reloc_count2 = 0;
    }

  /* Allocate space for 3 arelent structures for each Rel structure.  */
  amt = (reloc_count + reloc_count2) * 3 * sizeof (arelent);
  relents = bfd_alloc (abfd, amt);
  if (relents == NULL)
    return FALSE;

  /* The slurp_one_reloc_table routine increments reloc_count.  */
  asect->reloc_count = 0;

  if (rel_hdr != NULL
      && ! mips_elf64_slurp_one_reloc_table (abfd, asect,
					     rel_hdr, reloc_count,
					     relents,
					     symbols, dynamic))
    return FALSE;
  if (rel_hdr2 != NULL
      && ! mips_elf64_slurp_one_reloc_table (abfd, asect,
					     rel_hdr2, reloc_count2,
					     relents + reloc_count * 3,
					     symbols, dynamic))
    return FALSE;

  asect->relocation = relents;
  return TRUE;
}

/* Write out the relocations.  */

static void
mips_elf64_write_relocs (bfd *abfd, asection *sec, void *data)
{
  bfd_boolean *failedp = data;
  int count;
  Elf_Internal_Shdr *rel_hdr;
  unsigned int idx;

  /* If we have already failed, don't do anything.  */
  if (*failedp)
    return;

  if ((sec->flags & SEC_RELOC) == 0)
    return;

  /* The linker backend writes the relocs out itself, and sets the
     reloc_count field to zero to inhibit writing them here.  Also,
     sometimes the SEC_RELOC flag gets set even when there aren't any
     relocs.  */
  if (sec->reloc_count == 0)
    return;

  /* We can combine up to three relocs that refer to the same address
     if the latter relocs have no associated symbol.  */
  count = 0;
  for (idx = 0; idx < sec->reloc_count; idx++)
    {
      bfd_vma addr;
      unsigned int i;

      ++count;

      addr = sec->orelocation[idx]->address;
      for (i = 0; i < 2; i++)
	{
	  arelent *r;

	  if (idx + 1 >= sec->reloc_count)
	    break;
	  r = sec->orelocation[idx + 1];
	  if (r->address != addr
	      || ! bfd_is_abs_section ((*r->sym_ptr_ptr)->section)
	      || (*r->sym_ptr_ptr)->value != 0)
	    break;

	  /* We can merge the reloc at IDX + 1 with the reloc at IDX.  */

	  ++idx;
	}
    }

  rel_hdr = _bfd_elf_single_rel_hdr (sec);

  /* Do the actual relocation.  */

  if (rel_hdr->sh_entsize == sizeof(Elf64_Mips_External_Rel))
    mips_elf64_write_rel (abfd, sec, rel_hdr, &count, data);
  else if (rel_hdr->sh_entsize == sizeof(Elf64_Mips_External_Rela))
    mips_elf64_write_rela (abfd, sec, rel_hdr, &count, data);
  else
    BFD_ASSERT (0);
}

static void
mips_elf64_write_rel (bfd *abfd, asection *sec,
		      Elf_Internal_Shdr *rel_hdr,
		      int *count, void *data)
{
  bfd_boolean *failedp = data;
  Elf64_Mips_External_Rel *ext_rel;
  unsigned int idx;
  asymbol *last_sym = 0;
  int last_sym_idx = 0;

  rel_hdr->sh_size = rel_hdr->sh_entsize * *count;
  rel_hdr->contents = bfd_alloc (abfd, rel_hdr->sh_size);
  if (rel_hdr->contents == NULL)
    {
      *failedp = TRUE;
      return;
    }

  ext_rel = (Elf64_Mips_External_Rel *) rel_hdr->contents;
  for (idx = 0; idx < sec->reloc_count; idx++, ext_rel++)
    {
      arelent *ptr;
      Elf64_Mips_Internal_Rela int_rel;
      asymbol *sym;
      int n;
      unsigned int i;

      ptr = sec->orelocation[idx];

      /* The address of an ELF reloc is section relative for an object
	 file, and absolute for an executable file or shared library.
	 The address of a BFD reloc is always section relative.  */
      if ((abfd->flags & (EXEC_P | DYNAMIC)) == 0)
	int_rel.r_offset = ptr->address;
      else
	int_rel.r_offset = ptr->address + sec->vma;

      sym = *ptr->sym_ptr_ptr;
      if (sym == last_sym)
	n = last_sym_idx;
      else if (bfd_is_abs_section (sym->section) && sym->value == 0)
	n = STN_UNDEF;
      else
	{
	  last_sym = sym;
	  n = _bfd_elf_symbol_from_bfd_symbol (abfd, &sym);
	  if (n < 0)
	    {
	      *failedp = TRUE;
	      return;
	    }
	  last_sym_idx = n;
	}

      int_rel.r_sym = n;
      int_rel.r_ssym = RSS_UNDEF;

      if ((*ptr->sym_ptr_ptr)->the_bfd->xvec != abfd->xvec
	  && ! _bfd_elf_validate_reloc (abfd, ptr))
	{
	  *failedp = TRUE;
	  return;
	}

      int_rel.r_type = ptr->howto->type;
      int_rel.r_type2 = (int) R_RISCV_NONE;
      int_rel.r_type3 = (int) R_RISCV_NONE;

      for (i = 0; i < 2; i++)
	{
	  arelent *r;

	  if (idx + 1 >= sec->reloc_count)
	    break;
	  r = sec->orelocation[idx + 1];
	  if (r->address != ptr->address
	      || ! bfd_is_abs_section ((*r->sym_ptr_ptr)->section)
	      || (*r->sym_ptr_ptr)->value != 0)
	    break;

	  /* We can merge the reloc at IDX + 1 with the reloc at IDX.  */

	  if (i == 0)
	    int_rel.r_type2 = r->howto->type;
	  else
	    int_rel.r_type3 = r->howto->type;

	  ++idx;
	}

      mips_elf64_swap_reloc_out (abfd, &int_rel, ext_rel);
    }

  BFD_ASSERT (ext_rel - (Elf64_Mips_External_Rel *) rel_hdr->contents
	      == *count);
}

static void
mips_elf64_write_rela (bfd *abfd, asection *sec,
		       Elf_Internal_Shdr *rela_hdr,
		       int *count, void *data)
{
  bfd_boolean *failedp = data;
  Elf64_Mips_External_Rela *ext_rela;
  unsigned int idx;
  asymbol *last_sym = 0;
  int last_sym_idx = 0;

  rela_hdr->sh_size = rela_hdr->sh_entsize * *count;
  rela_hdr->contents = bfd_alloc (abfd, rela_hdr->sh_size);
  if (rela_hdr->contents == NULL)
    {
      *failedp = TRUE;
      return;
    }

  ext_rela = (Elf64_Mips_External_Rela *) rela_hdr->contents;
  for (idx = 0; idx < sec->reloc_count; idx++, ext_rela++)
    {
      arelent *ptr;
      Elf64_Mips_Internal_Rela int_rela;
      asymbol *sym;
      int n;
      unsigned int i;

      ptr = sec->orelocation[idx];

      /* The address of an ELF reloc is section relative for an object
	 file, and absolute for an executable file or shared library.
	 The address of a BFD reloc is always section relative.  */
      if ((abfd->flags & (EXEC_P | DYNAMIC)) == 0)
	int_rela.r_offset = ptr->address;
      else
	int_rela.r_offset = ptr->address + sec->vma;

      sym = *ptr->sym_ptr_ptr;
      if (sym == last_sym)
	n = last_sym_idx;
      else if (bfd_is_abs_section (sym->section) && sym->value == 0)
	n = STN_UNDEF;
      else
	{
	  last_sym = sym;
	  n = _bfd_elf_symbol_from_bfd_symbol (abfd, &sym);
	  if (n < 0)
	    {
	      *failedp = TRUE;
	      return;
	    }
	  last_sym_idx = n;
	}

      int_rela.r_sym = n;
      int_rela.r_addend = ptr->addend;
      int_rela.r_ssym = RSS_UNDEF;

      if ((*ptr->sym_ptr_ptr)->the_bfd->xvec != abfd->xvec
	  && ! _bfd_elf_validate_reloc (abfd, ptr))
	{
	  *failedp = TRUE;
	  return;
	}

      int_rela.r_type = ptr->howto->type;
      int_rela.r_type2 = (int) R_RISCV_NONE;
      int_rela.r_type3 = (int) R_RISCV_NONE;

      for (i = 0; i < 2; i++)
	{
	  arelent *r;

	  if (idx + 1 >= sec->reloc_count)
	    break;
	  r = sec->orelocation[idx + 1];
	  if (r->address != ptr->address
	      || ! bfd_is_abs_section ((*r->sym_ptr_ptr)->section)
	      || (*r->sym_ptr_ptr)->value != 0)
	    break;

	  /* We can merge the reloc at IDX + 1 with the reloc at IDX.  */

	  if (i == 0)
	    int_rela.r_type2 = r->howto->type;
	  else
	    int_rela.r_type3 = r->howto->type;

	  ++idx;
	}

      mips_elf64_swap_reloca_out (abfd, &int_rela, ext_rela);
    }

  BFD_ASSERT (ext_rela - (Elf64_Mips_External_Rela *) rela_hdr->contents
	      == *count);
}

/* Set the right machine number for a MIPS ELF file.  */

static bfd_boolean
mips_elf64_object_p (bfd *abfd)
{
  bfd_default_set_arch_mach (abfd, bfd_arch_riscv, bfd_mach_riscv64);
  return TRUE;
}

/* Support for core dump NOTE sections.  */
static bfd_boolean
elf64_mips_grok_prstatus (bfd *abfd, Elf_Internal_Note *note)
{
  int offset;
  unsigned int size;

  switch (note->descsz)
    {
      default:
	return FALSE;

      case 480:		/* Linux/MIPS - N64 kernel */
	/* pr_cursig */
	elf_tdata (abfd)->core_signal = bfd_get_16 (abfd, note->descdata + 12);

	/* pr_pid */
	elf_tdata (abfd)->core_lwpid = bfd_get_32 (abfd, note->descdata + 32);

	/* pr_reg */
	offset = 112;
	size = 360;

	break;
    }

  /* Make a ".reg/999" section.  */
  return _bfd_elfcore_make_pseudosection (abfd, ".reg",
					  size, note->descpos + offset);
}

static bfd_boolean
elf64_mips_grok_psinfo (bfd *abfd, Elf_Internal_Note *note)
{
  switch (note->descsz)
    {
      default:
	return FALSE;

      case 136:		/* Linux/MIPS - N64 kernel elf_prpsinfo */
	elf_tdata (abfd)->core_program
	 = _bfd_elfcore_strndup (abfd, note->descdata + 40, 16);
	elf_tdata (abfd)->core_command
	 = _bfd_elfcore_strndup (abfd, note->descdata + 56, 80);
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

/* Relocations in the 64 bit MIPS ELF ABI are more complex than in
   standard ELF.  This structure is used to redirect the relocation
   handling routines.  */

const struct elf_size_info mips_elf64_size_info =
{
  sizeof (Elf64_External_Ehdr),
  sizeof (Elf64_External_Phdr),
  sizeof (Elf64_External_Shdr),
  sizeof (Elf64_Mips_External_Rel),
  sizeof (Elf64_Mips_External_Rela),
  sizeof (Elf64_External_Sym),
  sizeof (Elf64_External_Dyn),
  sizeof (Elf_External_Note),
  4,		/* hash-table entry size */
  3,		/* internal relocations per external relocations */
  64,		/* arch_size */
  3,		/* log_file_align */
  ELFCLASS64,
  EV_CURRENT,
  bfd_elf64_write_out_phdrs,
  bfd_elf64_write_shdrs_and_ehdr,
  bfd_elf64_checksum_contents,
  mips_elf64_write_relocs,
  bfd_elf64_swap_symbol_in,
  bfd_elf64_swap_symbol_out,
  mips_elf64_slurp_reloc_table,
  bfd_elf64_slurp_symbol_table,
  bfd_elf64_swap_dyn_in,
  bfd_elf64_swap_dyn_out,
  mips_elf64_be_swap_reloc_in,
  mips_elf64_be_swap_reloc_out,
  mips_elf64_be_swap_reloca_in,
  mips_elf64_be_swap_reloca_out
};

#define ELF_ARCH			bfd_arch_riscv
#define ELF_TARGET_ID			MIPS_ELF_DATA
#define ELF_MACHINE_CODE		EM_RISCV

#define elf_backend_collect		TRUE
#define elf_backend_type_change_ok	TRUE
#define elf_backend_can_gc_sections	TRUE
#define elf_info_to_howto		mips_elf64_info_to_howto_rela
#define elf_info_to_howto_rel		mips_elf64_info_to_howto_rel
#define elf_backend_object_p		mips_elf64_object_p
#define elf_backend_symbol_processing	_bfd_riscv_elf_symbol_processing
#define elf_backend_add_symbol_hook	_bfd_riscv_elf_add_symbol_hook
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
#define elf_backend_relocate_section    _bfd_riscv_elf_relocate_section
#define elf_backend_finish_dynamic_symbol \
				_bfd_riscv_elf_finish_dynamic_symbol
#define elf_backend_finish_dynamic_sections \
				_bfd_riscv_elf_finish_dynamic_sections
#define elf_backend_additional_program_headers \
				_bfd_riscv_elf_additional_program_headers
#define elf_backend_modify_segment_map	_bfd_riscv_elf_modify_segment_map
#define elf_backend_gc_mark_hook	_bfd_riscv_elf_gc_mark_hook
#define elf_backend_gc_sweep_hook	_bfd_riscv_elf_gc_sweep_hook
#define elf_backend_copy_indirect_symbol \
					_bfd_riscv_elf_copy_indirect_symbol
#define elf_backend_ignore_discarded_relocs \
					_bfd_riscv_elf_ignore_discarded_relocs
#define elf_backend_mips_rtype_to_howto	mips_elf64_rtype_to_howto
#define elf_backend_size_info		mips_elf64_size_info

#define elf_backend_grok_prstatus	elf64_mips_grok_prstatus
#define elf_backend_grok_psinfo		elf64_mips_grok_psinfo

#define elf_backend_got_header_size	(4 * MIPS_RESERVED_GOTNO)

/* MIPS ELF64 can use a mixture of REL and RELA, but some Relocations
   work better/work only in RELA, so we default to this.  */
#define elf_backend_may_use_rel_p	1
#define elf_backend_may_use_rela_p	1
#define elf_backend_default_use_rela_p	1
#define elf_backend_rela_plts_and_copies_p 0
#define elf_backend_plt_readonly	1
#define elf_backend_plt_sym_val		_bfd_riscv_elf_plt_sym_val

#define elf_backend_sign_extend_vma	TRUE

#define elf_backend_write_section	_bfd_riscv_elf_write_section

/* We don't set bfd_elf64_bfd_is_local_label_name because the 32-bit
   MIPS-specific function only applies to IRIX5, which had no 64-bit
   ABI.  */
#define bfd_elf64_find_nearest_line	_bfd_riscv_elf_find_nearest_line
#define bfd_elf64_find_inliner_info	_bfd_riscv_elf_find_inliner_info
#define bfd_elf64_new_section_hook	_bfd_riscv_elf_new_section_hook
#define bfd_elf64_set_section_contents	_bfd_riscv_elf_set_section_contents
#define bfd_elf64_bfd_get_relocated_section_contents \
				bfd_generic_get_relocated_section_contents
#define bfd_elf64_bfd_link_hash_table_create \
				_bfd_riscv_elf_link_hash_table_create
#define bfd_elf64_bfd_final_link	_bfd_riscv_elf_final_link
#define bfd_elf64_bfd_merge_private_bfd_data \
				_bfd_riscv_elf_merge_private_bfd_data
#define bfd_elf64_bfd_print_private_bfd_data \
				_bfd_riscv_elf_print_private_bfd_data

#define bfd_elf64_get_reloc_upper_bound mips_elf64_get_reloc_upper_bound
#define bfd_elf64_canonicalize_reloc mips_elf64_canonicalize_reloc
#define bfd_elf64_get_dynamic_reloc_upper_bound mips_elf64_get_dynamic_reloc_upper_bound
#define bfd_elf64_canonicalize_dynamic_reloc mips_elf64_canonicalize_dynamic_reloc
#define bfd_elf64_bfd_relax_section     _bfd_riscv_relax_section

/* MIPS ELF64 archive functions.  */
#define bfd_elf64_archive_functions
extern bfd_boolean bfd_elf64_archive_slurp_armap
  (bfd *);
extern bfd_boolean bfd_elf64_archive_write_armap
  (bfd *, unsigned int, struct orl *, unsigned int, int);
#define bfd_elf64_archive_slurp_extended_name_table \
			_bfd_archive_coff_slurp_extended_name_table
#define bfd_elf64_archive_construct_extended_name_table \
			_bfd_archive_coff_construct_extended_name_table
#define bfd_elf64_archive_truncate_arname \
			_bfd_archive_coff_truncate_arname
#define bfd_elf64_archive_read_ar_hdr	_bfd_archive_coff_read_ar_hdr
#define bfd_elf64_archive_write_ar_hdr	_bfd_archive_coff_write_ar_hdr
#define bfd_elf64_archive_openr_next_archived_file \
			_bfd_archive_coff_openr_next_archived_file
#define bfd_elf64_archive_get_elt_at_index \
			_bfd_archive_coff_get_elt_at_index
#define bfd_elf64_archive_generic_stat_arch_elt \
			_bfd_archive_coff_generic_stat_arch_elt
#define bfd_elf64_archive_update_armap_timestamp \
			_bfd_archive_coff_update_armap_timestamp

/* The SGI style (n)64 NewABI.  */
#define TARGET_LITTLE_SYM		bfd_elf64_littleriscv_vec
#define TARGET_LITTLE_NAME		"elf64-littleriscv"
#define TARGET_BIG_SYM			bfd_elf64_bigriscv_vec
#define TARGET_BIG_NAME			"elf64-bigriscv"

#define ELF_MAXPAGESIZE			0x10000
#define ELF_COMMONPAGESIZE		0x1000

#include "elf64-target.h"
