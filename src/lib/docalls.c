/*
** docalls.c
**
** Copyright 1996 - 2001 Christer Gustavsson <cg@nocrew.org>
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

#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>

#include "aesbind.h"
#include "appl.h"
#include "debug.h"
#include "evnt.h"
#include "form.h"
#include "fsel.h"
#include "lib_global.h"
#include "graf.h"
#include "menu.h"
#include "objc.h"
#include "rsrc.h"
#include "scrp.h"
#include "shel.h"
#include "types.h"
#include "wind.h"


typedef void   (*AESCALL)(AES_PB *);

#ifdef AES_STRACE

#include "lib_strace.h"

typedef struct
{
  char * name;
  char * informat;
  char * outformat;
} AESCB;
 

static AESCB aesdebug[] =
{
  /* 0x0000 */
  { NULL, NULL, "%d" },
  
  /* 0x0001 */
  { NULL, NULL, "%d" },
  
  /* 0x0002 */
  { NULL, NULL, "%d" },
  
  /* 0x0003 */
  { NULL, NULL, "%d" },
  
  /* 0x0004 */
  { NULL, NULL, "%d" },
  
  /* 0x0005 */
  { NULL, NULL, "%d" },
  
  /* 0x0006 */
  { NULL, NULL, "%d" },
  
  /* 0x0007 */
  { NULL, NULL, "%d" },
  
  /* 0x0008 */
  { NULL, NULL, "%d" },
  
  /* 0x0009 */
  { NULL, NULL, "%d" },
  
  /* 0x000a */
  { "appl_init", "void", "%3Phd" },
  
  /* 0x000b */
  { "appl_read", "%2Phd, %hd, %4P08lx", "%3Phd" },
  
  /* 0x000c */
  { "appl_write", "%2Phd, %hd, %4P08lx", "%3Phd" },
  
  /* 0x000d */
  { "appl_find", "%4Ps", "%3Phd" },
  
  /* 0x000e */
  { "appl_tplay", "%4P08lx, %2Phd, %hd", "%3Phd" },
  
  /* 0x000f */
  { "appl_trecord", "%4P08lx, %2Phd", "%3Phd" },
  
  /* 0x0010 */
  { NULL, NULL, "%d" },
  
  /* 0x0011 */
  { NULL, NULL, "%d" },
  
  /* 0x0012 */
  { "appl_search", "%2Phd, %4Ps", "%3Phd, %hd, %hd" },
  
  /* 0x0013 */
  { "appl_exit", "void", "%3Phd" },
  
  /* 0x0014 */
  { "evnt_keybd", "void", "%3Phd" },
			
  /* 0x0015 */
  { "evnt_button", "2Phd, %hd, %hd", "%3Phd, %hd, %hd, %hd, %hd" },
  
  /* 0x0016 */
  { "evnt_mouse", "%2Phd, %hd, %hd, %hd, %hd", "%3Phd, %hd, %hd, %hd, %hd" },
  
  /* 0x0017 */
  { "evnt_mesag", "%4P08lx", "%3Phd" },
  
  /* 0x0018 */
  { "evnt_timer", "%2Phd, %hd", "%3Phd" },
  
  /* 0x0019 */
  { "evnt_multi", "%2P{MU_EVENT}, %hd, %hd, %hd, %hd, %hd, %hd, %hd, %hd, %hd, %hd, %hd, %hd, %hd, %4P08lx, %2Phd, %hd", "%3P{MU_EVENT}, %hd, %hd, %hd, %hd, %hd, %hd" },
  
  /* 0x001a */
  { "evnt_dclick", "%2Phd, %hd", "%3Phd" },
  
  /* 0x001b */
  { NULL, NULL, "%d" },
  
  /* 0x001c */
  { NULL, NULL, "%d" },
  
  /* 0x001d */
  { NULL, NULL, "%d" },
  
  /* 0x001e */
  { "menu_bar", "%4P08lx, %2Phd", "%3Phd" },
  
  /* 0x001f */
  { "menu_icheck", "%4P08lx, %2Phd, %hd", "%3Phd" },
  
  /* 0x0020 */
  { "menu_ienable", "%4P08lx, %2Phd, %hd", "%3Phd" },
			
  /* 0x0021 */
  { "menu_tnormal", "%4P08lx, %2Phd, %hd", "%3Phd" },
  
  /* 0x0022 */
  { "menu_text", "%4P08lx, %2Phd, %4Ps", "%3Phd" },
  
  /* 0x0023 */
  { "menu_register", "%2Phd, %4Ps", "%3Phd" },
  
  /* 0x0024 */
  { "menu_popup", "%4P08lx, %2Phd, %hd, %4P08lx", "%3Phd" },
  
  /* 0x0025 */
  { "menu_attach", "%2Phd, %4P08lx, %2Phd, %4P08lx", "%3Phd" },
  
  /* 0x0026 */
  { "menu_istart", "%2Phd, %4P08lx, %2Phd, %2Phd", "%3Phd" },

  /* 0x0027 */
  { "menu_settings", "%2Phd, %4P08lx", "%3Phd" },
  
  /* 0x0028 */
  { "objc_add", "%4P08lx, %2Phd, %hd", "%3Phd" },
  
  /* 0x0029 */
  { "objc_delete", "%4P08lx, %2Phd", "%3Phd" },
  
  /* 0x002a */
  { "objc_draw", "%4P08lx, %2Phd, %hd, %hd, %hd, %hd, %hd", "%3Phd" },
  
  /* 0x002b */
  { "objc_find", "%4P08lx, %2Phd, %hd, %hd, %hd", "%3Phd" },
  
  /* 0x002c */
  { "objc_offset", "%4P08lx, %2Phd", "%3Phd, %hd, %hd" },
  
  /* 0x002d */
  { "objc_order", "%4P08lx, %2Phd, %2Phd", "%3Phd" },
  
  /* 0x002e */
  { "objc_edit", "%4P08lx, %2Phd, %hd, %hd, %hd", "%3Phd, %hd" },
  
  /* 0x002f */
  { "objc_change", "%4P08lx, %2Phd, %hd, %hd, %hd, %hd, %hd, %hd, %hd", "%3Phd" },
  
  /* 0x0030 */
  { "objc_sysvar", "%2Phd, %hd, %hd, %hd", "%3Phd, %hd, %hd" },
  
  /* 0x0031 */
  { NULL, NULL, "%d" },
  
  /* 0x0032 */
  { "form_do", "%4P08lx, %2Phd", "%3Phd" },
  
  /* 0x0033 */
  { "form_dial", "%2Phd, %hd, %hd, %hd, %hd, %hd, %hd, %hd, %hd", "%3Phd" },
  
  /* 0x0034 */
  { "form_alert", "%2Phd, %4Ps", "%3Phd" },
  
  /* 0x0035 */
  { "form_error", "%2Phd", "%3Phd" },
  
  /* 0x0036 */
  { "form_center", "%4P08lx", "%3Phd, %hd, %hd, %hd, %hd" },
  
  /* 0x0037 */
  { "form_keybd", "%4P08lx, %3Phd, %hd, %hd", "%3Phd, %hd, %hd" },
  
  /* 0x0038 */
  { "form_button", "%4P08lx, %2Phd, %hd", "%3Phd, %hd" },
  
  /* 0x0039 */
  { NULL, NULL, "%d" },
	 
  /* 0x003a */
  { NULL, NULL, "%d" },
	
  /* 0x003b */
  { NULL, NULL, "%d" },
	
  /* 0x003c */
  { NULL, NULL, "%d" },
	
  /* 0x003d */
  { NULL, NULL, "%d" },
	
  /* 0x003e */
  { NULL, NULL, "%d" },
	
  /* 0x003f */
  { NULL, NULL, "%d" },
	
  /* 0x0040 */
  { NULL, NULL, "%d" },
	
  /* 0x0041 */
  { NULL, NULL, "%d" },
	
  /* 0x0042 */
  { NULL, NULL, "%d" },
	
  /* 0x0043 */
  { NULL, NULL, "%d" },
	
  /* 0x0044 */
  { NULL, NULL, "%d" },
	
  /* 0x0045 */
  { NULL, NULL, "%d" },
	
  /* 0x0046 */
  { "graf_rubberbox", "2Phd, %hd, %hd, %hd", "%3Phd, %hd, %hd" },
	
  /* 0x0047 */
  { "graf_dragbox", "%2Phd, %hd, %hd, %hd, %hd, %hd, %hd, %hd", "%3Phd, %hd, %hd" },
			
  /* 0x0048 */
  { "graf_movebox", "%2Phd, %hd, %hd, %hd, %hd, %hd", "%3Phd" },
	
  /* 0x0049 */
  { "graf_growbox", "%2Phd, %hd, %hd, %hd, %hd, %hd, %hd, %hd", "%3Phd" },
			
  /* 0x004a */
  { "graf_shrinkbox", "%2Phd, %hd, %hd, %hd, %hd, %hd, %hd, %hd", "%3Phd" },
			
  /* 0x004b */
  { "graf_watchbox", "%4P08lx, %2Phd, %hd, %hd", "%3Phd" },
			
  /* 0x004c */
  { "graf_slidebox", "%4P08lx, %2Phd, %hd, %hd", "%3Phd" },
			
  /* 0x004d */
  { "graf_handle", "void", "%3Phd, %hd, %hd, %hd, %hd" },
			
  /* 0x004e */
  { "graf_mouse", "%2Phd, %4P08lx", "%3Phd" },
			
  /* 0x004f */
  { "graf_mkstate", "void", "%3Phd, %hd, %hd, %hd, %hd" },
			
  /* 0x0050 */
  { "scrp_read", "%4P08lx", "%3Phd, %4Ps" },
			
  /* 0x0051 */
  { "scrp_write", "%4P08s", "%3Phd" },
			
  /* 0x0052 */
  { NULL, NULL, "%d" },
	
  /* 0x0053 */
  { NULL, NULL, "%d" },
	
  /* 0x0054 */
  { NULL, NULL, "%d" },
	
  /* 0x0055 */
  { NULL, NULL, "%d" },
	
  /* 0x0056 */
  { NULL, NULL, "%d" },
	
  /* 0x0057 */
  { NULL, NULL, "%d" },
	
  /* 0x0058 */
  { NULL, NULL, "%d" },
	
  /* 0x0059 */
  { NULL, NULL, "%d" },
	
  /* 0x005a */
  { "fsel_input", "%4Ps, %s", "%3Phd, %hd" },
			
  /* 0x005b */
  { "fsel_exinput", "%4Ps, %s, %s", "%3Phd, %hd" },

  /* 0x005c */
  { NULL, NULL, "%d" },
	
  /* 0x005d */
  { NULL, NULL, "%d" },
	
  /* 0x005e */
  { NULL, NULL, "%d" },
	
  /* 0x005f */
  { NULL, NULL, "%d" },
	
  /* 0x0060 */
  { NULL, NULL, "%d" },
	
  /* 0x0061 */
  { NULL, NULL, "%d" },
	
  /* 0x0062 */
  { NULL, NULL, "%d" },
	
  /* 0x0063 */
  { NULL, NULL, "%d" },
	
  /* 0x0064 */
  { "wind_create", "%2Phd, %hd, %hd, %hd, %hd", "%3Phd" },
			
  /* 0x0065 */
  { "wind_open", "%2Phd, %hd, %hd, %hd, %hd", "%3Phd" },
			
  /* 0x0066 */
  { "wind_close", "%2Phd", "%3Phd" },
			
  /* 0x0067 */
  { "wind_delete", "%2Phd", "%3Phd" },
			
  /* 0x0068 */
  { "wind_get", "%2Phd, %hd, %hd", "%3Phd, %hd, %hd, %hd, %hd"},
			
  /* 0x0069 */
  { "wind_set", "%2Phd, %hd, %hd, %hd, %hd, %hd", "%3Phd" },
			
  /* 0x006a */
  { "wind_find", "%2Phd, %hd", "%3Phd" },
			
  /* 0x006b */
  { "wind_update", "%2Phd", "%3Phd" },
			
  /* 0x006c */
  { "wind_calc", "%2Phd, %hd, %hd, %hd, %hd, %hd", "%3Phd, %hd, %hd, %hd, %hd" },
			
  /* 0x006d */
  { "wind_new", "void", "%3Phd" },

  /* 0x006e */
  { "rsrc_load", "%4Ps", "%3Phd" },
			
  /* 0x006f */
  { "rsrc_free", "void", "%3Phd" },
			
  /* 0x0070 */
  { "rsrc_gaddr", "%2Phd, %2Phd", "%3Phd, %5Px" },
			
  /* 0x0071 */
  { "rsrc_saddr", "%2Phd, %hd, %4P08lx", "%3Phd" },
			
  /* 0x0072 */
  { "rsrc_obfix", "%4P08lx, %2Phd", "%3Phd" },
			
  /* 0x0073 */
  { "rsrc_rcfix", "%4P08lx", "%3Phd" },
			
  /* 0x0074 */
  { NULL, NULL, "%d" },
	
  /* 0x0075 */
  { NULL, NULL, "%d" },
	
  /* 0x0076 */
  { NULL, NULL, "%d" },
	
  /* 0x0077 */
  { NULL, NULL, "%d" },

  /* 0x0078 */
  { "shel_read", "%4P08lx, %08lx", "%3Phd, %4Ps, %s" },

  /* 0x0079 */
  { "shel_write", "%2Phd, %hd, %hd, %4Ps, %s", "%3Phd" },
 			
  /* 0x007a */
  { "shel_get", "%4P08lx, %2Phd", "%3Phd" },
	
  /* 0x007b */
  { "shel_put", "%4P08lx, %2Phd", "%3Phd" },
	
  /* 0x007c */
  { "shel_find", "%4Ps", "%3Phd, %4Ps" },
			
  /* 0x007d */
  { "shel_envrn", "%4P08lx, %s", "%3Phd" },
	
  /* 0x007e */
  { NULL, NULL, "%d" },
	
  /* 0x007f */
  { NULL, NULL, "%d" },
	
  /* 0x0080 */
  { NULL, NULL, "%d" },
	
  /* 0x0081 */
  { NULL, NULL, "%d" },
	
  /* 0x0082 */
  { "appl_getinfo", "%2Phd", "%3Phd, %hd, %hd, %hd, %hd" }
};

