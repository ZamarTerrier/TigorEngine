#include "wManager/window_manager.h"
#include "wManager/x11_defines.h"

extern wManagerWindow _wMWindow;

void _wManagerPollMonitorsX11(void)
{

    /*wManagerX11 *wX11 = _wMWindow.WindowData;

    if (wX11->randr.available && !wX11->randr.monitorBroken)
    {
        int disconnectedCount, screenCount = 0;
        _wManagermonitor** disconnected = NULL;
        XineramaScreenInfo* screens = NULL;
        XRRScreenResources* sr = XRRGetScreenResourcesCurrent(wX11->display,
                                                              wX11->root);
        RROutput primary = XRRGetOutputPrimary(wX11->display,
                                               wX11->root);

        if (wX11->xinerama.available)
            screens = XineramaQueryScreens(wX11->display, &screenCount);

        disconnectedCount = _wManager.monitorCount;
        if (disconnectedCount)
        {
            disconnected = _wManager_calloc(_wManager.monitorCount, sizeof(_wManagermonitor*));
            memcpy(disconnected,
                   _wManager.monitors,
                   _wManager.monitorCount * sizeof(_wManagermonitor*));
        }

        for (int i = 0;  i < sr->noutput;  i++)
        {
            int j, type, widthMM, heightMM;

            XRROutputInfo* oi = XRRGetOutputInfo(wX11->display, sr, sr->outputs[i]);
            if (oi->connection != RR_Connected || oi->crtc == None)
            {
                XRRFreeOutputInfo(oi);
                continue;
            }

            for (j = 0;  j < disconnectedCount;  j++)
            {
                if (disconnected[j] &&
                    disconnected[j]->x11.output == sr->outputs[i])
                {
                    disconnected[j] = NULL;
                    break;
                }
            }

            if (j < disconnectedCount)
            {
                XRRFreeOutputInfo(oi);
                continue;
            }

            XRRCrtcInfo* ci = XRRGetCrtcInfo(wX11->display, sr, oi->crtc);
            if (ci->rotation == RR_Rotate_90 || ci->rotation == RR_Rotate_270)
            {
                widthMM  = oi->mm_height;
                heightMM = oi->mm_width;
            }
            else
            {
                widthMM  = oi->mm_width;
                heightMM = oi->mm_height;
            }

            if (widthMM <= 0 || heightMM <= 0)
            {
                // HACK: If RandR does not provide a physical size, assume the
                //       X11 default 96 DPI and calculate from the CRTC viewport
                // NOTE: These members are affected by rotation, unlike the mode
                //       info and output info members
                widthMM  = (int) (ci->width * 25.4f / 96.f);
                heightMM = (int) (ci->height * 25.4f / 96.f);
            }

            _wManagermonitor* monitor = _wManagerAllocMonitor(oi->name, widthMM, heightMM);
            monitor->x11.output = sr->outputs[i];
            monitor->x11.crtc   = oi->crtc;

            for (j = 0;  j < screenCount;  j++)
            {
                if (screens[j].x_org == ci->x &&
                    screens[j].y_org == ci->y &&
                    screens[j].width == ci->width &&
                    screens[j].height == ci->height)
                {
                    monitor->x11.index = j;
                    break;
                }
            }

            if (monitor->x11.output == primary)
                type = _TIGOR_INSERT_FIRST;
            else
                type = _TIGOR_INSERT_LAST;

            _wManagerInputMonitor(monitor, TIGOR_CONNECTED, type);

            XRRFreeOutputInfo(oi);
            XRRFreeCrtcInfo(ci);
        }

        XRRFreeScreenResources(sr);

        if (screens)
            XFree(screens);

        for (int i = 0;  i < disconnectedCount;  i++)
        {
            if (disconnected[i])
                _wManagerInputMonitor(disconnected[i], TIGOR_DISCONNECTED, 0);
        }

        _wManager_free(disconnected);
    }
    else
    {
        const int widthMM = DisplayWidthMM(wX11->display, wX11->screen);
        const int heightMM = DisplayHeightMM(wX11->display, wX11->screen);

        _wManagerInputMonitor(_wManagerAllocMonitor("Display", widthMM, heightMM),
                          TIGOR_CONNECTED,
                          _TIGOR_INSERT_FIRST);
    }*/
}