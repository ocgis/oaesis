/****************************************************************************

 Module
  appl.c
  
 Description
  Application handling routines in oAESis.
  
 Author(s)
 	cg (Christer Gustavsson <d2cg@dtek.chalmers.se>)
 	kkp (Klaus Pedersen) <kkp@gamma.dou.dk>)

 Revision history
 
  960101 cg
   Added standard header.
   Basic version of appl_getinfo implemented.

  960102 cg
   AES_WINDOW mode of appl_getinfo implemented.

  960115 cg
   Appl_get_vid() implemented.
      
  960228 kkp
   Changed the behaviour of TOPAPPL

  980622 cg
   Added check for macros defined by autoconf.

 Copyright notice
  The copyright to the program code herein belongs to the authors. It may
  be freely duplicated and distributed without fee, but not charged for.
 
 ****************************************************************************/

#define DEBUGLEVEL 0

/****************************************************************************
 * Used interfaces                                                          *
 ****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef MINT_TARGET
#include <ioctl.h>
#endif

#ifdef HAVE_ALLOC_H
#include <alloc.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_MINTBIND_H
#include <mintbind.h>
#endif

#include <netinet/in.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef HAVE_SUPPORT_H
#include <support.h>
#endif

#include <unistd.h>
#include <vdibind.h>

#include "aesbind.h"
#include "appl.h"
#include "debug.h"
#include "lib_comm.h"
#include "lib_global.h"
#include "lib_mem.h"
#include "srv_put.h"
#include "srv_interface.h"
#include "types.h"

#ifdef MINT_TARGET
#include "mintdefs.h"
#endif

#ifndef MINT_TARGET
extern char * program_invocation_short_name;
#endif

/* From ovdisis */
extern void (*vdi_handler)(VDIPB *);

/****************************************************************************
 * Local functions (use static!)                                            *
 ****************************************************************************/

/*
** Description
** Implementation of appl_read ()
**
** 1998-10-04 CG
** 1998-12-19 CG
*/
static
WORD
Appl_do_read (WORD   apid,
              WORD   length,
              void * m) {
  /* FIXME do call to server instead
  if((apid == APR_NOWAIT) && (Finstat(msgpipe) < length)) {
    return 0;
  };
	
  if(Fread(msgpipe,length,m) < 0) {
    return 0;
  };
		
  return 1;
  */

  DB_printf ("!!Implement Appl_do_read in appl.c");

  return 0;
}

/****************************************************************************
 * Public functions                                                         *
 ****************************************************************************/

#ifdef TUNNEL_VDI_CALLS

#define WORDS_TO_POINTER(m,l) ((void *)(((unsigned long)(m)<<16) | ((unsigned long)(l&0xffff))))
#define LO_WORD(ptr) ((LONG)ptr & 0xffff)
#define HI_WORD(ptr) (((LONG)ptr >> 16) & 0xffff)

/* Description
** Copy MFDB with network order conversion
**
** 1999-05-23 CG
*/
static
void
copy_mfdb (MFDB * dst,
           MFDB * src,
           void * address)
{
  dst->fd_addr = (void *)htonl(address);
  dst->fd_w = htons (src->fd_w);
  dst->fd_h = htons (src->fd_h);
  dst->fd_wdwidth = htons (src->fd_wdwidth);
  dst->fd_stand = htons (src->fd_stand);
  dst->fd_nplanes = htons (src->fd_nplanes);
  dst->fd_r1 = htons (src->fd_r1);
  dst->fd_r2 = htons (src->fd_r2);
  dst->fd_r3 = htons (src->fd_r3);
}


/*
** Description
** Calculate size of block specified by MFDB
*/
static
LONG
calculate_block_size(MFDB * block)
{
  return block->fd_wdwidth * block->fd_h * block->fd_nplanes * sizeof(WORD);
}


/*
** Description
** Check if vdi_call uses MFDB
*/
static
int
is_blit_call(int call_num)
{
  switch(call_num)
  {
  case 109:  /* vro_cpyfm */
  case 110:  /* vr_trnfm  */
  case 121:  /* vrt_cpyfm */
    return TRUE;
  
  default:
    return FALSE;
  }
}


