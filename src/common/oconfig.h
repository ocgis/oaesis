/*****************************************************************************
 * Configuration file for oAESis - a free multitasking AES
 *****************************************************************************/

#ifndef __CONFIG_H__
#define __CONFIG_H__

/* Don't use the server for wind_update calls */
#define DIRECT_WIND_UPDATE

/* Maximum simultaneous applications */
#define MAX_NUM_APPS 32

/* Size of the mouse event buffer */
#define MOUSE_BUFFER_SIZE 100

/* Size of the key event buffer */
#define KEY_BUFFER_SIZE 100

#endif
