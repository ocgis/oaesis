/****************************************************************************

 Module
  rsrc.c
  
 Description
  Resource handling functions in oAESis.
  
 Author(s)
        cg (Christer Gustavsson <d2cg@dtek.chalmers.se>)
        
 Revision history
 
  951224 cg
   Added standard header.
 
  951228 cg
   Fixed bug in Rsrc_load; if the resource couldn't be loaded no error
   code was returned.
   
  960413 cg
   Fixed bug in fixobcoord(); changed from signed to unsigned BYTE.

 To be done
   Check if there really are any colour icons in Rsrc_do_rcfix
 
 Copyright notice
  The copyright to the program code herein belongs to the authors. It may
  be freely duplicated and distributed without fee, but not charged for.
 
 ****************************************************************************/

#define DEBUGLEVEL 2

/****************************************************************************
 * Used interfaces                                                          *
 ****************************************************************************/

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

/****************************************************************************
 * Macros                                                                   *
 ****************************************************************************/

#define FO_READ 0

/****************************************************************************
 * Typedefs of module global interest                                       *
 ****************************************************************************/
 
typedef struct {
  LONG filesize;
  LONG cicon_offset;
  LONG terminator;
}RSHD_EXT;

/****************************************************************************
 * Module global variables                                                  *
 ****************************************************************************/

/****************************************************************************
 * Local functions (use static!)                                            *
 ****************************************************************************/

#define CONVCOORD(value, chsize) \
            (((value >> 8) & 0xff) + ((value & 0xff) * chsize))
/*
** Description
** Fix the coordinates of an object
*/
static
WORD
fix_object_coordinates (OBJECT *ob) {
  GLOBAL_COMMON * globals = get_global_common ();

  ob->ob_x      = CONVCOORD(ob->ob_x,  globals->clwidth);
  ob->ob_y      = CONVCOORD(ob->ob_y,  globals->clheight);
  ob->ob_width  = CONVCOORD(ob->ob_width,  globals->clwidth);
  ob->ob_height = CONVCOORD(ob->ob_height,  globals->clheight);

  return  0;
}


/*
** Description
** Load a resource file that is found in the path
**
** 1999-03-14 CG
** 1999-04-24 CG
*/
static
RSHDR *
Rsrc_do_load_mint (WORD   apid,
                   WORD   vid,
                   BYTE * filename) {
  LONG    fnr;
  BYTE    namebuf[200];
  LONG    flen;
        
  RSHDR   *rsc;

  strcpy (namebuf, filename);
  DEBUG2 ("rsrc.c: Rsrc_do_load_mint: trying to load %s", namebuf);
  if (Shel_do_find (apid, namebuf) == SHEL_FIND_ERROR) {
    DB_printf("rsrc.c: loadrsc: Could not find %s",namebuf);
                
    return NULL;
  }

  fnr = Fopen( namebuf, FO_READ);

  if(fnr < 0) {
    DB_printf("rsrc.c: loadrsc: Could not open %s",namebuf);

    return NULL;
  }
        
  flen = Fseek(0,(WORD)fnr,SEEK_END);
        
  rsc = (RSHDR *)Mxalloc(flen,GLOBALMEM);

  if(!rsc) {
    DB_printf("rsrc.c: loadrsc: Could not malloc memory for %s",namebuf);

    return NULL;
  }
        
  Fseek(0,(WORD)fnr,SEEK_SET);    
  Fread((WORD)fnr,flen,rsc);
  Fclose((WORD)fnr);

  Rsrc_do_rcfix(vid,
                rsc,
                FALSE /* FIXME: make better endian check
                         flen != rsc->rsh_rssize */);

  return rsc;
}


