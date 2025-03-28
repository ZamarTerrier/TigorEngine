#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include <stdint.h>

#include "key_defines.h"

typedef void (*e_charCallback)(void *, uint32_t);
typedef void (*e_keyCallback)(void *, int , int , int , int );

#include "manager_includes.h"

#include "win_defines.h"
#include "x11_defines.h"

#include <vulkan/vulkan.h>

#define WINPARAM void*


typedef void (*wManagerMonitorFun)(_wManagerMonitor* monitor, int event);

typedef void (*wManagerWindowCloseFun)(wManagerWindow* window);
typedef void (*wManagerCursorEnter)(wManagerWindow* , int32_t);
typedef void (*wManagerCharacterFunc)(wManagerWindow*, uint32_t);
typedef void (*wManagerWindowRefreshFun)(wManagerWindow* window);
typedef void (*wManagerCursorPos)(wManagerWindow* , int32_t, int32_t);
typedef void (*wManagerCharModsFunc)(wManagerWindow*, uint32_t, uint32_t);
typedef void (*wManagerWindowFocusFun)(wManagerWindow* window, int focused);
typedef void (*wManagerWindowiConifyFun)(wManagerWindow* window, int iconified);
typedef void (*wManagerWindowMaximizeFun)(wManagerWindow* window, int maximized);
typedef void (*wManagerMouseButtonFun)(wManagerWindow* , int32_t, int32_t, int32_t);
typedef void (*wManagerCursorPosFun)(wManagerWindow* window, double xpos, double ypos);
typedef void (*wManageWindowPosFun)(wManagerWindow* window, int32_t xpos, int32_t ypos);
typedef void (*wManagerKeyFunc)(wManagerWindow*, uint32_t, uint32_t, uint32_t, uint32_t);
typedef void (*wManagerScrollfFn)(wManagerWindow* window, double xoffset, double yoffset);
typedef void (*wManagerDropFun)(wManagerWindow* window, int path_count, const char* paths[]);
typedef void (*wManagerWindowSizeFunc)(wManagerWindow *window, int32_t width, int32_t height);
typedef void (*wManagerWindowContentScaleFun)(wManagerWindow* window, float xscale, float yscale);
typedef void (*wManagerFrameBufferSizeFun)(wManagerWindow* window, int32_t width, int32_t height);

typedef struct wManagerWindow{

    _wManagerPlatform platform;

    struct wManagerWindow *next;

    struct wManagerWindow *windowListHead;

    wManagerVidmode         videoMode;
    _wManagerMonitor*       monitor;

    _wManagerMonitor**      monitors;
    int                 monitorCount;

    int32_t frameAction;
    int32_t cursorTracked;
    int32_t iconified;
    int32_t maximized;
    int32_t transparent;
    int32_t scaleToMonitor;
    int32_t keymenu;
    int32_t rawMouseMotion;
    int32_t showDefault;
    int32_t decorated;
    int32_t floating;

    int32_t shouldClose;

    int32_t stickyKeys;
    int32_t stickyMouseButtons;
    int32_t lockKeyMods;
    int     cursorMode;
    char    mouseButtons[TIGOR_MOUSE_BUTTON_LAST + 1];
    char    keys[TIGOR_KEY_LAST + 1];

    double virtualCursorPosX, virtualCursorPosY;

    // Cached size used to filter out duplicate events
    int width, height;

    int minwidth, minheight;
    int maxwidth, maxheight;
    int numer, denom;

    WINPARAM WindowData;

    uint32_t isRemoteSession;

    uint64_t time_freq;
    uint64_t time_clock;
    uint64_t time_offset;

    _wManagercontext context;

    _wManagertls   errorSlot;
    _wManagertls   contextSlot;
    _wManagermutex errorLock;

    struct {
            _wManagerinitconfig init;
            _wManagerfbconfig   framebuffer;
            _wManagerwndconfig  window;
            int             refreshRate;
        } hints;

    struct {
            wManageWindowPosFun pos;
            wManagerWindowSizeFunc size;
            wManagerWindowCloseFun close;
            wManagerWindowRefreshFun refresh;
            wManagerWindowFocusFun focus;
            wManagerWindowiConifyFun iconify;
            wManagerWindowMaximizeFun maximize;
            wManagerFrameBufferSizeFun fbsize;
            wManagerWindowContentScaleFun scale;
            wManagerMouseButtonFun mouseButton;
            wManagerCursorPos cursorPos;
            wManagerCursorEnter cursorEnter;
            wManagerScrollfFn scroll;
            wManagerKeyFunc key;
            wManagerCharacterFunc character;
            wManagerCharModsFunc charmods;
            wManagerDropFun drop;

            wManagerMonitorFun  monitor;
        } callbacks;

} wManagerWindow;

typedef struct wManagerInfo{

    struct{
        int32_t        available;
        void*           handle;
        char*           extensions[2];
        PFN_vkGetInstanceProcAddr GetInstanceProcAddr;
        int32_t KHR_surface;
        int32_t KHR_win32_surface;
        int32_t MVK_macos_surface;
        int32_t EXT_metal_surface;
        int32_t KHR_xlib_surface;
        int32_t KHR_xcb_surface;
        int32_t KHR_wayland_surface;
    } vk;
} wManagerInfo;


int wManagerVulkanInit();

int wManagerInit();

int wManagerCreateWindow(wManagerWindow *window, int width, int height, const char* app_name);
void wManagerDestroyWindow(wManagerWindow *window);

int wManagerCreateWindowSurface(VkInstance instance, wManagerWindow *window, const VkAllocationCallbacks* allocator, VkSurfaceKHR *surface);

void wManagerPoolEvents();

uint32_t wManagerWindowIsClosed();

double wManagerGetTime();
void wManagerSetTime(double time);

void wManagerWindowHint(uint32_t hint, uint32_t value);

void wManagerSetCharCallback(wManagerWindow *window, wManagerCharacterFunc EngineCharacterCallback);
void wManagerSetKeyCallback(wManagerWindow *window, wManagerKeyFunc EngineKeyCallback);

int wManagerGetKey(wManagerWindow *window, int button);
int wManagerGetMouseButton(wManagerWindow *window, int key);

void wManagerSetInputMode(wManagerWindow *window, uint32_t mode, uint32_t value);

void wManagerSetMouseButtonCallback(wManagerWindow *window, wManagerMouseButtonFun func);

void wManagerGetCursorPos(wManagerWindow *window, double *xpos, double *ypos);
void wManagerSetCursorPos(wManagerWindow *window, double xpos, double ypos);

void wManagerSetCursorPosCallback(wManagerWindow *window, wManagerCursorPosFun callback);

void wManagerSetClipboardString(wManagerWindow *window, const char* string);
const char* wManagerGetClipboardString(wManagerWindow *window);

void wManagerSetFramebufferSizeCallback(wManagerWindow *window, wManagerFrameBufferSizeFun framebufferResizeCallback);
void wManagerGetFramebufferSize(wManagerWindow *window, int *width, int *height);

const char** wManagerGetRequiredInstanceExtensions(int *counter);

void wManagerWaitEvents();
void WManagerWaitEventsTimeout(double timeout);

void wManagerTerminate();

#endif // WINDOW_MANAGER_H
