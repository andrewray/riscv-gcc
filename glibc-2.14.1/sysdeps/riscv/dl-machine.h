/* Machine-dependent ELF dynamic relocation inline functions.  MIPS version.
   Copyright (C) 1996-2001, 2002, 2003, 2004, 2005, 2006, 2007
   Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Kazumoto Kojima <kkojima@info.kanagawa-u.ac.jp>.

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

/*  FIXME: Profiling of shared libraries is not implemented yet.  */
#ifndef dl_machine_h
#define dl_machine_h

#define ELF_MACHINE_NAME "RISC-V"

/* Relocs. */
#define R_RISCV_COPY      24
#define R_RISCV_JUMP_SLOT 25

#include <entry.h>

#ifndef ENTRY_POINT
#error ENTRY_POINT needs to be defined for MIPS.
#endif

#include <sys/asm.h>
#include <dl-tls.h>

#ifndef _RTLD_PROLOGUE
# define _RTLD_PROLOGUE(entry)						\
	".globl\t" __STRING(entry) "\n\t"				\
	".type\t" __STRING(entry) ", @function\n"			\
	__STRING(entry) ":\n\t"
#endif

#ifndef _RTLD_EPILOGUE
# define _RTLD_EPILOGUE(entry)						\
	".size\t" __STRING(entry) ", . - " __STRING(entry) "\n\t"
#endif

/* A reloc type used for ld.so cmdline arg lookups to reject PLT entries.
   This only makes sense on MIPS when using PLTs, so choose the
   PLT relocation (not encountered when not using PLTs).  */
#define ELF_MACHINE_JMP_SLOT			R_RISCV_JUMP_SLOT
#define elf_machine_type_class(type) \
  ((((type) == ELF_MACHINE_JMP_SLOT) * ELF_RTYPE_CLASS_PLT)	\
   | (((type) == R_RISCV_COPY) * ELF_RTYPE_CLASS_COPY))

#define ELF_MACHINE_PLT_REL 1

/* Translate a processor specific dynamic tag to the index
   in l_info array.  */
#define DT_MIPS(x) (DT_MIPS_##x - DT_LOPROC + DT_NUM)

#define ELF_MACHINE_DEBUG_SETUP(l,r)

/* Return nonzero iff ELF header is compatible with the running host.  */
static inline int __attribute_used__
elf_machine_matches_host (const ElfW(Ehdr) *ehdr)
{
  return 1;
}

static inline ElfW(Addr) *
elf_mips_got_from_gpreg (ElfW(Addr) gpreg)
{
  return (ElfW(Addr) *) gpreg;
}

/* Return the link-time address of _DYNAMIC.  Conveniently, this is the
   first element of the GOT.  This must be inlined in a function which
   uses global data.  */
static inline ElfW(Addr)
elf_machine_dynamic (void)
{
  ElfW(Addr) load, link, got;
  asm ("   la   %0, 1f\n"
       "   la   %1, _GLOBAL_OFFSET_TABLE_\n"
       "1: rdpc %2"
       : "=r"(link), "=r"(got), "=r"(load));

  return *elf_mips_got_from_gpreg(load - link + got);
}

#define STRINGXP(X) __STRING(X)
#define STRINGXV(X) STRINGV_(X)
#define STRINGV_(...) # __VA_ARGS__

/* Return the run-time load address of the shared object.  */
static inline ElfW(Addr)
elf_machine_load_address (void)
{
  ElfW(Addr) load, link;
  asm ("   la   %0, 1f\n"
       "1: rdpc %1\n"
       : "=r"(link), "=r"(load));

  return load - link;
}

/* We can't rely on elf_machine_got_rel because _dl_object_relocation_scope
   fiddles with global data.  */
