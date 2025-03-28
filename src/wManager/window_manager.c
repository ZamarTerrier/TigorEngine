#include "wManager/window_manager.h"
#include "wManager/input_manager.h"

#include "Core/e_window.h"

#include <stdbool.h>

#define USER_DEFAULT_SCREEN_DPI 96

wManagerWindow _wMWindow;
extern wManagerInfo _wMInfo;

extern TEngine engine;

#include "Data/e_resource_engine.h"

// Internal key state used for sticky keys
#define _TIGOR_STICK 3

extern const char* _wManagerGetVulkanResultString(VkResult result);
extern int wManagerVulkanInit();

extern void enableRawMouseMotion(wManagerWindow* window);
extern void disableRawMouseMotion(wManagerWindow* window);

extern void releaseCursor(wManagerWindow *window);
extern void enableCursor(wManagerWindow *window);

extern void _wManagerPlatformInitTimer(void);
extern uint64_t _wManagerPlatformGetTimerValue(void);
extern uint64_t _wManagerPlatformGetTimerFrequency(void);

extern void createKeyTables(void* wData);

extern void _wManagerCenterCursorInContentArea(wManagerWindow* window);

void wManagerDefaultWindowHints(wManagerWindow *window)
{
    // The default is a focused, visible, resizable window with decorations
    memset(&window->hints.window, 0, sizeof(window->hints.window));
    window->hints.window.resizable    = true;
    window->hints.window.visible      = true;
    window->hints.window.decorated    = true;
    window->hints.window.focused      = true;
    window->hints.window.autoIconify  = true;
    window->hints.window.centerCursor = true;
    window->hints.window.focusOnShow  = true;
    window->hints.window.xpos         = TIGOR_ANY_POSITION;
    window->hints.window.ypos         = TIGOR_ANY_POSITION;

    // The default is 24 bits of color, 24 bits of depth and 8 bits of stencil,
    // double buffered
    memset(&window->hints.framebuffer, 0, sizeof(window->hints.framebuffer));
    window->hints.framebuffer.redBits      = 8;
    window->hints.framebuffer.greenBits    = 8;
    window->hints.framebuffer.blueBits     = 8;
    window->hints.framebuffer.alphaBits    = 8;
    window->hints.framebuffer.depthBits    = 24;
    window->hints.framebuffer.stencilBits  = 8;
    window->hints.framebuffer.doublebuffer = true;

    // The default is to select the highest available refresh rate
    window->hints.refreshRate = TIGOR_DONT_CARE;

    // The default is to use full Retina resolution framebuffers
    window->hints.window.ns.retina = true;

    window->cursorMode = TIGOR_CURSOR_NORMAL;
}

double wManagerGetTime()
{
    return (double)(_wManagerPlatformGetTimerValue() - _wMWindow.time_offset)  /
            _wManagerPlatformGetTimerFrequency();
}

void wManagerSetTime(double time)
{
    if ((time != time) || time < 0.0 || (time > 18446744073.0))
    {
        printf("Invalid time %f\n", time);
        return;
    }

    _wMWindow.time_offset = _wManagerPlatformGetTimerValue() -
            (uint64_t) (time * _wManagerPlatformGetTimerFrequency());
}

int wManagerInit(){
    TWindow *window = (TWindow *)engine.window;

    window->e_window = calloc(1, sizeof(wManagerWindow));
    wManagerWindow *e_window = window->e_window;

    memset(&_wMInfo, 0, sizeof(wManagerInfo));

    #ifdef _X11_

        e_window->WindowData = calloc(1, sizeof(wManagerX11));        
        _wMWindow.WindowData = calloc(1, sizeof(wManagerX11));

        extern uint32_t _wManagerConnectX11(_wManagerPlatform* platform);

        _wManagerConnectX11(&_wMWindow.platform);
        _wManagerConnectX11(&e_window->platform);
    #elif _WIN_


        e_window->WindowData = calloc(1, sizeof(wManagerWin));
        _wMWindow.WindowData = calloc(1, sizeof(wManagerWin));

        extern int32_t _wManagerConnectWin32(_wManagerPlatform* platform);

        _wManagerConnectWin32(&_wMWindow.platform);
        _wManagerConnectWin32(&e_window->platform);
    #endif


    if (!_wMWindow.platform.init())
    {
        wManagerTerminate();
        return false;
    }

    _wManagerPlatformInitTimer();
    _wMWindow.time_offset = _wManagerPlatformGetTimerValue();

    wManagerDefaultWindowHints(&_wMWindow);
    wManagerDefaultWindowHints(window->e_window);

    createKeyTables(_wMWindow.WindowData);
    createKeyTables(e_window->WindowData);

    return true;
}

