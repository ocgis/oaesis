#include	"types.h"

WORD	max(WORD a,WORD b);
WORD	min(WORD a,WORD b);

/****************************************************************************
 *  Misc_copy_area                                                          *
 *   Copy one area of the screen to another.
 ****************************************************************************/
void              /*                                                        */
Misc_copy_area(   /*                                                        */
WORD vid,         /* VDI workstation id.                                    */
RECT *dst,        /* Where to the area is to be copied.                     */
RECT *src);       /* The original area.                                     */
/****************************************************************************/

/****************************************************************************
 *  Misc_intersect                                                          *
 *   Get intersection of two rectangles.                                    *
 ****************************************************************************/
WORD              /* 0  Rectangles don't intersect.                         */
                  /* 1  Rectangles intersect but not completely.            */
                  /* 2  r2 is completely covered by r1.                     */
Misc_intersect(   /*                                                        */
RECT *r1,         /* Rectangle r1.                                          */
RECT *r2,         /* Rectangle r2.                                          */
RECT *rinter);    /* Buffer where the intersecting part is returned.        */
/****************************************************************************/

/****************************************************************************
 * Misc_inside                                                              *
 *  Check if coordinates is within rectangle.                               *
 ****************************************************************************/
WORD              /* 0  Outside of rectangle.                               */
                  /* 1  Inside rectangle.                                   */
Misc_inside(      /*                                                        */
RECT *r,          /* Rectangle.                                             */
WORD x,           /* X coordinate.                                          */
WORD y);          /* Y coordinate.                                          */
/****************************************************************************/

/****************************************************************************
 * Misc_setpath                                                             *
 *  Set current working directory. This one is stolen from the Mint-libs    *
 *  and modified because of the idiotic functionality of Dsetpath().        *
 ****************************************************************************/
WORD              /* 0 ok, or -1.                                           */
Misc_setpath(     /*                                                        */
BYTE *dir);       /* New directory.                                         */
/****************************************************************************/

/****************************************************************************
 * Misc_get_loadinfo                                                        *
 *  Get loading information.                                                *
 ****************************************************************************/
void                /*                                                      */
Misc_get_loadinfo(  /*                                                      */
WORD pid,           /*                                                      */
WORD fnamelen,      /* Length of filename buffer.                           */
BYTE *cmdlin,       /* Command line buffer.                                 */
BYTE *fname);       /* File name buffer.                                    */
/****************************************************************************/

/****************************************************************************
 * Misc_Vdi_Malloc                                                          *
 *  Reserve a memory block for use with VDI calls. The server does the      *
 *  actual malloc if VDI calls are tunneled.                                *
 ****************************************************************************/
void *              /* Address of reserved memory.                          */
Misc_Vdi_Malloc(    /*                                                      */
size_t amount);     /* Amount of bytes to reserve.                          */ 
/****************************************************************************/

/****************************************************************************
 * Misc_Vdi_Free                                                            *
 *  Free a memory block reserved by Misc_Vdi_Malloc.                        *
 ****************************************************************************/
void                /*                                                      */
Misc_Vdi_Free(      /*                                                      */
void *address);     /* Pointer to block to free.                            */ 
/****************************************************************************/

