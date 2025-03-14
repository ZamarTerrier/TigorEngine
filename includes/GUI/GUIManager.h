#ifndef GUIMANAGER_H
#define GUIMANAGER_H
#include "Variabels/engine_includes.h"

#include "Core/e_buffer.h"

#include "Objects/gameObject2D.h"

#ifndef GUI_DRAWLIST_TEX_LINES_WIDTH_MAX
#define GUI_DRAWLIST_TEX_LINES_WIDTH_MAX     (63)
#endif

/// @brief Объект интерфейса
typedef struct{
    /// @brief вершины объекта
    Vertex2D *points;
    /// @brief количество вершин
    uint32_t vert_count;
    /// @brief индексы объекта
    uint32_t *indeces;
    /// @brief количество индексов
    uint32_t indx_count;
} GUIObj;

typedef struct{
    uint32_t Colored : 1;
    uint32_t Visible : 1;
    uint32_t Codepoint  : 30;
    float    AdvanceX;
    float    X0, Y0, X1, Y1;
    float    U0, V0, U1, V1;
} TFontGlyph;

typedef struct{
    TFontGlyph *Glyphs;    

} TFont;

typedef struct{
    GameObject2D go;
    ChildStack *draw_list;
    BufferObject vertBuffer;
    BufferObject indxBuffer;
    vec2 _Path[2048];
    uint32_t _Path_Size;
    float _FringeScale;
    uint32_t currIndx;
    uint32_t Flags;
    bool sellected;

    struct ChildStack* first_widget;
    struct ChildStack* last_widget;

    struct{
        uint32_t fontWidth;
        uint32_t fontHeight;
        void *info;
        void *cdata;
        void *texture;
        float fontSize;
    } font;
} GUIManager;

/// @brief Отрисовка примитива квадрата
/// @param a - левый верхний угол
/// @param c - правый нижний
/// @param color - цвет
void GUIManagerDrawRect(vec2 a, vec2 c, vec3 color);
/// @brief Отрисавка текста в формате char
/// @param xpos - Координата по х
/// @param ypos - Координата по у
/// @param color - Цвет текста
/// @param font_size - Размер текста
/// @param text - Текст
void GUIAddTextU8(float xpos, float ypos, vec3 color, float font_size, char *text);
/// @brief Отрисовка текста в формате uint32_t
/// @param xpos - Координата по х
/// @param ypos - Координата по у
/// @param color - Цвет текста
/// @param font_size - Размер текста
/// @param text - Текст
void GUIAddTextU32(float xpos, float ypos, vec3 color, float font_size, uint32_t *text);
/// @brief Формирование объекта фигуры с заполнением
/// @param points - точки фигуры
/// @param points_count - количество точек
/// @param col - цвет заполнения
void GUIManagerAddConvexPolyFilled(const vec2 *points, const int points_count, vec3 col);
/// @brief Формерование объекта фигуры без заполнения
/// @param points - точки фигуры
/// @param num_points - количество точек
/// @param color - цвет заполнения
/// @param flags - флаги фигуры 
/// @param thickness - толщина линий
void GUIManagerAddPolyline(const vec2* points, int num_points, vec3 color, uint32_t flags, float thickness);
/// @brief Добавить точку в стек
/// @param pos - коррдинаты точки
void PathLineTo(vec2 pos);
/// @brief Функция для вызова формирования фигуры из стека
/// @param col - цвет фигуры
void PathFillConvex(vec3 col);
/// @brief Функция формирования квадрата
/// @param a - левый верхний угол
/// @param b - правый нижний
/// @param rounding - степень закруглённости
/// @param flags - флаги фигуры
void PathRect(vec2 a, vec2 b, float rounding, uint32_t flags);
/// @brief Функция для вызова формирования фигуры из стека
/// @param col - цвет фигуры 
/// @param flags - флаги фигуры
/// @param thickness - толщина линий
void PathFillConvex(vec3 col);
void PathStroke(vec3 color, uint32_t flags, float thickness);
void PathArcTo(const vec2 center, float radius, float a_min, float a_max, int num_segments);
void PathBezierCubicCurveTo( vec2 p2,  vec2 p3, vec2 p4, int num_segments);
void PathBezierQuadraticCurveTo( vec2 p2, vec2 p3, int num_segments);
/// @brief Функция инициализация обхекта интрефейса
void GUIManagerInit();
/// @brief Функция проверки инициализации отрисовщика интерфейса
/// @return 0 - не инициализирован 1 - инициалиазирован
int GUIManagerIsInit();
/// @brief Функция обновления данных виджетов
void GUIManagerUpdate();
/// @brief Функция отрисовки объекта интерфейса
void GUIManagerDraw();
/// @brief Функция очистки объекта интерфейса
void GUIManagerClear();
/// @brief Функция пересоздания объекта интерфейса
void GUIManagerRecreate();
/// @brief Функция уничтожения объекта интерфейса
void GUIManagerDestroy();

#endif //GUIMANAGER_H