/*
** Description
** Load a resource file that is found in the path
**
** 1999-03-14 CG
** 1999-04-24 CG
*/
static
RSHDR *
Rsrc_do_load_unix (WORD   apid,
                   WORD   vid,
                   BYTE * filename) {
  int         fnr;
  BYTE        namebuf[200];
  RSHDR *     rsc;
  struct stat st;

  strcpy( namebuf, filename);
  if (Shel_do_find (apid, namebuf) == SHEL_FIND_ERROR) {
    DB_printf("rsrc.c: loadrsc: Could not find %s",namebuf);
                
    return NULL;
  }

  fnr = open (namebuf, 0);

  if (fnr < 0) {
    DB_printf ("rsrc.c: Rsrc_do_load_unix: Could not open %s", namebuf);

    return NULL;
  }

  if (fstat (fnr, &st) == -1) {
    DB_printf ("rsrc.c: Rsrc_do_load_unix: Could not stat %s", namebuf);
  }

  rsc = (RSHDR *)malloc (st.st_size);

  if (rsc == NULL) {
    DB_printf ("rsrc.c: loadrsc: Could not malloc memory for %s",namebuf);

    return NULL;
  }
        
  if (read (fnr, rsc, st.st_size) == -1) {
    DB_printf ("rsrc.c: loadrsc: Could not read from %s",namebuf);

    return NULL;
  }
  
  close(fnr);


  DEBUG3 ("Calling Rsrc_do_rcfix");
  Rsrc_do_rcfix (vid,
                 rsc,
                 (st.st_size != rsc->rsh_rssize) && (rsc->rsh_vrsn == 0));
  DEBUG3 ("Returned from Rsrc_do_rcfix");
        
  return rsc;
}


/*
** Description
** Load a resource file that is found in the path
*/
static
RSHDR *
Rsrc_do_load(WORD   apid,
             WORD   vid,
             BYTE * filename)
{
  GLOBAL_APPL * globals = get_globals (apid);

  if(globals->path_mode == OAESIS_PATH_MODE_MINT)
  {
    DEBUG2 ("rsrc.c: Rsrc_do_load: Calling Rsrc_do_load_mint");
    return Rsrc_do_load_mint(apid, vid, filename);
  }
  else
  {
    DEBUG2 ("rsrc.c: Rsrc_do_load: Calling Rsrc_do_load_unix");
    return Rsrc_do_load_unix(apid, vid, filename);
  }
}


typedef struct {
  OBJECT *o[1];
} PACKED OARRAY;

