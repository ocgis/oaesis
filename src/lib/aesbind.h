/*
** aesbind.h
**
** Copyright 1998-1999 Christer Gustavsson <cg@nocrew.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**  
** Read the file COPYING for more information.
**
*/

#ifndef _AESBIND_H_
#define _AESBIND_H_

#ifdef __GNUC__
#define PACKED __attribute__ ((packed))
#else
#define PACKED
#endif

#define NIL (-1)

/* appl_control modes */
#define APC_HIDE    10
#define APC_SHOW    11
#define APC_TOP     12
#define APC_HIDENOT 13

                /* appl_getinfo modes */
#define AES_LARGEFONT   0
#define AES_SMALLFONT   1
#define AES_SYSTEM      2
#define AES_LANGUAGE    3
#define AES_PROCESS     4
#define AES_PCGEM       5
#define AES_INQUIRE     6
#define AES_MOUSE       8
#define AES_MENU        9
#define AES_SHELL      10
#define AES_WINDOW     11
#define AES_MESSAGES   12
#define AES_OBJECTS    13
#define AES_FORM       14

                /* appl_getinfo return values */
#define SYSTEM_FONT     0
#define OUTLINE_FONT    1

#define AESLANG_ENGLISH 0
#define AESLANG_GERMAN  1
#define AESLANG_FRENCH  2
#define AESLANG_SPANISH 4
#define AESLANG_ITALIAN 5
#define AESLANG_SWEDISH 6

                /* appl_read modes */
#define APR_NOWAIT     -1

                /* appl_search modes */
#define APP_FIRST       0
#define APP_NEXT        1

                /* appl_search return values*/
#define APP_SYSTEM      0x01
#define APP_APPLICATION 0x02
#define APP_ACCESSORY   0x04
#define APP_SHELL       0x08

                /* appl_trecord types */
#define APPEVNT_TIMER    0
#define APPEVNT_BUTTON   1
#define APPEVNT_MOUSE    2
#define APPEVNT_KEYBOARD 3              

                /* struct used by appl_trecord and appl_tplay */
typedef struct pEvntrec {
  long ap_event;
  long ap_value;
}EVNTREC;
                

typedef struct graphic_rectangle {
  short g_x;
  short g_y;
  short g_w;
  short g_h;
} GRECT;

                /* evnt_button flags */
#define LEFT_BUTTON     0x0001
#define RIGHT_BUTTON    0x0002
#define MIDDLE_BUTTON   0x0004

#define K_RSHIFT        0x0001
#define K_LSHIFT        0x0002
#define K_CTRL          0x0004
#define K_ALT           0x0008

                /* evnt_dclick flags */
#define EDC_INQUIRE     0
#define EDC_SET         1

                /* event message values */
#define MN_SELECTED   10
#define WM_REDRAW     20
#define WM_TOPPED     21
#define WM_CLOSED     22
#define WM_FULLED     23
#define WM_ARROWED    24
#define WM_HSLID      25
#define WM_VSLID      26
#define WM_SIZED      27
#define WM_MOVED      28
#define WM_NEWTOP     29
#define WM_UNTOPPED   30
#define WM_ONTOP      31
#define WM_BOTTOM     33
#define WM_ICONIFY    34
#define WM_UNICONIFY  35
#define WM_ALLICONIFY 36
#define WM_TOOLBAR    37
#define AC_OPEN       40
#define AC_CLOSE      41
#define AP_TERM       50
#define AP_TFAIL      51
#define AP_RESCHG     57

                /* Xcontrol messages */
#define CT_UPDATE     50
#define CT_MOVE       51
#define CT_NEWTOP     52
#define CT_KEY        53

#define SHUT_COMPLETED   60
#define RESCHG_COMPLETED 61
#define RESCH_COMPLETED  61
#define AP_DRAGDROP      63
#define SH_WDRAW         72
#define CH_EXIT          80   /* should this be 90 like in the compendium?*/

                /* evnt_mouse modes */
#define MO_ENTER 0
#define MO_LEAVE 1

                /* evnt_multi flags */
#define MU_KEYBD        0x0001
#define MU_BUTTON       0x0002
#define MU_M1           0x0004
#define MU_M2           0x0008
#define MU_MESAG        0x0010
#define MU_TIMER        0x0020

                /* form_dial opcodes */