uint32_t wManagerWindowIsClosed(){

    return _wMWindow.shouldClose;
}

void wManagerSetCharCallback(wManagerWindow *window, wManagerCharacterFunc EngineCharacterCallback)
{
    window->callbacks.character = EngineCharacterCallback;
}

void wManagerSetKeyCallback(wManagerWindow *window, wManagerKeyFunc EngineKeyCallback)
{
    window->callbacks.key = EngineKeyCallback;
}

void wManagerSetInputMode(wManagerWindow *window, uint32_t mode, uint32_t value)
{

       switch (mode)
       {
           case TIGOR_CURSOR:
           {
               if (value != TIGOR_CURSOR_NORMAL &&
                   value != TIGOR_CURSOR_HIDDEN &&
                   value != TIGOR_CURSOR_DISABLED &&
                   value != TIGOR_CURSOR_CAPTURED)
               {
                   printf("Invalid cursor mode 0x%08X",
                                   value);
                   return;
               }

               if (window->cursorMode == value)
                   return;

               window->cursorMode = value;

               window->platform.getCursorPos(window, &window->virtualCursorPosX,
                                            &window->virtualCursorPosY);
               window->platform.setCursorMode(window, value);
               return;
           }

           case TIGOR_STICKY_KEYS:
           {
               value = value ? true : false;
               if (window->stickyKeys == value)
                   return;

               if (!value)
               {
                   int i;

                   // Release all sticky keys
                   for (i = 0;  i <= TIGOR_KEY_LAST;  i++)
                   {
                       if (window->keys[i] == _TIGOR_STICK)
                           window->keys[i] = TIGOR_RELEASE;
                   }
               }

               window->stickyKeys = value;
               return;
           }

           case TIGOR_STICKY_MOUSE_BUTTONS:
           {
               value = value ? true : false;
               if (window->stickyMouseButtons == value)
                   return;

               if (!value)
               {
                   int i;

                   // Release all sticky mouse buttons
                   for (i = 0;  i <= TIGOR_MOUSE_BUTTON_LAST;  i++)
                   {
                       if (window->mouseButtons[i] == _TIGOR_STICK)
                           window->mouseButtons[i] = TIGOR_RELEASE;
                   }
               }

               window->stickyMouseButtons = value;
               return;
           }

           case TIGOR_LOCK_KEY_MODS:
           {
               window->lockKeyMods = value ? true : false;
               return;
           }

           case TIGOR_RAW_MOUSE_MOTION:
           {
               value = value ? true : false;
               if (window->rawMouseMotion == value)
                   return;

               window->rawMouseMotion = value;
               window->platform.setRawMouseMotion(window, value);
               return;
           }
       }

       printf("Invalid input mode 0x%08X\n", mode);
}

int wManagerGetMouseButton(wManagerWindow *window, int button)
{
    if (button < TIGOR_MOUSE_BUTTON_1 || (button > TIGOR_MOUSE_BUTTON_LAST))
    {
        printf("Invalid mouse button %i\n", button);
        return TIGOR_RELEASE;
    }

    if (window->mouseButtons[button] == _TIGOR_STICK)
    {
        // Sticky mode: release mouse button now
        window->mouseButtons[button] = TIGOR_RELEASE;
        return TIGOR_PRESS;
    }

    return (int) window->mouseButtons[button];
}

int wManagerGetKey(wManagerWindow *window, int key)
{

    if (key < TIGOR_KEY_SPACE || (key > TIGOR_KEY_LAST))
    {
        printf("Invalid key %i\n", key);
        return TIGOR_RELEASE;
    }

    if (window->keys[key] == _TIGOR_STICK)
    {
        // Sticky mode: release key now
        window->keys[key] = TIGOR_RELEASE;
        return TIGOR_PRESS;
    }

    return (int) window->keys[key];
}

void wManagerSetClipboardString(wManagerWindow *window, const char *string)
{
    return _wMWindow.platform.setClipboardString(string);
}

const char *wManagerGetClipboardString(wManagerWindow *window)
{
    return _wMWindow.platform.getClipboardString();
}

void wManagerSetMouseButtonCallback(wManagerWindow *window, wManagerMouseButtonFun func)
{
    window->callbacks.mouseButton = func;
}

void wManagerSetCursorPosCallback(wManagerWindow *window, wManagerCursorPosFun callback)
{
    window->callbacks.cursorPos = (wManagerCursorPos)callback;
}