/*
** Description
** Calculate the address of an resource object
**
** 1998-10-01 CG
** 1999-04-24 CG
** 1999-06-26 CG
*/
static
void *
calculate_element_address (RSHDR * rsc,
                           WORD    type,
                           WORD    nr) {
  switch (type) {
  case R_TREE:    /* 0x00 */
    return ((OARRAY *)(rsc->rsh_trindex + (LONG)rsc))->o[nr];

  case R_BITBLK:  /* 0x04 */
    return &((BITBLK *)(rsc->rsh_bitblk + (LONG)rsc))[nr];

  case R_STRING:  /* 0x05 */
    return ((BYTE **)(rsc->rsh_frstr + (LONG)rsc))[nr];
                
  case R_FRSTR:   /* 0x0f */
    return &((BYTE **)(rsc->rsh_frstr + (LONG)rsc))[nr];
                        
  case R_FRIMG:   /* 0x10 */
    return &((void **)(rsc->rsh_frimg + (LONG)rsc))[nr];
                        
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
Rsrc_do_rcfix(WORD     vid,
              RSHDR  * rsc,
              WORD     swap_endian) {
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
  CICONBLK ** cicons = NULL;

  /*
  ** Detect previously undetected endian mismatch. This issue has to be
  ** solved in a better way.
  */
  if((rsc->rsh_nobs & 0xff00) && ((rsc->rsh_nobs & 0xff) == 0))
  {
    swap_endian = TRUE;
  }

  DEBUG3 ("Rsrc_do_rcfix: 1");
  /* Swap words if necessary */
  if (swap_endian) {
    DEBUG3 ("Rsrc_do_rcfix: Swapping words in header");
    for (iwalk = (UWORD *)rsc; iwalk <= (UWORD *)&rsc->rsh_rssize; iwalk++) {
      *iwalk = SWAP_WORD(*iwalk);
    }
  }

  DEBUG3 ("Rsrc_do_rcfix: 2");
  /* Initialize pointers */
  owalk = (OBJECT *)((LONG)rsc->rsh_object + (LONG)rsc); 
  tiwalk = (TEDINFO *)((LONG)rsc->rsh_tedinfo + (LONG)rsc);
  ibwalk = (ICONBLK *)((LONG)rsc->rsh_iconblk + (LONG)rsc);
  bbwalk = (BITBLK *)((LONG)rsc->rsh_bitblk + (LONG)rsc);
  treewalk = (LARRAY *)((LONG)rsc->rsh_trindex + (LONG)rsc);
  frstrwalk = (LARRAY *)((LONG)rsc->rsh_frstr + (LONG)rsc);
  DEBUG3 ("Rsrc_do_rcfix: 3");

  if (rsc->rsh_vrsn & 0x4) {
    RSHD_EXT *extension = (RSHD_EXT *)((LONG)rsc + (LONG)rsc->rsh_rssize);
    CICONBLK *cwalk;
  
    WORD     nr_cicon;
    WORD     i = 0;

    /* Swap endianess on extension array if necessary */
    if (swap_endian) {
      extension->filesize = SWAP_LONG(extension->filesize);
      extension->cicon_offset = SWAP_LONG(extension->cicon_offset);
      extension->terminator = SWAP_LONG(extension->terminator);
    }

    DEBUG3 ("Rsrc_do_rcfix: 4: extension = %p rsc = %p", extension, rsc);
    /* What if there are no colour icons? Check for cicon_offset == -1! */
    cicons = (CICONBLK **)((LONG)extension->cicon_offset + (LONG)rsc);
    DEBUG3 ("Rsrc_do_rcfix: 4.1: cicons = %p cicon_offset = 0x%x",
            cicons, extension->cicon_offset);
    
    while((cicons[i] =
           (CICONBLK *)SWAP_LONG((LONG)cicons[i])) != (CICONBLK *)-1)
    {
      DEBUG3("cicons[%d]@0x%x", i, (LONG)&cicons[i] - (LONG)rsc);
      i++;
    }
    
    nr_cicon = i;

    cwalk = (CICONBLK *)&cicons[i + 1];

    DEBUG3("cwalk = 0x%x, cwalk->monoblk = 0x%x",
           (LONG)cwalk - (LONG)rsc,
           (LONG)&cwalk->monoblk - (LONG)rsc);

    for(i = 0; i < nr_cicon; i++) {
      LONG monosize;
      CICON *cicwalk;
      WORD last_res = FALSE;

      DEBUG3 ("Rsrc_do_rcfix: 6");
      cicons[i] = cwalk;
      
      /* Swap endianess if necessary */
      if(swap_endian)
      {
        WORD * word_ptr;

        for(word_ptr = &cwalk->monoblk.ib_char;
            word_ptr <= &cwalk->monoblk.ib_htext;
            word_ptr++)
        {
          *word_ptr = SWAP_WORD(*word_ptr);
          DEBUG3("iconblk... 0x%4x@%p", *word_ptr, word_ptr);
        }
      }

      monosize =
        (((cwalk->monoblk.ib_wicon + 15) >> 4) << 1) * cwalk->monoblk.ib_hicon;
      DEBUG3("monosize = 0x%x", monosize);

      cwalk->monoblk.ib_pdata = (WORD *)((LONG)cwalk + sizeof(ICONBLK) + sizeof(LONG));
      cwalk->monoblk.ib_pmask = (WORD *)((LONG)cwalk->monoblk.ib_pdata + monosize);
      cwalk->monoblk.ib_ptext = (BYTE *)((LONG)cwalk->monoblk.ib_pmask + monosize);
      cicwalk = (CICON *)((LONG)cwalk->monoblk.ib_ptext + 12);
      cwalk->mainlist = cicwalk;
      
      DEBUG3 ("Rsrc_do_rcfix: 7: cicwalk = %p 0x%x", cicwalk,
              (LONG)cicwalk - (LONG)rsc);
      /* Go through all of the resolutions for this colour icon */
      while (!last_res) {
        LONG planesize;
        MFDB s,d;
        
        /* Swap endianess if necessary */
        if(swap_endian)
        {
          cicwalk->num_planes = SWAP_WORD(cicwalk->num_planes);
          (LONG)cicwalk->next_res = SWAP_LONG((LONG)cicwalk->next_res);
        }

        DEBUG3 ("Rsrc_do_rcfix: 8: cicwalk = %p cicwalk->num_planes = 0x%x",
                cicwalk, cicwalk->num_planes);
        planesize = monosize * cicwalk->num_planes;

        /* If next_res is equal to 1 there are more resolutions to follow */
        if ((LONG)cicwalk->next_res == 1) {
          last_res = FALSE;
        } else {
          last_res = TRUE;
        }

        cicwalk->col_data = (WORD *)((LONG)cicwalk + sizeof(CICON));
        cicwalk->col_mask = (WORD *)((LONG)cicwalk->col_data + planesize);
        
        DEBUG3 ("Rsrc_do_rcfix: 9");
        if(cicwalk->sel_data) {
          cicwalk->sel_data = (WORD *)((LONG)cicwalk->col_mask + monosize);
          cicwalk->sel_mask = (WORD *)((LONG)cicwalk->sel_data + planesize);
          cicwalk->next_res = (CICON *)((LONG)cicwalk->sel_mask + monosize);                            
        }
        else
        {
          cicwalk->sel_data = NULL;
          cicwalk->sel_mask = NULL;
          cicwalk->next_res = (CICON *)((LONG)cicwalk->col_mask + monosize);                            
        }
        
        DEBUG3 ("Rsrc_do_rcfix: 10");
        (LONG)s.fd_addr = (LONG)cicwalk->col_data;
        s.fd_w = cwalk->monoblk.ib_wicon;
        s.fd_h = cwalk->monoblk.ib_hicon;
        s.fd_wdwidth = ((cwalk->monoblk.ib_wicon +15) >> 4);
        s.fd_stand = 1;
        s.fd_nplanes = cicwalk->num_planes;
        
        d = s;
        d.fd_stand = 0;
        
        DEBUG3 ("Rsrc_do_rcfix: 11");
        vr_trnfm(vid,&s,&d);
        DEBUG3 ("Rsrc_do_rcfix: 12");

        if(cicwalk->sel_data) {
          (LONG)s.fd_addr = (LONG)cicwalk->sel_data;
          (LONG)d.fd_addr = (LONG)cicwalk->sel_data;
          
          vr_trnfm(vid,&s,&d);
        }

        if(last_res == TRUE) {
          /*
          ** This is the last resolution for this icon.
          ** Update cwalk to point to the beginning of the next icon.
          */
          cwalk = (CICONBLK *)cicwalk->next_res;
          cicwalk->next_res = NULL;
        } else {
          cicwalk = cicwalk->next_res;
        }
      }
    }
  }
  
  DEBUG2 ("rsrc.c: Rsrc_do_rcfix: rsh_nobs = %d (0x%x) swap_endian = %d",
	  rsc->rsh_nobs,
	  rsc->rsh_nobs,
	  swap_endian);
				     
  for (i = 0; i < rsc->rsh_nobs; i++)
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

    switch (owalk[i].ob_type & 0xff)
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
      owalk[i].ob_spec.index = owalk[i].ob_spec.index + (LONG)rsc;
      break;
      
    case        G_CICON:
      owalk[i].ob_spec.index = (LONG)cicons[owalk[i].ob_spec.index];
      break;
      
    default:
      DEBUG2 ("rsrc.c: Rsrc_do_rcfix: Unsupported type: 0x%04x at %p for object %d",
	      owalk[i].ob_type, &owalk[i].ob_type, i);
    }
    
    fix_object_coordinates (&owalk[i]);
  }
  DEBUG3 ("Rsrc_do_rcfix: 5");
  
  for (i = 0; i < rsc->rsh_nted; i++) {
    (LONG)tiwalk[i].te_ptext =
      SWAP_LONG((LONG)tiwalk[i].te_ptext) + (LONG)rsc;
    (LONG)tiwalk[i].te_ptmplt =
      SWAP_LONG((LONG)tiwalk[i].te_ptmplt) + (LONG)rsc;
    (LONG)tiwalk[i].te_pvalid =
      SWAP_LONG((LONG)tiwalk[i].te_pvalid) + (LONG)rsc;

    if(swap_endian)
    {
      tiwalk[i].te_font = SWAP_WORD(tiwalk[i].te_font);
      tiwalk[i].te_fontid = SWAP_WORD(tiwalk[i].te_fontid);
      tiwalk[i].te_just = SWAP_WORD(tiwalk[i].te_just);
      /* FIXME: Check if te_color should be swapped */
      tiwalk[i].te_fontsize = SWAP_WORD(tiwalk[i].te_fontsize);
      tiwalk[i].te_thickness = SWAP_WORD(tiwalk[i].te_thickness);
      tiwalk[i].te_txtlen = SWAP_WORD(tiwalk[i].te_txtlen);
      tiwalk[i].te_tmplen = SWAP_WORD(tiwalk[i].te_tmplen);
    }
  }
  DEBUG3 ("Rsrc_do_rcfix: 6");
  
  for (i = 0; i < rsc->rsh_nib; i++) {
    (LONG)ibwalk[i].ib_pmask = SWAP_LONG((LONG)ibwalk[i].ib_pmask) + (LONG)rsc;
    (LONG)ibwalk[i].ib_pdata = SWAP_LONG((LONG)ibwalk[i].ib_pdata) + (LONG)rsc;
    (LONG)ibwalk[i].ib_ptext = SWAP_LONG((LONG)ibwalk[i].ib_ptext) + (LONG)rsc;
  }
  DEBUG3 ("Rsrc_do_rcfix: 7");
  
  for (i = 0; i < rsc->rsh_nbb; i++) {
    (LONG)bbwalk[i].bi_pdata = SWAP_LONG((LONG)bbwalk[i].bi_pdata) + (LONG)rsc;
  }
  DEBUG3 ("Rsrc_do_rcfix: 8");
  
  for (i = 0; i < rsc->rsh_ntree; i++) {
    treewalk->l[i] = SWAP_LONG(treewalk->l[i]) + (LONG)rsc;
  }    
  
  DEBUG3 ("Rsrc_do_rcfix: 9");
  for (i = 0; i < rsc->rsh_nstring; i++) {
    frstrwalk->l[i] = SWAP_LONG(frstrwalk->l[i]) + (LONG)rsc;
  }    
  DEBUG3 ("Rsrc_do_rcfix: 10");
  
  return 0;

#undef SWAP_WORD
#undef SWAP_LONG
}


