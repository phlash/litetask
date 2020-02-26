/* MOUSE.H */

/********************************/
/* COPYRIGHT (c) ASHBYSOFT 1990 */
/********************************
 * $Revision:   1.1  $
 * $Author:   Philip Ashby  $
 * $Date:   24 Sep 1990 14:04:16  $
 *******************************/

/* declarations and defines for interfacing to Microsoft mouse driver */

#define MOUSE_ON   1
#define MOUSE_OFF  2

#define ME_MOVED        0x0001
#define ME_LEFT_DOWN    0x0002
#define ME_LEFT_UP      0x0004
#define ME_RIGHT_DOWN   0x0008
#define ME_RIGHT_UP     0x0010
#define ME_CENTRE_DOWN  0x0020
#define ME_CENTRE_UP    0x0040

typedef struct {
		short events;
		short buttons;
		short x_pos;
		short y_pos;
		} ME_QUEUE_ENTRY;

extern int far cdecl callmouse(int far *, int far *, int far *, void far *);
extern void far cdecl exclude_zone(int, int, int, int);
