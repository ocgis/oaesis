/*
** rsrc.c
**
** Copyright 1995 - 2001 Christer Gustavsson <cg@nocrew.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**  
** Read the file COPYING for more information.
**
*/

#define DEBUGLEVEL 0

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_OSBIND_H
#include <osbind.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vdibind.h>

#include "aesbind.h"
#include "debug.h"
#include "lib_global.h"
#include "mintdefs.h"
#include "objc.h"
#include "rsrc.h"
#include "shel.h"
#include "types.h"

#define FO_READ 0

typedef struct {
  LONG filesize;
  LONG cicon_offset;
  LONG terminator;
}RSHD_EXT;



static
inline
int
CONVCOORD(WORD value,
          WORD chsize)
{
  return (((value >> 8) & 0xff) + ((value & 0xff) * chsize));
}


/*
** Description
** Fix the coordinates of an object
*/
static
WORD
fix_object_coordinates(OBJECT * ob,
                       int      is_internal)
{
  GLOBAL_COMMON * globals = get_global_common ();

  ob->ob_x =
    HW_TO_CW(CONVCOORD(CW_TO_HW(ob->ob_x), globals->clwidth));
  ob->ob_y =
    HW_TO_CW(CONVCOORD(CW_TO_HW(ob->ob_y), globals->clheight));
  ob->ob_width =
    HW_TO_CW(CONVCOORD(CW_TO_HW(ob->ob_width), globals->clwidth));
  ob->ob_height =
    HW_TO_CW(CONVCOORD(CW_TO_HW(ob->ob_height), globals->clheight));

  return  0;
}


/*
** Description
** Load a resource file that is found in the path
*/
static
RSHDR *
Rsrc_do_load_mint (int    apid,
                   int    vid,
                   BYTE * filename,
                   int    is_internal)
{
  LONG    fnr;
  BYTE    namebuf[200];
  LONG    flen;
        
  RSHDR   *rsc;

  strcpy (namebuf, filename);
  DEBUG2 ("rsrc.c: Rsrc_do_load_mint: trying to load %s", namebuf);
  if (Shel_do_find (apid, namebuf) == SHEL_FIND_ERROR) {
    DEBUG0("rsrc.c: loadrsc: Could not find %s",namebuf);
                
    return NULL;
  }

  fnr = Fopen( namebuf, FO_READ);

  if(fnr < 0) {
    DEBUG0("rsrc.c: loadrsc: Could not open %s",namebuf);

    return NULL;
  }
        
  flen = Fseek(0,(WORD)fnr,SEEK_END);
        
  rsc = (RSHDR *)malloc(flen);

  if(!rsc) {
    DEBUG0("rsrc.c: loadrsc: Could not malloc memory for %s",namebuf);

    return NULL;
  }
        
  Fseek(0,(WORD)fnr,SEEK_SET);    
  Fread((WORD)fnr,flen,rsc);
  Fclose((WORD)fnr);

  Rsrc_do_rcfix(vid,
                rsc,
                FALSE, /* FIXME: make better endian check
                          flen != rsc->rsh_rssize */
                is_internal);

  return rsc;
}


/*
** Description
** Load a resource file that is found in the path
*/
static
RSHDR *
Rsrc_do_load_unix (int    apid,
                   int    vid,
                   BYTE * filename,
                   int    is_internal)
{
  int         fnr;
  BYTE        namebuf[200];
  RSHDR *     rsc;
  struct stat st;

  strcpy( namebuf, filename);
  if (Shel_do_find (apid, namebuf) == SHEL_FIND_ERROR) {
    DEBUG0("rsrc.c: loadrsc: Could not find %s",namebuf);
                
    return NULL;
  }

  fnr = open (namebuf, 0);

  if (fnr < 0) {
    DEBUG0("rsrc.c: Rsrc_do_load_unix: Could not open %s", namebuf);

    return NULL;
  }

  if (fstat (fnr, &st) == -1) {
    DEBUG0("rsrc.c: Rsrc_do_load_unix: Could not stat %s", namebuf);
  }

  rsc = (RSHDR *)malloc (st.st_size);

  if (rsc == NULL) {
    DEBUG0("rsrc.c: loadrsc: Could not malloc memory for %s",namebuf);

    return NULL;
  }
        
  if (read (fnr, rsc, st.st_size) == -1) {
    DEBUG0("rsrc.c: loadrsc: Could not read from %s",namebuf);

    return NULL;
  }
  
  close(fnr);


  DEBUG3 ("Calling Rsrc_do_rcfix");
  Rsrc_do_rcfix (vid,
                 rsc,
                 FALSE,
                 is_internal);
  DEBUG3 ("Returned from Rsrc_do_rcfix");
        
  return rsc;
}