/*
** Exported
**
** 1998-12-20 CG
** 1999-03-14 CG
*/
void
Rsrc_load (AES_PB *apb)  /*0x006e*/ {
  RSHDR *       rsc;
  GLOBAL_APPL * globals = get_globals (apb->global->apid);

  rsc = Rsrc_do_load (apb->global->apid,
                      globals->vid,
                      (BYTE *)apb->addr_in[0]);
  
  if (rsc != NULL) {
    apb->global->rscfile = (OBJECT **)((LONG)rsc->rsh_trindex + (LONG)rsc);
    apb->global->rshdr = (RSHDR *)rsc;
    globals->rshdr = (RSHDR *)rsc;

    apb->int_out[0] = 1;
  } else {
    apb->int_out[0] = 0;
  }
}


void    Rsrc_free(AES_PB *apb)  /*0x006f*/ {
  /*
  if(apb->global->int_info->rshdr) {
    Mfree(apb->global->int_info->rshdr);
    apb->global->int_info->rshdr = NULL;
                
    apb->int_out[0] = 1;
  }
  else {
    apb->int_out[0] = 0;
  };
  */
}

/****************************************************************************
 *  Rsrc_do_gaddr                                                           *
 *   Implementation of rsrc_gaddr().                                        *
 ****************************************************************************/