#define ELF_MACHINE_BEFORE_RTLD_RELOC(dynamic_info)			\
do {									\
  struct link_map *map = &bootstrap_map;				\
  ElfW(Sym) *sym;							\
  ElfW(Addr) *got;							\
  int i, n;								\
									\
  got = (ElfW(Addr) *) D_PTR (map, l_info[DT_PLTGOT]);			\
									\
  if (__builtin_expect (map->l_addr == 0, 1))				\
    break;								\
									\
  i = 2; /* got[0] and got[1] are reserved. */				\
  n = map->l_info[DT_MIPS (LOCAL_GOTNO)]->d_un.d_val;			\
									\
  /* Add the run-time displacement to all local got entries. */		\
  while (i < n)								\
    got[i++] += map->l_addr;						\
									\
  /* Handle global got entries. */					\
  got += n;								\
  sym = (ElfW(Sym) *) D_PTR(map, l_info[DT_SYMTAB])			\
       + map->l_info[DT_MIPS (GOTSYM)]->d_un.d_val;			\
  i = (map->l_info[DT_MIPS (SYMTABNO)]->d_un.d_val			\
       - map->l_info[DT_MIPS (GOTSYM)]->d_un.d_val);			\
									\
  while (i--)								\
    {									\
      if (sym->st_shndx == SHN_UNDEF || sym->st_shndx == SHN_COMMON)	\
	*got = map->l_addr + sym->st_value;				\
      else if (ELFW(ST_TYPE) (sym->st_info) == STT_FUNC			\
	       && *got != sym->st_value)				\
	*got += map->l_addr;						\
      else if (ELFW(ST_TYPE) (sym->st_info) == STT_SECTION)		\
	{								\
	  if (sym->st_other == 0)					\
	    *got += map->l_addr;					\
	}								\
      else								\
	*got = map->l_addr + sym->st_value;				\
									\
      got++;								\
      sym++;								\
    }									\
} while(0)


/* Mask identifying addresses reserved for the user program,
   where the dynamic linker should not map anything.  */
#define ELF_MACHINE_USER_ADDRESS_MASK	0x80000000UL


/* Initial entry point code for the dynamic linker.
   The C function `_dl_start' is the real entry point;
   its return value is the user program's entry point. */

#define RTLD_START asm (\
	".text\n\
	" _RTLD_PROLOGUE(ENTRY_POINT) "\
	# Store &_DYNAMIC in the first entry of the GOT.\n\
	la a1, 1f\n\
	la a2, _GLOBAL_OFFSET_TABLE_\n\
	la a3, _DYNAMIC\n\
	1: rdpc a0\n\
	sub a0, a0, a1\n\
	add a0, a0, a2\n\
	" STRINGXP(REG_S) " a3, 0(a0)\n\
	move a0, sp\n\
	jal _dl_start\n\
	# Fall through to _dl_start_user \
	" _RTLD_EPILOGUE(ENTRY_POINT) "\
	\n\
	\n\
	" _RTLD_PROLOGUE(_dl_start_user) "\
	# Stash user entry point in s0.\n\
	move s0, v0\n\
	# See if we were run as a command with the executable file\n\
	# name as an extra leading argument.\n\
	la v0, _dl_skip_args\n\
	lw v0, 0(v0)\n\
	beqz v0, 1f\n\
	# Load the original argument count.\n\
	" STRINGXP(REG_L) " a0, 0(sp)\n\
	# Subtract _dl_skip_args from it.\n\
	sub a0, a0, v0\n\
	# Adjust the stack pointer to skip _dl_skip_args words.\n\
	sll v0, v0, " STRINGXP (PTRLOG) "\n\
	add sp, sp, v0\n\
	# Save back the modified argument count.\n\
	" STRINGXP(REG_S) " a0, 0(sp)\n\
1:	# Call _dl_init (struct link_map *main_map, int argc, char **argv, char **env) \n\
	la a0, _rtld_local\n\
	" STRINGXP(REG_L) " a0, 0(a0)\n\
	" STRINGXP(REG_L) " a1, 0(sp)\n\
	add a2, sp, " STRINGXP (SZREG) "\n\
	sll a3, a1, " STRINGXP (PTRLOG) "\n\
	add a3, a3, a2\n\
	add a3, a3, " STRINGXP (SZREG) "\n\
	# Call the function to run the initializers.\n\
	jal _dl_init_internal\n\
	# Pass our finalizer function to the user in v0 as per ELF ABI.\n\
	la v0, _dl_fini\n\
	# Jump to the user entry point.\n\
	jr s0\n\t"\
	_RTLD_EPILOGUE(_dl_start_user)\
	".previous"\
);

