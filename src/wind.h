#ifndef	__WIND__
#define	__WIND__

#include "rlist.h"
#include "types.h"

void Wind_create(AES_PB *apb); /*0x0064*/
void Wind_open(AES_PB *apb);   /*0x0065*/
void Wind_close(AES_PB *apb);  /*0x0066*/
void Wind_delete(AES_PB *apb); /*0x0067*/
void Wind_get(AES_PB *apb);    /*0x0068*/
void Wind_set(AES_PB *apb);    /*0x0069*/
void Wind_find(AES_PB *apb);   /*0x006a*/
void Wind_update(AES_PB *apb); /*0x006b*/
void Wind_calc(AES_PB *apb);   /*0x006c*/

/****************************************************************************
 * Wind_new                                                                 *
 *  0x006d wind_new().                                                      *
 ****************************************************************************/
void              /*                                                        */
Wind_new(         /*                                                        */
AES_PB *apb);     /* AES parameter block.                                   */
/****************************************************************************/

#endif