WORD              /* 0 if ok or != 0 if error.                              */
Rsrc_do_gaddr(    /*                                                        */
RSHDR  *  rshdr,  /* Resource structure to search.                          */
WORD      type,   /* Type of object.                                        */
WORD      index,  /* Index of object.                                       */
OBJECT ** addr)   /* Object address.                                        */
/****************************************************************************/
{
  if (rshdr) {
    *addr = calculate_element_address (rshdr, type, index);
                
    if (*addr) {
      return 1;
    };
  };
        
  return 0;
}


/*
** Exported
**
** 1999-01-06 CG
*/
void
Rsrc_gaddr (AES_PB *apb) {
  GLOBAL_APPL * globals = get_globals (apb->global->apid);

  apb->int_out[0] = Rsrc_do_gaddr (globals->rshdr,
                                   apb->int_in[0],
                                   apb->int_in[1],
                                   (OBJECT **)&apb->addr_out[0]);
}

void    Rsrc_saddr(AES_PB *apb) /*0x0071*/ {
  apb->int_out[0] = 0;
};

void
Rsrc_obfix(AES_PB * apb) /*0x0072*/ {
  apb->int_out[0] =
    fix_object_coordinates (&((OBJECT *)apb->addr_in[0])[apb->int_in[0]]);
}