/*
** Description
** Tunnel a vdi call to the oaesis server
*/
static
void
vdi_tunnel(VDIPB * vpb)
{
  C_VDI_CALL   par_data;
  C_VDI_CALL * par = &par_data;
  R_VDI_CALL   ret_data;
  R_VDI_CALL * ret = &ret_data;
  int        i;
  int        j;
  int        apid = 0;
  MFDB *        src = NULL;
  MFDB *        dst = NULL;
  MEMORY_HANDLE src_mem = NULL;
  MEMORY_HANDLE dst_mem = NULL;


  DEBUG3("vdi_tunnel: call %d", vpb->contrl[0]);

  if(is_blit_call(vpb->contrl[0]))
  {
    src = (MFDB *)WORDS_TO_POINTER(vpb->contrl[7],vpb->contrl[8]);
    dst = (MFDB *)WORDS_TO_POINTER(vpb->contrl[9],vpb->contrl[10]);

    if(src->fd_addr != NULL)
    {
      src_mem = Mem_register(src->fd_addr,
                             calculate_block_size(src),
                             MEMORY_READ);
    }

    if(dst->fd_addr != NULL)
    {
      dst_mem = Mem_register(dst->fd_addr,
                             calculate_block_size(dst),
                             MEMORY_READWRITE);
    }
  }

  /* Copy contrl array */
  for (i = 0; i < 15; i++) {
    par->contrl[i] = vpb->contrl[i];
  }

  /* Copy ptsin parameters */
  for (i = 0, j = 0; i < (vpb->contrl[1] * 2); i++, j++) {
    par->inpar[j] = vpb->ptsin[i];
  }

  /* Copy intin parameters */
  for (i = 0; i < vpb->contrl[3]; i++, j++) {
    par->inpar[j] = vpb->intin[i];
  }

  PUT_C_ALL_W(VDI_CALL,par,C_ALL_WORDS + 15 + j);

  /* Copy MFDBs when available */
  if(is_blit_call(vpb->contrl[0]))
  {
    copy_mfdb((MFDB *)&par->inpar[j],
              src,
              Mem_server_addr(src_mem));
    j += sizeof (MFDB) / 2;

    copy_mfdb((MFDB *)&par->inpar[j],
              dst,
              Mem_server_addr(dst_mem));
    j += sizeof (MFDB) / 2;
  }

  /* Pass the call to the server */
  CLIENT_SEND_RECV(par,
                   sizeof(C_ALL) +
                   sizeof(WORD) * (15 + j),
                   ret,
                   sizeof(R_VDI_CALL) + sizeof(WORD));

  /* Copy contrl array */
  for (i = 0; i < 15; i++) {
    vpb->contrl[i] = ret->contrl[i];
  }

  /* Copy ptsout parameters */
  for (i = 0, j = 0; i < (vpb->contrl[2] * 2); i++, j++) {
    vpb->ptsout[i] = ret->outpar[j];
  }

  /* Copy intout parameters */
  for (i = 0; i < vpb->contrl[4]; i++, j++) {
    vpb->intout[i] = ret->outpar[j];
  }

  if(is_blit_call(vpb->contrl[0]))
  {
    if(src_mem != MEMORY_HANDLE_NIL)
    {
      Mem_unregister(src_mem);
    }

    if(dst_mem != MEMORY_HANDLE_NIL)
    {
      Mem_unregister(dst_mem);
    }
  }

  DEBUG3("vdi_tunnel: completed call %d", vpb->contrl[0]);
}
#endif /* TUNNEL_VDI_CALLS */

/* 0x000a appl_init */

#ifdef MINT_TARGET
/*
** Description
** Get program name and command line
**
** 1999-08-22 CG
*/
static
void
get_loadinfo (WORD   pid,
	      WORD   fnamelen,
	      BYTE * cmdlin,
	      BYTE * fname)
{
  BYTE pname[30];
  _DTA *olddta,newdta;
	
  olddta = Fgetdta();
  Fsetdta(&newdta);
	
  sprintf(pname,"u:\\proc\\*.%03d",pid);
  if(Fsfirst(pname,0) == 0) {
    LONG fd;
		
    sprintf(pname,"u:\\proc\\%s",newdta.dta_name);
		
    if((fd = Fopen(pname,0)) >= 0) {
      struct __ploadinfo li;
			
      li.fnamelen = fnamelen;
      li.cmdlin = cmdlin;
      li.fname = fname;
			
      Fcntl((WORD)fd,&li,PLOADINFO);
      Fclose((WORD)fd);
    }
  }
	
  Fsetdta(olddta);
}


/*
** Description
** Get name of process unde MiNT
**
** 1999-08-24 CG
*/
static
void
get_process_name (int    pid,
		  char * name,
		  int    max_length) {
  char   command_line[256];
  char   process_name[256];
  char * short_name;

  get_loadinfo (pid,
		255,
		command_line,
		process_name);

  short_name = strrchr (process_name, '\\');
  if (short_name == NULL) {
    short_name = process_name;
  } else {
    short_name++;
  }

  strncpy (name, short_name, max_length);
}

