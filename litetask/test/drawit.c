#include "litetask.h"
#include "xlib.h"
#include "..\x11\xlib\colors.h"

/* Debugging */
#define DBG(str, wait)  if(debug) {        \
									printk(str);    \
									if(wait) {      \
										while(!kbhit());\
											getch();     \
									}               \
								}
int debug = 0;

/* colour map manipulation */
#define RED          0
#define GREEN        1
#define BLUE         2

/* Mouse constants */
#define MOUSE_ON     1
#define MOUSE_OFF    0
extern int far callmouse(int far *, int far *, int far *, int far *);

/* Stack size for LiteTask */
int mainStackSize = MINSTACK;

/* prototypes */
void far mainTask(void);
int near setup_mouse(void);
void near remove_mouse(void);
int near show_mouse(int);
void near display_colours(int curr_colour);
void near changeColour(int curr_colour, int rgb, int offset);
int near selectColour(int key, int curr_colour);
void near setXColour(unsigned long pixel, unsigned long rgb);
int near kbhit(void);
int near getch(void);

/* local colour map */
#define N_PIXELS  128
static long pixels[N_PIXELS];
static long rgbs[N_PIXELS];
static long planes[1];

/* X Window stuff */
static Display far *dpy;
static Window win;
static GC gc;

/* Code */
void far mainTask(void)
{
int i, key, ax, buttons, x_pos, y_pos;

int curr_b = 0, curr_x = 0, curr_y = 0, curr_colour = 15;
int box_x, box_y, boxing=0;

/* set up LiteTask */
	setPreEmptive(0);

/* set up the mouse */
	if(setup_mouse())
		return;

/* Open the display */
	InstallXWindows();
	dpy = XOpenDisplay("VGA");

/* Set X variables */
	win = RootWindow(dpy, 0);
	gc = DefaultGC(dpy, 0);

/* Allocate N_PIXEL/2 colour cells with 1 overlay plane (= N_PIXELS cells) */
	if(!XAllocColorCells(dpy, DefaultColormap(dpy, 0), True,
		planes, 1, pixels, N_PIXELS/2))
	{
		printk("Unable to allocate required colors\r\n");
		XCloseDisplay(dpy);
		RemoveXWindows();
		return;
	}

/* set initial colors */
	setXColour(pixels[0], rgbs[0] = _BLACK);
	setXColour(pixels[1], rgbs[1] = _BLUE);
	setXColour(pixels[2], rgbs[2] = _GREEN);
	setXColour(pixels[3], rgbs[3] = _CYAN);
	setXColour(pixels[4], rgbs[4] = _RED);
	setXColour(pixels[5], rgbs[5] = _MAGENTA);
	setXColour(pixels[6], rgbs[6] = _BROWN);
	setXColour(pixels[7], rgbs[7] = _LIGHTGRAY);
	setXColour(pixels[8], rgbs[8] = _GRAY);
	setXColour(pixels[9], rgbs[9] = _LIGHTBLUE);
	setXColour(pixels[10], rgbs[10] = _LIGHTGREEN);
	setXColour(pixels[11], rgbs[11] = _LIGHTCYAN);
	setXColour(pixels[12], rgbs[12] = _LIGHTRED);
	setXColour(pixels[13], rgbs[13] = _LIGHTMAGENTA);
	setXColour(pixels[14], rgbs[14] = _LIGHTYELLOW);
	setXColour(pixels[15], rgbs[15] = _WHITE);
	for(i=0; i<N_PIXELS/8; i++)
		setXColour(pixels[i+16],
					  rgbs[i+16] = ((long)((512/N_PIXELS)*i)) |
					  ((long)((512/N_PIXELS)*((N_PIXELS/8-1)-i)) << 8) |
					  0x3f0000L);
	for(i=0; i<N_PIXELS/8; i++)
		setXColour(pixels[i+16+N_PIXELS/8],
					  rgbs[i+16+N_PIXELS/8] = 0x00003fL |
					  ((long)((512/N_PIXELS)*i) << 8) |
					  ((long)((512/N_PIXELS)*((N_PIXELS/8-1)-i)) << 16));
	for(i=0; i<N_PIXELS/8; i++)
		setXColour(pixels[i+16+2*N_PIXELS/8],
					  rgbs[i+16+2*N_PIXELS/8] = 
					  ((long)((512/N_PIXELS)*((N_PIXELS/8-1)-i))) |
					  0x003f00L |
					  ((long)((512/N_PIXELS)*i) << 16));
	for(i=16+3*N_PIXELS/8; i<N_PIXELS/2; i++)
		setXColour(pixels[i], rgbs[i] = _BLACK);

/* set overlay plane colours */
	for(i=0; i<N_PIXELS/2; i++)
		setXColour(planes[0] | pixels[i], (~rgbs[i]) & _WHITE);

/* display current colors */
	display_colours(curr_colour);
	DBG("Screen setup\r\n", 0);

/* display mouse on screen */
	show_mouse(MOUSE_ON);
	
/* main event loop */
	key = 0;
	while(key != 27)
	{
	/* check for keypress event */
		if(kbhit())
		{
		/* process keyboard event */
			DBG("Key pressed\r\n", 0);
			show_mouse(MOUSE_OFF);
			switch(key=getch())
			{
			case 27:             /* ESC */
				break;
	
			case '0':
				setXColour(pixels[curr_colour], _BLACK);
				setXColour(planes[0] | pixels[curr_colour], _WHITE);
				display_colours(curr_colour);
				break;

			case 'r':
				changeColour(curr_colour, RED, 1);
				display_colours(curr_colour);
				break;

			case 'R':
				changeColour(curr_colour, RED, -1);
				display_colours(curr_colour);
				break;

			case 'g':
				changeColour(curr_colour, GREEN, 1);
				display_colours(curr_colour);
				break;

			case 'G':
				changeColour(curr_colour, GREEN, -1);
				display_colours(curr_colour);
				break;

			case 'b':
				changeColour(curr_colour, BLUE, 1);
				display_colours(curr_colour);
				break;

			case 'B':
				changeColour(curr_colour, BLUE, -1);
				  display_colours(curr_colour);
				break;

			case 0:
				curr_colour = selectColour(getch(), curr_colour);
				break;

			default:
				break;
			}
			show_mouse(MOUSE_ON);
		}
  
	/* read mouse position/status */
		ax = 3;
		callmouse(&ax, &buttons, &x_pos, &y_pos);

	/* has anything changed? */
		if(curr_b == buttons && curr_x == x_pos/2 && curr_y == y_pos)
			continue;
	
	/* if so, process the change */
		show_mouse(MOUSE_OFF);
		if(buttons == 0)
		{
		/* no buttons pressed, check for end of boxing */
			if(boxing)
			{
				boxing = 0;
				XSetForeground(dpy, gc, BlackPixel(dpy, 0));
				XSetPlaneMask(dpy, gc, planes[0]);
				XDrawRectangle(dpy, win, gc, box_x, box_y,
					curr_x-box_x, curr_y-box_y);
				XSetForeground(dpy, gc, pixels[curr_colour]);
				XSetPlaneMask(dpy, gc, AllPlanes);
				XFillRectangle(dpy, win, gc, box_x, box_y,
					x_pos/2-box_x, y_pos-box_y);
			}
		}
		else if(buttons == 1)
		{
		/* left button pressed */
			if((y_pos < 9) && (x_pos/2 < N_PIXELS+1))
			{
				curr_colour = (x_pos/2 - 1)/4 + ((y_pos - 1)/4)*(N_PIXELS/4);
				display_colours(curr_colour);
			}
			else
			{
				XSetForeground(dpy, gc, pixels[curr_colour]);
				XSetPlaneMask(dpy, gc, AllPlanes);
				XDrawLine(dpy, win, gc, curr_x, curr_y,
					x_pos/2, y_pos);
			}
		}
		else if(buttons == 2)
		{
		/* right button pressed, track box */
			if(boxing)
			{
				if((x_pos/2 != curr_x) || (y_pos != curr_y))
				{
					XSetPlaneMask(dpy, gc, planes[0]);
					XSetForeground(dpy, gc, BlackPixel(dpy, 0));
					XDrawRectangle(dpy, win, gc, box_x, box_y,
						curr_x-box_x, curr_y-box_y);
					XSetForeground(dpy, gc, WhitePixel(dpy, 0));
					XDrawRectangle(dpy, win, gc, box_x, box_y,
						x_pos/2-box_x, y_pos-box_y);
				}
			}
			else
			{
				boxing = 1;
				box_x = x_pos/2;
				box_y = y_pos;
			}
		}
		else if(buttons == 3)
		{
		/* both buttons pressed */
			XSetPlaneMask(dpy, gc, AllPlanes);
			XSetForeground(dpy, gc, BlackPixel(dpy, 0));
			XFillRectangle(dpy, win, gc, 0, 0, 319, 199);
			display_colours(curr_colour);
		}
		else if(buttons == 4)
		{
		/* middle button pressed */
			DBG("Debug off\r\n", 0);
			debug = 1-debug;
			DBG("Debug on\r\n", 0);
		}

		/* update current mouse status */
		curr_b = buttons;
		curr_x = x_pos/2;
		curr_y = y_pos;
		show_mouse(MOUSE_ON);

	} /* while key != 27 */

/* clear mouse pointer from screen */
	remove_mouse();

/* restore video mode */
	XCloseDisplay(dpy);
	RemoveXWindows();
	return;

} /* main */