void wManagerGetFramebufferSize(wManagerWindow *window, int *width, int *height)
{
    window->platform.getWindowSize(window, width, height);
}

void wManagerSetFramebufferSizeCallback(wManagerWindow *window, wManagerFrameBufferSizeFun framebufferResizeCallback)
{
    window->callbacks.fbsize = framebufferResizeCallback;
}

void wManagerWaitEvents()
{
    _wMWindow.platform.waitEvents();
}

void wManagerTerminate()
{
    TWindow *window = (TWindow *)engine.window;
    wManagerWindow *e_window = window->e_window;

    free(e_window->WindowData);
    free(_wMWindow.WindowData);

    free(window->e_window);
}

const char **wManagerGetRequiredInstanceExtensions(int *counter)
{
    *counter = 0;

    if (!wManagerVulkanInit())
        return NULL;

    if (!_wMInfo.vk.extensions[0])
        return NULL;

    *counter = 2;
    return (const char**) _wMInfo.vk.extensions;
}

void wManagerWindowHint(uint32_t hint, uint32_t value)
{

    switch (hint)
    {
        case TIGOR_RED_BITS:
            _wMWindow.hints.framebuffer.redBits = value;
            return;
        case TIGOR_GREEN_BITS:
            _wMWindow.hints.framebuffer.greenBits = value;
            return;
        case TIGOR_BLUE_BITS:
            _wMWindow.hints.framebuffer.blueBits = value;
            return;
        case TIGOR_ALPHA_BITS:
            _wMWindow.hints.framebuffer.alphaBits = value;
            return;
        case TIGOR_DEPTH_BITS:
            _wMWindow.hints.framebuffer.depthBits = value;
            return;
        case TIGOR_STENCIL_BITS:
            _wMWindow.hints.framebuffer.stencilBits = value;
            return;
        case TIGOR_ACCUM_RED_BITS:
            _wMWindow.hints.framebuffer.accumRedBits = value;
            return;
        case TIGOR_ACCUM_GREEN_BITS:
            _wMWindow.hints.framebuffer.accumGreenBits = value;
            return;
        case TIGOR_ACCUM_BLUE_BITS:
            _wMWindow.hints.framebuffer.accumBlueBits = value;
            return;
        case TIGOR_ACCUM_ALPHA_BITS:
            _wMWindow.hints.framebuffer.accumAlphaBits = value;
            return;
        case TIGOR_AUX_BUFFERS:
            _wMWindow.hints.framebuffer.auxBuffers = value;
            return;
        case TIGOR_STEREO:
            _wMWindow.hints.framebuffer.stereo = value ? true : false;
            return;
        case TIGOR_DOUBLEBUFFER:
            _wMWindow.hints.framebuffer.doublebuffer = value ? true : false;
            return;
        case TIGOR_TRANSPARENT_FRAMEBUFFER:
            _wMWindow.hints.framebuffer.transparent = value ? true : false;
            return;
        case TIGOR_SAMPLES:
            _wMWindow.hints.framebuffer.samples = value;
            return;
        case TIGOR_SRGB_CAPABLE:
            _wMWindow.hints.framebuffer.sRGB = value ? true : false;
            return;
        case TIGOR_RESIZABLE:
            _wMWindow.hints.window.resizable = value ? true : false;
            return;
        case TIGOR_DECORATED:
            _wMWindow.hints.window.decorated = value ? true : false;
            return;
        case TIGOR_FOCUSED:
            _wMWindow.hints.window.focused = value ? true : false;
            return;
        case TIGOR_AUTO_ICONIFY:
            _wMWindow.hints.window.autoIconify = value ? true : false;
            return;
        case TIGOR_FLOATING:
            _wMWindow.hints.window.floating = value ? true : false;
            return;
        case TIGOR_MAXIMIZED:
            _wMWindow.hints.window.maximized = value ? true : false;
            return;
        case TIGOR_VISIBLE:
            _wMWindow.hints.window.visible = value ? true : false;
            return;
        case TIGOR_POSITION_X:
            _wMWindow.hints.window.xpos = value;
            return;
        case TIGOR_POSITION_Y:
            _wMWindow.hints.window.ypos = value;
            return;
        case TIGOR_COCOA_RETINA_FRAMEBUFFER:
            _wMWindow.hints.window.ns.retina = value ? true : false;
            return;
        case TIGOR_WIN32_KEYBOARD_MENU:
            _wMWindow.hints.window.win32.keymenu = value ? true : false;
            return;
        case TIGOR_SCALE_TO_MONITOR:
            _wMWindow.hints.window.scaleToMonitor = value ? true : false;
            return;
        case TIGOR_CENTER_CURSOR:
            _wMWindow.hints.window.centerCursor = value ? true : false;
            return;
        case TIGOR_FOCUS_ON_SHOW:
            _wMWindow.hints.window.focusOnShow = value ? true : false;
            return;
        case TIGOR_MOUSE_PASSTHROUGH:
            _wMWindow.hints.window.mousePassthrough = value ? true : false;
            return;
        case TIGOR_REFRESH_RATE:
            _wMWindow.hints.refreshRate = value;
            return;
    }

    printf("Invalid window hint 0x%08X", hint);
}