#endif


/*
** Exported
*/
WORD
Appl_do_reserve(WORD apid,
                WORD type,
                WORD pid)
{
  C_APPL_RESERVE par;
  R_APPL_RESERVE ret;

  PUT_C_ALL(APPL_RESERVE, &par);

  par.type = type;
  par.pid  = pid;

  CLIENT_SEND_RECV(&par,
                   sizeof (C_APPL_RESERVE),
                   &ret,
                   sizeof (R_APPL_RESERVE));

  return ret.common.retval; /* The allocated ap_id or 0 */
}


/*
** Exported
*/
WORD
Appl_do_init (GLOBAL_ARRAY * global) {
  C_APPL_INIT   par;
  R_APPL_INIT   ret;

  WORD          apid = -1;

#ifdef MINT_TARGET
  Psemaphore(SEM_LOCK, SHEL_WRITE_LOCK, -1);
  Psemaphore(SEM_UNLOCK, SHEL_WRITE_LOCK, 0);
#endif

  DEBUG3 ("appl.c: Appl_do_init");
  /* Open connection to server */
  if (Client_open () == -1) {
    return -1;
  }

#ifdef TUNNEL_VDI_CALLS
  /* Tunnel VDI calls through oaesis' connection with the server */
  vdi_handler = vdi_tunnel;
#endif /* TUNNEL_VDI_CALLS */

  PUT_C_ALL(APPL_INIT, &par);

#ifdef MINT_TARGET
  get_process_name (par.common.pid,
		    par.appl_name,
		    sizeof (par.appl_name) - 1);
#else
  DEBUG3 ("appl.c: Appl_do_init: program_invocation_short_name %s",
          program_invocation_short_name);
  strncpy (par.appl_name,
           program_invocation_short_name,
           sizeof (par.appl_name) - 1);
#endif
  par.appl_name[sizeof (par.appl_name) - 1] = 0;

  CLIENT_SEND_RECV(&par,
                   sizeof (C_APPL_INIT),
                   &ret,
                   sizeof (R_APPL_INIT));

  global->apid = ret.apid;	
  global->version = 0x0410;
  global->numapps = -1;
  global->appglobal = 0L;
  global->rscfile = 0L;
  global->rshdr = 0L;
  global->resvd1 = 0;
  global->resvd2 = 0;
  global->int_info = 0L;
  global->maxchar = 0;
  global->minchar = 0;

  if(global->apid >= 0) {
    init_global_appl (global->apid, ret.physical_vdi_id, par.appl_name);
#ifndef MINT_TARGET
    {
      GLOBAL_APPL * globals_appl = get_globals (global->apid);

      init_global (globals_appl->vid);
    }
#endif

    return global->apid;
  } else {
    return -1;
  }
}


void
Appl_init (AES_PB *apb) {
  apb->int_out[0] = Appl_do_init(apb->global);
}


/*
** Exported
**
** 1998-12-19 CG
*/
void
Appl_read (AES_PB *apb) {
  apb->int_out[0] = Appl_do_read(apb->int_in[0],
                                 apb->int_in[1],
                                 (void *)apb->addr_in[0]);
}


/*
** Exported
*/
WORD
Appl_do_write (WORD   apid,
               WORD   addressee,
               WORD   length,
               void * m) {
  C_APPL_WRITE par;
  R_APPL_WRITE ret;

  PUT_C_ALL(APPL_WRITE, &par);

  par.addressee = apid;
  par.length = length;
  par.is_reference = FALSE;
  par.msg.event = *(COMMSG *)m; /* FIXME handle msgs larger than 16 bytes */
	
  CLIENT_SEND_RECV(&par,
                   sizeof (C_APPL_WRITE),
                   &ret,
                   sizeof (R_APPL_WRITE));
  
  return ret.common.retval;
}


/*
** Exported
**
** 1998-12-20 CG
*/
void
Appl_write (AES_PB *apb) {
  apb->int_out[0] =	
    Appl_do_write (apb->global->apid,
                   apb->int_in[0],apb->int_in[1],
                   (void *)apb->addr_in[0]);
}



/*
** Description
** Implementation of appl_find ()
*/
static
inline
WORD
Appl_do_find(WORD   apid,
             BYTE * fname)
{
  C_APPL_FIND par;
  R_APPL_FIND ret;

  PUT_C_ALL(APPL_FIND,&par);

  switch (((LONG)fname >> 16) & 0xffff) {
  case 0xffff :
    par.mode = APPL_FIND_PID_TO_APID;
    par.data.pid = (LONG)fname & 0xffff;
    break;

  case 0xfffe :
    par.mode = APPL_FIND_APID_TO_PID;
    par.data.apid = (LONG)fname & 0xffff;
    break;

  default :
    par.mode = APPL_FIND_NAME_TO_APID;
    strcpy (par.data.name, fname);
    break;
  }

  CLIENT_SEND_RECV(&par,
                   sizeof (C_APPL_FIND),
                   &ret,
                   sizeof (R_APPL_FIND));

  return ret.common.retval;
}