#endif /* AES_STRACE */


static AESCALL aescalls[] =
{
  /* 0x0000 */
  NULL,
  
  /* 0x0001 */
  NULL,
  
  /* 0x0002 */
  NULL,
  
  /* 0x0003 */
  NULL,
  
  /* 0x0004 */
  NULL,
  
  /* 0x0005 */
  NULL,
  
  /* 0x0006 */
  NULL,
  
  /* 0x0007 */
  NULL,
  
  /* 0x0008 */
  NULL,
  
  /* 0x0009 */
  NULL,
  
  /* 0x000a */
  Appl_init,
  
  /* 0x000b */
  Appl_read,
  
  /* 0x000c */
  Appl_write,
  
  /* 0x000d */
  Appl_find,
  
  /* 0x000e */
  NULL, /* appl_tplay */
  
  /* 0x000f */
  NULL, /* appl_trecord */
  
  /* 0x0010 */
  NULL,
  
  /* 0x0011 */
  NULL,
  
  /* 0x0012 */
  Appl_search,
  
  /* 0x0013 */
  Appl_exit,
  
  /* 0x0014 */
  Evnt_keybd,
			
  /* 0x0015 */
  Evnt_button,
  
  /* 0x0016 */
  Evnt_mouse,
  
  /* 0x0017 */
  Evnt_mesag,
  
  /* 0x0018 */
  Evnt_timer,
  
  /* 0x0019 */
  Evnt_multi,
  
  /* 0x001a */
  Evnt_dclick,
  
  /* 0x001b */
  NULL,
  
  /* 0x001c */
  NULL,
  
  /* 0x001d */
  NULL,
  
  /* 0x001e */
  Menu_bar,
  
  /* 0x001f */
  Menu_icheck,
  
  /* 0x0020 */
  Menu_ienable,
			
  /* 0x0021 */
  Menu_tnormal,
  
  /* 0x0022 */
  Menu_text,
  
  /* 0x0023 */
  Menu_register,
  
  /* 0x0024 */
  Menu_popup,
  
  /* 0x0025 */
  NULL, /* menu_attach */
  
  /* 0x0026 */
  NULL, /* menu_istart */

  /* 0x0027 */
  NULL, /* menu_settings */
  
  /* 0x0028 */
  Objc_add,
  
  /* 0x0029 */
  Objc_delete,
  
  /* 0x002a */
  Objc_draw,
  
  /* 0x002b */
  Objc_find,
  
  /* 0x002c */
  Objc_offset,
  
  /* 0x002d */
  NULL, /* objc_order */
  
  /* 0x002e */
  Objc_edit,
  
  /* 0x002f */
  Objc_change,
  
  /* 0x0030 */
  Objc_sysvar,
  
  /* 0x0031 */
  NULL,
  
  /* 0x0032 */
  Form_do,
  
  /* 0x0033 */
  Form_dial,
  
  /* 0x0034 */
  Form_alert,
  
  /* 0x0035 */
  Form_error,
  
  /* 0x0036 */
  Form_center,
  
  /* 0x0037 */
  Form_keybd,
  
  /* 0x0038 */
  Form_button,
  
  /* 0x0039 */
  NULL,
	
  /* 0x003a */
  NULL,
	
  /* 0x003b */
  NULL,
	
  /* 0x003c */
  NULL,
	
  /* 0x003d */
  NULL,
	
  /* 0x003e */
  NULL,
	
  /* 0x003f */
  NULL,
	
  /* 0x0040 */
  NULL,
	
  /* 0x0041 */
  NULL,
	
  /* 0x0042 */
  NULL,
	
  /* 0x0043 */
  NULL,
	
  /* 0x0044 */
  NULL,
	
  /* 0x0045 */
  NULL,
	
  /* 0x0046 */
  Graf_rubberbox,
	
  /* 0x0047 */
  Graf_dragbox,
			
  /* 0x0048 */
  Graf_movebox,
	
  /* 0x0049 */
  Graf_growbox,
			
  /* 0x004a */
  Graf_shrinkbox,
			
  /* 0x004b */
  Graf_watchbox,
			
  /* 0x004c */
  Graf_slidebox,
			
  /* 0x004d */
  Graf_handle,
			
  /* 0x004e */
  Graf_mouse,
			
  /* 0x004f */
  Graf_mkstate,
			
  /* 0x0050 */
  Scrp_read,
			
  /* 0x0051 */
  Scrp_write,
			
  /* 0x0052 */
  NULL,
	
  /* 0x0053 */
  NULL,
	
  /* 0x0054 */
  NULL,
	
  /* 0x0055 */
  NULL,
	
  /* 0x0056 */
  NULL,
	
  /* 0x0057 */
  NULL,
	
  /* 0x0058 */
  NULL,
	
  /* 0x0059 */
  NULL,
	
  /* 0x005a */
  Fsel_input,
			
  /* 0x005b */
  Fsel_exinput,

  /* 0x005c */
  NULL,
	
  /* 0x005d */
  NULL,
	
  /* 0x005e */
  NULL,
	
  /* 0x005f */
  NULL,
	
  /* 0x0060 */
  NULL,
	
  /* 0x0061 */
  NULL,
	
  /* 0x0062 */
  NULL,
	
  /* 0x0063 */
  NULL,
	
  /* 0x0064 */
  Wind_create,
			
  /* 0x0065 */
  Wind_open,
			
  /* 0x0066 */
  Wind_close,
			
  /* 0x0067 */
  Wind_delete,
			
  /* 0x0068 */
  Wind_get,
			
  /* 0x0069 */
  Wind_set,
			
  /* 0x006a */
  Wind_find,
			
  /* 0x006b */
  Wind_update,
			
  /* 0x006c */
  Wind_calc,
			
  /* 0x006d */
  Wind_new,

  /* 0x006e */
  Rsrc_load,
			
  /* 0x006f */
  Rsrc_free,
			
  /* 0x0070 */
  Rsrc_gaddr,
			
  /* 0x0071 */
  Rsrc_saddr,
			
  /* 0x0072 */
  Rsrc_obfix,
			
  /* 0x0073 */
  Rsrc_rcfix,
			
  /* 0x0074 */
  NULL,
	
  /* 0x0075 */
  NULL,
	
  /* 0x0076 */
  NULL,
	
  /* 0x0077 */
  NULL,

  /* 0x0078 */
  Shel_read,

  /* 0x0079 */
  Shel_write,
 			
  /* 0x007a */
  NULL, /* shel_get */
	
  /* 0x007b */
  NULL, /* shel_put */
	
  /* 0x007c */
  Shel_find,
			
  /* 0x007d */
  Shel_envrn,
	
  /* 0x007e */
  NULL,
	
  /* 0x007f */
  NULL,
	
  /* 0x0080 */
  NULL,
	
  /* 0x0081 */
  NULL,
	
  /* 0x0082 */
  Appl_getinfo
};

