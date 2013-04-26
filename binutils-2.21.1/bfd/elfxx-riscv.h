/* RISC-V ELF specific backend routines.
   Copyright 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009
   Free Software Foundation, Inc.

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

#include "elf/common.h"
#include "elf/internal.h"
#include "elf/riscv.h"

extern bfd_boolean _bfd_riscv_elf_new_section_hook
  (bfd *, asection *);
extern void _bfd_riscv_elf_symbol_processing
  (bfd *, asymbol *);
extern unsigned int _bfd_riscv_elf_eh_frame_address_size
  (bfd *, asection *);
extern bfd_boolean _bfd_riscv_elf_section_from_shdr
  (bfd *, Elf_Internal_Shdr *, const char *, int);
extern bfd_boolean _bfd_riscv_elf_fake_sections
  (bfd *, Elf_Internal_Shdr *, asection *);
extern bfd_boolean _bfd_riscv_elf_section_from_bfd_section
  (bfd *, asection *, int *);
extern bfd_boolean _bfd_riscv_elf_add_symbol_hook
  (bfd *, struct bfd_link_info *, Elf_Internal_Sym *,
   const char **, flagword *, asection **, bfd_vma *);
extern int _bfd_riscv_elf_link_output_symbol_hook
  (struct bfd_link_info *, const char *, Elf_Internal_Sym *,
   asection *, struct elf_link_hash_entry *);
extern bfd_boolean _bfd_riscv_elf_create_dynamic_sections
  (bfd *, struct bfd_link_info *);
extern bfd_boolean _bfd_riscv_elf_check_relocs
  (bfd *, struct bfd_link_info *, asection *, const Elf_Internal_Rela *);
extern bfd_boolean _bfd_riscv_elf_adjust_dynamic_symbol
  (struct bfd_link_info *, struct elf_link_hash_entry *);
extern bfd_boolean _bfd_riscv_elf_always_size_sections
  (bfd *, struct bfd_link_info *);
extern bfd_boolean _bfd_riscv_elf_size_dynamic_sections
  (bfd *, struct bfd_link_info *);
extern bfd_boolean _bfd_riscv_elf_relocate_section
  (bfd *, struct bfd_link_info *, bfd *, asection *, bfd_byte *,
   Elf_Internal_Rela *, Elf_Internal_Sym *, asection **);
extern bfd_boolean _bfd_riscv_elf_finish_dynamic_symbol
  (bfd *, struct bfd_link_info *, struct elf_link_hash_entry *,
   Elf_Internal_Sym *);
extern bfd_boolean _bfd_riscv_elf_finish_dynamic_sections
  (bfd *, struct bfd_link_info *);
extern void _bfd_riscv_elf_final_write_processing
  (bfd *, bfd_boolean);
extern int _bfd_riscv_elf_additional_program_headers
  (bfd *, struct bfd_link_info *);
extern bfd_boolean _bfd_riscv_elf_modify_segment_map
  (bfd *, struct bfd_link_info *);
extern asection * _bfd_riscv_elf_gc_mark_hook
  (asection *, struct bfd_link_info *, Elf_Internal_Rela *,
   struct elf_link_hash_entry *, Elf_Internal_Sym *);
extern bfd_boolean _bfd_riscv_elf_gc_sweep_hook
  (bfd *, struct bfd_link_info *, asection *, const Elf_Internal_Rela *);
extern void _bfd_riscv_elf_copy_indirect_symbol
  (struct bfd_link_info *, struct elf_link_hash_entry *,
   struct elf_link_hash_entry *);
extern bfd_boolean _bfd_riscv_elf_ignore_discarded_relocs
  (asection *);
extern bfd_boolean _bfd_riscv_elf_find_nearest_line
  (bfd *, asection *, asymbol **, bfd_vma, const char **,
   const char **, unsigned int *);
extern bfd_boolean _bfd_riscv_elf_find_inliner_info
  (bfd *, const char **, const char **, unsigned int *);
extern bfd_boolean _bfd_riscv_elf_set_section_contents
  (bfd *, asection *, const void *, file_ptr, bfd_size_type);
extern bfd_byte *_bfd_elf_riscv_get_relocated_section_contents
  (bfd *, struct bfd_link_info *, struct bfd_link_order *,
   bfd_byte *, bfd_boolean, asymbol **);
extern struct bfd_link_hash_table *_bfd_riscv_elf_link_hash_table_create
  (bfd *);
extern bfd_boolean _bfd_riscv_elf_final_link
  (bfd *, struct bfd_link_info *);
extern bfd_boolean _bfd_riscv_elf_merge_private_bfd_data
  (bfd *, bfd *);
extern bfd_boolean _bfd_riscv_elf_set_private_flags
  (bfd *, flagword);
extern bfd_boolean _bfd_riscv_elf_print_private_bfd_data
  (bfd *, void *);
extern bfd_boolean _bfd_riscv_elf_discard_info
  (bfd *, struct elf_reloc_cookie *, struct bfd_link_info *);
extern bfd_boolean _bfd_riscv_elf_write_section
  (bfd *, struct bfd_link_info *, asection *, bfd_byte *);

extern bfd_reloc_status_type _bfd_riscv_elf_gprel16_with_gp
  (bfd *, asymbol *, arelent *, asection *, bfd_boolean, void *, bfd_vma);
extern bfd_reloc_status_type _bfd_riscv_elf32_gprel16_reloc
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
extern bfd_reloc_status_type _bfd_riscv_elf_hi16_reloc
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
extern bfd_reloc_status_type _bfd_riscv_elf_got16_reloc
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
extern bfd_reloc_status_type _bfd_riscv_elf_lo16_reloc
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
extern bfd_reloc_status_type _bfd_riscv_elf_generic_reloc
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
extern unsigned long _bfd_elf_riscv_mach
  (flagword);
extern bfd_boolean _bfd_riscv_relax_section
  (bfd *, asection *, struct bfd_link_info *, bfd_boolean *);
extern bfd_vma _bfd_riscv_elf_sign_extend
  (bfd_vma, int);
extern void _bfd_riscv_elf_merge_symbol_attribute
  (struct elf_link_hash_entry *, const Elf_Internal_Sym *, bfd_boolean, bfd_boolean);
extern char *_bfd_riscv_elf_get_target_dtag (bfd_vma);
extern void _bfd_riscv_elf_use_plts_and_copy_relocs
  (struct bfd_link_info *);
extern bfd_vma _bfd_riscv_elf_plt_sym_val
  (bfd_vma, const asection *, const arelent *rel);

extern const struct bfd_elf_special_section _bfd_riscv_elf_special_sections [];

extern bfd_boolean _bfd_riscv_elf_common_definition (Elf_Internal_Sym *);

#define elf_backend_common_definition   _bfd_riscv_elf_common_definition
#define elf_backend_special_sections _bfd_riscv_elf_special_sections
#define elf_backend_eh_frame_address_size _bfd_riscv_elf_eh_frame_address_size
#define elf_backend_merge_symbol_attribute  _bfd_riscv_elf_merge_symbol_attribute
