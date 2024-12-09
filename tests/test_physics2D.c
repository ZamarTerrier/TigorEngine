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

typedef struct{
    float impulse;
    GameObject2D *go;
    RigidBodyFlags flags;
} RigidBody2D;

Camera2D cam2D;
Camera3D cam3D;

ShapeObject so;
ShapeObject so2;
ShapeObject so3;
ShapeObject so4;

RigidBody2D bodies[8];
uint32_t num_bodies = 4;

extern TEngine engine;

extern float RAD2DEG(float x);

float start_rot = 35, grav = -1.0f, last_angle = 0;

void CalcRot(vec2 dir, GameObject2D *shape, float ang, float impulse){

    float res = ang * impulse;

    vec2 pos = Transform2DGetPosition(shape);
    float rot = Transform2DGetRotation(shape);

    if(dir.x < 0)
    {
        last_angle += ang;
        rot += res;
        pos.x -= res;
    }else{   
        last_angle -= ang;                 
        rot -= res;
        pos.x += res;
    }

    Transform2DSetPosition(shape, pos.x, pos.y);
    Transform2DSetRotation(shape, rot);
}

extern Point2D GetVertices2D(float rot, vec2 pos, vec2 size, vec3 color);

void ResolveCollisions(RigidBody2D *body, Manfloid2D *coll, float dTime){
    
    if(!(body->flags & TIGOR_RIGIDBODY_DYNAMIC))
        return;

    vec2 normal = vec2_f(0, 1), dir;

    char buff[2048];
    
    vec2 pos2 = Transform2DGetPosition(body->go);
    float rot2 = Transform2DGetRotation(body->go);
    vec2 size = Transform2DGetScale(body->go);
    
    Point2D points2 = GetVertices2D(rot2, pos2, Transform2DGetScale(body->go), vec3_f(0, 1, 0));

    if(coll->collisions.num_points > 0){
        
        vec2 temp = coll->collisions.points[0];
        
        float mass = 1.0f;

        float weight = 1 / mass / 4;

        bool stable = false;
        if(coll->collisions.num_points  > 1){
            vec2 p1_norm = v2_norm(v2_sub(pos2, coll->collisions.points[0]));
            int find = 0;
            for(int i=1;i<coll->collisions.num_points;i++){
                temp = v2_add(temp, coll->collisions.points[i]);

                vec2 t_n = v2_norm(v2_sub(pos2, coll->collisions.points[i]));

                float t_d = fabs(v2_dot(p1_norm, t_n));
                
                float f_angle = fabs(atan2(coll->collisions.points[0].y - coll->collisions.points[i].y, coll->collisions.points[0].x - coll->collisions.points[i].x));

                if(t_d > 0.9f || f_angle <= FLT_EPSILON){
                    stable = true;
                    find = 1;
                }
                
                memset(buff, 0, 256);
                sprintf(buff, "Point %i angle is : %0.2f | dot is : %0.2f \0", i, f_angle, t_d);
                GUIAddText(50, 150 + i * 50, vec3_f(1, 1, 1), 9, buff);


                if(!find)
                    break;
            }
            
            temp = v2_divs(temp, coll->collisions.num_points);

            float dist = v2_distance(coll->collisions.points[0], coll->collisions.points[1]);
            for(int i=0;i<coll->collisions.num_points;i++){
                for(int j=0;j<coll->collisions.num_points;j++){
                    if(i == j)
                        continue;

                    float t_dist =  v2_distance(coll->collisions.points[i], coll->collisions.points[j]);

                    if(dist > t_dist)
                        dist = t_dist;

                }
            }

            vec2 n_norm = v2_norm(m2_v2_mult(m2_rotate(rot2), normal));
            vec2 p_norm = v2_norm(v2_sub(pos2, temp));

            float d_dot = fabs(v2_dot(n_norm, p_norm));

            if(/*d_dot < 0.85f ||*/ dist < size.x / 4.f){
                stable = false;
            }else
            {

                int stable_count = 0;
                
                for(int j=0;j<coll->collisions.num_points;j++){
                    vec2 t_point = v2_sub(pos2, coll->collisions.points[j]);
                    for(int i=0;i<4;i++){
                        
                        vec2 t_point2 = v2_sub(pos2, points2.points[i]);
                        float f_angle = fabs(atan2(t_point.y - t_point2.y, t_point.x - t_point2.x));

                        if(f_angle < 1.0f){
                            stable_count ++;
                        }

                        
                        memset(buff, 0, 256);
                        sprintf(buff, "Point %i angle is : %0.2f | dot is : %0.2f \0", i, f_angle, d_dot);
                        GUIAddText(550, 250 + i * 50, vec3_f(1, 1, 1), 9, buff);
                    }

                    if(stable_count >= 2)
                        stable = true;
                    else{
                        stable = false;
                        break;
                    }
                }
            }

        }else{
            temp = coll->collisions.points[0];
            stable = false;
        }


        if(stable)
        {
            body->impulse -= 1.0f * dTime;

            if(body->impulse < 0)
                body->impulse = 0;
        }
        

        if(!stable){

            dir = v2_muls(v2_norm(v2_sub(coll->collisions.points[0], pos2)), 0.15);
            pos2 = v2_sub(pos2, v2_muls(dir, coll->depths[0]));
            Transform2DSetPosition(body->go, pos2.x, pos2.y);
        
            float g_angle = 0;

            dir = v2_sub(pos2, temp);
            dir = v2_norm(dir);

            vec2 normal = vec2_f(0, 1);
      
            vec2 t_pos2 = pos2;

            if(coll->collisions.num_points == 1){

                last_angle = 0;

                vec2 t_vec = v2_norm(v2_sub(t_pos2, coll->collisions.points[0]));
                    
                float ang = atan2(normal.y - t_vec.y, normal.x - t_vec.x);
                
                CalcRot(dir, body->go, ang, body->impulse);
                
                body->impulse += 0.01f * dTime;

            }else{
                
                float f_angle = fabs(atan2(coll->collisions.points[1].y - coll->collisions.points[0].y, coll->collisions.points[1].x - coll->collisions.points[0].x));
        
                //if(f_angle > FLT_EPSILON)
                {                    
                    last_angle = 0;
                    for(int i=0;i<4;i++)
                    {
                        vec2 t_vec = v2_sub(t_pos2, points2.points[i]);
                        for(int j=0;j < coll->collisions.num_points;j++)
                        {
                            vec2 t_p = v2_sub(t_pos2, coll->collisions.points[j]);
                            
                            float ang = atan2(t_vec.y - t_p.y, t_vec.x - t_p.x) * weight;
                            
                            CalcRot(dir, body->go, ang, body->impulse);
                        }
                    }
                    

                    float ang = atan2(pos2.y - temp.y, pos2.x - temp.x);

                    body->impulse -= 0.01f * dTime;

                    ang *= 180 / M_PI;
                    
                    memset(buff, 0, 256);
                    sprintf(buff, "Current angle is : %0.2f", ang);
                    GUIAddText(50, 150, vec3_f(1, 1, 1), 9, buff);
                }
            }
        }
        
        if(body->impulse < 0)
            body->impulse = 0;
            
        memset(buff, 0, 256);
        sprintf(buff, "Is stable : %s \0", stable ? "yes": "not");

        GUIAddText(50, 50, vec3_f(1, 1, 1), 9, buff);
    }else{
        pos2.y -= grav * dTime;

        rot2 += last_angle * body->impulse;
        pos2.x -= last_angle * body->impulse;
        
        Transform2DSetPosition(body->go, pos2.x, pos2.y);
        Transform2DSetRotation(body->go, rot2);
    }


    
    memset(buff, 0, 256);
    sprintf(buff, "Intesect is %s", coll->collisions.num_points > 0 ? "intescted" : "not");

    GUIAddText(50, 100, vec3_f(1, 1, 1), 9, buff);
    
    memset(buff, 0, 2048);
    float t_ang = atan2(pos2.y - coll->collisions.points[0].y, pos2.x - coll->collisions.points[0].x);
    sprintf(buff, " Impulse : %.2f | Last angle : %.2f | Point angle : %.2f | Num contacts : %i |\0", body->impulse, last_angle, t_ang, coll->collisions.num_points);

    GUIAddText(400, 100, vec3_f(1, 1, 1), 9, buff);

    memset(buff, 0, 2048);
    sprintf(buff, " Point 0 is : x - %.2f, y - %.2f, Center : x - %.2f, y -%.2f | All : x - %.2f, y - %.2f |\0", coll->collisions.points[0].x, coll->collisions.points[0].y, pos2.x, pos2.y, dir.x, dir.y);

    GUIAddText(400, 150, vec3_f(1, 1, 1), 9, buff);

    if(coll->collisions.num_points > 0){
        memset(buff, 0, 2048);
        vec2 point_norm = v2_norm(v2_sub(pos2, coll->collisions.points[0]));
        sprintf(buff, " Dot product is : %.2f | Point norm : x - %.2f, y - %.2f | \0", v2_dot(normal, point_norm), point_norm.x, point_norm.y);
        GUIAddText(550, 200, vec3_f(1, 1, 1), 9, buff);
    }
}

