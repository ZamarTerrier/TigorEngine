#ifndef TigorGUI_H
#define TigorGUI_H

#include "Variabels/engine_includes.h"


typedef enum DrawListFlags
{
    GUIDrawListFlags_None                    = 0,
    GUIDrawListFlags_AntiAliasedLines        = 1 << 0,  // Enable anti-aliased lines/borders (*2 the number of triangles for 1.0f wide line or lines thin enough to be drawn using textures, otherwise *3 the number of triangles)
    GUIDrawListFlags_AntiAliasedLinesUseTex  = 1 << 1,  // Enable anti-aliased lines/borders using textures when possible. Require backend to render with bilinear filtering (NOT point/nearest filtering).
    GUIDrawListFlags_AntiAliasedFill         = 1 << 2,  // Enable anti-aliased edge around filled shapes (rounded rectangles, circles).
    GUIDrawListFlags_AllowVtxOffset          = 1 << 3,  // Can emit 'VtxOffset > 0' to allow large meshes. Set when 'ImGuiBackendFlags_RendererHasVtxOffset' is enabled.
} DrawListFlags;

typedef enum DrawFlags
{
    GUIDrawFlags_None                        = 0,
    GUIDrawFlags_Closed                      = 1 << 0, // PathStroke(), AddPolyline(): specify that shape should be closed (Important: this is always == 1 for legacy reason)
    GUIDrawFlags_RoundCornersTopLeft         = 1 << 4, // AddRect(), AddRectFilled(), PathRect(): enable rounding top-left corner only (when rounding > 0.0f, we default to all corners). Was 0x01.
    GUIDrawFlags_RoundCornersTopRight        = 1 << 5, // AddRect(), AddRectFilled(), PathRect(): enable rounding top-right corner only (when rounding > 0.0f, we default to all corners). Was 0x02.
    GUIDrawFlags_RoundCornersBottomLeft      = 1 << 6, // AddRect(), AddRectFilled(), PathRect(): enable rounding bottom-left corner only (when rounding > 0.0f, we default to all corners). Was 0x04.
    GUIDrawFlags_RoundCornersBottomRight     = 1 << 7, // AddRect(), AddRectFilled(), PathRect(): enable rounding bottom-right corner only (when rounding > 0.0f, we default to all corners). Wax 0x08.
    GUIDrawFlags_RoundCornersNone            = 1 << 8, // AddRect(), AddRectFilled(), PathRect(): disable rounding on all corners (when rounding > 0.0f). This is NOT zero, NOT an implicit flag!
    GUIDrawFlags_RoundCornersTop             = GUIDrawFlags_RoundCornersTopLeft | GUIDrawFlags_RoundCornersTopRight,
    GUIDrawFlags_RoundCornersBottom          = GUIDrawFlags_RoundCornersBottomLeft | GUIDrawFlags_RoundCornersBottomRight,
    GUIDrawFlags_RoundCornersLeft            = GUIDrawFlags_RoundCornersBottomLeft | GUIDrawFlags_RoundCornersTopLeft,
    GUIDrawFlags_RoundCornersRight           = GUIDrawFlags_RoundCornersBottomRight | GUIDrawFlags_RoundCornersTopRight,
    GUIDrawFlags_RoundCornersAll             = GUIDrawFlags_RoundCornersTopLeft | GUIDrawFlags_RoundCornersTopRight | GUIDrawFlags_RoundCornersBottomLeft | GUIDrawFlags_RoundCornersBottomRight,
    GUIDrawFlags_RoundCornersDefault_        = GUIDrawFlags_RoundCornersAll, // Default to ALL corners if none of the _RoundCornersXX flags are specified.
    GUIDrawFlags_RoundCornersMask_           = GUIDrawFlags_RoundCornersAll | GUIDrawFlags_RoundCornersNone,
}DrawFlags;

// Flags for ImFontAtlas build
typedef enum FontAtlasFlags
{
    GUIFontAtlasFlags_None               = 0,
    GUIFontAtlasFlags_NoPowerOfTwoHeight = 1 << 0,   // Don't round the height to next power of two
    GUIFontAtlasFlags_NoMouseCursors     = 1 << 1,   // Don't build software mouse cursors into the atlas (save a little texture memory)
    GUIFontAtlasFlags_NoBakedLines       = 1 << 2,   // Don't build thick line textures into the atlas (save a little texture memory, allow support for point/nearest filtering). The AntiAliasedLinesUseTex features uses them, otherwise they will be rendered using polygons (more expensive for CPU/GPU).
} FontAtlasFlags;