/*
** Description
** Load a resource file that is found in the path
*/
static
RSHDR *
Rsrc_do_load(int    apid,
             int    vid,
             BYTE * filename,
             int    is_internal)
{
  GLOBAL_APPL * globals = get_globals (apid);

  if(globals->path_mode == OAESIS_PATH_MODE_MINT)
  {
    DEBUG2 ("rsrc.c: Rsrc_do_load: Calling Rsrc_do_load_mint");
    return Rsrc_do_load_mint(apid, vid, filename, is_internal);
  }
  else
  {
    DEBUG2 ("rsrc.c: Rsrc_do_load: Calling Rsrc_do_load_unix");
    return Rsrc_do_load_unix(apid, vid, filename, is_internal);
  }
}


typedef struct {
  OBJECT *o[1];
} PACKED OARRAY;

/*
** Description
** Calculate the address of an resource object
*/
static
void *
calculate_element_address(RSHDR * rsc,
                          int     type,
                          int     nr,
                          int     is_internal)
{
  switch (type)
  {
  case R_TREE:    /* 0x00 */
    return (OBJECT *)CL_TO_HL(((OARRAY *)(CW_TO_HW(rsc->rsh_trindex) +
                                          (LONG)rsc))->o[nr]);

  case R_BITBLK:  /* 0x04 */
    return &((BITBLK *)(CW_TO_HW(rsc->rsh_bitblk) + (LONG)rsc))[nr];

  case R_STRING:  /* 0x05 */
    return ((BYTE **)CL_TO_HL(CW_TO_HW(rsc->rsh_frstr) + (LONG)rsc))[nr];

  case R_IMAGEDATA:  /* 0x06 */
    return ((void **)CL_TO_HL(CW_TO_HW(rsc->rsh_frimg) + (LONG)rsc))[nr];
                
  case R_FRSTR:   /* 0x0f */
    return &((BYTE **)(CW_TO_HW(rsc->rsh_frstr) + (LONG)rsc))[nr];
                        
  case R_FRIMG:   /* 0x10 */
    return &((void **)(CW_TO_HW(rsc->rsh_frimg) + (LONG)rsc))[nr];
                        
  default:
    DEBUG2 ("%s: Line %d: Rsrc_gaddr: unknown type %d",
            __FILE__,
            __LINE__,
            type);
    return 0L;
  }
}


/****************************************************************************
 * Public functions                                                         *
 ****************************************************************************/

typedef struct {
  LONG l[1];
} PACKED LARRAY;

