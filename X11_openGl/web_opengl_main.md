Programming OpenGL in Linux: GLX and Xlib

Xlib is a library which provides functions for applications running under the X Window System (also referred to as X). This includes window management as well as event handling. X is a network-oriented system: An application which is running on computer A can send its graphical output to computer B, which is located somewhere else in the network (the network can be a LAN as well as the internet), and can receive events like keyboard or mouse input from computer B. This requires that a program called the "X-Server" is running on both computers. In Linux, the X-server is started with the command startx. You will most probably not have to start the X-server manually, because most Linux Distributions will set up the system to automatically start X after booting.

In the following, a small program framework will be developed, which uses the GLX extension to the X windows system. Only little knowledge of X will be needed; for a comprehensive introduction to X the "Xlib Programming Manual" by Adrian Nye can be recommended.

The program starts with the inclusion of the header files:

```
#include<stdio.h>
#include<stdlib.h>
#include<X11/X.h>
#include<X11/Xlib.h>
#include<GL/gl.h>
#include<GL/glx.h>
#include<GL/glu.h>
```

stdio.h and stdlib.h are included because the functions printf() and exit() will be used. X11/X.h and GL/gl.h are not necessarily needed in the code, because they will be included by X11/Xlib.h and GL/glx.h automatically. They are mentioned for completeness.

We will need the following variables:

```
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
```

Their purpose will be explained below. Since the program should somehow be related to OpenGL, we create a function which uses OpenGL to display something. What about...a quad? With different colors, maybe at each vertex?

```
void DrawAQuad() {
 glClearColor(1.0, 1.0, 1.0, 1.0);
 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

 glMatrixMode(GL_PROJECTION);
 glLoadIdentity();
 glOrtho(-1., 1., -1., 1., 1., 20.);

 glMatrixMode(GL_MODELVIEW);
 glLoadIdentity();
 gluLookAt(0., 0., 10., 0., 0., 0., 0., 1., 0.);

 glBegin(GL_QUADS);
  glColor3f(1., 0., 0.); glVertex3f(-.75, -.75, 0.);
  glColor3f(0., 1., 0.); glVertex3f( .75, -.75, 0.);
  glColor3f(0., 0., 1.); glVertex3f( .75,  .75, 0.);
  glColor3f(1., 1., 0.); glVertex3f(-.75,  .75, 0.);
 glEnd();
} 
```

Afterwards, we define the main program and the first call to Xlib.

```
int main(int argc, char *argv[]) {

 dpy = XOpenDisplay(NULL);
 
 if(dpy == NULL) {
 	printf("\n\tcannot connect to X server\n\n");
        exit(0);
 }
```

The argument to XOpenDisplay() is NULL. This means the graphical output will be sent to the computer on which it is executed.

Next, a handle to the root window is needed.

```
root = DefaultRootWindow(dpy);
```

The root window is the "desktop background" window.

Before we continue, we have to think about what OpenGL capabilities the program needs: The color depth, depth buffer and/or double buffer, stencil buffer etc. Remember the variable att[] (visual attributes) we defined at the beginning: The value GLX_RGBA tells GLX that the color depth shall be true color. The depth buffer shall be 24 bits deep, and a double buffer shall be used. The list is terminated by the value None. More possible options can be found in GL/glx.h. If you want to know about the capabilities that your graphics adapter provides, open a shell and type glxinfo. You will get information about your OpenGL driver and which extensions it supports. At the end of the output all provided visuals are listed, with different properties concerning color depth, depth/double/stencil buffer etc. To select a visual which matches our needs, we call

```
vi = glXChooseVisual(dpy, 0, att);

if(vi == NULL) {
	printf("\n\tno appropriate visual found\n\n");
        exit(0);
} 
else {
	printf("\n\tvisual %p selected\n", (void *)vi->visualid); /* %p creates hexadecimal output like in glxinfo */
}
```

If glXChooseVisual returns with success, the visual's id will be output. If NULL is returned, there is no visual that fulfills your needs. In that case, check the output of glxinfo again. Maybe you have to use a different depth buffer size (GLX_DEPTH_SIZE, 16 instead of GLX_DEPTH_SIZE, 24), or you could even have to remove the GLX_DEPTH_SIZE (or the GLX_DOUBLEBUFFER) entry. This should be considered especially if you want to create programs not only for your computer, but for other ones: You should code your program in a way that it can check a list of different combinations of visual attributes, because the capabilities depend heavily on the hardware.

Now we create a Colormap for the window:

```
cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
```

Then, a structure of the type XSetWindowAttributes has to be initialized. The complete structure definition can be found in X11/Xlib.h, we need only 2 fields to be filled with values:

```
swa.colormap = cmap;
 swa.event_mask = ExposureMask | KeyPressMask;
```

This tells the X server that the colormap we created before shall be used for the window, and the window shall respond to Exposure and KeyPress events (this will be explained later). At this point, the window can be created:

```
win = XCreateWindow(dpy, root, 0, 0, 600, 600, 0, vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
```

The arguments are : The display pointer (dpy), which determines on which display the window shall be created. It is indeed possible to call XOpenDisplay twice (or more often) within a program, each time getting a new Display pointer. This allows the program to send graphical output not only to the local computer, but as well to a remote one.

The handle of the root window is passed as the parent window. Each window- except the root window- has a parent; a window whose parent is the root window is often called top-level window. top-level windows have decorations (title bar, minimize-/maximize-button etc.), which are provided by the window-manager (in linux there are quite a lot of different window managers. some make your windows look like you are running win95 or mac os).

The initial x-/y-position for the window is (0/0). The initial window position always refers to the parent window (which is the root window here). It should be mentioned that these values are ignored by most windows managers, which means that top-level windows maybe placed somewhere else on the desktop. But if you create a child window within a top-level window, these values are used.

