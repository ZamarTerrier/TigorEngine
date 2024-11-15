#ifndef E_MATH_VARIABLES_H
#define E_MATH_VARIABLES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct{
    float x;
    float y;
} vec2;

typedef struct{
    float x;
    float y;
    float z;
} vec3;

typedef struct{
    float x;
    float y;
    float z;
    float w;
} vec4;

typedef struct{
    uint8_t x;
    uint8_t y;
    uint8_t z;
    uint8_t w;
} vec4_u8;

typedef struct{
    union{
        float m[2][2];
        float arr[4];
    };
} mat2;

typedef struct{
    union{
        float m[3][3];
        float arr[9];
    };
} mat3;

typedef struct{
    union{
		struct {
			float	_11, _12, _13, _14,
					_21, _22, _23, _24,
					_31, _32, _33, _34,
					_41, _42, _43, _44;
		};
        float m[4][4];
    };
} mat4;

typedef struct{
    float m[16];
} mat4_p;

#ifdef __cplusplus
}
#endif

#endif // E_MATH_VARIABLES_H