/*
** Exported
*/
WORD
Rsrc_do_rcfix(int      vid,
              RSHDR  * rsc,
              int      swap_endian,
              int      is_internal)
{
#define SWAP_WORD(w) (swap_endian ? (((UWORD)w >> 8) | ((UWORD)w << 8)) : w)
#define SWAP_LONG(l) (swap_endian ? (((l >> 24) & 0x000000ff) |    \
                                     ((l >>  8) & 0x0000ff00) |    \
                                     ((l <<  8) & 0x00ff0000) |    \
                                     ((l << 24) & 0xff000000)) : l)
  WORD  i;

  UWORD *     iwalk;
  OBJECT *    owalk;
  TEDINFO *   tiwalk;
  ICONBLK *   ibwalk;
  BITBLK *    bbwalk;
  LARRAY *    treewalk;
  LARRAY *    frstrwalk;
  LARRAY *    frimgwalk;
  CICONBLK ** cicons = NULL;
  int         msb = 0;
  int         lsb = 0;

  DEBUG3("Rsrc_do_rcfix: rsc = %p", rsc);
  /*
  ** Detect endian mismatch
  */
  for(iwalk = &rsc->rsh_nobs; iwalk != (WORD *)&rsc->rsh_rssize; iwalk++)
  {
    WORD check_word;

    check_word = CW_TO_HW(*iwalk);

    if((check_word & 0xff00) == 0)
    {
      msb++;
    }

    if((check_word & 0x00ff) == 0)
    {
      lsb++;
    }
  }

  if(lsb > msb)
  {
    swap_endian = TRUE;
  }

  /* Swap words if necessary */
  if(swap_endian)
  {
    DEBUG3 ("Rsrc_do_rcfix: Swapping words in header");
    for (iwalk = (UWORD *)rsc; iwalk <= (UWORD *)&rsc->rsh_rssize; iwalk++) {
      *iwalk = SWAP_WORD(*iwalk);
    }
  }

  DEBUG3("Resource header:");
  DEBUG3("=====================");
  DEBUG3("rsh_vrsn     = 0x%04x", CW_TO_HW(rsc->rsh_vrsn));
  DEBUG3("rsh_object   = 0x%04x", CW_TO_HW(rsc->rsh_object));
  DEBUG3("rsh_tedinfo  = 0x%04x", CW_TO_HW(rsc->rsh_tedinfo));
  DEBUG3("rsh_iconblk  = 0x%04x", CW_TO_HW(rsc->rsh_iconblk));
  DEBUG3("rsh_bitblk   = 0x%04x", CW_TO_HW(rsc->rsh_bitblk));
  DEBUG3("rsh_frstr    = 0x%04x", CW_TO_HW(rsc->rsh_frstr));
  DEBUG3("rsh_string   = 0x%04x", CW_TO_HW(rsc->rsh_string));
  DEBUG3("rsh_imdata   = 0x%04x", CW_TO_HW(rsc->rsh_imdata));
  DEBUG3("rsh_frimg    = 0x%04x", CW_TO_HW(rsc->rsh_frimg));
  DEBUG3("rsh_trindex  = 0x%04x", CW_TO_HW(rsc->rsh_trindex));
  DEBUG3("rsh_nobs     = 0x%04x", CW_TO_HW(rsc->rsh_nobs));
  DEBUG3("rsh_ntree    = 0x%04x", CW_TO_HW(rsc->rsh_ntree));
  DEBUG3("rsh_nted     = 0x%04x", CW_TO_HW(rsc->rsh_nted));
  DEBUG3("rsh_nib      = 0x%04x", CW_TO_HW(rsc->rsh_nib));
  DEBUG3("rsh_nbb      = 0x%04x", CW_TO_HW(rsc->rsh_nbb));
  DEBUG3("rsh_nstring  = 0x%04x", CW_TO_HW(rsc->rsh_nstring));
  DEBUG3("rsh_nimages  = 0x%04x", CW_TO_HW(rsc->rsh_nimages));
  DEBUG3("rsh_rssize   = 0x%04x", CW_TO_HW(rsc->rsh_rssize));
  DEBUG3("=====================");

  /* Initialize pointers */
  owalk = (OBJECT *)((LONG)CW_TO_HW(rsc->rsh_object) + (LONG)rsc); 
  tiwalk = (TEDINFO *)((LONG)CW_TO_HW(rsc->rsh_tedinfo) + (LONG)rsc);
  ibwalk = (ICONBLK *)((LONG)CW_TO_HW(rsc->rsh_iconblk) + (LONG)rsc);
  bbwalk = (BITBLK *)((LONG)CW_TO_HW(rsc->rsh_bitblk) + (LONG)rsc);
  treewalk = (LARRAY *)((LONG)CW_TO_HW(rsc->rsh_trindex) + (LONG)rsc);
  frstrwalk = (LARRAY *)((LONG)CW_TO_HW(rsc->rsh_frstr) + (LONG)rsc);
  frimgwalk = (LARRAY *)((LONG)CW_TO_HW(rsc->rsh_frimg) + (LONG)rsc);

  if(CW_TO_HW(rsc->rsh_vrsn) & 0x4)
  {
    RSHD_EXT * extension;
    CICONBLK * cwalk;
  
    WORD       nr_cicon;
    WORD       i;

    extension =
      (RSHD_EXT *)((LONG)rsc + (LONG)CW_TO_HW(rsc->rsh_rssize));
    i = 0;

    /* Swap endianess on extension array if necessary */
    if (swap_endian) {
      extension->filesize = SWAP_LONG(extension->filesize);
      extension->cicon_offset = SWAP_LONG(extension->cicon_offset);
      extension->terminator = SWAP_LONG(extension->terminator);
    }

    DEBUG3("Extension header:");
    DEBUG3("=====================");
    DEBUG3("filesize     = 0x%04x", CL_TO_HL(extension->filesize));
    DEBUG3("cicon_offset = 0x%04x", CL_TO_HL(extension->cicon_offset));
    DEBUG3("terminator   = 0x%04x", CL_TO_HL(extension->terminator));
    DEBUG3("=====================");

    /*
    ** FIXME:
    ** What if there are no colour icons? Check for cicon_offset == -1!
    */
    cicons =
      (CICONBLK **)(CL_TO_HL(extension->cicon_offset) + (LONG)rsc);
    DEBUG3 ("Cicons:");
    DEBUG3("=====================");
    
    while((cicons[i] =
           (CICONBLK *)SWAP_LONG((LONG)cicons[i])) != (CICONBLK *)-1)
    {
      DEBUG3("cicons[%d]@0x%04x = 0x%08x",
             i,
             (LONG)&cicons[i] - (LONG)rsc,
             CL_TO_HL(cicons[i]));
      i++;
    }
    
    nr_cicon = i;

    cwalk = (CICONBLK *)&cicons[i + 1];

    DEBUG3("=====================");
    DEBUG3("cwalk = 0x%x, cwalk->monoblk = 0x%x",
           (LONG)cwalk - (LONG)rsc,
           (LONG)&cwalk->monoblk - (LONG)rsc);
    DEBUG3("=====================");

    for(i = 0; i < nr_cicon; i++)
    {
      LONG    monosize;
      CICON * cicwalk;
      WORD    last_res;

      last_res = FALSE;

      cicons[i] = (CICONBLK *)HL_TO_CL(cwalk);
      DEBUG3("cicons[%d] = 0x%x", i, (LONG)cicons[i] - (LONG)rsc);
      
      /* Swap endianess if necessary */
      if(swap_endian)
      {
        WORD * word_ptr;

        for(word_ptr = &cwalk->monoblk.ib_char;
            word_ptr <= &cwalk->monoblk.ib_htext;
            word_ptr++)
        {
          *word_ptr = SWAP_WORD(*word_ptr);
        }
      }


      monosize =
        (((CW_TO_HW(cwalk->monoblk.ib_wicon) + 15) >> 4) << 1) *
        CW_TO_HW(cwalk->monoblk.ib_hicon);
      DEBUG3("monosize = 0x%x", monosize);

      cwalk->monoblk.ib_pdata =
        (WORD *)HL_TO_CL((LONG)cwalk + sizeof(ICONBLK) + sizeof(LONG));
      cwalk->monoblk.ib_pmask =
              (WORD *)HL_TO_CL(CL_TO_HL(cwalk->monoblk.ib_pdata) + monosize);
      cwalk->monoblk.ib_ptext =
        (BYTE *)HL_TO_CL((LONG)CL_TO_HL(cwalk->monoblk.ib_pmask) + monosize);
      cicwalk = (CICON *)((LONG)CL_TO_HL(cwalk->monoblk.ib_ptext) + 12);
      cwalk->mainlist = (CICON *)HL_TO_CL(cicwalk);
      
      DEBUG3("ib_pmask     = 0x%08x",
             CL_TO_HL(cwalk->monoblk.ib_pmask) - (LONG)rsc);
      DEBUG3("ib_pdata     = 0x%08x",
             CL_TO_HL(cwalk->monoblk.ib_pdata) - (LONG)rsc);
      DEBUG3("ib_ptext     = 0x%08x",
             CL_TO_HL(cwalk->monoblk.ib_ptext) - (LONG)rsc);
      DEBUG3("ib_char      = 0x%04x", CW_TO_HW(cwalk->monoblk.ib_char));
      DEBUG3("ib_xchar     = 0x%04x", CW_TO_HW(cwalk->monoblk.ib_xchar));
      DEBUG3("ib_ychar     = 0x%04x", CW_TO_HW(cwalk->monoblk.ib_ychar));
      DEBUG3("ib_xicon     = 0x%04x", CW_TO_HW(cwalk->monoblk.ib_xicon));
      DEBUG3("ib_yicon     = 0x%04x", CW_TO_HW(cwalk->monoblk.ib_yicon));
      DEBUG3("ib_wicon     = 0x%04x", CW_TO_HW(cwalk->monoblk.ib_wicon));
      DEBUG3("ib_hicon     = 0x%04x", CW_TO_HW(cwalk->monoblk.ib_hicon));
      DEBUG3("ib_xtext     = 0x%04x", CW_TO_HW(cwalk->monoblk.ib_xtext));
      DEBUG3("ib_ytext     = 0x%04x", CW_TO_HW(cwalk->monoblk.ib_ytext));
      DEBUG3("ib_wtext     = 0x%04x", CW_TO_HW(cwalk->monoblk.ib_wtext));
      DEBUG3("ib_htext     = 0x%04x", CW_TO_HW(cwalk->monoblk.ib_htext));
      DEBUG3("mainlist     = 0x%08x", CL_TO_HL(cwalk->mainlist) - (LONG)rsc);

      DEBUG3 ("cicwalk = 0x%08x", (LONG)cicwalk - (LONG)rsc);
      /* Go through all of the resolutions for this colour icon */
      while(!last_res)
      {
        LONG planesize;
        MFDB s,d;
        
        /* Swap endianess if necessary */
        if(swap_endian)
        {
          cicwalk->num_planes = SWAP_WORD(cicwalk->num_planes);
          (LONG)cicwalk->next_res = SWAP_LONG((LONG)cicwalk->next_res);
        }

        DEBUG3("num_planes   = 0x%04x", CW_TO_HW(cicwalk->num_planes));
        DEBUG3("next_res     = 0x%04x", CL_TO_HL(cicwalk->next_res));

        DEBUG3 ("Rsrc_do_rcfix: 8: cicwalk = %p cicwalk->num_planes = 0x%x",
                cicwalk, CW_TO_HW(cicwalk->num_planes));
        planesize = monosize * CW_TO_HW(cicwalk->num_planes);

        /* If next_res is equal to 1 there are more resolutions to follow */
        if(CL_TO_HL(cicwalk->next_res) == 1)
        {
          last_res = FALSE;
        }
        else
        {
          last_res = TRUE;
        }

        cicwalk->col_data =
          (WORD *)HL_TO_CL((LONG)cicwalk + sizeof(CICON));
        cicwalk->col_mask =
          (WORD *)HL_TO_CL(CL_TO_HL(cicwalk->col_data) + planesize);
        
        DEBUG3 ("Rsrc_do_rcfix: 9");
        if(cicwalk->sel_data)
        {
          cicwalk->sel_data =
            (WORD *)HL_TO_CL(CL_TO_HL(cicwalk->col_mask) + monosize);
          cicwalk->sel_mask =
            (WORD *)HL_TO_CL(CL_TO_HL(cicwalk->sel_data) + planesize);
          cicwalk->next_res =
            (CICON *)HL_TO_CL(CL_TO_HL(cicwalk->sel_mask) + monosize);
        }
        else
        {
          cicwalk->sel_data = NULL;
          cicwalk->sel_mask = NULL;
          cicwalk->next_res =
            (CICON *)HL_TO_CL(CL_TO_HL(cicwalk->col_mask) + monosize);
        }
        
        DEBUG3 ("Rsrc_do_rcfix: cicwalk = %p", cicwalk);
        (LONG)s.fd_addr = CL_TO_HL(cicwalk->col_data);
        DEBUG3 ("Rsrc_do_rcfix: 10.1");
        s.fd_w = CW_TO_HW(cwalk->monoblk.ib_wicon);
        DEBUG3 ("Rsrc_do_rcfix: 10.2");
        s.fd_h = CW_TO_HW(cwalk->monoblk.ib_hicon);
        DEBUG3 ("Rsrc_do_rcfix: 10.3");
        s.fd_wdwidth = ((CW_TO_HW(cwalk->monoblk.ib_wicon) +15) >> 4);
        DEBUG3 ("Rsrc_do_rcfix: 10.4");
        s.fd_stand = 1;
        DEBUG3 ("Rsrc_do_rcfix: 10.5");
        s.fd_nplanes = CW_TO_HW(cicwalk->num_planes);
        DEBUG3 ("Rsrc_do_rcfix: 10.6");
        
        d = s;
        d.fd_stand = 0;
        
        DEBUG3 ("Rsrc_do_rcfix: 11");
        vr_trnfm(vid,&s,&d);
        DEBUG3 ("Rsrc_do_rcfix: 12");

        if(cicwalk->sel_data)
        {
          (LONG)s.fd_addr = CL_TO_HL(cicwalk->sel_data);
          (LONG)d.fd_addr = CL_TO_HL(cicwalk->sel_data);
          
          vr_trnfm(vid,&s,&d);
        }

        if(last_res == TRUE)
        {
          /*
          ** This is the last resolution for this icon.
          ** Update cwalk to point to the beginning of the next icon.
          */
          cwalk = (CICONBLK *)CL_TO_HL(cicwalk->next_res);
          cicwalk->next_res = NULL;
        }
        else
        {
          cicwalk = (CICON *)CL_TO_HL(cicwalk->next_res);
        }
      }
    }
  }
  
  DEBUG3("=====================");

  DEBUG2 ("rsrc.c: Rsrc_do_rcfix: rsh_nobs = %d (0x%x) swap_endian = %d",
	  CW_TO_HW(rsc->rsh_nobs),
	  CW_TO_HW(rsc->rsh_nobs),
	  swap_endian);
				     
  for (i = 0; i < CW_TO_HW(rsc->rsh_nobs); i++)
  {
    /* Swap endian if necessary */
    if (swap_endian) {
      owalk[i].ob_next = SWAP_WORD(owalk[i].ob_next);
      owalk[i].ob_head = SWAP_WORD(owalk[i].ob_head);
      owalk[i].ob_tail = SWAP_WORD(owalk[i].ob_tail);
      owalk[i].ob_type = SWAP_WORD(owalk[i].ob_type);
      owalk[i].ob_flags = SWAP_WORD(owalk[i].ob_flags);
      owalk[i].ob_state = SWAP_WORD(owalk[i].ob_state);
      owalk[i].ob_spec.index = SWAP_LONG(owalk[i].ob_spec.index);
      owalk[i].ob_x = SWAP_WORD(owalk[i].ob_x);
      owalk[i].ob_y = SWAP_WORD(owalk[i].ob_y);
      owalk[i].ob_width = SWAP_WORD(owalk[i].ob_width);
      owalk[i].ob_height = SWAP_WORD(owalk[i].ob_height);
    }

    DEBUG3("OBJECT[0x%04x]:", i);
    DEBUG3("======================");
    DEBUG3("ob_next   =     0x%04x", CW_TO_HW(owalk[i].ob_next));
    DEBUG3("ob_head   =     0x%04x", CW_TO_HW(owalk[i].ob_head));
    DEBUG3("ob_tail   =     0x%04x", CW_TO_HW(owalk[i].ob_tail));
    DEBUG3("ob_type   =     0x%04x", CW_TO_HW(owalk[i].ob_type));
    DEBUG3("ob_flags  =     0x%04x", CW_TO_HW(owalk[i].ob_flags));
    DEBUG3("ob_state  =     0x%04x", CW_TO_HW(owalk[i].ob_state));
    DEBUG3("ob_spec   = 0x%08x",     CL_TO_HL(owalk[i].ob_spec.index));
    DEBUG3("ob_x      =     0x%04x", CW_TO_HW(owalk[i].ob_x));
    DEBUG3("ob_y      =     0x%04x", CW_TO_HW(owalk[i].ob_y));
    DEBUG3("ob_width  =     0x%04x", CW_TO_HW(owalk[i].ob_width));
    DEBUG3("ob_height =     0x%04x", CW_TO_HW(owalk[i].ob_height));
    DEBUG3("**********************");

    switch (CW_TO_HW(owalk[i].ob_type) & 0xff)
    {
    case        G_BOX:
    case        G_IBOX:
    case        G_BOXCHAR:
      break;
    case        G_TEXT:
    case        G_BOXTEXT:
    case        G_FTEXT:
    case        G_FBOXTEXT:
    case        G_IMAGE:
    case        G_PROGDEF:
    case        G_BUTTON:
    case        G_STRING:
    case        G_TITLE:
    case        G_ICON:
      owalk[i].ob_spec.index =
        HL_TO_CL(CL_TO_HL(owalk[i].ob_spec.index) + (LONG)rsc);
      break;
      
    case        G_CICON:
      owalk[i].ob_spec.index = (LONG)cicons[CL_TO_HL(owalk[i].ob_spec.index)];
      break;
      
    default:
      DEBUG2 ("rsrc.c: Rsrc_do_rcfix: Unsupported type: 0x%04x at %p for object %d",
	      CW_TO_HW(owalk[i].ob_type), &owalk[i].ob_type, i);
    }
    
    fix_object_coordinates(&owalk[i], is_internal);
  }
  DEBUG3 ("Rsrc_do_rcfix: 5");
  
  for (i = 0; i < CW_TO_HW(rsc->rsh_nted); i++)
  {
    if(swap_endian)
    {
      (LONG)tiwalk[i].te_ptext = SWAP_LONG((LONG)tiwalk[i].te_ptext);
      (LONG)tiwalk[i].te_ptmplt = SWAP_LONG((LONG)tiwalk[i].te_ptmplt);
      (LONG)tiwalk[i].te_pvalid = SWAP_LONG((LONG)tiwalk[i].te_pvalid);
      tiwalk[i].te_font = SWAP_WORD(tiwalk[i].te_font);
      tiwalk[i].te_fontid = SWAP_WORD(tiwalk[i].te_fontid);
      tiwalk[i].te_just = SWAP_WORD(tiwalk[i].te_just);
      tiwalk[i].te_color = SWAP_WORD(tiwalk[i].te_color);
      tiwalk[i].te_fontsize = SWAP_WORD(tiwalk[i].te_fontsize);
      tiwalk[i].te_thickness = SWAP_WORD(tiwalk[i].te_thickness);
      tiwalk[i].te_txtlen = SWAP_WORD(tiwalk[i].te_txtlen);
      tiwalk[i].te_tmplen = SWAP_WORD(tiwalk[i].te_tmplen);
    }

    (LONG)tiwalk[i].te_ptext =
      HL_TO_CL(CL_TO_HL(tiwalk[i].te_ptext) + (LONG)rsc);
    (LONG)tiwalk[i].te_ptmplt =
      HL_TO_CL(CL_TO_HL(tiwalk[i].te_ptmplt) + (LONG)rsc);
    (LONG)tiwalk[i].te_pvalid =
      HL_TO_CL(CL_TO_HL(tiwalk[i].te_pvalid) + (LONG)rsc);

  }
  DEBUG3 ("Rsrc_do_rcfix: 6");
  
  for (i = 0; i < CW_TO_HW(rsc->rsh_nib); i++)
  {
    if(swap_endian)
    {
      (LONG)ibwalk[i].ib_pmask = SWAP_LONG((LONG)ibwalk[i].ib_pmask);
      (LONG)ibwalk[i].ib_pdata = SWAP_LONG((LONG)ibwalk[i].ib_pdata);
      (LONG)ibwalk[i].ib_ptext = SWAP_LONG((LONG)ibwalk[i].ib_ptext);
      ibwalk[i].ib_char = SWAP_WORD(ibwalk[i].ib_char);
      ibwalk[i].ib_xchar = SWAP_WORD(ibwalk[i].ib_xchar);
      ibwalk[i].ib_ychar = SWAP_WORD(ibwalk[i].ib_ychar);
      ibwalk[i].ib_xicon = SWAP_WORD(ibwalk[i].ib_xicon);
      ibwalk[i].ib_yicon = SWAP_WORD(ibwalk[i].ib_yicon);
      ibwalk[i].ib_wicon = SWAP_WORD(ibwalk[i].ib_wicon);
      ibwalk[i].ib_hicon = SWAP_WORD(ibwalk[i].ib_hicon);
      ibwalk[i].ib_xtext = SWAP_WORD(ibwalk[i].ib_xtext);
      ibwalk[i].ib_ytext = SWAP_WORD(ibwalk[i].ib_ytext);
      ibwalk[i].ib_wtext = SWAP_WORD(ibwalk[i].ib_wtext);
      ibwalk[i].ib_htext = SWAP_WORD(ibwalk[i].ib_htext);
    }

    (LONG)ibwalk[i].ib_pmask =
      HL_TO_CL(CL_TO_HL(ibwalk[i].ib_pmask) + (LONG)rsc);
    (LONG)ibwalk[i].ib_pdata =
      HL_TO_CL(CL_TO_HL(ibwalk[i].ib_pdata) + (LONG)rsc);
    (LONG)ibwalk[i].ib_ptext =
      HL_TO_CL(CL_TO_HL(ibwalk[i].ib_ptext) + (LONG)rsc);
  }
  DEBUG3 ("Rsrc_do_rcfix: 7");
  
  for (i = 0; i < CW_TO_HW(rsc->rsh_nbb); i++)
  {
    (LONG)bbwalk[i].bi_pdata =
      SWAP_LONG((LONG)bbwalk[i].bi_pdata);
    (LONG)bbwalk[i].bi_pdata =
      HL_TO_CL(CL_TO_HL(bbwalk[i].bi_pdata) + (LONG)rsc);
  }
  DEBUG3 ("Rsrc_do_rcfix: 8");
  
  for (i = 0; i < CW_TO_HW(rsc->rsh_ntree); i++)
  {
    treewalk->l[i] = SWAP_LONG(treewalk->l[i]);
    treewalk->l[i] = HL_TO_CL(CL_TO_HL(treewalk->l[i]) + (LONG)rsc);
  }    
  
  DEBUG3 ("Rsrc_do_rcfix: nstring = %d", CW_TO_HW(rsc->rsh_nstring));
  for (i = 0; i < CW_TO_HW(rsc->rsh_nstring); i++)
  {
    frstrwalk->l[i] = SWAP_LONG(frstrwalk->l[i]);
    frstrwalk->l[i] = HL_TO_CL(CL_TO_HL(frstrwalk->l[i]) + (LONG)rsc);
  }    
  DEBUG3 ("Rsrc_do_rcfix: nimages = %d", CW_TO_HW(rsc->rsh_nimages));
  
  for (i = 0; i < CW_TO_HW(rsc->rsh_nimages); i++)
  {
    frimgwalk->l[i] = SWAP_LONG(frimgwalk->l[i]);
    frimgwalk->l[i] = HL_TO_CL(CL_TO_HL(frimgwalk->l[i]) + (LONG)rsc);
  }    
  return 0;