int setup_mouse(void)
{
int ax;

/* test for mouse driver present */
	ax = 0;
	if(callmouse(&ax, &ax, &ax, &ax) == 0)
	{
		printk("No mouse driver installed !\r\n");
		return 1;
	}
	return 0;
}

void remove_mouse(void)
{
int ax;

	ax = 0;
	callmouse(&ax, &ax, &ax, &ax);
}

int show_mouse(int state)
{
int ax;

/* select action */
	switch(state)
	{
	case MOUSE_ON:
	/* display mouse */
		ax = 1;
		return callmouse(&ax, &ax, &ax, &ax);
	case MOUSE_OFF:
	/* hide mouse */
		ax = 2;
		return callmouse(&ax, &ax, &ax, &ax);
	default:
		return -1;
	}
} /* show_mouse */

void display_colours(int curr_colour)
{
int x, y, colour=0;

/* draw N_PIXELS/2 small boxes one in each colour */
	XSetPlaneMask(dpy, gc, AllPlanes);
	for(y=0; y<2; y++)
		for(x=0; x<N_PIXELS/4; x++)
		{
			XSetForeground(dpy, gc, pixels[colour]);
			XFillRectangle(dpy, win, gc, 4*x+1, 4*y+1, 4, 4);

		/* Put a reverse colour rectangle around the selected colour */
			if(colour == curr_colour)
			{
				XSetForeground(dpy, gc, planes[0] | pixels[colour]);
				XDrawRectangle(dpy, win, gc, 4*x+1, 4*y+1, 4, 4);
			}
			colour++;
		}
		XSetForeground(dpy, gc, WhitePixel(dpy, 0));
		XDrawRectangle(dpy, win, gc, 0, 0, N_PIXELS+1, 9);
}

