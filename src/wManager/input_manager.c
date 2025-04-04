#include "wManager/input_manager.h"
#include "wManager/manager_includes.h"

#include <stdbool.h>
#include <assert.h>

extern wManagerWindow _wMWindow;

// Internal key state used for sticky keys
#define _TIGOR_STICK 3

// Internal constants for gamepad mapping source types
#define _TIGOR_JOYSTICK_AXIS     1
#define _TIGOR_JOYSTICK_BUTTON   2
#define _TIGOR_JOYSTICK_HATBIT   3

#define TIGOR_MOD_MASK (TIGOR_MOD_SHIFT | \
                       TIGOR_MOD_CONTROL | \
                       TIGOR_MOD_ALT | \
                       TIGOR_MOD_SUPER | \
                       TIGOR_MOD_CAPS_LOCK | \
                       TIGOR_MOD_NUM_LOCK)


void _wManagerCenterCursorInContentArea(wManagerWindow* window)
{
    int width, height;

    _wMWindow.platform.getWindowSize(window, &width, &height);
    _wMWindow.platform.setCursorPos(window, width / 2.0, height / 2.0);
}

// Notifies shared code of a physical key event
//
void _wManagerInputKey(wManagerWindow* window, int key, int scancode, int action, int mods)
{
    if (key >= 0 && key <= TIGOR_KEY_LAST)
    {
        int32_t repeated = false;

        if (action == TIGOR_RELEASE && window->keys[key] == TIGOR_RELEASE)
            return;

        if (action == TIGOR_PRESS && window->keys[key] == TIGOR_PRESS)
            repeated = true;

        if (action == TIGOR_RELEASE && window->stickyKeys)
            window->keys[key] = _TIGOR_STICK;
        else
            window->keys[key] = (char) action;

        if (repeated)
            action = TIGOR_REPEAT;
    }

    if (!window->lockKeyMods)
        mods &= ~(TIGOR_MOD_CAPS_LOCK | TIGOR_MOD_NUM_LOCK);

    if (window->callbacks.key)
        window->callbacks.key((wManagerWindow*) window, key, scancode, action, mods);
}

// Notifies shared code of a Unicode codepoint input event
// The 'plain' parameter determines whether to emit a regular character event
//
void _wManagerInputChar(wManagerWindow* window, uint32_t codepoint, int mods, int32_t plain)
{
    if (codepoint < 32 || (codepoint > 126 && codepoint < 160))
        return;

    if (!window->lockKeyMods)
        mods &= ~(TIGOR_MOD_CAPS_LOCK | TIGOR_MOD_NUM_LOCK);

    if (window->callbacks.charmods)
        window->callbacks.charmods((wManagerWindow*) window, codepoint, mods);

    if (plain)
    {
        if (window->callbacks.character)
            window->callbacks.character((wManagerWindow*) window, codepoint);
    }
}

void _wManagerInputCursorEnter(wManagerWindow* window, int32_t entered)
{
    if (window->callbacks.cursorEnter)
        window->callbacks.cursorEnter((wManagerWindow*) window, entered);
}

// Notifies shared code of a cursor motion event
// The position is specified in content area relative screen coordinates
//
void _wManagerInputCursorPos(wManagerWindow* window, double xpos, double ypos)
{

    if (window->virtualCursorPosX == xpos && window->virtualCursorPosY == ypos)
        return;

    window->virtualCursorPosX = xpos;
    window->virtualCursorPosY = ypos;

    if (window->callbacks.cursorPos)
        window->callbacks.cursorPos((wManagerWindow*) window, xpos, ypos);
}

void _wManagerInputMouseClick(wManagerWindow* window, int button, int action, int mods)
{
    if (button < 0 || (button > TIGOR_MOUSE_BUTTON_LAST))
        return;

    if (!window->lockKeyMods)
        mods &= ~(TIGOR_MOD_CAPS_LOCK | TIGOR_MOD_NUM_LOCK);

    if (action == TIGOR_RELEASE && window->stickyMouseButtons)
        window->mouseButtons[button] = _TIGOR_STICK;
    else
        window->mouseButtons[button] = (char) action;

    if (window->callbacks.mouseButton)
        window->callbacks.mouseButton((wManagerWindow*) window, button, action, mods);
}

// Notifies shared code of a scroll event
//
void _wManagerInputScroll(wManagerWindow* window, double xoffset, double yoffset)
{
    if (window->callbacks.scroll)
        window->callbacks.scroll((wManagerWindow*) window, xoffset, yoffset);
}

// Notifies shared code of files or directories dropped on a window
//
void _wManagerInputDrop(wManagerWindow* window, int count, const char** paths)
{
    if (window->callbacks.drop)
        window->callbacks.drop((wManagerWindow*) window, count, paths);
}