/// @brief Нарисовать линию
/// @param p1 - начало линии
/// @param p2 - конец линии
/// @param col - цвет линии
/// @param thickness - толщина линии
void GUIAddLine(const vec2 p1, const vec2 p2, vec3 col, float thickness);
/// @brief Нарисовать квадрат
/// @param p_min - левый верхний угол квадрата
/// @param p_max - правый нижний угол квадрата
/// @param col - цвет квадрата
/// @param rounding - степень закругленности квадрата
/// @param flags - флаги фигуры
/// @param thickness - толщина линий квадрата
void GUIAddRect(const vec2 p_min, const vec2 p_max, vec3 col, float rounding, uint32_t flags, float thickness);
/// @brief Нарисовать и заполнить квадрат цветом
/// @param p_min - левый верхний угол квадрата
/// @param p_max - правый нижний угол квадрата
/// @param col - цвет квадрата
/// @param rounding - степень закругленности квадрата
/// @param flags - флаги фигуры
void GUIAddRectFilled(const vec2 p_min, const vec2 p_max, vec3 col, float rounding, uint32_t flags);
/// @brief Нарисовать четырёх угольную фигуру
/// @param p1 - точка номер 1
/// @param p2 - точка номер 2
/// @param p3 - точка номер 3
/// @param p4 - точка номер 4
/// @param col - цвет фигуры
/// @param thickness - толщина линий фигуры
void GUIAddQuad(const vec2 p1, const vec2 p2, const vec2 p3, const vec2 p4, vec3 col, float thickness);
/// @brief Нарисовать четырёх угольную фигуру и залить цветом
/// @param p1 - точка номер 1
/// @param p2 - точка номер 2
/// @param p3 - точка номер 3
/// @param p4 - точка номер 4
/// @param col - цвет фигуры
void GUIAddQuadFilled(const vec2 p1, const vec2 p2, const vec2 p3, const vec2 p4, vec3 col);
/// @brief Нарисовать произвольный треугольник
/// @param p1 - точка номер 1
/// @param p2 - точка номер 2
/// @param p3 - точка номер 3
/// @param col - цвет фигуры
/// @param thickness - толщина линий фигуры
void GUIAddTriangle(const vec2 p1, const vec2 p2, const vec2 p3, vec3 col, float thickness);
/// @brief Нарисовать произвольный треугольник и заполнить его цветом
/// @param p1 - точка номер 1
/// @param p2 - точка номер 2
/// @param p3 - точка номер 3
/// @param col - цвет фигуры
void GUIAddTriangleFilled(const vec2 p1, const vec2 p2, const vec2 p3, vec3 col);
/// @brief Нарисовать круг
/// @param center - центр круга
/// @param radius - радиус круга
/// @param col - цвет круга
/// @param num_segments - количество сегменов круга
/// @param thickness - толщина линий круга
void GUIAddCircle(vec2 center, float radius, vec3 col, int num_segments, float thickness);
/// @brief Нарисовать круг и заполнить его цветом
/// @param center - центр круга
/// @param radius - радиус круга
/// @param col - цвет круга
/// @param num_segments - количество сегменов круга
void GUIAddCircleFilled(vec2 center, float radius, vec3 col, int num_segments);
void GUIAddNgon(vec2 center, float radius, vec3 col, int num_segments, float thickness);
void GUIAddNgonFilled(vec2 center, float radius, vec3 col, int num_segments);
/// @brief Нарисовать эллипс
/// @param center - центр эллипса
/// @param radius -радиус эллипса
/// @param col - цвет эллипса
/// @param rot - вращение эллипса
/// @param num_segments - количество сегментов эллипса
/// @param thickness - толщина линий эллипса
void GUIAddEllipse(vec2 center, const vec2 radius, vec3 col, float rot, int num_segments, float thickness);
/// @brief Нарисовать эллипс и заполнить его цветом
/// @param center - центр эллипса
/// @param radius -радиус эллипса
/// @param col - цвет эллипса
/// @param rot - вращение эллипса
/// @param num_segments - количество сегментов эллипса
void GUIAddEllipseFilled(vec2 center, const vec2 radius, vec3 col, float rot, int num_segments);

void GUIAddPolyline(vec2* points, int points_count, vec3 color, DrawListFlags flags, float thicknes);

#ifndef GUIAddText

/// @brief Отрисавка текста в формате char
/// @param xpos - Координата по х
/// @param ypos - Координата по у
/// @param color - Цвет текста
/// @param font_size - Размер текста
/// @param text - Текст
extern void GUIAddTextU8(float xpos, float ypos, vec3 color, float font_size, char *text);
/// @brief Отрисавка текста в формате uint32_t
/// @param xpos - Координата по х
/// @param ypos - Координата по у
/// @param color - Цвет текста
/// @param font_size - Размер текста
/// @param text - Текст
extern void GUIAddTextU32(float xpos, float ypos, vec3 color, float font_size, uint32_t *text);

/// @brief Отрисовка текста
/// @param xpos - Координата по х
/// @param ypos - Координата по у
/// @param color - Цвет текста
/// @param font_size - Размер текста
/// @param text - Текст
#define GUIAddText(xpos, ypos, color, font_size, text)\
    _Generic((text),\
    char *: GUIAddTextU8,\
    const char *: GUIAddTextU8,\
    unsigned int *: GUIAddTextU32\
    )(xpos, ypos, color, font_size, text)

#endif

#endif //#ifndef TigorGUI_H