Manfloid2D CheckOOBOOBCollision(RigidBody2D *body1, RigidBody2D *body2, float dTime){
    return IntersectionSquareOOBSquareOOB(body1->go, body2->go);
}

void Update(float dTime){
    
    double time = TEngineGetTime();

    float cnst = 256;

    Manfloid2D result;
    for(int i=0;i < num_bodies;i++){
        memset(&result, 0, sizeof(Manfloid2D));
        for(int j=0;j < num_bodies;j++){
            if(i == j)
                continue;

            Manfloid2D temp = CheckOOBOOBCollision(&bodies[i], &bodies[j], dTime);

            memcpy(result.collisions.points + result.collisions.num_points, temp.collisions.points, sizeof(vec2) * temp.collisions.num_points);
            memcpy(result.depths + result.collisions.num_points, temp.depths, sizeof(float) * temp.collisions.num_points);
            result.collisions.num_points += temp.collisions.num_points;
        }

        
        for(int i=0;i < result.collisions.num_points;i++)
            GUIAddCircleFilled(result.collisions.points[i], 4, vec3_f(0, 0, 0), 0);

        ResolveCollisions(&bodies[i], &result, dTime);
    }    
}


int main(){

    memset(bodies, 0, sizeof(RigidBody2D) * num_bodies);

    TEngineInitSystem(800, 600, "Test");

    Camera2DInit(&cam2D);
    Camera3DInit(&cam3D);
        
    DrawParam dParam;
    memset(&dParam, 0, sizeof(DrawParam));

    dParam.diffuse = "res\\texture.jpg";
    //dParam.normal = "res\\normal.jpg";

    ShapeObjectInit(&so, &dParam, TIGOR_SHAPE_OBJECT_QUAD, NULL);
    Transform2DSetPosition(&so, 100, 400);    
    Transform2DSetScale(&so, 50, 50);    

    ShapeObjectInit(&so2, &dParam, TIGOR_SHAPE_OBJECT_QUAD, NULL);
    Transform2DSetPosition(&so2, 140, 250);    
    Transform2DSetScale(&so2, 50, 50);  
    Transform2DSetRotation(&so2, start_rot);  
    
    ShapeObjectInit(&so3, &dParam, TIGOR_SHAPE_OBJECT_QUAD, NULL);
    Transform2DSetPosition(&so3, 100, 500);    
    Transform2DSetScale(&so3, 400, 50);  

    ShapeObjectInit(&so4, &dParam, TIGOR_SHAPE_OBJECT_QUAD, NULL);
    Transform2DSetPosition(&so4, 100, 100);    
    Transform2DSetScale(&so4, 50, 50);   
    Transform2DSetRotation(&so4, 24);  

    bodies[0].go = &so;
    bodies[1].go = &so2;
    bodies[1].flags = TIGOR_RIGIDBODY_DYNAMIC;
    bodies[2].go = &so3;
    bodies[3].go = &so4;
    bodies[3].flags = TIGOR_RIGIDBODY_DYNAMIC;

    //Camera2DSetPosition(-engine.width / 2, 0);
    while (!TEngineWindowIsClosed())
    {
        TEnginePoolEvents();

        Update(1.0f);
        
        TEngineDraw(&so);
        TEngineDraw(&so2);
        TEngineDraw(&so3);
        TEngineDraw(&so4);

        TEngineRender();
    }
    
    GameObjectDestroy((GameObject *)&so);
    GameObjectDestroy((GameObject *)&so2);
    GameObjectDestroy((GameObject *)&so3);
    GameObjectDestroy((GameObject *)&so4);
    
    EngineDeviceWaitIdle();
    
    TEngineCleanUp();

    return 0;
}