#define FMD_START       0
#define FMD_GROW        1
#define FMD_SHRINK      2
#define FMD_FINISH      3

                /* form_error modes */
#define FERR_FILENOTFOUND   2
#define FERR_PATHNOTFOUND   3
#define FERR_NOHANDLES      4
#define FERR_ACCESSDENIED   5
#define FERR_LOWMEM         8
#define FERR_BADENVIRON    10
#define FERR_BADFORMAT     11
#define FERR_BADDRIVE      15
#define FERR_DELETEDIR     16
#define FERR_NOFILES       18

                /* fsel_(ex)input return values*/
#define FSEL_CANCEL         0
#define FSEL_OK             1
                
                /* menu_attach modes */
#define ME_INQUIRE      0
#define ME_ATTACH       1
#define ME_REMOVE       2

                /* menu_attach attributes */
#define SCROLL_NO       0
#define SCROLL_YES      1

                /* menu_bar modes */
#define MENU_REMOVE     0
#define MENU_INSTALL    1
#define MENU_INQUIRE   -1

                /* menu_icheck modes */
#define UNCHECK         0
#define CHECK           1

                /* menu_ienable modes */
#define DISABLE         0
#define ENABLE          1

                /* menu_istart modes */
#define MIS_GETALIGN    0
#define MIS_SETALIGN    1

                /* menu_popup modes */
#define SCROLL_LISTBOX -1

                /* menu_register modes */
#define REG_NEWNAME    -1

/* menu_tnormal modes */
#define HIGHLIGHT   0
#define UNHIGHLIGHT 1

/* menu_settings uses a new structure for setting and inquiring the submenu
 * delay values and the menu scroll height.  The delay values are measured in
 * milliseconds and the height is based upon the number of menu items.
 */

typedef struct _mn_set {
#ifdef OLD_MNSET
    long  Display;   /*  the submenu display delay     */
    long  Drag;      /*  the submenu drag delay        */
    long  Delay;     /*  the single-click scroll delay */
    long  Speed;     /*  the continuous scroll delay   */
    short Height;    /*  the menu scroll height        */
#else
    long  display;   /*  the submenu display delay     */
    long  drag;      /*  the submenu drag delay        */
    long  delay;     /*  the single-click scroll delay */
    long  speed;     /*  the continuous scroll delay   */
    short height;    /*  the menu scroll height        */
#endif
} PACKED MN_SET;

/* shel_get modes */
#define SHEL_BUFSIZE (-1)

                /* shel_write modes */
#define SWM_LAUNCH     0
#define SWM_LAUNCHNOW  1
#define SWM_LAUNCHACC  3
#define SWM_SHUTDOWN   4
#define SWM_REZCHANGE  5
#define SWM_BROADCAST  7
#define SWM_ENVIRON    8
#define SWM_NEWMSG     9
#define SWM_AESMSG    10

                /* shel_write flags */
#define SW_PSETLIMIT 0x0100
#define SW_PRENICE   0x0200
#define SW_DEFDIR    0x0400
#define SW_ENVIRON   0x0800

#define TOSAPP 0
#define GEMAPP 1

#define SD_ABORT    0
#define SD_PARTIAL  1
#define SD_COMPLETE 2

#define ENVIRON_SIZE   0
#define ENVIRON_CHANGE 1
#define ENVIRON_COPY   2

                /* rsrc_gaddr structure types */
#define R_TREE       0 
#define R_OBJECT     1
#define R_TEDINFO    2
#define R_ICONBLK    3
#define R_BITBLK     4
#define R_STRING     5
#define R_IMAGEDATA  6
#define R_OBSPEC     7
#define R_TEPTEXT    8
#define R_TEPTMPLT   9
#define R_TEPVALID  10
#define R_IBPMASK   11 
#define R_IBPDATA   12  
#define R_IBPTEXT   13
#define R_BIPDATA   14
#define R_FRSTR     15
#define R_FRIMG     16 



                /* Window Attributes */
#define NAME         0x0001
#define CLOSER       0x0002
#define FULLER       0x0004
#define MOVER        0x0008
#define INFO         0x0010
#define SIZER        0x0020
#define UPARROW      0x0040
#define DNARROW      0x0080
#define VSLIDE       0x0100
#define LFARROW      0x0200
#define RTARROW      0x0400
#define HSLIDE       0x0800
#define SMALLER      0x4000

                /* wind_create flags */