/* Names of the architecture-specific auditing callback functions.  */
# ifdef __riscv64
#  define ARCH_LA_PLTENTER mips_n64_gnu_pltenter
#  define ARCH_LA_PLTEXIT mips_n64_gnu_pltexit
# else
#  define ARCH_LA_PLTENTER mips_n32_gnu_pltenter
#  define ARCH_LA_PLTEXIT mips_n32_gnu_pltexit
# endif

/* For a non-writable PLT, rewrite the .got.plt entry at RELOC_ADDR to
   point at the symbol with address VALUE.  For a writable PLT, rewrite
   the corresponding PLT entry instead.  */
static inline ElfW(Addr)
elf_machine_fixup_plt (struct link_map *map, lookup_t t,
		       const ElfW(Rel) *reloc,
		       ElfW(Addr) *reloc_addr, ElfW(Addr) value)
{
  return *reloc_addr = value;
}

static inline ElfW(Addr)
elf_machine_plt_value (struct link_map *map, const ElfW(Rel) *reloc,
		       ElfW(Addr) value)
{
  return value;
}

#endif /* !dl_machine_h */

#ifdef RESOLVE_MAP

/* Perform a relocation described by R_INFO at the location pointed to
   by RELOC_ADDR.  SYM is the relocation symbol specified by R_INFO and
   MAP is the object containing the reloc.  */

