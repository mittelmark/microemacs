/* tests/xsendkey.c - send Ctrl+G to a window (performance check helper)
 *
 * Some xdotool builds fail to deliver key events (neither XSendEvent nor
 * XTEST reach the client). This sends the XSendEvent directly.
 *
 * build: cc -o perf-xsendkey xsendkey.c -lX11
 * usage: perf-xsendkey <window-id>
 */
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <stdlib.h>

int
main(int argc, char *argv[])
{
    Display *d;
    Window w;
    XKeyEvent e;
    KeyCode kc;

    if (argc != 2)
    {
	fprintf(stderr, "usage: %s <window-id>\n", argv[0]);
	return 2;
    }
    if ((d = XOpenDisplay(NULL)) == NULL)
    {
	fprintf(stderr, "cannot open display\n");
	return 1;
    }
    w = (Window) strtoul(argv[1], NULL, 0);
    kc = XKeysymToKeycode(d, XK_G);
    e.display = d;
    e.window = w;
    e.root = DefaultRootWindow(d);
    e.subwindow = None;
    e.time = CurrentTime;
    e.x = e.y = e.x_root = e.y_root = 1;
    e.same_screen = True;
    e.keycode = kc;
    e.state = ControlMask;
    e.type = KeyPress;
    XSendEvent(d, w, True, KeyPressMask, (XEvent *) &e);
    e.type = KeyRelease;
    XSendEvent(d, w, True, KeyReleaseMask, (XEvent *) &e);
    XFlush(d);
    XCloseDisplay(d);
    return 0;
}