The next values are window width and height (600x600 pixels).

The border width is set to 0, like the initial window position, it is meaningless for top-level windows, too.

Then comes the depth, which is defined in the XVisualInfo structure *vi.

The window type is InputOutput. There are other types, but you will probably never need them.

The next parameter is a bitwise-OR of the values CWColormap and CWEventMask. This tells the X server which fields of the XSetWindowAttributes structure swa were filled by the program and should be taken into account when creating the window. Finally, a pointer to the structure itself is passed.

XCreateWindow returns a window id (which is actually just a long int). Now we make the window appear

```
 XMapWindow(dpy, win);
```

and change the string in the title bar:

```
XStoreName(dpy, win, "VERY SIMPLE APPLICATION");
```

One thing is still missing: Since we want to display 3D things with OpenGL, we have to create a GL context and bind it to the window

```
glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
 glXMakeCurrent(dpy, win, glc);
```

The last parameter decides if direct rendering is enabled. If you want to send the graphical output via network, you have to set it to GL_FALSE. If your application puts its output to the computer you are sitting in front of, use GL_TRUE. Note that some capabilities like vertex buffer objects can only be used with a direct gl context (GL_TRUE).

After creating the context, it is made current to our top-level window.

Since we wanted to use depth-buffering (using GLX_DEPTH_SIZE), we should enable the depth test:

```
glEnable(GL_DEPTH_TEST);
```

Remember how we decided which events our application should listen to by setting the swa.event_mask field? Now we have to make the application listen:

```
 while(1) {
 	XNextEvent(dpy, &xev);
        
        if(xev.type == Expose) {
        	XGetWindowAttributes(dpy, win, &gwa);
                glViewport(0, 0, gwa.width, gwa.height);
        	DrawAQuad(); 
                glXSwapBuffers(dpy, win);
        }
                
	else if(xev.type == KeyPress) {
        	glXMakeCurrent(dpy, None, NULL);
 		glXDestroyContext(dpy, glc);
 		XDestroyWindow(dpy, win);
 		XCloseDisplay(dpy);
 		exit(0);
        }
    } /* this closes while(1) { */
} /* this is the } which closes int main(int argc, char *argv[]) { */
```

With while(1) an infinite loop is started. In this loop, XNextEvent is called. This function blocks program execution until one of the events that we allowed (with ExposureMask and KeyPressMask in the XSetWindowAttributes structure) occurs. Note that the program consumes very little cpu time while waiting for the next event.

If an event is received, the XEvent structure xev is filled with information. Since we permitted two kinds of events, we have to check what event it is (with xev.type == ...). In case it is an Exposure Event, we get information about the current window size (XGetWindowAttributes) and resize the viewport, then DrawAQuad and finally swap the buffers (remember that we did choose a double-buffered visual).

Still you wonder what an Exposure Event is: On the desktop, you usually have many windows overlapping each other. If a part of a window- which was occluded by another window before- appears back on the screen, because the window by which it was occluded is minimized, moved or closed, such an event is generated. Another reason for an Exposure Event could be that a window is being resized. In short terms: Exposure Events are generated when the system thinks that the content of a window should be updated.

If a key is pressed (xev.type == KeyPress), the program is terminated. To make it in a clean way, the GL context binding to the window is released (glXMakeCurrent(dpy, None, NULL);), and the GL context is destroyed. Then we kill the window, close the display and exit the program.

You may have noted one thing: We selected swa.event_mask = ExposureMask | KeyPressMask. In the event loop we checked for xev.type == KeyPress, but for xev.type == Expose instead of xev.type == Exposure. The event masks do not always correspond to the event names. You can check this in X11/X.h. Or buy the Xlib Programming Manual...


Now we put all the stuff together:


###Sample Code (C)
```
// -- Written in C -- //

#include<stdio.h>
#include<stdlib.h>
#include<X11/X.h>
#include<X11/Xlib.h>
#include<GL/gl.h>
#include<GL/glx.h>
#include<GL/glu.h>

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
 glOrtho(-1., 1., -1., 1., 1., 20.);

 glMatrixMode(GL_MODELVIEW);
 glLoadIdentity();
 gluLookAt(0., 0., 10., 0., 0., 0., 0., 1., 0.);

 glBegin(GL_QUADS);
  glColor3f(1., 0., 0.); glVertex3f(-.75, -.75, 0.);
  glColor3f(0., 1., 0.); glVertex3f( .75, -.75, 0.);
  glColor3f(0., 0., 1.); glVertex3f( .75,  .75, 0.);
  glColor3f(1., 1., 0.); glVertex3f(-.75,  .75, 0.);
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
	printf("\n\tvisual %p selected\n", (void *)vi->visualid); /* %p creates hexadecimal output like in glxinfo */
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
 	XNextEvent(dpy, &xev);
        
        if(xev.type == Expose) {
        	XGetWindowAttributes(dpy, win, &gwa);
                glViewport(0, 0, gwa.width, gwa.height);
        	DrawAQuad(); 
                glXSwapBuffers(dpy, win);
        }
                
	else if(xev.type == KeyPress) {
        	glXMakeCurrent(dpy, None, NULL);
 		glXDestroyContext(dpy, glc);
 		XDestroyWindow(dpy, win);
 		XCloseDisplay(dpy);
 		exit(0);
        }
    } /* this closes while(1) { */
} /* this is the } which closes int main(int argc, char *argv[]) { */
```



This program is written in plain C (although i, personally, prefer C++ syntax). Save it as quad.c, and compile it with


###compiling flags
```

gcc -o quad quad.c -lX11 -lGL -lGLU


```

The -l flags refer to the Xlib, the OpenGL lib and the GLU lib. An executable named quad is created.