auto inline void
__attribute__ ((always_inline))
elf_machine_reloc (struct link_map *map, ElfW(Addr) r_info,
		   const ElfW(Sym) *sym, const struct r_found_version *version,
		   void *reloc_addr, ElfW(Addr) r_addend, int inplace_p)
{
  const unsigned long int r_type = ELFW(R_TYPE) (r_info);
  ElfW(Addr) *addr_field = (ElfW(Addr) *) reloc_addr;

#if !defined RTLD_BOOTSTRAP && !defined SHARED
  /* This is defined in rtld.c, but nowhere in the static libc.a;
     make the reference weak so static programs can still link.  This
     declaration cannot be done when compiling rtld.c (i.e.  #ifdef
     RTLD_BOOTSTRAP) because rtld.c contains the common defn for
     _dl_rtld_map, which is incompatible with a weak decl in the same
     file.  */
  weak_extern (GL(dl_rtld_map));
#endif

  switch (r_type)
    {
#if defined (USE_TLS) && !defined (RTLD_BOOTSTRAP)
# if _RISCV_SIM == _ABI64
    case R_MIPS_TLS_DTPMOD64:
    case R_MIPS_TLS_DTPREL64:
    case R_MIPS_TLS_TPREL64:
# else
    case R_MIPS_TLS_DTPMOD32:
    case R_MIPS_TLS_DTPREL32:
    case R_MIPS_TLS_TPREL32:
# endif
      {
	struct link_map *sym_map = RESOLVE_MAP (&sym, version, r_type);

	switch (r_type)
	  {
	  case R_MIPS_TLS_DTPMOD64:
	  case R_MIPS_TLS_DTPMOD32:
	    if (sym_map)
	      *addr_field = sym_map->l_tls_modid;
	    break;

	  case R_MIPS_TLS_DTPREL64:
	  case R_MIPS_TLS_DTPREL32:
	    if (sym)
	      {
		if (inplace_p)
		  r_addend = *addr_field;
		*addr_field = r_addend + TLS_DTPREL_VALUE (sym);
	      }
	    break;

	  case R_MIPS_TLS_TPREL32:
	  case R_MIPS_TLS_TPREL64:
	    if (sym)
	      {
		CHECK_STATIC_TLS (map, sym_map);
		if (inplace_p)
		  r_addend = *addr_field;
		*addr_field = r_addend + TLS_TPREL_VALUE (sym_map, sym);
	      }
	    break;
	  }

	break;
      }
#endif

    case R_MIPS_REL32:
      {
	int symidx = ELFW(R_SYM) (r_info);
	ElfW(Addr) reloc_value;

	if (inplace_p)
	  /* Support relocations on mis-aligned offsets.  */
	  __builtin_memcpy (&reloc_value, reloc_addr, sizeof (reloc_value));
	else
	  reloc_value = r_addend;

	if (symidx)
	  {
	    const ElfW(Word) gotsym
	      = (const ElfW(Word)) map->l_info[DT_MIPS (GOTSYM)]->d_un.d_val;

	    if ((ElfW(Word))symidx < gotsym)
	      {
		/* This wouldn't work for a symbol imported from other
		   libraries for which there's no GOT entry, but MIPS
		   requires every symbol referenced in a dynamic
		   relocation to have a GOT entry in the primary GOT,
		   so we only get here for locally-defined symbols.
		   For section symbols, we should *NOT* be adding
		   sym->st_value (per the definition of the meaning of
		   S in reloc expressions in the ELF64 MIPS ABI),
		   since it should have already been added to
		   reloc_value by the linker, but older versions of
		   GNU ld didn't add it, and newer versions don't emit
		   useless relocations to section symbols any more, so
		   it is safe to keep on adding sym->st_value, even
		   though it's not ABI compliant.  Some day we should
		   bite the bullet and stop doing this.  */
#ifndef RTLD_BOOTSTRAP
		if (map != &GL(dl_rtld_map))
#endif
		  reloc_value += sym->st_value + map->l_addr;
	      }
	    else
	      {
#ifndef RTLD_BOOTSTRAP
		const ElfW(Addr) *got
		  = (const ElfW(Addr) *) D_PTR (map, l_info[DT_PLTGOT]);
		const ElfW(Word) local_gotno
		  = (const ElfW(Word))
		    map->l_info[DT_MIPS (LOCAL_GOTNO)]->d_un.d_val;

		reloc_value += got[symidx + local_gotno - gotsym];
#endif
	      }
	  }
	else
#ifndef RTLD_BOOTSTRAP
	  if (map != &GL(dl_rtld_map))
#endif
	    reloc_value += map->l_addr;

	__builtin_memcpy (reloc_addr, &reloc_value, sizeof (reloc_value));
      }
      break;
#ifndef RTLD_BOOTSTRAP
    case R_MIPS_GLOB_DAT:
      {
	int symidx = ELFW(R_SYM) (r_info);
	const ElfW(Word) gotsym
	  = (const ElfW(Word)) map->l_info[DT_MIPS (GOTSYM)]->d_un.d_val;

	if (__builtin_expect ((ElfW(Word)) symidx >= gotsym, 1))
	  {
	    const ElfW(Addr) *got
	      = (const ElfW(Addr) *) D_PTR (map, l_info[DT_PLTGOT]);
	    const ElfW(Word) local_gotno
	      = ((const ElfW(Word))
		 map->l_info[DT_MIPS (LOCAL_GOTNO)]->d_un.d_val);

	    ElfW(Addr) reloc_value = got[symidx + local_gotno - gotsym];
	    __builtin_memcpy (reloc_addr, &reloc_value, sizeof (reloc_value));
	  }
      }
      break;
#endif
    case R_MIPS_NONE:		/* Alright, Wilbur.  */
      break;

    case R_RISCV_JUMP_SLOT:
      {
	struct link_map *sym_map;
	ElfW(Addr) value;

	/* The addend for a jump slot relocation must always be zero:
	   calls via the PLT always branch to the symbol's address and
	   not to the address plus a non-zero offset.  */
	if (r_addend != 0)
	  _dl_signal_error (0, map->l_name, NULL,
			    "found jump slot relocation with non-zero addend");

	sym_map = RESOLVE_MAP (&sym, version, r_type);
	value = sym_map == NULL ? 0 : sym_map->l_addr + sym->st_value;
	*addr_field = value;

	break;
      }

    case R_RISCV_COPY:
      {
	const ElfW(Sym) *const refsym = sym;
	struct link_map *sym_map;
	ElfW(Addr) value;

	/* Calculate the address of the symbol.  */
	sym_map = RESOLVE_MAP (&sym, version, r_type);
	value = sym_map == NULL ? 0 : sym_map->l_addr + sym->st_value;

	if (__builtin_expect (sym == NULL, 0))
	  /* This can happen in trace mode if an object could not be
	     found.  */
	  break;
	if (__builtin_expect (sym->st_size > refsym->st_size, 0)
	    || (__builtin_expect (sym->st_size < refsym->st_size, 0)
		&& GLRO(dl_verbose)))
	  {
	    const char *strtab;

	    strtab = (const void *) D_PTR (map, l_info[DT_STRTAB]);
	    _dl_error_printf ("\
  %s: Symbol `%s' has different size in shared object, consider re-linking\n",
			      rtld_progname ?: "<program name unknown>",
			      strtab + refsym->st_name);
	  }
	memcpy (reloc_addr, (void *) value,
	        MIN (sym->st_size, refsym->st_size));
	break;
      }

    default:
      _dl_reloc_bad_type (map, r_type, 0);
      break;
    }
}

