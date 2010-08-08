/* x86 fat binary initializers.

   Contributed to the GNU project by Kevin Ryde (original x86_32 code) and
   Torbjorn Granlund (port to x86_64)

   THE FUNCTIONS AND VARIABLES IN THIS FILE ARE FOR INTERNAL USE ONLY.
   THEY'RE ALMOST CERTAIN TO BE SUBJECT TO INCOMPATIBLE CHANGES OR DISAPPEAR
   COMPLETELY IN FUTURE GNU MP RELEASES.

Copyright 2003, 2004, 2009 Free Software Foundation, Inc.

This file is part of the GNU MP Library.

The GNU MP Library is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 3 of the License, or (at your
option) any later version.

The GNU MP Library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details.

You should have received a copy of the GNU Lesser General Public License
along with the GNU MP Library.  If not, see http://www.gnu.org/licenses/.  */

#include <stdio.h>    /* for printf */
#include <stdlib.h>   /* for getenv */
#include <string.h>

#include "gmp.h"
#include "gmp-impl.h"

/* Change this to "#define TRACE(x) x" for some traces. */
#define TRACE(x)


/* fat_entry.asm */
long __gmpn_cpuid __GMP_PROTO ((char dst[12], int id));


typedef DECL_preinv_divrem_1 ((*preinv_divrem_1_t));
typedef DECL_preinv_mod_1    ((*preinv_mod_1_t));

struct cpuvec_t __gmpn_cpuvec = {
  __MPN(add_n_init),
  __MPN(addmul_1_init),
  __MPN(copyd_init),
  __MPN(copyi_init),
  __MPN(divexact_1_init),
  __MPN(divexact_by3c_init),
  __MPN(divrem_1_init),
  __MPN(gcd_1_init),
  __MPN(lshift_init),
  __MPN(mod_1_init),
  __MPN(mod_34lsub1_init),
  __MPN(modexact_1c_odd_init),
  __MPN(mul_1_init),
  __MPN(mul_basecase_init),
  __MPN(preinv_divrem_1_init),
  __MPN(preinv_mod_1_init),
  __MPN(rshift_init),
  __MPN(sqr_basecase_init),
  __MPN(sub_n_init),
  __MPN(submul_1_init),
  0
};


/* The following setups start with generic x86, then overwrite with
   specifics for a chip, and higher versions of that chip.

   The arrangement of the setups here will normally be the same as the $path
   selections in configure.in for the respective chips.

   This code is reentrant and thread safe.  We always calculate the same
   decided_cpuvec, so if two copies of the code are running it doesn't
   matter which completes first, both write the same to __gmpn_cpuvec.

   We need to go via decided_cpuvec because if one thread has completed
   __gmpn_cpuvec then it may be making use of the threshold values in that
   vector.  If another thread is still running __gmpn_cpuvec_init then we
   don't want it to write different values to those fields since some of the
   asm routines only operate correctly up to their own defined threshold,
   not an arbitrary value.  */

void
__gmpn_cpuvec_init (void)
{
  struct cpuvec_t  decided_cpuvec;
  char vendor_string[13];
  char dummy_string[12];
  long fms;
  int family, model;

  TRACE (printf ("__gmpn_cpuvec_init:\n"));

  memset (&decided_cpuvec, '\0', sizeof (decided_cpuvec));

  CPUVEC_SETUP_x86_64;
  CPUVEC_SETUP_fat;

  __gmpn_cpuid (vendor_string, 0);
  vendor_string[12] = 0;

  fms = __gmpn_cpuid (dummy_string, 1);
  family = ((fms >> 8) & 0xf) + ((fms >> 20) & 0xff);
  model = ((fms >> 4) & 0xf) + ((fms >> 12) & 0xf0);

  if (strcmp (vendor_string, "GenuineIntel") == 0)
    {
      switch (family)
	{
	case 4:
	case 5:
	  abort ();		/* 32-bit processors */

	case 6:
	  switch (model)
	    {
	    case 0x00:
	    case 0x01:
	    case 0x02:
	    case 0x03:
	    case 0x04:
	    case 0x05:
	    case 0x06:
	    case 0x07:
	    case 0x08:
	    case 0x09:		/* Banias */
	    case 0x0a:
	    case 0x0b:
	    case 0x0c:
	    case 0x0d:		/* Dothan */
	    case 0x0e:		/* Yonah */
	      abort ();		/* 32-bit processors */

	    case 0x0f:
	    case 0x10:
	    case 0x11:
	    case 0x12:
	    case 0x13:
	    case 0x14:
	    case 0x15:
	    case 0x16:
	    case 0x17:
	    case 0x18:
	    case 0x19:
	    case 0x1d:		/* PNR Dunnington */
	      CPUVEC_SETUP_core2;
	      break;

	    case 0x1c:		/* Silverthorne */
	    case 0x26:		/* Lincroft */
	    case 0x27:		/* Saltwell */
	      CPUVEC_SETUP_atom;
	      break;

	    case 0x1a:		/* NHM Gainestown */
	    case 0x1b:
	    case 0x1e:		/* NHM Lynnfield/Jasper */
	    case 0x1f:
	    case 0x20:
	    case 0x21:
	    case 0x22:
	    case 0x23:
	    case 0x24:
	    case 0x25:		/* WSM Clarkdale/Arrandale */
	    case 0x28:
	    case 0x29:
	    case 0x2a:
	    case 0x2b:
	    case 0x2c:		/* WSM Gulftown */
	    case 0x2d:
	    case 0x2e:		/* NHM Beckton */
	    case 0x2f:
	      CPUVEC_SETUP_core2;
	      CPUVEC_SETUP_corei;
	      break;
	    }

	case 15:
	  CPUVEC_SETUP_pentium4;
	  break;
	}
    }
  else if (strcmp (vendor_string, "AuthenticAMD") == 0)
    {
      switch (family)
	{
	case 5:
	case 6:
	  abort ();

	case 15:		/* k8 */
	case 16:		/* k10 */
	  /* CPUVEC_SETUP_athlon */
	  break;
	}
    }
  else if (strcmp (vendor_string, "CentaurHauls") == 0)
    {
      switch (family)
	{
	case 5:
	  abort ();		/* 32-bit processors */

	case 6:
	  if (model < 15)
	    abort ();		/* 32-bit processors */

	  CPUVEC_SETUP_nano;
	  break;
	}
    }

  /* There's no x86 generic mpn_preinv_divrem_1 or mpn_preinv_mod_1.
     Instead default to the plain versions from whichever CPU we detected.
     The function arguments are compatible, no need for any glue code.  */
  if (decided_cpuvec.preinv_divrem_1 == NULL)
    decided_cpuvec.preinv_divrem_1 =(preinv_divrem_1_t)decided_cpuvec.divrem_1;
  if (decided_cpuvec.preinv_mod_1 == NULL)
    decided_cpuvec.preinv_mod_1    =(preinv_mod_1_t)   decided_cpuvec.mod_1;

  ASSERT_CPUVEC (decided_cpuvec);
  CPUVEC_INSTALL (decided_cpuvec);

  /* Set this once the threshold fields are ready.
     Use volatile to prevent it getting moved.  */
  ((volatile struct cpuvec_t *) &__gmpn_cpuvec)->initialized = 1;
}