/****************************************************************************
 * Public functions                                                         *
 ****************************************************************************/

/*
** Exported
*/
void
#ifdef MINT_TARGET
lib_aes_call(AESPB * apb)
#else
aes_call(AESPB * apb)
#endif
{
#ifdef AES_STRACE
  aes_strace_begin(aesdebug[apb->contrl[0]].name,
                   aesdebug[apb->contrl[0]].informat,
                   apb);
#endif AES_STRACE
  DEBUG2("Aes call %d (0x%x) %s apid %d pid %d",
         apb->contrl[0],
         apb->contrl[0],
         aescalls[apb->contrl[0]].name ? aescalls[apb->contrl[0]].name : "##",
         ((AES_PB *)apb)->global->apid,
         getpid());
  
  if(aescalls[apb->contrl[0]])
  {
    aescalls[apb->contrl[0]]((AES_PB *)apb);
  }
  else
  {
#ifdef AES_STRACE
    if(aesdebug[apb->contrl[0]].name)
    {
      DEBUG0("%s: Line %d:\r\n"
             "Unimplemented AES call %d %s",
             __FILE__,__LINE__,apb->contrl[0],
             aesdebug[apb->contrl[0]].name);
    }
    else
#endif /* AES_STRACE */
    {
      DEBUG0( "%s: Line %d:\r\n"
              "Illegal AES call %d",
              __FILE__,__LINE__,apb->contrl[0]);
    }
  }

#ifdef AES_STRACE  
  aes_strace_end(aesdebug[apb->contrl[0]].name,
                 aesdebug[apb->contrl[0]].outformat,
                 apb);
#endif /* AES_STRACE */

  DEBUG2("Returned from aes call %d (0x%x) apid %d pid %d",
         apb->contrl[0],
         apb->contrl[0],
         ((AES_PB *)apb)->global->apid,
         getpid());
}