/* Perform the relocation specified by RELOC and SYM (which is fully resolved).
   MAP is the object containing the reloc.  */

auto inline void
__attribute__ ((always_inline))
elf_machine_rel (struct link_map *map, const ElfW(Rel) *reloc,
		 const ElfW(Sym) *sym, const struct r_found_version *version,
		 void *const reloc_addr)
{
  elf_machine_reloc (map, reloc->r_info, sym, version, reloc_addr, 0, 1);
}

auto inline void
__attribute__((always_inline))
elf_machine_rel_relative (ElfW(Addr) l_addr, const ElfW(Rel) *reloc,
			  void *const reloc_addr)
{
  /* XXX Nothing to do.  There is no relative relocation, right?  */
}

auto inline void
__attribute__((always_inline))
elf_machine_lazy_rel (struct link_map *map,
		      ElfW(Addr) l_addr, const ElfW(Rel) *reloc)
{
  ElfW(Addr) *const reloc_addr = (void *) (l_addr + reloc->r_offset);
  const unsigned int r_type = ELFW(R_TYPE) (reloc->r_info);
  /* Check for unexpected PLT reloc type.  */
  if (__builtin_expect (r_type == R_RISCV_JUMP_SLOT, 1))
    {
      if (__builtin_expect (map->l_mach.plt, 0) == 0)
	{
	  /* Nothing is required here since we only support lazy
	     relocation in executables.  */
	}
      else
	*reloc_addr = map->l_mach.plt;
    }
  else
    _dl_reloc_bad_type (map, r_type, 1);
}

auto inline void
__attribute__ ((always_inline))
elf_machine_rela (struct link_map *map, const ElfW(Rela) *reloc,
		  const ElfW(Sym) *sym, const struct r_found_version *version,
		 void *const reloc_addr)
{
  elf_machine_reloc (map, reloc->r_info, sym, version, reloc_addr,
		     reloc->r_addend, 0);
}

auto inline void
__attribute__((always_inline))
elf_machine_rela_relative (ElfW(Addr) l_addr, const ElfW(Rela) *reloc,
			   void *const reloc_addr)
{
}