/*
** Exported
** 0x000d appl_find ()
**
** 1999-06-10 CG
*/
void
Appl_find (AES_PB * apb)
{
  if(apb->addr_in[0] == 0) {
    /* return our own pid */
    apb->int_out[0] = apb->global->apid;
  } else {
    apb->int_out[0] = Appl_do_find (apb->global->apid,
                                    (BYTE *)apb->addr_in[0]);
  }
}


/*
** Exported
** Implementation of appl_search ()
*/
WORD
Appl_do_search (WORD   apid,
                WORD   mode,
                BYTE * name,
                WORD * type,
                WORD * ap_id) {
  C_APPL_SEARCH par;
  R_APPL_SEARCH ret;

  PUT_C_ALL(APPL_SEARCH, &par);

  par.mode = mode; /* FIXME: Needed? */

  CLIENT_SEND_RECV(&par,
                   sizeof (C_APPL_SEARCH),
                   &ret,
                   sizeof (R_APPL_SEARCH));

  strcpy (name, ret.info.name);
  *type = ret.info.type;
  *ap_id = ret.info.ap_id;

  return ret.common.retval;
}


/*
** Exported
** 0x0012 appl_search ()
**
** 1999-04-11 CG
*/
void
Appl_search (AES_PB * apb) {
  apb->int_out[ 0] = Appl_do_search (apb->global->apid,
                                     apb->int_in[0], 
                                     (BYTE *)apb->addr_in[0],
                                     &apb->int_out[1],
                                     &apb->int_out[2]);
}


/* 0x0013 appl_exit */

/*
** Exported
*/
WORD
Appl_do_exit (WORD apid) {
  C_APPL_EXIT   par;
  R_APPL_EXIT   ret;
  GLOBAL_APPL * globals = get_globals (apid);

  PUT_C_ALL(APPL_EXIT, &par);

  CLIENT_SEND_RECV(&par,
                   sizeof (C_APPL_EXIT),
                   &ret,
                   sizeof (R_APPL_EXIT));
  
  v_clsvwk (globals->vid);

  return ret.common.retval;
}


/*
** Exported
**
** 1998-12-28 CG
*/
void
Appl_exit (AES_PB * apb) {
  apb->int_out[0] = Appl_do_exit (apb->global->apid);
}

/****************************************************************************
 * Appl_getinfo                                                             *
 *  0x0082 appl_getinfo().                                                  *
 ****************************************************************************/