#define WC_BORDER     0
#define WC_WORK       1

                /* wind_get flags */
#define WF_KIND           1
#define WF_NAME           2
#define WF_INFO           3
#define WF_WORKXYWH       4
#define WF_CURRXYWH       5
#define WF_PREVXYWH       6
#define WF_FULLXYWH       7
#define WF_HSLIDE         8
#define WF_VSLIDE         9
#define WF_TOP           10
#define WF_FIRSTXYWH     11
#define WF_NEXTXYWH      12
#define WF_RESVD         13
#define WF_NEWDESK       14
#define WF_HSLSIZE       15
#define WF_VSLSIZE       16
#define WF_SCREEN        17
#define WF_COLOR         18
#define WF_DCOLOR        19
#define WF_OWNER         20
#define WF_BEVENT        24
#define WF_BOTTOM        25
#define WF_ICONIFY       26
#define WF_UNICONIFY     27
#define WF_UNICONIFYXYWH 28
#define WF_TOOLBAR       30
#define WF_FTOOLBAR      31
#define WF_NTOOLBAR      32
#define WF_WINX          22360
#define WF_WINXCFG       22361

                /* window elements      */
#define W_BOX        0
#define W_TITLE      1
#define W_CLOSER     2
#define W_NAME       3
#define W_FULLER     4
#define W_INFO       5
#define W_DATA       6
#define W_WORK       7
#define W_SIZER      8
#define W_VBAR       9
#define W_UPARROW   10
#define W_DNARROW   11
#define W_VSLIDE    12
#define W_VELEV     13
#define W_HBAR      14
#define W_LFARROW   15
#define W_RTARROW   16
#define W_HSLIDE    17
#define W_HELEV     18
#define W_SMALLER   19

                /* arrow message        */
#define WA_UPPAGE       0
#define WA_DNPAGE       1
#define WA_UPLINE       2
#define WA_DNLINE       3
#define WA_LFPAGE       4
#define WA_RTPAGE       5
#define WA_LFLINE       6
#define WA_RTLINE       7

                /* wind_update flags */
#define END_UPDATE 0
#define BEG_UPDATE 1
#define END_MCTRL  2
#define BEG_MCTRL  3

/* graf_mouse mouse types*/
#define ARROW            0
#define TEXT_CRSR        1
#define BEE              2
#define BUSY_BEE       BEE              /* alias */
#define BUSYBEE        BEE              /* alias */
#define HOURGLASS        2
#define POINT_HAND       3
#define FLAT_HAND        4
#define THIN_CROSS       5
#define THICK_CROSS      6
#define OUTLN_CROSS      7
#define USER_DEF       255
#define M_OFF          256
#define M_ON           257
#define M_SAVE         258
#define M_LAST         259
#define M_RESTORE      260
#define M_FORCE     0x8000

/* objects - general */
#define ROOT       0     /* index of ROOT */
#define MAX_LEN   81     /* max string length */
#define MAX_DEPTH  8     /* max depth of search or draw */

/* inside fill patterns */
#define IP_HOLLOW       0
#define IP_1PATT        1
#define IP_2PATT        2
#define IP_3PATT        3
#define IP_4PATT        4
#define IP_5PATT        5
#define IP_6PATT        6
#define IP_SOLID        7

/* normal graphics drawing modes */
#define MD_REPLACE 1
#define MD_TRANS   2
#define MD_XOR     3
#define MD_ERASE   4

                /* bit blt rules */
#define ALL_WHITE   0
#define S_AND_D     1
#define S_AND_NOTD  2
#define S_ONLY      3
#define NOTS_AND_D  4
#define D_ONLY      5
#define S_XOR_D     6
#define S_OR_D      7
#define NOT_SORD    8
#define NOT_SXORD   9
#define D_INVERT         10
#define NOT_D      10
#define S_OR_NOTD  11
#define NOT_S      12
#define NOTS_OR_D  13
#define NOT_SANDD  14
#define ALL_BLACK  15

                /* font types */
#define GDOS_PROP   0
#define GDOS_MONO   1
#define GDOS_BITM   2
#define IBM         3
#define SMALL       5

                /* object types */