#ifndef RTLD_BOOTSTRAP
/* Relocate GOT. */
auto inline void
__attribute__((always_inline))
elf_machine_got_rel (struct link_map *map, int lazy)
{
  ElfW(Addr) *got;
  ElfW(Sym) *sym;
  const ElfW(Half) *vernum;
  int i, n, symidx;

#define RESOLVE_GOTSYM(sym,vernum,sym_index,reloc)			  \
    ({									  \
      const ElfW(Sym) *ref = sym;					  \
      const struct r_found_version *version				  \
        = vernum ? &map->l_versions[vernum[sym_index] & 0x7fff] : NULL;	  \
      struct link_map *sym_map;						  \
      sym_map = RESOLVE_MAP (&ref, version, reloc);			  \
      ref ? sym_map->l_addr + ref->st_value : 0;			  \
    })

  if (map->l_info[VERSYMIDX (DT_VERSYM)] != NULL)
    vernum = (const void *) D_PTR (map, l_info[VERSYMIDX (DT_VERSYM)]);
  else
    vernum = NULL;

  got = (ElfW(Addr) *) D_PTR (map, l_info[DT_PLTGOT]);

  n = map->l_info[DT_MIPS (LOCAL_GOTNO)]->d_un.d_val;
  /* The dynamic linker's local got entries have already been relocated.  */
  if (map != &GL(dl_rtld_map))
    {
      /* got[0] and got[1] are reserved. */
      i = 2;

      /* Add the run-time displacement to all local got entries if
         needed.  */
      if (__builtin_expect (map->l_addr != 0, 0))
	{
	  while (i < n)
	    got[i++] += map->l_addr;
	}
    }

  /* Handle global got entries. */
  got += n;
  /* Keep track of the symbol index.  */
  symidx = map->l_info[DT_MIPS (GOTSYM)]->d_un.d_val;
  sym = (ElfW(Sym) *) D_PTR (map, l_info[DT_SYMTAB]) + symidx;
  i = (map->l_info[DT_MIPS (SYMTABNO)]->d_un.d_val
       - map->l_info[DT_MIPS (GOTSYM)]->d_un.d_val);

  /* This loop doesn't handle Quickstart.  */
  while (i--)
    {
      if (sym->st_shndx == SHN_UNDEF || sym->st_shndx == SHN_COMMON)
	*got = RESOLVE_GOTSYM (sym, vernum, symidx, R_MIPS_32);
      else if (ELFW(ST_TYPE) (sym->st_info) == STT_FUNC
	       && *got != sym->st_value)
	{
	  if (lazy)
	    *got += map->l_addr;
	  else
	    /* This is a lazy-binding stub, so we don't need the
	       canonical address.  */
	    *got = RESOLVE_GOTSYM (sym, vernum, symidx, R_RISCV_JUMP_SLOT);
	}
      else if (ELFW(ST_TYPE) (sym->st_info) == STT_SECTION)
	{
	  if (sym->st_other == 0)
	    *got += map->l_addr;
	}
      else
	*got = RESOLVE_GOTSYM (sym, vernum, symidx, R_MIPS_32);

      ++got;
      ++sym;
      ++symidx;
    }

#undef RESOLVE_GOTSYM
}
#endif

/* Set up the loaded object described by L so its stub function
   will jump to the on-demand fixup code __dl_runtime_resolve.  */

auto inline int
__attribute__((always_inline))
elf_machine_runtime_setup (struct link_map *l, int lazy, int profile)
{
# ifndef RTLD_BOOTSTRAP
  ElfW(Addr) *got;
  extern void _dl_runtime_resolve (ElfW(Word));
  extern void _dl_fixup (void);

  if (lazy)
    {
      /* The GOT entries for functions have not yet been filled in.
	 Their initial contents will arrange when called to put an
	 offset into the .dynsym section in t8, the return address
	 in t7 and then jump to _GLOBAL_OFFSET_TABLE[0].  */
      got = (ElfW(Addr) *) D_PTR (l, l_info[DT_PLTGOT]);

      /* Store the runtime resolver's address in got[0]. */
      got[0] = (ElfW(Addr)) &_dl_runtime_resolve;
      /* Store the link map in got[1]. */
      got[1] = (ElfW(Addr)) l;
    }

  /* Relocate global offset table.  */
  elf_machine_got_rel (l, lazy);

  /* If using PLTs, fill in the first two entries of .got.plt.  */
  if (l->l_info[DT_JMPREL] && lazy)
    {
      ElfW(Addr) *gotplt;
      gotplt = (ElfW(Addr) *) D_PTR (l, l_info[DT_MIPS (PLTGOT)]);
      /* If a library is prelinked but we have to relocate anyway,
	 we have to be able to undo the prelinking of .got.plt.
	 The prelinker saved the address of .plt for us here.  */
      if (gotplt[1])
	l->l_mach.plt = gotplt[1] + l->l_addr;
      gotplt[0] = (ElfW(Addr)) &_dl_fixup;
      gotplt[1] = (ElfW(Addr)) l;
      /* Relocate subsequent .got.plt entries. */
      for (gotplt += 2; *gotplt; gotplt++)
	*gotplt += l->l_addr;
    }

# endif
  return lazy;
}

#endif /* RESOLVE_MAP */