#undef SWAP_WORD
#undef SWAP_LONG
}


/*
** Exported
*/
void
Rsrc_load (AES_PB *apb)  /*0x006e*/ {
  RSHDR *       rsc;
  GLOBAL_APPL * globals;
  int           is_internal = FALSE;

  CHECK_APID(apb->global->apid);

  globals = get_globals (apb->global->apid);

  rsc = Rsrc_do_load (apb->global->apid,
                      globals->vid,
                      (BYTE *)apb->addr_in[0],
                      is_internal);
  
  if (rsc != NULL)
  {
    apb->global->rscfile =
      (OBJECT **)HL_TO_CL(CW_TO_HW(rsc->rsh_trindex) + (LONG)rsc);
    apb->global->rshdr = (RSHDR *)HL_TO_CL(rsc);
    globals->rshdr = (RSHDR *)rsc;

    apb->int_out[0] = 1;
  }
  else
  {
    apb->int_out[0] = 0;
  }
}


void    Rsrc_free(AES_PB *apb)  /*0x006f*/ {
  /*
  if(apb->global->int_info->rshdr) {
    free(apb->global->int_info->rshdr);
    apb->global->int_info->rshdr = NULL;
                
    apb->int_out[0] = 1;
  }
  else {
    apb->int_out[0] = 0;
  };
  */
}