void                      /*                                                */
Appl_getinfo(AES_PB *apb) /* AES parameter block.                           */
     /***********************************************************************/
{
  GLOBAL_APPL * globals;

  apb->int_out[0] = 1; /* default: return OK ( ret != 0) */

  switch(apb->int_in[0]) {
  case AES_LARGEFONT: /* 0 */
    globals = get_globals (apb->global->apid);
    apb->int_out[ 1] = globals->common->fnt_regul_sz;
    apb->int_out[ 2] = globals->common->fnt_regul_id;
    apb->int_out[ 3] = 0;
    break;
  case AES_SMALLFONT: /* 1 */
    globals = get_globals (apb->global->apid);
    apb->int_out[ 1] = globals->common->fnt_small_sz;
    apb->int_out[ 2] = globals->common->fnt_small_id;
    apb->int_out[ 3] = 0; /* system font */
    break;
  case AES_SYSTEM:    /* 2 */
    apb->int_out[ 1] = 0;  /* rez nr. */
    apb->int_out[ 2] = 16; /* n_colors */
    apb->int_out[ 3] = 1;  /* new .RCS (?) */
    break;
  case AES_LANGUAGE:  /* 3 */
    apb->int_out[ 1] = 0;  /* english */	
    break;
    /* We don't get these unless vAES>=4.1 - hopefully... */
  case AES_PROCESS:   /* 4 */
    apb->int_out[ 1] = 1;  /* real multitask */
    apb->int_out[ 2] = 1;  /* appl_find can mintid->aespid */
    apb->int_out[ 3] = 0;  /* no appl_search */
    apb->int_out[ 4] = 0;  /* no rsrc_rsfix */
    break;
  case AES_PCGEM:     /* 5 */
    apb->int_out[ 1] = 0;  /* objc_xfind */
    apb->int_out[ 2] = 0;  /* 0 */
    apb->int_out[ 3] = 0;  /* menu_click */
    apb->int_out[ 4] = 0;  /* shel_r/wdef */
    break;
  case AES_INQUIRE:   /* 6 */
    apb->int_out[ 1] = 1;  /* appl_read(-1) */
    apb->int_out[ 2] = 0;  /* shel_get(-1) */
    apb->int_out[ 3] = 1;  /* menu_bar(-1) */
    apb->int_out[ 4] = 0;  /* menu_bar(MENU_INSTL) */
    break;
  case AES_MOUSE:     /* 8 */
    apb->int_out[1] = 1;  /* graf_mouse modes 258-260 supported */
    apb->int_out[2] = 0;  /* mouse form maintained by OS */

  case AES_MENU:      /* 9 */
    apb->int_out[ 1] = 0;  /* MultiTOS style submenus */
    apb->int_out[ 2] = 0;  /* MultiTOS style popup menus */
    apb->int_out[ 3] = 0;  /* MultiTOS style scrollable menus */
    apb->int_out[ 4] = 0;  /* words 5/6/7 in MN_SELECTED message give extra info */
    break;
 
  case AES_SHELL:   /* 10 */
    apb->int_out[1] = SWM_AESMSG;
    apb->int_out[2] = 0;
    apb->int_out[3] = 0;
    apb->int_out[4] = 0;
    break;

  case AES_WINDOW:    /* 11 */
    apb->int_out[1] = 0x1a1;
    /* 0x001: WF_TOP returns window below current one
       0x002: wind_get(WF_NEWDESK) supported
       0x004: WF_COLOR get/set supported
       0x008: WF_DCOLOR get/set supported
       0x010: WF_OWNER supported in wind_get
       0x020: WF_BEVENT get/set supported
       0x040: WF_BOTTOM supported
       0x080: WF_ICONIFY supported
       0x100: WF_UNICONIFY supported      */
    apb->int_out[2] = 0;  /* resv */
    apb->int_out[3] = 0x1;  /* 0x1: iconifier
                               0x2: explicit "bottomer" gadget
                               0x4: shift+click to send window to bottom
                               0x8: "hot" close box            */
    apb->int_out[ 4] = 1;  /* wind_update check and set allowed  */
    break;
  case AES_MESSAGES:  /* 12 */
    apb->int_out[ 1] = 0;  /* bit 0: WM_NEWTOP message meaningful
                              bit 1: WM_UNTOPPED message sent
                              bit 2: WM_ONTOP message sent
                              bit 3: AP_TERM message sent
                              bit 4: MultiTOS shutdown and resolution change
                              messages supported
                              bit 5: AES sends CH_EXIT
                              bit 6: WM_BOTTOM message sent
                              bit 7: WM_ICONIFY message sent
                              bit 8: WM_UNICONIFY message sent
                              bit 9: WM_ALLICONIFY message sent */
    apb->int_out[ 2] = 0;  /* 0 */
    apb->int_out[ 3] = 0;  /* 0 */
    break;
  case AES_OBJECTS:   /* 13 object information */
    apb->int_out[ 1] = 0;  /* 3D objects supported via objc_flags */
    apb->int_out[ 2] = 2;  /* 1=objc_sysvar, 2=extended sysvar */
    apb->int_out[ 3] = 0;  /* SPEEDO and GDOS fonts allowed in TEDINFO */
    apb->int_out[ 4] = 0;  /* 0 */
    break;
  case AES_FORM:      /* 14 form library information */
    apb->int_out[ 1] = 0;  /* flying dialogs */
    apb->int_out[ 2] = 0;  /* Mag!X style keyboard tables */
    apb->int_out[ 3] = 0;  /* last cursor position returned */
    apb->int_out[ 4] = 0;  /* 0 */
    break;

  default:
    DB_printf("%s: Line %d: Appl_getinfo:\r\n"
              "Unknown type %d\r\n",__FILE__,__LINE__,apb->int_in[0]);
    apb->int_out[0] = 0;
  }
}


/*
** Exported
** Library part of appl_control ()
*/
WORD
Appl_do_control (WORD apid,
                 WORD ap_id,
                 WORD mode) {
  C_APPL_CONTROL par;
  R_APPL_CONTROL ret;
	
  PUT_C_ALL(APPL_CONTROL, &par);

  par.ap_id = ap_id;
  par.mode = mode;

  CLIENT_SEND_RECV(&par,
                   sizeof (C_APPL_CONTROL),
                   &ret,
                   sizeof (R_APPL_CONTROL));
  
  return ret.common.retval;
}