void changeColour(int curr_colour, int rgb, int offset)
{
long temp1, temp2, mask;
int shift;

/* add offset to the selected R,G or B value of the current colour */
	switch(rgb)
	{
	case RED:
		mask = 0x00ffff00L;
		shift = 0;
		break;
	case GREEN:
		mask = 0x00ff00ffL;
		shift = 8;
		break;
	case BLUE:
		mask = 0x0000ffffL;
		shift = 16;
		break;
	default:
		return;
	}
	temp1 = rgbs[curr_colour] & mask;
	temp2 = (rgbs[curr_colour] >> shift) & 0x000000ffL;
	temp2 = (temp2 + (long)offset) & 0x0000003fL;
	rgbs[curr_colour] = temp1 | (temp2 << shift);
	setXColour(pixels[curr_colour], rgbs[curr_colour]);
	setXColour(planes[0] | pixels[curr_colour], ~(rgbs[curr_colour]) & _WHITE);
}

int selectColour(int key, int curr_colour)
{
int old_colour = curr_colour;

/* Choose next colour */
	switch(key)
	{
	case 72: /* up arrow */
	/* ignore if in first row */
		if(curr_colour < N_PIXELS/4)
			return curr_colour;
		curr_colour -= N_PIXELS/4;
		break;

	case 80: /* down arrow */
	/* ignore if in last row */
		if(curr_colour > N_PIXELS/4-1)
			return curr_colour;
		curr_colour += N_PIXELS/4;
		break;

	case 75: /* left arrow */
	/* ignore if in first column */
		if((curr_colour % N_PIXELS/4) == 0)
			return curr_colour;
		curr_colour -= 1;
		break;

	case 77: /* right arrow */
	/* ignore if in last column */
		if((curr_colour % N_PIXELS/4) == N_PIXELS/4-1)
			return curr_colour;
		curr_colour += 1;
		break;

	default:
		return curr_colour;
	}

/* select on colour bar */
	XSetPlaneMask(dpy, gc, AllPlanes);
	XSetForeground(dpy, gc, pixels[old_colour]);
	XDrawRectangle(dpy, win, gc,
						4*(old_colour % (N_PIXELS/4))+1,
						4*(old_colour / (N_PIXELS/4))+1,
						4, 4);
	XSetForeground(dpy, gc, planes[0] | pixels[curr_colour]);
	XDrawRectangle(dpy, win, gc,
						4*(curr_colour % (N_PIXELS/4))+1, 
						4*(curr_colour / N_PIXELS/4)+1,
						4, 4);
	return curr_colour;
}

void setXColour(unsigned long pixel, unsigned long rgb)
{
XColor xc;

/* Set up the XColor structure, and store the new color */
	xc.pixel = pixel;
	xc.rgb = rgb;
	XStoreColor(dpy, DefaultColormap(dpy, 0), xc);
}

int kbhit(void)
{
union REGS regs;

/* poll for a key */
	regs.h.ah = 1;
	int86(0x16, &regs);
	if(regs.x.flags & 0x40)                   /* Zero flag */
		return 0;
	return 1;
}

int getch(void)
{
union REGS regs;
int rv;
static int extended = 0;

/* See if we have an extended key waiting */
	if(extended)
	{
		rv = extended;
		extended = 0;
		return rv;
	}

/* Read a key from BIOS */
	regs.h.ah = 0;
	int86(0x16, &regs);
	if(regs.h.al == 0)
		extended = regs.h.ah;
	return regs.h.al;
}

