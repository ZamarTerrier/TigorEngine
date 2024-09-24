#ifndef KEY_DEFINES_H
#define KEY_DEFINES_H

#define TIGOR_MOD_SHIFT           0x0001
#define TIGOR_MOD_CONTROL         0x0002
#define TIGOR_MOD_ALT             0x0004
#define TIGOR_MOD_SUPER           0x0008
#define TIGOR_MOD_CAPS_LOCK       0x0010
#define TIGOR_MOD_NUM_LOCK        0x0020

#define TIGOR_MOUSE_BUTTON_1         0
#define TIGOR_MOUSE_BUTTON_2         1
#define TIGOR_MOUSE_BUTTON_3         2
#define TIGOR_MOUSE_BUTTON_4         3
#define TIGOR_MOUSE_BUTTON_5         4
#define TIGOR_MOUSE_BUTTON_6         5
#define TIGOR_MOUSE_BUTTON_7         6
#define TIGOR_MOUSE_BUTTON_8         7
#define TIGOR_MOUSE_BUTTON_LAST      TIGOR_MOUSE_BUTTON_8
#define TIGOR_MOUSE_BUTTON_LEFT      TIGOR_MOUSE_BUTTON_1
#define TIGOR_MOUSE_BUTTON_RIGHT     TIGOR_MOUSE_BUTTON_2
#define TIGOR_MOUSE_BUTTON_MIDDLE    TIGOR_MOUSE_BUTTON_3

#define TIGOR_JOYSTICK_1             0
#define TIGOR_JOYSTICK_2             1
#define TIGOR_JOYSTICK_3             2
#define TIGOR_JOYSTICK_4             3
#define TIGOR_JOYSTICK_5             4
#define TIGOR_JOYSTICK_6             5
#define TIGOR_JOYSTICK_7             6
#define TIGOR_JOYSTICK_8             7
#define TIGOR_JOYSTICK_9             8
#define TIGOR_JOYSTICK_10            9
#define TIGOR_JOYSTICK_11            10
#define TIGOR_JOYSTICK_12            11
#define TIGOR_JOYSTICK_13            12
#define TIGOR_JOYSTICK_14            13
#define TIGOR_JOYSTICK_15            14
#define TIGOR_JOYSTICK_16            15
#define TIGOR_JOYSTICK_LAST          TIGOR_JOYSTICK_16

#define TIGOR_RELEASE                0
#define TIGOR_PRESS                  1
#define TIGOR_REPEAT                 2

#define TIGOR_DONT_CARE              -1

#define TIGOR_KEY_UNKNOWN            -1

/* Printable keys */
#define TIGOR_KEY_SPACE              32
#define TIGOR_KEY_APOSTROPHE         39  /* ' */
#define TIGOR_KEY_COMMA              44  /* , */
#define TIGOR_KEY_MINUS              45  /* - */
#define TIGOR_KEY_PERIOD             46  /* . */
#define TIGOR_KEY_SLASH              47  /* / */
#define TIGOR_KEY_0                  48
#define TIGOR_KEY_1                  49
#define TIGOR_KEY_2                  50
#define TIGOR_KEY_3                  51
#define TIGOR_KEY_4                  52
#define TIGOR_KEY_5                  53
#define TIGOR_KEY_6                  54
#define TIGOR_KEY_7                  55
#define TIGOR_KEY_8                  56
#define TIGOR_KEY_9                  57
#define TIGOR_KEY_SEMICOLON          59  /* ; */
#define TIGOR_KEY_EQUAL              61  /* = */
#define TIGOR_KEY_A                  65
#define TIGOR_KEY_B                  66
#define TIGOR_KEY_C                  67
#define TIGOR_KEY_D                  68
#define TIGOR_KEY_E                  69
#define TIGOR_KEY_F                  70
#define TIGOR_KEY_G                  71
#define TIGOR_KEY_H                  72
#define TIGOR_KEY_I                  73
#define TIGOR_KEY_J                  74
#define TIGOR_KEY_K                  75
#define TIGOR_KEY_L                  76
#define TIGOR_KEY_M                  77
#define TIGOR_KEY_N                  78
#define TIGOR_KEY_O                  79
#define TIGOR_KEY_P                  80
#define TIGOR_KEY_Q                  81
#define TIGOR_KEY_R                  82
#define TIGOR_KEY_S                  83
#define TIGOR_KEY_T                  84
#define TIGOR_KEY_U                  85
#define TIGOR_KEY_V                  86
#define TIGOR_KEY_W                  87
#define TIGOR_KEY_X                  88
#define TIGOR_KEY_Y                  89
#define TIGOR_KEY_Z                  90
#define TIGOR_KEY_LEFT_BRACKET       91  /* [ */
#define TIGOR_KEY_BACKSLASH          92  /* \ */
#define TIGOR_KEY_RIGHT_BRACKET      93  /* ] */
#define TIGOR_KEY_GRAVE_ACCENT       96  /* ` */
#define TIGOR_KEY_WORLD_1            161 /* non-US #1 */
#define TIGOR_KEY_WORLD_2            162 /* non-US #2 */

