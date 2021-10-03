#include<iostream>
#include<thread>
#include<X11/X.h>
#include<X11/Xlib.h>
#include<GL/gl.h>
#include<GL/glx.h>
#include<GL/glu.h>

using namespace std;

float re = 0.1;

Display                 *dpy;
Window                  root;
GLint                   att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
XVisualInfo             *vi;
Colormap                cmap;
XSetWindowAttributes    swa;
Window                  win;
GLXContext              glc;
XWindowAttributes       gwa;
XEvent                  xev;

void DrawAQuad() {
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 320, 320, 0, -1., 1.);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0., 0., 0., 160., 160., 0., 0., 1., 0.);
	if ( re > 0.5 ) re = 0.1;
	else re = 0.9;
	glBegin(GL_QUADS);
		glColor3f(re, 0., 0.); glVertex3f(10., 20, 0.);
		glColor3f(0., re, 0.); glVertex3f( 10., 20., 0.);
		glColor3f(0., 0., re); glVertex3f( 20,  20, 0.);
		glColor3f(re, re, 0.); glVertex3f( 10,  20, 0.);
	glEnd();
} 
 
int main(int argc, char *argv[]) {	
	dpy = XOpenDisplay(NULL);

	if(dpy == NULL) {
		printf("\n\tcannot connect to X server\n\n");
	    exit(0);
	}
	
	root = DefaultRootWindow(dpy);	
	vi = glXChooseVisual(dpy, 0, att);	
	if(vi == NULL) {
		printf("\n\tno appropriate visual found\n\n");
	    exit(0);
	} 
	else {
		printf("\n\tvisual %p selected\n", (void *)vi->visualid);
		/* %p creates hexadecimal output like in glxinfo */
	}	
	cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);	
	swa.colormap = cmap;
	swa.event_mask = ExposureMask | KeyPressMask;

	win = XCreateWindow(dpy, root, 0, 0, 600, 600, 0, vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);	
	XMapWindow(dpy, win);
	XStoreName(dpy, win, "VERY SIMPLE APPLICATION");

	glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	glXMakeCurrent(dpy, win, glc);

	glEnable(GL_DEPTH_TEST); 

while(1) {
    while(XPending(dpy)) {
        XNextEvent(dpy, &xev);
        
        if(xev.type == Expose) {
            XGetWindowAttributes(dpy, win, &gwa);
            glViewport(0, 0, gwa.width, gwa.height);
        } else if(xev.type == KeyPress) {
            glXMakeCurrent(dpy, None, NULL);
            glXDestroyContext(dpy, glc);
	    XFreeColormap ( dpy, cmap );
            XDestroyWindow(dpy, win);
            XCloseDisplay(dpy);
            //exit(0);
	    return 0;
        }
    }
    
    DrawAQuad();
    glXSwapBuffers(dpy, win);
}

/*	while(1) {
			XNextEvent(dpy, &xev);
	       	DrawAQuad();
	        glXSwapBuffers(dpy, win);
	
	    	if(xev.type == Expose) {
				XGetWindowAttributes(dpy, win, &gwa);
				glViewport(0, 0, gwa.width, gwa.height);
		} else if(xev.type == KeyPress) {
	       		glXMakeCurrent(dpy, None, NULL);
			glXDestroyContext(dpy, glc);
				XDestroyWindow(dpy, win);
				XCloseDisplay(dpy);
				exit(0);
	    	} else {
				DrawAQuad();
	        glXSwapBuffers(dpy, win);
			}
	}
*/
//	glXMakeCurrent(dpy, None, NULL);
//	glXDestroyContext(dpy, glc);
//	XDestroyWindow(dpy, win);
//	XCloseDisplay(dpy);
//	exit(0);
}