/*
** Exported
**
** 1998-12-20 CG
** 1999-01-06 CG
** 1999-04-24 CG
*/
void
Rsrc_rcfix (AES_PB *apb) /*0x0073*/ {
  RSHDR *       rsc = (RSHDR *)apb->addr_in[0];
  GLOBAL_APPL * globals = get_globals (apb->global->apid);
  
  Rsrc_do_rcfix (globals->vid,
                 rsc,
                 FALSE);
        
  globals->rscfile = (OBJECT **)((LONG)rsc->rsh_trindex + (LONG)rsc);
  globals->rshdr = rsc;

  /* Return OK */
  apb->int_out[0] = 1;
}

/****************************************************************************
 *  Rsrc_duplicate                                                          *
 *   Create copy of resource tree. When the copy isn't needed anymore it    *
 *   should be freed using Rsrc_free_tree().                                *
 ****************************************************************************/
OBJECT *          /* New resource tree, or NULL.                            */
Rsrc_duplicate(   /*                                                        */
OBJECT *src)      /* Original resource tree.                                */
/****************************************************************************/
{
  OBJECT *twalk = src,*newrsc;
  TEDINFO *ti;
  WORD    num_obj = 0;
  WORD    num_ti = 0;
        
  do
  {
    num_obj++;
                
    switch(twalk->ob_type)
    {
    case G_TEXT:
      num_ti++;
    }
  }while(!(OB_FLAGS(twalk++) & LASTOB));

  newrsc =
    (OBJECT *)malloc(sizeof(OBJECT) * num_obj + sizeof(TEDINFO) * num_ti);
  ti = (TEDINFO *)(((LONG)newrsc) + sizeof(OBJECT) * num_obj);

  memcpy(newrsc,src,sizeof(OBJECT) * num_obj);

  twalk = newrsc;

  do {
    switch(twalk->ob_type) {
    case G_TEXT:
      memcpy(ti,twalk->ob_spec.tedinfo,sizeof(TEDINFO));
      twalk->ob_spec.tedinfo = ti;
      ti++;
    };
  }while(!(OB_FLAGS(twalk++) & LASTOB));

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