#define G_BOX      20
#define G_TEXT     21
#define G_BOXTEXT  22
#define G_IMAGE    23
#define G_USERDEF  24
#define G_PROGDEF  G_USERDEF
#define G_IBOX     25
#define G_BUTTON   26
#define G_BOXCHAR  27
#define G_STRING   28
#define G_FTEXT    29
#define G_FBOXTEXT 30
#define G_ICON     31
#define G_TITLE    32
#define G_CICON    33

/* object flags */
#define NONE       0x0000
#define SELECTABLE 0x0001
#define DEFAULT    0x0002
#define EXIT       0x0004
#define EDITABLE   0x0008
#define RBUTTON    0x0010
#define LASTOB     0x0020
#define TOUCHEXIT  0x0040
#define HIDETREE   0x0080
#define INDIRECT   0x0100
#define FL3DIND    0x0200
#define FL3DBAK    0x0400
#define FL3DACT    0x0600
#define SUBMENU    0x0800

/* Object states */
#define NORMAL     0x0000
#define SELECTED   0x0001
#define CROSSED    0x0002
#define CHECKED    0x0004
#define DISABLED   0x0008
#define OUTLINED   0x0010
#define SHADOWED   0x0020
#define WHITEBAK   0x0040 /* Should WHITEBAK and DRAW3D be swapped? */
#define DRAW3D     0x0080

/* Object colors - default pall. */
#define WHITE    0
#define BLACK    1
#define RED      2
#define GREEN    3
#define BLUE     4
#define CYAN     5
#define YELLOW   6
#define MAGENTA  7
#define LWHITE   8
#define LBLACK   9
#define LRED     10
#define LGREEN   11
#define LBLUE    12
#define LCYAN    13
#define LYELLOW  14
#define LMAGENTA 15

/* editable text field definitions */
#define EDSTART      0
#define EDINIT       1
#define EDCHAR       2
#define EDEND        3

#define ED_START     EDSTART
#define ED_INIT      EDINIT
#define ED_CHAR      EDCHAR
#define ED_END       EDEND

/* editable text justification */
#define TE_LEFT      0
#define TE_RIGHT     1
#define TE_CNTR      2

/* objc_change modes */
#define NO_DRAW      0
#define REDRAW       1

/* objc_order modes */
#define OO_LAST     -1
#define OO_FIRST     0

/* objc_sysvar modes */
#define SV_INQUIRE   0
#define SV_SET       1

/* objc_sysvar values */
#define LK3DIND      1
#define LK3DACT      2
#define INDBUTCOL    3
#define ACTBUTCOL    4
#define BACKGRCOL    5
#define AD3DVAL      6

typedef struct objc_colorword {
   unsigned borderc : 4;
   unsigned textc   : 4;
   unsigned opaque  : 1;
   unsigned pattern : 3;
   unsigned fillc   : 4;
} PACKED OBJC_COLORWORD;

typedef struct text_edinfo
{
  char *         te_ptext;      /* ptr to text */
  char *         te_ptmplt;     /* ptr to template */
  char *         te_pvalid;     /* ptr to validation chrs. */
  short          te_font;       /* font */
  short          te_fontid;     /* font id */
  short          te_just;       /* justification */
  OBJC_COLORWORD te_color;      /* color information word */
  short          te_fontsize;   /* font size */
  short          te_thickness;  /* border thickness */
  short          te_txtlen;     /* length of text string */
  short          te_tmplen;     /* length of template string */
} PACKED TEDINFO;

typedef struct icon_block {
  short * ib_pmask;
  short * ib_pdata;
  char  * ib_ptext;
  short   ib_char;
  short   ib_xchar;
  short   ib_ychar;
  short   ib_xicon;
  short   ib_yicon;
  short   ib_wicon;
  short   ib_hicon;
  short   ib_xtext;
  short   ib_ytext;
  short   ib_wtext;
  short   ib_htext;
} PACKED ICONBLK;

typedef struct bit_block {
  char  * bi_pdata;  /* ptr to bit forms data  */
  short   bi_wb;      /* width of form in bytes */
  short   bi_hl;      /* height in lines */
  short   bi_x;       /* source x in bit form */
  short   bi_y;       /* source y in bit form */
  short   bi_color;   /* fg color of blt */
} PACKED BITBLK;