/*
** Description
** Implementation of rsrc_gaddr()
*/
int
Rsrc_do_gaddr(RSHDR  *  rshdr,
              int       type,
              int       index,
              OBJECT ** addr,
              int       is_internal)
{
  if(rshdr)
  {
    *addr = calculate_element_address(rshdr,
                                      type,
                                      index,
                                      is_internal);
                
    if(*addr)
    {
      return 1;
    }
  }
        
  return 0;
}


/*
** Exported
*/
void
Rsrc_gaddr (AES_PB *apb) {
  GLOBAL_APPL * globals;

  CHECK_APID(apb->global->apid);

  globals = get_globals (apb->global->apid);

  apb->int_out[0] = Rsrc_do_gaddr(globals->rshdr,
                                  apb->int_in[0],
                                  apb->int_in[1],
                                  (OBJECT **)&apb->addr_out[0],
                                  FALSE);
}

void    Rsrc_saddr(AES_PB *apb) /*0x0071*/ {
  apb->int_out[0] = 0;
};

void
Rsrc_obfix(AES_PB * apb) /*0x0072*/ {
  apb->int_out[0] =
    fix_object_coordinates (&((OBJECT *)apb->addr_in[0])[apb->int_in[0]],
                            FALSE);
}


/*
** Exported
*/
void
Rsrc_rcfix (AES_PB *apb) /*0x0073*/ {
  RSHDR *       rsc = (RSHDR *)apb->addr_in[0];
  GLOBAL_APPL * globals;
  int           is_internal = FALSE;
  
  CHECK_APID(apb->global->apid);

  globals = get_globals (apb->global->apid);

  Rsrc_do_rcfix(globals->vid,
                rsc,
                FALSE,
                is_internal);

  apb->global->rscfile =
    (OBJECT **)HL_TO_CL(CW_TO_HW(rsc->rsh_trindex) + (LONG)rsc);
  apb->global->rshdr = (RSHDR *)HL_TO_CL(rsc);
  globals->rshdr = rsc;

  /* Return OK */
  apb->int_out[0] = 1;
}