/* Function keys */
#define TIGOR_KEY_ESCAPE             256
#define TIGOR_KEY_ENTER              257
#define TIGOR_KEY_TAB                258
#define TIGOR_KEY_BACKSPACE          259
#define TIGOR_KEY_INSERT             260
#define TIGOR_KEY_DELETE             261
#define TIGOR_KEY_RIGHT              262
#define TIGOR_KEY_LEFT               263
#define TIGOR_KEY_DOWN               264
#define TIGOR_KEY_UP                 265
#define TIGOR_KEY_PAGE_UP            266
#define TIGOR_KEY_PAGE_DOWN          267
#define TIGOR_KEY_HOME               268
#define TIGOR_KEY_END                269
#define TIGOR_KEY_CAPS_LOCK          280
#define TIGOR_KEY_SCROLL_LOCK        281
#define TIGOR_KEY_NUM_LOCK           282
#define TIGOR_KEY_PRINT_SCREEN       283
#define TIGOR_KEY_PAUSE              284
#define TIGOR_KEY_F1                 290
#define TIGOR_KEY_F2                 291
#define TIGOR_KEY_F3                 292
#define TIGOR_KEY_F4                 293
#define TIGOR_KEY_F5                 294
#define TIGOR_KEY_F6                 295
#define TIGOR_KEY_F7                 296
#define TIGOR_KEY_F8                 297
#define TIGOR_KEY_F9                 298
#define TIGOR_KEY_F10                299
#define TIGOR_KEY_F11                300
#define TIGOR_KEY_F12                301
#define TIGOR_KEY_F13                302
#define TIGOR_KEY_F14                303
#define TIGOR_KEY_F15                304
#define TIGOR_KEY_F16                305
#define TIGOR_KEY_F17                306
#define TIGOR_KEY_F18                307
#define TIGOR_KEY_F19                308
#define TIGOR_KEY_F20                309
#define TIGOR_KEY_F21                310
#define TIGOR_KEY_F22                311
#define TIGOR_KEY_F23                312
#define TIGOR_KEY_F24                313
#define TIGOR_KEY_F25                314
#define TIGOR_KEY_KP_0               320
#define TIGOR_KEY_KP_1               321
#define TIGOR_KEY_KP_2               322
#define TIGOR_KEY_KP_3               323
#define TIGOR_KEY_KP_4               324
#define TIGOR_KEY_KP_5               325
#define TIGOR_KEY_KP_6               326
#define TIGOR_KEY_KP_7               327
#define TIGOR_KEY_KP_8               328
#define TIGOR_KEY_KP_9               329
#define TIGOR_KEY_KP_DECIMAL         330
#define TIGOR_KEY_KP_DIVIDE          331
#define TIGOR_KEY_KP_MULTIPLY        332
#define TIGOR_KEY_KP_SUBTRACT        333
#define TIGOR_KEY_KP_ADD             334
#define TIGOR_KEY_KP_ENTER           335
#define TIGOR_KEY_KP_EQUAL           336
#define TIGOR_KEY_LEFT_SHIFT         340
#define TIGOR_KEY_LEFT_CONTROL       341
#define TIGOR_KEY_LEFT_ALT           342
#define TIGOR_KEY_LEFT_SUPER         343
#define TIGOR_KEY_RIGHT_SHIFT        344
#define TIGOR_KEY_RIGHT_CONTROL      345
#define TIGOR_KEY_RIGHT_ALT          346
#define TIGOR_KEY_RIGHT_SUPER        347
#define TIGOR_KEY_MENU               348

#define TIGOR_KEY_LAST               TIGOR_KEY_MENU

#define TIGOR_CURSOR                 0x00033001
#define TIGOR_STICKY_KEYS            0x00033002
#define TIGOR_STICKY_MOUSE_BUTTONS   0x00033003
#define TIGOR_LOCK_KEY_MODS          0x00033004
#define TIGOR_RAW_MOUSE_MOTION       0x00033005

#define TIGOR_CURSOR_NORMAL          0x00034001
#define TIGOR_CURSOR_HIDDEN          0x00034002
#define TIGOR_CURSOR_DISABLED        0x00034003
#define TIGOR_CURSOR_CAPTURED        0x00034004

#define TIGOR_ANY_POSITION           0x80000000

#endif // KEY_DEFINES_H