typedef struct cicon_data {
  short               num_planes;
  short *             col_data;
  short *             col_mask;
  short *             sel_data;
  short *             sel_mask;
  struct cicon_data * next_res;
} PACKED CICON;
        
typedef struct cicon_blk {
  ICONBLK monoblk;
  CICON * mainlist;
} PACKED CICONBLK;


typedef struct {
  unsigned int character   :  8;
  signed   int framesize   :  8;
  unsigned int framecol    :  4;
  unsigned int textcol     :  4;
  unsigned int textmode    :  1;
  unsigned int fillpattern :  3;
  unsigned int interiorcol :  4;
} PACKED bfobspec;

struct user_block;      /* forward declaration */

typedef union __u_ob_spec {
  TEDINFO *           tedinfo;
  long                index;
  char *              free_string;
  union __u_ob_spec * indirect;
  bfobspec            obspec;
  BITBLK *            bitblk;
  ICONBLK *           iconblk;
  CICONBLK *          ciconblk;  
  struct user_block * userblk;
  /*      APPLBLK           *applblk;
          char              *string;      */
} PACKED U_OB_SPEC;

typedef struct object
{
  short          ob_next;   /* -> object's next sibling               */
  short          ob_head;   /* -> head of object's children           */
  short          ob_tail;   /* -> tail of object's children           */
  unsigned short ob_type;   /* type of object                         */
  unsigned short ob_flags;  /* flags                                  */
  unsigned short ob_state;  /* state                                  */
  U_OB_SPEC      ob_spec;   /* object-specific data                   */
  short          ob_x;      /* upper left corner of object            */
  short          ob_y;      /* upper left corner of object            */
  short          ob_width;  /* width of obj                           */
  short          ob_height; /* height of obj                          */
} PACKED OBJECT;

typedef struct parm_block {
   OBJECT * pb_tree;
   short    pb_obj;
   short    pb_prevstate;
   short    pb_currstate;
   short    pb_x, pb_y, pb_w, pb_h;
   short    pb_xc, pb_yc, pb_wc, pb_hc;
   long     pb_parm;
} PACKED PARMBLK;

typedef struct user_block {
        short /*__CDECL*/ (*ub_code)(PARMBLK *parmblock);

        long ub_parm;
}USERBLK;

/* Alternative to USERBLK, as found in Atari Compendium */

typedef struct appl_blk {
   short (*ab_code)(PARMBLK *);
   long  ab_parm;
}APPLBLK;

                                                /* used in RSCREATE.C   */
typedef struct rshdr
{
  short           rsh_vrsn;
  unsigned short  rsh_object;
  unsigned short  rsh_tedinfo;
  unsigned short  rsh_iconblk;    /* list of ICONBLKS             */
  unsigned short  rsh_bitblk;
  unsigned short  rsh_frstr;      
  unsigned short  rsh_string;
  unsigned short  rsh_imdata;     /* image data                   */
  unsigned short  rsh_frimg;      
  unsigned short  rsh_trindex;
  short           rsh_nobs;       /* counts of various structs    */
  short           rsh_ntree;
  short           rsh_nted;
  short           rsh_nib;
  short           rsh_nbb;
  short           rsh_nstring;
  short           rsh_nimages;
  unsigned short  rsh_rssize;     /* total bytes in resource      */
} RSHDR;


#define DESKTOP_HANDLE 0
#define DESK           DESKTOP_HANDLE /* alias */

/* falcon aes menu_popup and menu_attach structure for passing and receiving
 * submenu data.
 */

typedef struct _menu
{
    OBJECT *mn_tree;    /* the object tree of the menu */
    short   mn_menu;    /* the parent object of the menu items */
    short   mn_item;    /* the starting menu item */
    short   mn_scroll;  /* the scroll field status of the menu 
                           0  - The menu will not scroll
                           !0 - The menu will scroll if the number of menu
                                items exceed the menu scroll height. The 
                                non-zero value is the object at which 
                                scrolling will begin.  This will allow one
                                to have a menu in which the scrollable region
                                is only a part of the whole menu.  The value
                                must be a menu item in the menu.
                                
                                menu_settings can be used to change the menu
                                scroll height. 

                         NOTE: If the scroll field status is !0, the menu
                               items must consist entirely of G_STRINGS. */
    short   mn_keystate; /* The CTRL, ALT, SHIFT Key state at the time the
                            mouse button was pressed. */
}MENU_T;

