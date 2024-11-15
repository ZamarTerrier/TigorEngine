#include <TigorEngine.h>
#include <TigorGUI.h>

#include <Core/engine.h>
#include <Core/e_camera.h>
#include <Core/e_physics.h>

#include <Objects/render_texture.h>
#include <Objects/primitiveObject.h>
#include <Objects/light_object.h>
#include <Objects/shape_object.h>

#include <Tools/e_math.h>
#include <Tools/intersections2D.h>

#include <float.h>

Camera2D cam2D;
Camera3D cam3D;

ShapeObject so;
ShapeObject so2;

extern TEngine engine;

extern float RAD2DEG(float x);

float rot = 65, grav = 1.0f;

Point2D RigidBodyGetVertices2D(float rot, vec2 pos, vec2 size, vec3 color) {
	Point2D v;
    memset(&v, 0, sizeof(Point2D));
    v.num_points = 4;

    mat2 orr = m2_rotate(rot);
	const float* o = orr.arr;
	vec2 A[] = {			// OBB Axis
		{o[0], o[1]},
		{o[3], o[4]},
	};
	
	vec2 t1 = v2_muls(A[0], size.x);
	vec2 t2 = v2_muls(A[1], size.y);

	v.points[0] = v2_add(pos, m2_v2_mult(orr, v2_muls(size, -1)));
	v.points[1] = v2_add(pos, m2_v2_mult(orr, vec2_f(-size.x, size.y)));
	v.points[2] = v2_add(pos, m2_v2_mult(orr, vec2_f(size.x, size.y)));
	v.points[3] = v2_add(pos, m2_v2_mult(orr, vec2_f(size.x, -size.y)));

    for(int i=0; i < 4;i++)
        GUIAddCircleFilled(v.points[i], 4, color, 0);

    v.center = pos;
	return v;
}

void Update(float dTime){

    double time = TEngineGetTime();

    float cnst = 256;

    Point2D points1 = RigidBodyGetVertices2D(0, Transform2DGetPosition(&so), Transform2DGetScale(&so), vec3_f(1, 0, 0));
    Point2D points2 = RigidBodyGetVertices2D(rot, Transform2DGetPosition(&so2), Transform2DGetScale(&so2), vec3_f(0, 1, 0));

    float t = 0;
    vec2 t_v;

    vec2 points[32];
    float d_points[32];
    uint32_t num_points = 0;

    int res = 0;
    
    for(int i=0;i<4;i++){
        for(int j=0;j<4;j++){
            res = IntersectLineToLine(points1.points[i], points1.points[i < 3 ? i + 1 : 0], points2.points[j], points2.points[j < 3 ? j + 1 : 0], &t, &t_v);

            if(res){
                points[num_points] = t_v;
                d_points[num_points] = t;
                num_points ++;
            }
        }
    }

    for(int i=0;i<4;i++){
        for(int j=0;j<4;j++){
            res = IntersectLineToLine(points2.points[i], points2.points[i < 3 ? i + 1 : 0], points1.points[j], points1.points[j < 3 ? j + 1 : 0], &t, &t_v);

            if(res){
                points[num_points] = t_v;
                d_points[num_points] = t;
                num_points ++;
            }
        }
    }

    vec2 t_points[32];
    float t_d_points[32];
    uint32_t t_num_points = num_points;
    num_points = 0; 
    memcpy(t_points, points, sizeof(vec2) * 32);
    memcpy(t_d_points, d_points, sizeof(float) * 32);
    memset(points, 0, sizeof(vec2) * 32);
    memset(d_points, 0, sizeof(float) * 32);

    int finded = false;
    for(int i=0;i < t_num_points;i++){
        finded = false;
        for(int j=0;j < num_points;j++){
            if(v2_distance(t_points[i], points[j]) < 1.0f){
                finded = true;
                break;
            }
        }

        if(!finded){
            points[num_points] = t_points[i];
            d_points[num_points] = t_d_points[i];
            num_points ++;
        }        
    }

    for(int i=0;i<num_points;i++)
        GUIAddCircleFilled(points[i], 4, vec3_f(0, 0, 0), 0);

    vec2 pos1 = Transform2DGetPosition(&so);
    vec2 pos2 = Transform2DGetPosition(&so2);

    if(num_points > 0){

        if(fabs(atan2(points[1].y - points[0].y, points[1].x - points[0].x)) > FLT_EPSILON){
            vec2 dir = v2_muls(v2_norm(v2_sub(pos1, pos2)), 0.45);
            //pos1 = v2_add(pos1, dir);
            pos2 = v2_sub(pos2, dir);

            float t_rot = 0;
            for(int i=0;i<num_points;i++)
            {
                t_rot += atan2(pos2.y - points[i].y, pos2.x - points[i].x) * d_points[i] * dTime;
            }

            t_rot = t_rot;

            rot -= t_rot;

            pos2.x -= t_rot;
        }
    }else
        pos2.y -= grav * dTime;


    Transform2DSetPosition(&so, pos1.x, pos1.y);
    Transform2DSetPosition(&so2, pos2.x, pos2.y);
    
    char buff[256];

    sprintf(buff, "Intesect is %s", num_points > 0 ? "intescted" : "not");

    GUIAddText(100, 100, vec3_f(1, 1, 1), 9, buff);
}


int main(){

    TEngineInitSystem(800, 600, "Test");

    Camera2DInit(&cam2D);
    Camera3DInit(&cam3D);
        
    DrawParam dParam;
    memset(&dParam, 0, sizeof(DrawParam));

    dParam.diffuse = "res\\texture.jpg";
    //dParam.normal = "res\\normal.jpg";

    ShapeObjectInit(&so, &dParam, TIGOR_SHAPE_OBJECT_QUAD, NULL);
    Transform2DSetPosition(&so, 100, 100);    
    Transform2DSetScale(&so, 50, 50);    

    ShapeObjectInit(&so2, &dParam, TIGOR_SHAPE_OBJECT_QUAD, NULL);
    Transform2DSetPosition(&so2, 120, 250);    
    Transform2DSetScale(&so2, 50, 50);  

    
    while (!TEngineWindowIsClosed())
    {
        TEnginePoolEvents();

        Update(1.0);
        
        Transform2DSetRotate(&so2, rot);  

        TEngineDraw(&so);
        TEngineDraw(&so2);

        TEngineRender();
    }
    
    GameObjectDestroy((GameObject *)&so);
    GameObjectDestroy((GameObject *)&so2);
    
    EngineDeviceWaitIdle();
    
    TEngineCleanUp();

    return 0;
}