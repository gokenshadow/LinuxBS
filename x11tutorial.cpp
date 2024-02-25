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

int GlobalWidth = 400;
int GlobalHeight = 300;

int main(void) {
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

   s = DefaultScreen(d);
   w = XCreateSimpleWindow(d, RootWindow(d, s), 10, 10, GlobalWidth, GlobalHeight, 1,
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

   while (1) {
      XNextEvent(d, &e);
	  if(e.type == ClientMessage) {
		  if((Atom) e.xclient.data.l[0] == wm_delete_window) {
			  printf("WM_DELETE_AND_SAVE_YOURSELF\n");
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
			int XOffset = 0;
			int YOffset = 0;
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
			
		  //memset(grid_px, 122, 4*GlobalWidth*GlobalHeight);
		  gc = XCreateGC(d, w, 0, NULL);
		  image = XCreateImage(d, wa.visual, wa.depth, ZPixmap, 0, (char *)grid_px, GlobalWidth, GlobalHeight, 32, GlobalWidth*4);
		  XPutImage(d, w, gc, image, 0, 0, 0, 0, GlobalWidth, GlobalHeight);
		 
      }
      if (e.type == KeyPress)
         break;
   }
   printf("We're done now.\n");

   XCloseDisplay(d);
   return 0;
}