extern void* _wManagerPlatformGetTls(_wManagertls* tls);

void wManagerDestroyWindow(wManagerWindow* window)
{
    // Allow closing of NULL (to match the behavior of free)
    if (window == NULL)
        return;

    // Clear all callbacks to avoid exposing a half torn-down window object
    memset(&window->callbacks, 0, sizeof(window->callbacks));

    // The window's context must not be current on another thread when the
    // window is destroyed
    /*if (window == _wManagerPlatformGetTls(&_wMWindow.contextSlot))
        wManagerMakeContextCurrent(NULL);*/

    _wMWindow.platform.destroyWindow(window);

    // Unlink window from global linked list
    /*{
        _wManagerWindow** prev = &_wMWindow.windowListHead;

        while (*prev != window)
            prev = &((*prev)->next);

        *prev = window->next;
    }*/
}

int wManagerCreateWindow(wManagerWindow *window, int width, int height, const char *app_name)
{
    _wManagerfbconfig  fbconfig;
    _wManagerwndconfig wndconfig;

    fbconfig = _wMWindow.hints.framebuffer;
    wndconfig = _wMWindow.hints.window;

    wndconfig.width = width;
    wndconfig.height = height;
    wndconfig.title = app_name;

    if (!_wMWindow.platform.createWindow(window, &wndconfig, &fbconfig))
    {
        wManagerDestroyWindow((wManagerWindow*) window);
        return 0;
    }
}

int wManagerCreateWindowSurface(VkInstance instance, wManagerWindow *window, const VkAllocationCallbacks* allocator, VkSurfaceKHR *surface)
{
    *surface = VK_NULL_HANDLE;

    if (!wManagerVulkanInit())
        return VK_ERROR_INITIALIZATION_FAILED;

    if (!_wMInfo.vk.extensions[0])
    {
        printf("Vulkan: Window surface creation extensions not found\n");
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    if (window->context.client != TIGOR_NO_API)
    {
        printf("Vulkan: Window surface creation requires the window to have the client API set to TIGOR_NO_API\n");
        return VK_ERROR_NATIVE_WINDOW_IN_USE_KHR;
    }

    return _wMWindow.platform.createWindowSurface(instance, window, allocator, surface);//_wManagerCreateWindowSurfaceWin32(instance, window, allocator, surface);
}

char** _wManagerParseUriList(char* text, int* count)
{
    const char* prefix = "file://";
    char** paths = NULL;
    char* line;

    *count = 0;

    while ((line = strtok(text, "\r\n")))
    {
        char* path;

        text = NULL;

        if (line[0] == '#')
            continue;

        if (strncmp(line, prefix, strlen(prefix)) == 0)
        {
            line += strlen(prefix);
            // TODO: Validate hostname
            while (*line != '/')
                line++;
        }

        (*count)++;

        path = calloc(strlen(line) + 1, 1);
        paths = realloc(paths, *count * sizeof(char*));
        paths[*count - 1] = path;

        while (*line)
        {
            if (line[0] == '%' && line[1] && line[2])
            {
                const char digits[3] = { line[1], line[2], '\0' };
                *path = (char) strtol(digits, NULL, 16);
                line += 2;
            }
            else
                *path = *line;

            path++;
            line++;
        }
    }

    return paths;
}

void wManagerGetCursorPos(wManagerWindow *window, double *xpos, double *ypos)
{
    window->platform.getCursorPos(window, xpos, ypos);
}

void wManagerSetCursorPos(wManagerWindow *window, double xpos, double ypos)
{
    window->platform.setCursorPos(window, xpos, ypos);
}

void wManagerPoolEvents()
{
    _wMWindow.platform.pollEvents();
}

void WManagerWaitEventsTimeout(double timeout)
{
    if (timeout != timeout || timeout < 0.0)
    {
        printf("Invalid time %f\n", timeout);
        return;
    }

    _wMWindow.platform.waitEventsTimeout(timeout);
}