#define FIX(dst, src, len, fun) \
  for(i = 0; i < len; i++) \
  { \
    (dst)[i] = fun((src)[i]); \
  }

void
aes_call_be32(AESPB * apb)
{
  int          i;
  short *      tmp_short;
  long *       tmp_long;
  static short contrl[5];
  static short global[15];
  static short intin[16];
  static short intout[7];
  static long  addrin[3];
  static long  addrout[1];
  static AESPB native_apb =
  {
    contrl, global, intin, intout, addrin, addrout
  };

  tmp_short = (short *)ntohl((LONG)apb->contrl);
  FIX(native_apb.contrl, tmp_short, 5, ntohs);

  tmp_short = (short *)ntohl((LONG)apb->intin);
  FIX(native_apb.intin, tmp_short, native_apb.contrl[1], ntohs);

  tmp_long = (long *)ntohl((LONG)apb->addrin);
  FIX(native_apb.addrin, tmp_long, native_apb.contrl[2], ntohl);

  aes_call(&native_apb);

  tmp_short = (short *)ntohl((LONG)apb->contrl);
  FIX(tmp_short, native_apb.contrl, 5, htons);

  tmp_short = (short *)ntohl((LONG)apb->intout);
  FIX(tmp_short, native_apb.intout, 7 /* FIXME native_apb.contrl[3] */, htons);

  tmp_long = (long *)ntohl((LONG)apb->addrout);
  FIX(tmp_long, native_apb.addrout, 1 /* FIXME native_apb.contrl[4] */, htonl);

  if((native_apb.contrl[0] == 23) || /* evnt_mesag */
     (native_apb.contrl[0] == 25))   /* evnt_multi */
  {
    DEBUG3("Fix evnt_multi");
    if(native_apb.addrin[0] != 0)
    {          
      FIX((WORD *)native_apb.addrin[0],
          (WORD *)native_apb.addrin[0],
          (MSG_LENGTH / sizeof(WORD)),
          htons);
    }
    DEBUG3("Fix evnt_multi finished");
  }
}