typedef MENU_T MENU;


typedef struct
{
  int     m_out;
  int     m_x;
  int     m_y;
  int     m_w;
  int     m_h;
} MOBLK;

typedef struct {
  short * contrl;
  short * global;
  short * intin;
  short * intout;
  long  * addrin;
  long  * addrout;
}AESPB;

typedef struct _shelw {
  char * newcmd;
  long   psetlimit;
  long   prenice;
  char * defdir;
  char * env;
}SHELW;

extern short _global[];

#define	_AESversion   (_global[0])
#define	_AESnumapps   (_global[1])
#define	_AESapid      (_global[2])
#define	_AESappglobal (*((long *)&_global[3]))
#define	_AESrscfile   ((OBJECT **)(*((long *)&_global[5])))
#define	_AESrshdr     ((RSHDR *)(*((long *)&_global[7])))   /* undocumented feature */
#define	_AESmaxchar   (_global[13])
#define	_AESminchar   (_global[14])

#define gl_ap_version _AESversion

extern void  aes_call (AESPB * aespb);

extern short appl_exit (void);
short
appl_find (char * fname);
int     appl_getinfo (int type, int *out1, int *out2,
                                    int *out3, int *out4);
extern short appl_init (void);
int     appl_read (int ApId, int Length, void *ApPbuff);
int     appl_search (int mode, char *fname, int *type,
                                   int *ap_id);
int     appl_tplay (void *Mem, int Num, int Scale);
int     appl_trecord (void *Mem, int Count);
short
appl_write (short  ap_id,
            short  length,
            void * msg);

int
evnt_button (int clicks,
             int mask,
             int state,
             int * mx,
             int * my,
             int * button,
             int * kstate); 
int
evnt_dclick (int new,
             int flag);
int     evnt_keybd (void);
short
evnt_mesag (short MesagBuf[]);
int     evnt_mouse (int EnterExit, int InX, int InY,
                                  int InW, int InH, int *OutX, int *OutY, 
                                  int *ButtonState, int *KeyState); 
int
evnt_multi (int           Type,
            int           Clicks,
            int           WhichButton,
            int           WhichState,
            int           EnterExit1,
            int           In1X,
            int           In1Y,
            int           In1W,
            int           In1H,
            int           EnterExit2,
            int           In2X, 
            int           In2Y,
            int           In2W,
            int           In2H,
            short         MesagBuf[],
            unsigned long Interval,
            int *         OutX,
            int *         OutY,
            int *         ButtonState,
            int *         KeyState,
            int *         Key,
            int *         ReturnCount);
short
evnt_timer (unsigned long interval);

short
form_alert (short  DefButton,
            char * Str);
int form_button (void *Btree, int Bobject, int Bclicks,
                                  int *Bnxtobj);
int
form_center (OBJECT * tree,
             int *    cx,
             int *    cy,
             int *    cw,
             int *    ch);
short
form_dial (short flag,
           short sx,
           short sy,
           short sw,
           short sh,
           short bx,
           short by,
           short bw,
           short bh);
short
form_do (void * tree,
         short  startobj);
short
form_error (short code);
int form_keybd (void *Ktree, int Kobject, int Kobnext,
                                 int Kchar, int *Knxtobject, int *Knxtchar);

int
fsel_exinput (char *  Path,
              char *  File,
              int *   ExitButton,
              char *  Prompt);
int
fsel_input (char *  path,
            char *  file,
            int *   button);

int
graf_dragbox (int   w,
              int   h,
              int   sx,
              int   sy,
              int   bx,
              int   by,
              int   bw,
              int   bh,
              int * endx,
              int * endy); 
short
graf_growbox (short x1,
              short y1,
              short w1,
              short h1,
              short x2,
              short y2,
              short w2,
              short h2); 

int
graf_handle (int * Wchar,
             int * Hchar,
             int * Wbox, 
             int * Hbox);
int
graf_mkstate (int * mx,
              int * my,
              int * mb,
              int * ks); 
short
graf_mouse (int    Form,
            void * FormAddress);
int
graf_movebox (int   bw,
              int   bh,
              int   sx,
              int   sy,
              int   ex,
              int   ey);
int
graf_rubberbox (int   bx,
                int   by,
                int   minw,
                int   minh,
                int * endw,
                int * endh);
