#ifndef	__LIB_GLOBAL__
#define	__LIB_GLOBAL__

#include "config.h"
#include "types.h"

#define	STKSIZE	4096

/* Semaphore used in Shel_do_write under MiNT */
#define SHEL_WRITE_LOCK 0x6f415357L /* oASW */

/*
** global_common contains variables that are global across applications.
*/
typedef struct global_common {
  WORD    vmode;
  WORD    vmodecode;
  LONG    video;
  RECT    screen;     /* size of physical screen */	
  WORD    num_planes; /* number of bitplanes in the display */

  WORD    realmove;   /* if set realtime moving of windows is enabled */
  WORD    realsize;
  WORD    realslide;  /* if set realtime sliding of windows is enabled */
  WORD    wind_appl;  /* show application name in window mover */

  WORD fsel_extern;    /* set 1 to enable extern fileselectors */
  WORD fsel_sorted;    /* set 1 to have sorted directories with internal fileselecto r*/

  WORD graf_mbox;      /* set 1 to enable graf_mbox/graf_movebox */
  WORD graf_growbox;   /* set 1 to enable graf_growbox */
  WORD graf_shrinkbox; /* set 1 to enable graf_shrinkbox */

  WORD aes_trace;      /* set 1 to trace aes calls */

  int     physical_vdi_id;
  WORD    num_pens;   /* number of available vdi pens */

  WORD    mouse_x;
  WORD    mouse_y;
  WORD    mouse_button;
	
  WORD    arrowrepeat;
  ULONG   time;
	
  WORD    cswidth;
  WORD    csheight;
  WORD    bswidth;
  WORD    bsheight;

  WORD    clwidth;
  WORD    clheight;
  WORD    blwidth;
  WORD    blheight;

  WORD    fnt_small_id; /* id & size of system fonts */
  WORD    fnt_small_sz;
  WORD    fnt_regul_id;
  WORD    fnt_regul_sz;

  WORD    icon_width;   /* Width of iconified window */
  WORD    icon_height;  /* Height of iconified window */

  OBJECT  *aiconstad;
  OBJECT  *alerttad;
  OBJECT  *deskbgtad;
  OBJECT  *fiseltad;
  OBJECT  *informtad;
  OBJECT  *menutad;
  OBJECT  *mouseformstad;
  OBJECT  *pmenutad;
  BYTE    **fr_string;

  /* Window elements resource and counters */
  OBJECT  * windowtad;
  WORD      elemnumber;
  WORD      tednumber;

  WORD    mouse_owner;
  WORD    mouse_mode;

  OBJC_COLORWORD top_colour[20];
  OBJC_COLORWORD untop_colour[20];

#ifndef MINT_TARGET
  WORD           (*callback_handler)(void *, void *);
#endif
}GLOBAL_COMMON;


typedef struct {
  char name[20];
  WORD type;
  WORD ap_id;
} APPL_ENTRY;

typedef struct {
  WORD         count;
  WORD         size;
  APPL_ENTRY * entries;
} APPL_LIST;


/*
** Description
** global_appl contains variables that are global within a special application
*/
typedef struct global_appl {
  GLOBAL_COMMON * common;
  WORD            vid;
  void          * windows;
  OBJECT        * desktop_background;
  OBJECT        * menu;
  RECT            menu_bar_rect;
  RSHDR         * rshdr;
  OBJECT       ** rscfile;
  WORD            use_mint_paths;
  APPL_LIST       appl_menu;
  APPL_LIST       acc_menu;
  BYTE            application_name[20];
} GLOBAL_APPL;

/*
** Description
** Initialize global variables, open vdi workstation etc
**
** 1998-11-15 CG
** 1999-08-25 CG
*/
void
init_global (WORD physical_vdi_id);

/*
** Description
** Initialize application specific variables
*/
void
init_global_appl (int    apid,
		  int    physical_vdi_id,
                  char * appl_name);

void	exit_global(void);

#ifdef MINT_TARGET
GLOBAL_APPL   * get_globals (WORD apid);
#else
extern GLOBAL_APPL global_appl;
#define get_globals(apid) (&global_appl)
#endif

extern GLOBAL_COMMON global_common;
#define get_global_common() (&global_common)

#endif
