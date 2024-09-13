#ifndef E_WIDGET_IMAGE_H
#define E_WIDGET_IMAGE_H

#include "Objects/gameObject2D.h"

#include "e_widget.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct{
    EWidget widget;
    GameObject2D image;
} EWidgetImage;

void ImageWidgetInit(EWidgetImage *img, char *image_path, EWidget *parent);

#ifdef __cplusplus
}
#endif

#endif // E_WIDGET_IMAGE_H