short
graf_shrinkbox (short x1,
                short y1,
                short w1,
                short h1,
                short x2,
                short y2,
                short w2,
                short h2); 
short
graf_slidebox (OBJECT * tree,
               short    parent,
               short    object,
               short    orient); 
int graf_watchbox (void *Tree, int Object, int InState,
                                    int OutState);

int menu_attach (int me_flag, OBJECT *me_tree, int me_item,
                                  MENU_T *me_mdata);
short
menu_bar (void * tree,
          short  mode);
short
menu_icheck (OBJECT * tree,
             short    obj,
             short    flag);
short
menu_ienable (OBJECT * tree,
              short    obj,
              short    flag);
int menu_istart (int me_flag, OBJECT *me_tree,
                                  int me_imenu, int me_item);
int menu_popup (MENU_T *me_menu, int me_xpos, int me_ypos,
                                 MENU_T *me_mdata);
short
menu_register (short  ap_id,
               char * title);
int menu_settings (int me_flag, MN_SET *me_values);
int
menu_text (OBJECT * tree,
           int      obj,
           char *   text);
short
menu_tnormal (OBJECT * tree,
              short    obj,
              short    flag);

int
objc_add (OBJECT * tree,
          int      parent,
          int      child);
short
objc_change (OBJECT * Tree,
             short    Object,
             short    Res,
             short    Cx,
             short    Cy,
             short    Cw,
             short    Ch,
             short    NewState,
             short    Redraw);
int     objc_delete (void *Tree, int Object);
short
objc_draw (OBJECT * tree,
           short    start,
           short    depth,
           short    cx,
           short    cy,
           short    cw,
           short    ch);
int     objc_edit (void *Tree, int Object, int Char, int Index,
                                   int Kind, int *NewIndex); 
short
objc_find (OBJECT * tree,
           short    obj,
           short    depth,
           short    ox,
           short    oy);
int
objc_offset (OBJECT * Tree,
             int      Object,
             int *    X,
             int *    Y);
int
objc_order (OBJECT * tree,
            int      obj,
            int      pos);
int     objc_sysvar (int mode, int which, int in1, int in2, int *out1, int *out2);

int     rsrc_free (void);
int
rsrc_gaddr (int     type,
            int     index,
            void *  addr);
short
rsrc_load (char * fname);
short
rsrc_obfix (OBJECT * tree, int obj);
short
rsrc_rcfix (void * rc_header);
int     rsrc_saddr (int Type, int Index, void *Address);

int     scrp_clear (void);
short
scrp_read (char * cpath);
short
scrp_write (char * cpath);

int
shel_envrn (char ** value,
            char *  name);
short
shel_find (char * buf);
int
shel_get (char * buf,
          int    length);
int
shel_put (char * buf,
          int    length);
int     shel_read (char *Command, char *Tail);
int
shel_write (int    mode,
            int    wisgr,
            int    wiscr,
            char * cmd,
            char * tail);

int
wind_calc (int   request,
           int   kind,
           int   x1,
           int   y1,
           int   w1,
           int   h1,
           int * x2,
           int * y2,
           int * w2,
           int * h2);  
extern short wind_close (short WindowHandle);
extern short wind_create (short Parts,
                          short Wx,
                          short Wy,
                          short Ww,
                          short Wh); 
short
wind_delete (short handle);
int
wind_find (int x,
           int y);
int
wind_get (int   WindowHandle,
          int   What,
          int * W1,
          int * W2,
          int * W3,
          int * W4); 
void    wind_new (void);
extern short wind_open (short WindowHandle,
                        short Wx,
                        short Wy,
                        short Ww,
                        short Wh);
extern short wind_set (short WindowHandle,
                       short What,
                       short parm1,
                       short parm2,
                       short parm3,
                       short parm4);
extern short wind_update (short Code);

void
r_get (GRECT * r,
       int   * x,
       int   * y,
       int   * width,
       int   * height);
void
r_set (GRECT * r,
       int     x,
       int     y,
       int     width,
       int     height);
void
rc_copy (GRECT * src,
         GRECT * dest);
int
rc_equal (GRECT * src,
          GRECT * dest);
short
rc_intersect (GRECT * r1,
              GRECT * r2);
#endif /* _AESBIND_H_ */