/*
** Description
** Create copy of resource tree. When the copy isn't needed anymore it
** should be freed using Rsrc_free_tree()
*/
OBJECT *
Rsrc_duplicate(OBJECT * src)
{
  OBJECT *  twalk;
  OBJECT *  newrsc;
  TEDINFO * ti;
  int       num_obj;
  int       num_ti;
  
  twalk   = src;
  num_obj = 0;
  num_ti  = 0;

  do
  {
    num_obj++;
                
    if(OB_TYPE(twalk) == G_TEXT)
    {
      num_ti++;
    }
  } while(!(OB_FLAGS(twalk++) & LASTOB));

  newrsc =
    (OBJECT *)malloc(sizeof(OBJECT) * num_obj + sizeof(TEDINFO) * num_ti);
  ti = (TEDINFO *)(((LONG)newrsc) + sizeof(OBJECT) * num_obj);

  memcpy(newrsc, src, sizeof(OBJECT) * num_obj);

  twalk = newrsc;

  do
  {
    if(OB_TYPE(twalk) == G_TEXT)
    {
      memcpy(ti, (TEDINFO *)OB_SPEC(twalk), sizeof(TEDINFO));
      OB_SPEC_PUT(twalk, ti);
      ti++;
    }
  } while(!(OB_FLAGS(twalk++) & LASTOB));

  return newrsc;
}


/****************************************************************************
 *  Rsrc_free_tree                                                          *
 *   Erase resource tree created with Rsrc_duplicate.                       *
 ****************************************************************************/
void              /*                                                        */
Rsrc_free_tree(   /*                                                        */
OBJECT *src)      /* Tree to erase.                                         */
/****************************************************************************/
{
  free(src);
}
