// This is a way to open a window using X11, and draw pixel data (32 bit) into it. 
// It will probably open a window in any linux that supports x11.
#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float real32;
typedef double real64;
#define XA_ATOM ((Atom) 4)

#define VK_ESCAPE 9
#define VK_F4 70


// Stuff for getting the window to be full screen
typedef struct
{
	unsigned long   flags;
	unsigned long   functions;
	unsigned long   decorations;
	long            inputMode;
	unsigned long   status;
} Hints;

int GlobalWidth = 400;
int GlobalHeight = 300;
int GlobalNotFullScreenWidth = GlobalWidth;
int GlobalNotFullScreenHeight = GlobalHeight;
bool GlobalRunning = true;
bool GlobalFullScreen = false;

void v_fullscreen(int i_State, int i_screen, Window x_application_window, Display *h_display)
{
   XEvent x_event;
   Atom wm_fullscreen;

   x_event.type = ClientMessage;
   x_event.xclient.window = x_application_window;
   x_event.xclient.message_type = XInternAtom(h_display, "_NET_WM_STATE", False);
   x_event.xclient.format = 32;
   x_event.xclient.data.l[0] = i_State;  /* 0 = Windowed, 1 = Fullscreen */
   wm_fullscreen = XInternAtom(h_display, "_NET_WM_STATE_FULLSCREEN", False);
   x_event.xclient.data.l[1] = wm_fullscreen;
   x_event.xclient.data.l[2] = wm_fullscreen;
   XSendEvent(h_display, RootWindow(h_display, i_screen), False, ClientMessage, &x_event);
}

int main(void) {
	// stuff for getting window to be full screen
	Hints   hints;
	Atom    property;
	hints.flags = 2;        // Specify that we're changing the window decorations.
	hints.decorations = 0;  // 0 (false) means that window decorations should go bye-bye.
   
	Display *d;
	Window w;
	XEvent e;
	const char *msg = "Hello, World!";
	int s;

	d = XOpenDisplay(NULL);
	if (d == NULL) {
		fprintf(stderr, "Cannot open display\n");
		exit(1);
	}
	property = XInternAtom(d,"_MOTIF_WM_HINTS",True);

	s = DefaultScreen(d);
	w = XCreateSimpleWindow(d, RootWindow(d, s), 10, -10, GlobalWidth, GlobalHeight, 1,
						   BlackPixel(d, s), WhitePixel(d, s));
	
	XWindowAttributes wa = {};
	XGetWindowAttributes(d, w, &wa);

	void * grid_px = malloc(4*GlobalWidth*GlobalHeight);
	memset(grid_px, 122, 4*GlobalWidth*GlobalHeight);

	XImage *image = XCreateImage(d, wa.visual, wa.depth, ZPixmap, 0, (char *)grid_px, GlobalWidth, GlobalHeight, 32, GlobalWidth*4);
	GC gc = XCreateGC(d, w, 0, NULL);
	XPutImage(d, w, gc, image, 0, 0, 0, 0, GlobalWidth, GlobalHeight);

	XSelectInput(d, w, ExposureMask | KeyPressMask);
	XMapWindow(d, w);

	Atom wm_delete_window = XInternAtom(d, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(d, w, &wm_delete_window, 1);
	int XOffset = 0;
	int YOffset = 0;
	
	while (GlobalRunning==true) {
	   //printf("wut\n");
	  //XNextEvent(d, &e);
	  
	  // Take care of all the pending events
	  while(XPending(d)>0) {
		XNextEvent(d, &e);
		//XPeekEvent(d, &e);
		if(e.type == ClientMessage) {
			if((Atom) e.xclient.data.l[0] == wm_delete_window) {
				printf("WM_DELETE_AND_SAVE_YOURSELF\n");
				GlobalRunning = false;
				break;
			}
		}

		if (e.type == Expose) {
			free(grid_px);
			XGetWindowAttributes(d, w, &wa);
			GlobalWidth = wa.width;
			GlobalHeight = wa.height;
			printf("GlobalHeight:+%i\n", GlobalHeight);
			grid_px = malloc(4*GlobalWidth*GlobalHeight);

			//memset(grid_px, 122, 4*GlobalWidth*GlobalHeight);


		}

		if (e.type == KeyPress) {
			int KeyCode = e.xkey.keycode;
			
			if(KeyCode==VK_ESCAPE) {
				GlobalRunning = false;
			}
			if(KeyCode==VK_F4) {
				if(GlobalFullScreen == true) {
					GlobalFullScreen = false;
					hints.decorations = 1;
					XChangeProperty(d,w,property,property,32,PropModeReplace,(unsigned char *)&hints,5);	
					v_fullscreen(0, s, w, d);
					XMapRaised(d,w);
				} else {
					GlobalNotFullScreenHeight = GlobalHeight;
					GlobalNotFullScreenWidth = GlobalWidth;
					Screen *screen = XDefaultScreenOfDisplay(d);
					printf("Width Of Screen: %i", XWidthOfScreen(screen));
					
					// Full screen stuffasaefe
					//XSetWindowAttributes attributes = {};
					//attributes.override_redirect = true;
					//XChangeWindowAttributes(d, w, CWOverrideRedirect, &attributes);
					
					//XSetTransientForHint(d, w, RootWindow(d, s));
					
					//XF86VidModeSwitchToMode(d,screen,video_mode);
					//XF86VidModeSetViewPort(d,screen,0,0);
					//CWOverrideRedirect
					
					v_fullscreen(1, s, w, d);
					
					
					//XMoveResizeWindow(d, w, 0, 0, XWidthOfScreen(screen), XHeightOfScreen(screen));
					
					//XMoveResizeWindow(d, RootWindow(d, s), 0, 0, 400, 400);
					//XMoveResizeWindow(d,w,0,0,width,height);
					XMapRaised(d,w);
					//XGrabPointer(d,w,True,0,GrabModeAsync,GrabModeAsync,w,0L,CurrentTime);
					//XGrabKeyboard(d,w,False,GrabModeAsync,GrabModeAsync,CurrentTime);
					
					GlobalFullScreen = true;
					//hints.decorations = 0;
					//XChangeProperty(d,w,property,property,32,PropModeReplace,(unsigned char *)&hints,5);
				}
			}
			printf("Keycode: %i\n", KeyCode);
			 //GlobalRunning = false;
			 //break;
		} 
	}
	  
	// Draw weird gradient thing that moves and stuff
	XOffset++;
	YOffset++;
	// draw weird gradient
	uint8 *Row = (uint8*)grid_px;
	for(int Y = 0; Y<GlobalHeight; ++Y){
		uint32 *Pixel = (uint32 *)Row;
		for(int X = 0; X < GlobalWidth;
		++X) {
			uint8 Red = (X + XOffset) % 200;
			uint8 Green = (Y + YOffset) % 200;
			uint8 Blue = (X + YOffset) % 200;
			*Pixel++ = ((Red << 16) | (Green << 8) | Blue);
		}
		Row += GlobalWidth*4;
	}
	gc = XCreateGC(d, w, 0, NULL);
	image = XCreateImage(d, wa.visual, wa.depth, ZPixmap, 0, (char *)grid_px, GlobalWidth, GlobalHeight, 32, GlobalWidth*4);
	XPutImage(d, w, gc, image, 0, 0, 0, 0, GlobalWidth, GlobalHeight);
		
	}
	printf("We're done now.\n");

	XCloseDisplay(d);
	return 0;
}