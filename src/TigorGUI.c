#include "TigorGUI.h"

#include "GUI/GUIManager.h"

#include "Tools/e_math.h"

// ImDrawList: Lookup table size for adaptive arc drawing, cover full circle.
#ifndef GUI_DRAWLIST_ARCFAST_TABLE_SIZE
#define GUI_DRAWLIST_ARCFAST_TABLE_SIZE                          48 // Number of samples in lookup table.
#endif
#ifndef GUI_DRAWLIST_ARCFAST_SAMPLE_MAX
#define GUI_DRAWLIST_ARCFAST_SAMPLE_MAX                          GUI_DRAWLIST_ARCFAST_TABLE_SIZE // Sample index _PathArcToFastEx() for 360 angle
#endif

#ifndef GUI_DRAWLIST_CIRCLE_AUTO_SEGMENT_MAX
#define GUI_DRAWLIST_CIRCLE_AUTO_SEGMENT_MAX                     512
#endif

extern GUIManager gui;
extern TEngine engine;

extern void PathEllipticalArcTo(const vec2 center, const vec2 radius, float rot, float a_min, float a_max, int num_segments);
extern void _PathArcToFastEx(const vec2 center, float radius, int a_min_sample, int a_max_sample, int a_step);
extern void PathArcTo(const vec2 center, float radius, float a_min, float a_max, int num_segments);
extern int _CalcCircleAutoSegmentCount(float radius);

void GUISetFlags(DrawListFlags flags){
    gui.Flags = flags;
}

DrawListFlags GUIGetFlags(){
    return gui.Flags;
}

void GUIAddLine(const vec2 p1, const vec2 p2, vec4 color, float thickness)
{
    PathLineTo(v2_add(p1, vec2_f(0.5f, 0.5f)));
    PathLineTo(v2_add(p2, vec2_f(0.5f, 0.5f)));
    PathStroke(color, 0, thickness, false);
}

void GUIAddRect(const vec2 p_min, const vec2 p_max, vec4 color, float rounding, uint32_t flags, float thickness)
{
    PathRect(v2_add(p_min, vec2_f(0.50f, 0.50f)), v2_sub(p_max, vec2_f(0.49f, 0.49f)), rounding, flags); // Better looking lower-right corner and rounded non-AA shapes.

    PathStroke(color, GUIDrawFlags_Closed, thickness, false);
}

void GUIAddQuad(const vec2 p1, const vec2 p2, const vec2 p3, const vec2 p4, vec4 color, float thickness){
    PathLineTo(p1);
    PathLineTo(p2);
    PathLineTo(p3);
    PathLineTo(p4);
    PathStroke(color, GUIDrawFlags_Closed, thickness, false);
}

void GUIAddQuadFilled(const vec2 p1, const vec2 p2, const vec2 p3, const vec2 p4, vec4 color){
    PathLineTo(p1);
    PathLineTo(p2);
    PathLineTo(p3);
    PathLineTo(p4);
    PathFillConvex(color, false);
}

void GUIAddTriangle(const vec2 p1, const vec2 p2, const vec2 p3, vec4 color, float thickness){
    PathLineTo(p1);
    PathLineTo(p2);
    PathLineTo(p3);
    PathStroke(color, GUIDrawFlags_Closed, thickness, false);
}

void GUIAddTriangleFilled(const vec2 p1, const vec2 p2, const vec2 p3, vec4 color){
    PathLineTo(p1);
    PathLineTo(p2);
    PathLineTo(p3);
    PathFillConvex(color, false);
}

void GUIAddCircle(vec2 center, float radius, vec4 color, int num_segments, float thickness){

    if (num_segments <= 0)
    {
        // Use arc with automatic segment count
        _PathArcToFastEx(center, radius - 0.5f, 0, GUI_DRAWLIST_ARCFAST_SAMPLE_MAX, 0);
        gui._Path_Size--;
    }
    else
    {
        // Explicit segment count (still clamp to avoid drawing insanely tessellated shapes)
        num_segments = clamp(num_segments, 3, GUI_DRAWLIST_CIRCLE_AUTO_SEGMENT_MAX);

        // Because we are filling a closed shape we remove 1 from the count of segments/points
        const float a_max = (M_PI * 2.0f) * ((float)num_segments - 1.0f) / (float)num_segments;
        (center, radius - 0.5f, 0.0f, a_max, num_segments - 1);
    }

    PathStroke(color, GUIDrawFlags_Closed, thickness, false);
}

void GUIAddCircleFilled(vec2 center, float radius, vec4 color, int num_segments){

    if (num_segments <= 0)
    {
        // Use arc with automatic segment count
        _PathArcToFastEx(center, radius, 0, GUI_DRAWLIST_ARCFAST_SAMPLE_MAX, 0);
        gui._Path_Size--;
    }
    else
    {
        // Explicit segment count (still clamp to avoid drawing insanely tessellated shapes)
        num_segments = clamp(num_segments, 3, GUI_DRAWLIST_CIRCLE_AUTO_SEGMENT_MAX);

        // Because we are filling a closed shape we remove 1 from the count of segments/points
        const float a_max = (M_PI * 2.0f) * ((float)num_segments - 1.0f) / (float)num_segments;
        PathArcTo(center, radius, 0.0f, a_max, num_segments - 1);
    }

    PathFillConvex(color, false);
}

void GUIAddNgon(vec2 center, float radius, vec4 color, int num_segments, float thickness){

    // Because we are filling a closed shape we remove 1 from the count of segments/points
    const float a_max = (M_PI * 2.0f) * ((float)num_segments - 1.0f) / (float)num_segments;
    PathArcTo(center, radius - 0.5f, 0.0f, a_max, num_segments - 1);
    PathStroke(color, GUIDrawFlags_Closed, thickness, false);
}

void GUIAddNgonFilled(vec2 center, float radius, vec4 color, int num_segments){

    // Because we are filling a closed shape we remove 1 from the count of segments/points
    const float a_max = (M_PI * 2.0f) * ((float)num_segments - 1.0f) / (float)num_segments;
    PathArcTo(center, radius, 0.0f, a_max, num_segments - 1);
    PathFillConvex(color, false);
}

void GUIAddEllipse(vec2 center, const vec2 radius, vec4 color, float rot, int num_segments, float thickness){

    if (num_segments <= 0)
        num_segments = _CalcCircleAutoSegmentCount(e_max(radius.x, radius.y)); // A bit pessimistic, maybe there's a better computation to do here.

    // Because we are filling a closed shape we remove 1 from the count of segments/points
    const float a_max = M_PI * 2.0f * ((float)num_segments - 1.0f) / (float)num_segments;
    PathEllipticalArcTo(center, radius, rot, 0.0f, a_max, num_segments - 1);
    PathStroke(color, true, thickness, false);
}

void GUIAddEllipseFilled(vec2 center, const vec2 radius, vec4 color, float rot, int num_segments){
    
    if (num_segments <= 0)
        num_segments = _CalcCircleAutoSegmentCount(e_max(radius.x, radius.y)); // A bit pessimistic, maybe there's a better computation to do here.

    // Because we are filling a closed shape we remove 1 from the count of segments/points
    const float a_max = M_PI * 2.0f * ((float)num_segments - 1.0f) / (float)num_segments;
    PathEllipticalArcTo(center, radius, rot, 0.0f, a_max, num_segments - 1);
    PathFillConvex(color, false);
}

void GUIAddPolyline(vec2* points, int points_count, vec4 color, DrawListFlags flags, float thickness){

    GUIManagerAddPolyline(points, points_count, color, flags, thickness);
}