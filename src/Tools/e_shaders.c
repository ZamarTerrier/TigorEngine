#include "Tools/e_shaders.h"

#include <spirv-headers/GLSL.std.450.h>

void ShadersMakeDefault2DShader(ShaderBuilder *vert, ShaderBuilder *frag, bool hasTexture){

    memset(vert, 0, sizeof(ShaderBuilder));
    memset(frag, 0, sizeof(ShaderBuilder));

    //------------------------------------------------------
    {
        ShaderBuilderInit(vert, SHADER_TYPE_VERTEX);

        ShaderStructConstr uniform_arr[] = {
            {SHADER_VARIABLE_TYPE_MATRIX, 4, 0, "model", NULL, 0, NULL},
            {SHADER_VARIABLE_TYPE_MATRIX, 4, 0, "view", NULL, 0, NULL},
            {SHADER_VARIABLE_TYPE_MATRIX, 4, 0, "proj", NULL, 0, NULL},
        };

        uint32_t uniform = ShaderBuilderAddUniform(uniform_arr, 3, "TransformBufferObjects", 0, 1);

        uint32_t posit = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 2, "position", 0, 0);
        uint32_t clr_indx = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "color", 1, 0);
        uint32_t txt_indx = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 2, "inTexCoord", 2, 0);

        uint32_t clr_dst = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 3, "fragColor", 0, 0);
        uint32_t txt_dst = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 2, "fragTexCoord", 1, 0);

        uint32_t res = ShaderBuilderAddFuncMult(uniform, 1, SHADER_VARIABLE_TYPE_MATRIX, 4, uniform, 0, SHADER_VARIABLE_TYPE_MATRIX, 4, 4);
        res = ShaderBuilderAddFuncMult(uniform, 2, SHADER_VARIABLE_TYPE_MATRIX, 4, res, 0, SHADER_VARIABLE_TYPE_MATRIX, 4, 4);
        res = ShaderBuilderAddFuncMult(res, 0, SHADER_VARIABLE_TYPE_MATRIX, 4, posit, 0, SHADER_VARIABLE_TYPE_VECTOR, 2, 4);

        uint32_t acc = ShaderBuilderAcceptAccess(vert->gl_struct_indx, SHADER_VARIABLE_TYPE_VECTOR, 4, (uint32_t []){ 0 }, 1, false);
        ShaderBuilderStoreValue((uint32_t []){ acc, res }, 2);

        ShaderBuilderAddFuncMove(clr_indx, 3, clr_dst, 3);
        ShaderBuilderAddFuncMove(txt_indx, 2, txt_dst, 2);

        ShaderBuilderMake();
    }
    //----------------------------------------
    {
        ShaderBuilderInit(frag, SHADER_TYPE_FRAGMENT);

        ShaderStructConstr uniform_arr_2[] = {
            {SHADER_VARIABLE_TYPE_MATRIX, 4, 0, "model", NULL, 0, NULL},
            {SHADER_VARIABLE_TYPE_MATRIX, 4, 0, "proj", NULL, 0, NULL},
        };

        uint32_t uniform2 = ShaderBuilderAddUniform(uniform_arr_2, 2, "ImageBufferObjects", 0, 2);

        uint32_t fragColor = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "fragColor", 0, 0);
        uint32_t fragTexCoord = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 2, "fragTexCoord", 1, 0);
        uint32_t outColor = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 4, "outColor", 0, 0);

        if(hasTexture){
            uint32_t texture = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_IMAGE, SHADER_DATA_FLAG_UNIFORM_CONSTANT, NULL, 0, "Texture2D", 0, 3);
            
            uint32_t res = ShaderBuilderAddFuncMult(uniform2, 0, SHADER_VARIABLE_TYPE_MATRIX, 4, fragTexCoord, 0, SHADER_VARIABLE_TYPE_VECTOR, 2, 4);
            //res = ShaderBuilderAddFuncMult(res, 0, SHADER_VARIABLE_TYPE_MATRIX, 4, fragTexCoord, 0, SHADER_VARIABLE_TYPE_VECTOR, 2, 4);
            
            VectorExtract extr = ShaderBuilderGetElemenets(SHADER_VARIABLE_TYPE_FLOAT, res, 4, 0, 2);

            uint32_t vec_type = ShaderBuilderAddVector(2, NULL);

            res = ShaderBuilderCompositeConstruct((uint32_t []){vec_type, extr.elems[0], extr.elems[1]}, 3);

            uint32_t text = ShaderBuilderGetTexture(texture, res, 0);

            ShaderBuilderStoreValue((uint32_t []){outColor, text}, 2);
        }else{            
            uint32_t res = ShaderBuilderAcceptLoad(fragColor, 0);

            VectorExtract extr = ShaderBuilderGetElemenets(SHADER_VARIABLE_TYPE_FLOAT, res, 3, 0, 3);

            uint32_t vec_type = ShaderBuilderAddVector(4, NULL);

            float v_f = 1.0f;
            uint32_t v_u = 0;
            memcpy(&v_u, &v_f, sizeof(uint32_t));        
            uint32_t cnst = ShaderBuilderAddConstant(SHADER_VARIABLE_TYPE_FLOAT, 0, v_u, 1);

            uint32_t arr[] = { vec_type, extr.elems[0], extr.elems[1], extr.elems[2], cnst };
            res = ShaderBuilderCompositeConstruct(arr, 5);
            
            uint32_t arr2[] = {outColor, res};
            
            ShaderBuilderStoreValue(arr2, sizeof(arr2));
        }

        ShaderBuilderMake();
    }
}

void ShaderMakeDefaultParticle2DShader(ShaderBuilder *vert, ShaderBuilder *frag){

    memset(vert, 0, sizeof(ShaderBuilder));
    memset(frag, 0, sizeof(ShaderBuilder));

//------------------------------------------------------
    {
        ShaderBuilderInit(vert, SHADER_TYPE_VERTEX);
        
        ShaderStructConstr uniform_arr[] = {
            {SHADER_VARIABLE_TYPE_MATRIX, 4, 0, "model", NULL, 0, NULL},
            {SHADER_VARIABLE_TYPE_MATRIX, 4, 0, "view", NULL, 0, NULL},
            {SHADER_VARIABLE_TYPE_MATRIX, 4, 0, "proj", NULL, 0, NULL},
        };

        uint32_t uniform = ShaderBuilderAddUniform(uniform_arr, 3, "TransformBufferObjects", 0, 1);

        uint32_t posit = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 2, "position", 0, 0);
        uint32_t p_size = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_FLOAT, 0, NULL, 0, "size", 1, 0);
        uint32_t clr_indx = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "color", 2, 0);
        
        uint32_t clr_dst = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 3, "fragColor", 0, 0);

        float f = 100.0f;
        uint32_t c = 0;
        memcpy(&c, &f, sizeof(uint32_t));
        uint32_t cons = ShaderBuilderAddConstant(SHADER_VARIABLE_TYPE_FLOAT, 0, c, 1);

        uint32_t res = ShaderBuilderAddFuncMult(cons, 0, SHADER_VARIABLE_TYPE_FLOAT, 0, p_size, 0, SHADER_VARIABLE_TYPE_FLOAT, 0, 0);
        uint32_t acc = ShaderBuilderAcceptAccess(vert->gl_struct_indx, SHADER_VARIABLE_TYPE_FLOAT, 0, (uint32_t []){ 1 }, 1, false);
        ShaderBuilderStoreValue((uint32_t []){ acc,  res}, 2);

        res = ShaderBuilderAddFuncMult(uniform, 1, SHADER_VARIABLE_TYPE_MATRIX, 4, uniform, 0, SHADER_VARIABLE_TYPE_MATRIX, 4, 4);
        res = ShaderBuilderAddFuncMult(uniform, 2, SHADER_VARIABLE_TYPE_MATRIX, 4, res, 0, SHADER_VARIABLE_TYPE_MATRIX, 4, 4);
        res = ShaderBuilderAddFuncMult(res, 0, SHADER_VARIABLE_TYPE_MATRIX, 4, posit, 0, SHADER_VARIABLE_TYPE_VECTOR, 2, 4);
        acc = ShaderBuilderAcceptAccess(vert->gl_struct_indx, SHADER_VARIABLE_TYPE_VECTOR, 4, (uint32_t []){ 0 }, 1, false);
        ShaderBuilderStoreValue((uint32_t []){ acc,  res}, 2);

        ShaderBuilderAddFuncMove(clr_indx, 3, clr_dst, 3);

        ShaderBuilderMake();
    }
//----------------------------------------
    {
        ShaderBuilderInit(frag, SHADER_TYPE_FRAGMENT);
        
        uint32_t fragColor = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "fragColor", 0, 0);
        uint32_t outColor = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 4, "outColor", 0, 0);
        
        uint32_t gl_Point_Coord = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_SYSTEM, NULL, 2, "gl_PointCoord", 0, 16);

        uint32_t texture = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_IMAGE, SHADER_DATA_FLAG_UNIFORM_CONSTANT, NULL, 0, "Texture2D", 0, 2);
        
        uint32_t res = ShaderBuilderGetTexture(texture, gl_Point_Coord, 0);

        uint32_t l_kill = ShaderBuilderNextLabel(false, 1);

        uint32_t end_label = ShaderBuilderNextLabel(true, 2);
        
        float f = 0.09f;
        uint32_t c = 0;
        memcpy(&c, &f, sizeof(uint32_t));
        uint32_t cons = ShaderBuilderAddConstant(SHADER_VARIABLE_TYPE_FLOAT, 0, c, 1);

        VectorExtract extr = ShaderBuilderGetElemenets(SHADER_VARIABLE_TYPE_FLOAT, res, 4, 3, 1);

        ShaderBuilderMakeBranchConditional(SHADER_CONDITIONAL_TYPE_FLESS_THAN, (uint32_t []){ extr.elems[0], cons}, 2, end_label, l_kill, end_label);

        ShaderBuilderSetCurrentLabel(l_kill);

        ShaderBuilderMakeKill();

        ShaderBuilderSetCurrentLabel(end_label);

        res = ShaderBuilderAddFuncMult(res, 0, SHADER_VARIABLE_TYPE_VECTOR, 4, fragColor, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, 4);

        ShaderBuilderStoreValue((uint32_t []){outColor, res}, 2);

        ShaderBuilderMake();
    }
}

void ShadersMakeDefault3DShader(ShaderBuilder *vert, ShaderBuilder *frag, bool hasTexture){

    memset(vert, 0, sizeof(ShaderBuilder));
    memset(frag, 0, sizeof(ShaderBuilder));

//------------------------------------------------------
    ShaderBuilderInit(vert, SHADER_TYPE_VERTEX);

    ShaderStructConstr uniform_arr[] = {
        {SHADER_VARIABLE_TYPE_MATRIX, 4, 0, "model", NULL, 0, NULL},
        {SHADER_VARIABLE_TYPE_MATRIX, 4, 0, "view", NULL, 0, NULL},
        {SHADER_VARIABLE_TYPE_MATRIX, 4, 0, "proj", NULL, 0, NULL},
    };

    uint32_t uniform = ShaderBuilderAddUniform(uniform_arr, 3, "ModelBufferObjects", 0, 1);

    uint32_t posit = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "position", 0, 0);
    uint32_t clr_indx = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "color", 1, 0);
    uint32_t txt_indx = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 2, "inTexCoord", 2, 0);

    uint32_t clr_dst = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 3, "fragColor", 0, 0);
    uint32_t txt_dst = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 2, "fragTexCoord", 1, 0);

    uint32_t res = ShaderBuilderAddFuncMult(uniform, 1, SHADER_VARIABLE_TYPE_MATRIX, 4, uniform, 0, SHADER_VARIABLE_TYPE_MATRIX, 4, 4);
    res = ShaderBuilderAddFuncMult(uniform, 2,  SHADER_VARIABLE_TYPE_MATRIX, 4, res, 0, SHADER_VARIABLE_TYPE_MATRIX, 4, 4);
    res = ShaderBuilderAddFuncMult(res, 0,  SHADER_VARIABLE_TYPE_MATRIX, 4, posit, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, 4);
    
    uint32_t glPos = ShaderBuilderAcceptAccess(vert->gl_struct_indx, SHADER_VARIABLE_TYPE_VECTOR, 4, (uint32_t []){ 0 }, 1, false);
    ShaderBuilderStoreValue((uint32_t []){ glPos, res}, 2);
    
    ShaderBuilderAddFuncMove(clr_indx, 3, clr_dst, 3);
    ShaderBuilderAddFuncMove(txt_indx, 2, txt_dst, 2);

    ShaderBuilderMake();
//----------------------------------------
    ShaderBuilderInit(frag, SHADER_TYPE_FRAGMENT);

    uint32_t fragColor = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "fragColor", 0, 0);
    uint32_t fragTexCoord = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 2, "fragTexCoord", 1, 0);
    uint32_t outColor = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 4, "outColor", 0, 0);


    if(hasTexture){
        
        uint32_t texture = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_IMAGE, SHADER_DATA_FLAG_UNIFORM_CONSTANT, NULL, 0, "Texture2D", 0, 2);
        
        uint32_t res = ShaderBuilderAcceptLoad(fragTexCoord, 0);
        uint32_t text = ShaderBuilderGetTexture(texture, res, 0);

        ShaderBuilderStoreValue((uint32_t []){outColor, text}, 2);
    }else{
        
        uint32_t res = ShaderBuilderAcceptLoad(fragColor, 0);

        VectorExtract extr = ShaderBuilderGetElemenets(SHADER_VARIABLE_TYPE_FLOAT, res, 3, 0, 3);

        uint32_t vec_type = ShaderBuilderAddVector(4, NULL);

        float v_f = 1.0f;
        uint32_t v_u = 0;
        memcpy(&v_u, &v_f, sizeof(uint32_t));        
        uint32_t cnst = ShaderBuilderAddConstant(SHADER_VARIABLE_TYPE_FLOAT, 0, v_u, 1);

        uint32_t arr[] = { vec_type, extr.elems[0], extr.elems[1], extr.elems[2], cnst };
        res = ShaderBuilderCompositeConstruct(arr, 5);
        
        uint32_t arr2[] = {outColor, res};
        
        ShaderBuilderStoreValue(arr2, sizeof(arr2));
    }

    ShaderBuilderMake();
}

void MakeBaseLightCode(uint32_t texture, uint32_t normal, uint32_t indx_mat_num, uint32_t fragNormal, uint32_t fragTexCoord, uint32_t fragPos, uint32_t lights, uint32_t outColor){

            float v = 0.1f;
            uint32_t c = 0;
            memcpy(&c, &v, sizeof(uint32_t));
            uint32_t c_c1 = ShaderBuilderAddConstantComposite(SHADER_VARIABLE_TYPE_VECTOR, 3, c);
            uint32_t c_c2 = ShaderBuilderAddConstantComposite(SHADER_VARIABLE_TYPE_VECTOR, 3, 0);
            
            uint32_t c_1 = ShaderBuilderAddConstant(SHADER_VARIABLE_TYPE_FLOAT, 0, 0, 1);
            uint32_t c_2 = ShaderBuilderAddConstant(SHADER_VARIABLE_TYPE_INT, 0, 0, 1);
            
            v = 1.0f;
            memcpy(&c, &v, sizeof(uint32_t));
            uint32_t c_3 = ShaderBuilderAddConstant(SHADER_VARIABLE_TYPE_FLOAT, 0, c, 1);
            
            v = 0.5f;
            memcpy(&c, &v, sizeof(uint32_t));
            uint32_t quadr = ShaderBuilderAddConstant(SHADER_VARIABLE_TYPE_FLOAT, 0, c, 1);

            uint32_t pointer_vec3 = ShaderBuilderAddPointer(SHADER_VARIABLE_TYPE_VECTOR, 3, SHADER_DATA_FLAG_FUNCTION /*Function*/);

            uint32_t arr[] = { pointer_vec3,  7 /*Function*/};
            uint32_t v_result = ShaderBuilderAddOperand(arr, 2, SHADER_OPERAND_TYPE_VARIABLE);
            uint32_t v_some_vec = ShaderBuilderAddOperand(arr, 2, SHADER_OPERAND_TYPE_VARIABLE);
            uint32_t v_normal = ShaderBuilderAddOperand(arr, 2, SHADER_OPERAND_TYPE_VARIABLE);
            uint32_t v_lightDir = ShaderBuilderAddOperand(arr, 2, SHADER_OPERAND_TYPE_VARIABLE);
            uint32_t v_textureColor = ShaderBuilderAddOperand(arr, 2, SHADER_OPERAND_TYPE_VARIABLE);
            uint32_t v_lightColor = ShaderBuilderAddOperand(arr, 2, SHADER_OPERAND_TYPE_VARIABLE);
            
            uint32_t pointer_int = ShaderBuilderAddPointer(SHADER_VARIABLE_TYPE_INT, 1, SHADER_DATA_FLAG_FUNCTION /*Function*/);
            uint32_t pointer_float = ShaderBuilderAddPointer(SHADER_VARIABLE_TYPE_FLOAT, 0, SHADER_DATA_FLAG_FUNCTION /*Function*/);

            uint32_t arr2[] = { pointer_int,  7 /*Function*/};
            uint32_t i_iter = ShaderBuilderAddOperand(arr2, 2, SHADER_OPERAND_TYPE_VARIABLE);
            ShaderBuilderStoreValue((uint32_t []){ i_iter,  c_2}, 2);
            
            uint32_t arr3[] = { pointer_float,  7 /*Function*/};
            uint32_t f_diff = ShaderBuilderAddOperand(arr3, 2, SHADER_OPERAND_TYPE_VARIABLE);
            uint32_t f_atten = ShaderBuilderAddOperand(arr3, 2, SHADER_OPERAND_TYPE_VARIABLE);
            uint32_t f_theta = ShaderBuilderAddOperand(arr3, 2, SHADER_OPERAND_TYPE_VARIABLE);
            uint32_t f_lightResult = ShaderBuilderAddOperand(arr3, 2, SHADER_OPERAND_TYPE_VARIABLE);
            ShaderBuilderStoreValue((uint32_t []){ f_diff,  c_1}, 2);

            uint32_t type_vec3 = ShaderBuilderAddVector(3, NULL);
            
            uint32_t arr4[] = { v_result,  c_c2};
            ShaderBuilderStoreValue(arr4, 2);
            
            uint32_t tex_res = ShaderBuilderGetTexture(texture, fragTexCoord, indx_mat_num);
            VectorExtract extr = ShaderBuilderGetElemenets(SHADER_VARIABLE_TYPE_FLOAT, tex_res, 4, 3, 1);    

            uint32_t l_kill = ShaderBuilderNextLabel(false, 18);
            uint32_t contin_label = ShaderBuilderNextLabel(false, 1);
            
            float f = 0.1f;
            c = 0;
            memcpy(&c, &f, sizeof(uint32_t));
            uint32_t cons = ShaderBuilderAddConstant(SHADER_VARIABLE_TYPE_FLOAT, 0, c, 1);

            ShaderBuilderMakeBranchConditional(SHADER_CONDITIONAL_TYPE_FLESS_THAN, (uint32_t []){ extr.elems[0], cons}, 2, contin_label, l_kill, contin_label);
    //-------------------------------------------------------------------------------------------------------------------------------------------
            ShaderBuilderSetCurrentLabel(l_kill);

            ShaderBuilderMakeKill();
    //-------------------------------------------------------------------------------------------------------------------------------------------
            ShaderBuilderSetCurrentLabel(contin_label);
    
            tex_res = ShaderBuilderGetTexture(texture, fragTexCoord, indx_mat_num);
            extr = ShaderBuilderGetElemenets(SHADER_VARIABLE_TYPE_FLOAT, tex_res, 4, 0, 3);  
            tex_res = ShaderBuilderCompositeConstruct((uint32_t []){ type_vec3,  extr.elems[0], extr.elems[1], extr.elems[2]}, 4);                
            ShaderBuilderStoreValue((uint32_t []){ v_textureColor,  tex_res}, 2);
            
            if(normal > 0){
                tex_res = ShaderBuilderGetTexture(normal, fragTexCoord, indx_mat_num);        
                extr = ShaderBuilderGetElemenets(SHADER_VARIABLE_TYPE_FLOAT, tex_res, 4, 0, 3);        
                tex_res = ShaderBuilderCompositeConstruct((uint32_t []){ type_vec3,  extr.elems[0], extr.elems[1], extr.elems[2]}, 4);  

                v = 2.0f;
                memcpy(&c, &v, sizeof(uint32_t));
                uint32_t t_const = ShaderBuilderAddConstant(SHADER_VARIABLE_TYPE_FLOAT, 0, c, 1);
                
                tex_res = ShaderBuilderAddFuncMult(tex_res, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, t_const, 0, SHADER_VARIABLE_TYPE_FLOAT, 0, 3);
                tex_res = ShaderBuilderAddFuncSub(tex_res, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, c_3, 0, SHADER_VARIABLE_TYPE_FLOAT, 0, 3);  

                uint32_t type_vec2 = ShaderBuilderAddVector(2, NULL);

                uint32_t acc_1 = ShaderBuilderAcceptLoad(fragPos, 0);
                uint32_t acc_2 = ShaderBuilderAcceptLoad(fragTexCoord, 0);
                uint32_t acc_3 = ShaderBuilderAcceptLoad(fragNormal, 0);
                uint32_t q1 = ShaderBuilderAddOperand((uint32_t []){ type_vec3, acc_1 }, 2, SHADER_OPERAND_TYPE_DPDX);
                uint32_t q2 = ShaderBuilderAddOperand((uint32_t []){ type_vec3, acc_1 }, 2, SHADER_OPERAND_TYPE_DPDY);
                uint32_t st1 = ShaderBuilderAddOperand((uint32_t []){ type_vec2, acc_2 }, 2, SHADER_OPERAND_TYPE_DPDX);
                uint32_t st2 = ShaderBuilderAddOperand((uint32_t []){ type_vec2, acc_2 }, 2, SHADER_OPERAND_TYPE_DPDY);

                uint32_t type_float = ShaderBuilderAddFloat();

                uint32_t N = ShaderBuilderMakeExternalFunction((uint32_t []){type_vec3, acc_3}, 2, GLSLstd450Normalize);

                VectorExtract extr_1 = ShaderBuilderGetElemenets(SHADER_VARIABLE_TYPE_FLOAT, st1, 3, 1, 1);
                VectorExtract extr_2 = ShaderBuilderGetElemenets(SHADER_VARIABLE_TYPE_FLOAT, st2, 3, 1, 1);

                uint32_t t_res_1 = ShaderBuilderAddFuncMult(q1, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, extr_1.elems[0], 0, SHADER_VARIABLE_TYPE_FLOAT, 0, 3);
                uint32_t t_res_2 = ShaderBuilderAddFuncMult(q2, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, extr_2.elems[0], 0, SHADER_VARIABLE_TYPE_FLOAT, 0, 3);

                uint32_t T = ShaderBuilderAddFuncSub(t_res_1, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, t_res_2, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, 3);
                T = ShaderBuilderMakeExternalFunction((uint32_t []){type_vec3, T}, 2, GLSLstd450Normalize);

                uint32_t B = ShaderBuilderMakeExternalFunction((uint32_t []){type_vec3, N, T}, 3, GLSLstd450Cross);
                B = ShaderBuilderMakeExternalFunction((uint32_t []){type_vec3, B}, 2, GLSLstd450Normalize);
                B = ShaderBuilderMakeNegative(B, 0, 3, SHADER_VARIABLE_TYPE_VECTOR);

                uint32_t type_mat3 = ShaderBuilderAddMatrix(3, NULL);

                uint32_t TBN = ShaderBuilderCompositeConstruct((uint32_t []){ type_mat3, T, B, N }, 4);

                tex_res = ShaderBuilderAddFuncMult(TBN, 0, SHADER_VARIABLE_TYPE_MATRIX, 3, tex_res, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, 3);              
                ShaderBuilderStoreValue((uint32_t []){ v_normal,  tex_res}, 2);
            }else{
                uint32_t acc_1 = ShaderBuilderAcceptLoad(fragNormal, 0);
                            
                ShaderBuilderStoreValue((uint32_t []){ v_normal,  acc_1}, 2);
            }

                        
            uint32_t just_color = ShaderBuilderNextLabel(false, 2);
            uint32_t label1 = ShaderBuilderNextLabel(false, 3);
            uint32_t ret_label = ShaderBuilderNextLabel(true, 17);

            uint32_t res = ShaderBuilderAcceptAccess(lights, SHADER_VARIABLE_TYPE_INT, 1, (uint32_t []){ 2 }, 1, true);

            ShaderBuilderMakeBranchConditional(SHADER_CONDITIONAL_TYPE_EQUAL, (uint32_t []){ res, c_2 }, 2, ret_label, just_color, label1);  
            
    //-------------------------------------------------------------------------------------------------------------------------------------------
            ShaderBuilderSetCurrentLabel(just_color);

            res = ShaderBuilderAcceptLoad(v_textureColor, 0);
            res = ShaderBuilderMutateVector(res, 3, 4);

            ShaderBuilderStoreValue((uint32_t []){outColor, res}, 2);
            ShaderBuilderMakeTransition(ret_label);
    //-------------------------------------------------------------------------------------------------------------------------------------------
            ShaderBuilderSetCurrentLabel(label1);
            uint32_t label2 = ShaderBuilderNextLabel(false, 5);
            uint32_t end_label = ShaderBuilderNextLabel(false, 16);
            uint32_t label4 = ShaderBuilderNextLabel(false, 15);
            uint32_t label5 = ShaderBuilderNextLabel(false, 4);
            ShaderBuilderAddOperand((uint32_t []){end_label, label4}, 2, SHADER_OPERAND_TYPE_LOOP);                
            ShaderBuilderMakeTransition(label5);

    //-------------------------------------------------------------------------------------------------------------------------------------------
            uint32_t acc = ShaderBuilderAcceptLoad(i_iter, 0);

            res = ShaderBuilderAcceptAccess(lights, SHADER_VARIABLE_TYPE_INT, 1, (uint32_t []){ 1 }, 1, true);

            ShaderBuilderMakeBranchConditional(SHADER_CONDITIONAL_TYPE_SLESS_THAN, (uint32_t []){ acc, res }, 2, 0, label2, end_label);
    //-------------------------------------------------------------------------------------------------------------------------------------------
            ShaderBuilderSetCurrentLabel(label2);
            
            acc = ShaderBuilderAcceptLoad(i_iter, 0);

            uint32_t var1 = ShaderBuilderAcceptAccess(lights, SHADER_VARIABLE_TYPE_VECTOR, 3, (uint32_t []){ 0, acc, 1 }, 3, true);
            uint32_t var2 = ShaderBuilderAcceptAccess(lights, SHADER_VARIABLE_TYPE_FLOAT, 0, (uint32_t []){ 0, acc, 3 }, 3, true);

            res = ShaderBuilderAddFuncMult(var1, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, var2, 0, SHADER_VARIABLE_TYPE_FLOAT, 0, 3);

            ShaderBuilderStoreValue((uint32_t []){v_lightColor, res}, 2);
            ShaderBuilderStoreValue((uint32_t []){f_diff, c_1}, 2);
            ShaderBuilderStoreValue((uint32_t []){f_lightResult, c_3}, 2);

            var1 = ShaderBuilderAcceptAccess(lights, SHADER_VARIABLE_TYPE_INT, 1, (uint32_t []){ 0, acc, 4 }, 3, true);
            uint32_t sel_merg = ShaderBuilderNextLabel(false, 14);
            uint32_t label6 = ShaderBuilderNextLabel(false, 6);
            uint32_t label7 = ShaderBuilderNextLabel(false, 7);
            ShaderBuilderMakeBranchConditional(SHADER_CONDITIONAL_TYPE_EQUAL, (uint32_t []){ c_2, var1 }, 2, sel_merg, label6, label7);   
    //-------------------------------------------------------------------------------------------------------------------------------------------   
            ShaderBuilderSetCurrentLabel(label6);
                     
            acc = ShaderBuilderAcceptLoad(i_iter, 0);
            var1 = ShaderBuilderAcceptAccess(lights, SHADER_VARIABLE_TYPE_VECTOR, 3, (uint32_t []){ 0, acc, 0 }, 3, true);        
            var2 = ShaderBuilderAcceptLoad(fragPos, 0);       

            res = ShaderBuilderAddFuncSub(var1, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, var2, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, 3);
            res = ShaderBuilderMakeExternalFunction((uint32_t []){ type_vec3, res}, 2, GLSLstd450Normalize);
            ShaderBuilderStoreValue((uint32_t []){v_lightDir, res}, 2);

            var1 = ShaderBuilderAcceptLoad(v_normal, 0);
            var1 = ShaderBuilderMakeExternalFunction((uint32_t []){ type_vec3, var1}, 2, GLSLstd450Normalize);
            var2 = ShaderBuilderAcceptLoad(v_lightDir, 0);

            res = ShaderBuilderMakeDotProduct(var1, 0, var2, 0);

            uint32_t f_type = ShaderBuilderAddFloat();

            res = ShaderBuilderMakeExternalFunction((uint32_t []){ f_type, res, c_1}, 3, GLSLstd450FMax);
            ShaderBuilderStoreValue((uint32_t []){f_diff, res}, 2);
            
            {                
                var1 = ShaderBuilderAcceptLoad(fragPos, 0);

                acc = ShaderBuilderAcceptLoad(i_iter, 0);
                var2 = ShaderBuilderAcceptAccess(lights, SHADER_VARIABLE_TYPE_VECTOR, 3, (uint32_t []){ 0, acc, 0 }, 3, true);

                res = ShaderBuilderAddFuncSub(var2, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, var1, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, 3); 

                res = ShaderBuilderMakeExternalFunction((uint32_t []){ f_type, res}, 2, GLSLstd450Length);

                uint32_t att =  ShaderBuilderAddFuncMult(res, 0, SHADER_VARIABLE_TYPE_FLOAT, 0, res, 0, SHADER_VARIABLE_TYPE_FLOAT, 0, 3); 

                var1 = ShaderBuilderAcceptAccess(lights, SHADER_VARIABLE_TYPE_FLOAT, 3, (uint32_t []){ 0, acc, 7 }, 3, true);
                
                att = ShaderBuilderAddFuncDiv(var1, 0, SHADER_VARIABLE_TYPE_FLOAT, 0, att, 0, SHADER_VARIABLE_TYPE_FLOAT, 0, 3);

                ShaderBuilderStoreValue((uint32_t []){ f_atten, att }, 2);
            }
            
            ShaderBuilderMakeTransition(sel_merg);

    //-------------------------------------------------------------------------------
            ShaderBuilderSetCurrentLabel(label7);

            uint32_t cnst_2 = ShaderBuilderAddConstant(SHADER_VARIABLE_TYPE_INT, 0, 1, 1);

            uint32_t label8 = ShaderBuilderNextLabel(false, 8);
            uint32_t label9 = ShaderBuilderNextLabel(false, 9);
            uint32_t label_t_1 = ShaderBuilderNextLabel(false, 13);
            uint32_t label_t_2 = ShaderBuilderNextLabel(false, 12);
            uint32_t label_t_3 = ShaderBuilderNextLabel(false, 11);
            acc = ShaderBuilderAcceptLoad(i_iter, 0);
            var1 = ShaderBuilderAcceptAccess(lights, SHADER_VARIABLE_TYPE_INT, 1, (uint32_t []){ 0, acc, 4 }, 3, true);
            ShaderBuilderMakeBranchConditional(SHADER_CONDITIONAL_TYPE_EQUAL, (uint32_t []){ var1, cnst_2 }, 2, label_t_1, label8, label9);  
    //-------------------------------------------------------------------------------
            ShaderBuilderSetCurrentLabel(label_t_1);
            ShaderBuilderMakeTransition(sel_merg);
    //-------------------------------------------------------------------------------
            ShaderBuilderSetCurrentLabel(label8);
            acc = ShaderBuilderAcceptLoad(i_iter, 0);
            res = ShaderBuilderAcceptAccess(lights, SHADER_VARIABLE_TYPE_VECTOR, 3, (uint32_t []){ 0, acc, 2 }, 3, true);
            res = ShaderBuilderMakeExternalFunction((uint32_t []){ type_vec3, res}, 2, GLSLstd450Normalize);
            ShaderBuilderStoreValue((uint32_t []){v_lightDir, res}, 2);

            var1 = ShaderBuilderAcceptLoad(v_normal, 0);
            var1 = ShaderBuilderMakeExternalFunction((uint32_t []){ type_vec3, var1}, 2, GLSLstd450Normalize);
            var2 = ShaderBuilderAcceptLoad(v_lightDir, 0);

            res = ShaderBuilderMakeDotProduct(var1, 0, var2, 0);

            res = ShaderBuilderMakeExternalFunction((uint32_t []){ f_type, res, c_1}, 3, GLSLstd450FMax);
            ShaderBuilderStoreValue((uint32_t []){f_diff, res}, 2);

            ShaderBuilderStoreValue((uint32_t []){ f_atten, c_3}, 2);

            ShaderBuilderMakeTransition(label_t_1);

    //-------------------------------------------------------------------------------
            ShaderBuilderSetCurrentLabel(label9);
            
            cnst_2 = ShaderBuilderAddConstant(SHADER_VARIABLE_TYPE_INT, 0, 2, 1);
            
            uint32_t label10 = ShaderBuilderNextLabel(false, 10);
            acc = ShaderBuilderAcceptLoad(i_iter, 0);
            var1 = ShaderBuilderAcceptAccess(lights, SHADER_VARIABLE_TYPE_INT, 1, (uint32_t []){ 0, acc, 4 }, 3, true);
            ShaderBuilderMakeBranchConditional(SHADER_CONDITIONAL_TYPE_EQUAL, (uint32_t []){ var1, cnst_2 }, 2, label_t_2, label10, label_t_2);  

    //-------------------------------------------------------------------------------
            ShaderBuilderSetCurrentLabel(label_t_2);
            ShaderBuilderMakeTransition(label_t_1);
    //-------------------------------------------------------------------------------
            ShaderBuilderSetCurrentLabel(label_t_3);
            ShaderBuilderMakeTransition(label_t_2);
    //-------------------------------------------------------------------------------
            ShaderBuilderSetCurrentLabel(label10);
            acc = ShaderBuilderAcceptLoad(i_iter, 0);
            var1 = ShaderBuilderAcceptAccess(lights, SHADER_VARIABLE_TYPE_VECTOR, 3, (uint32_t []){ 0, acc, 0 }, 3, true);
            var2 = ShaderBuilderAcceptLoad(fragPos, 0);

            res = ShaderBuilderAddFuncSub(var1, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, var2, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, 3);
            res = ShaderBuilderMakeExternalFunction((uint32_t []){ type_vec3, res}, 2, GLSLstd450Normalize);
            ShaderBuilderStoreValue((uint32_t []){v_lightDir, res}, 2);
            
            var1 = ShaderBuilderAcceptLoad(v_normal, 0);
            var1 = ShaderBuilderMakeExternalFunction((uint32_t []){ type_vec3, var1}, 2, GLSLstd450Normalize);
            var2 = ShaderBuilderAcceptLoad(v_lightDir, 0);
            res = ShaderBuilderMakeDotProduct(var1, 0, var2, 0);
            res = ShaderBuilderMakeExternalFunction((uint32_t []){ f_type, res, c_1}, 3, GLSLstd450FMax);
            ShaderBuilderStoreValue((uint32_t []){f_diff, res}, 2);
            
            var1 = ShaderBuilderAcceptLoad(v_lightDir, 0);
            var2 = ShaderBuilderAcceptAccess(lights, SHADER_VARIABLE_TYPE_VECTOR, 3, (uint32_t []){ 0, acc, 2 }, 3, true);
            var2 = ShaderBuilderMakeNegative(var2, 0, 3, SHADER_VARIABLE_TYPE_VECTOR);
            var2 = ShaderBuilderMakeExternalFunction((uint32_t []){ type_vec3, var2}, 2, GLSLstd450Normalize);
            uint32_t theta = ShaderBuilderMakeDotProduct(var1, 0, var2, 0);
            ShaderBuilderStoreValue((uint32_t []){f_theta, theta}, 2);

            {
                var1 = ShaderBuilderAcceptAccess(lights, SHADER_VARIABLE_TYPE_FLOAT, 3, (uint32_t []){ 0, acc, 5 }, 3, true);
                var2 = ShaderBuilderAcceptAccess(lights, SHADER_VARIABLE_TYPE_FLOAT, 3, (uint32_t []){ 0, acc, 6 }, 3, true);
                uint32_t t_att = ShaderBuilderAddFuncSub(var1, 0, SHADER_VARIABLE_TYPE_FLOAT, 0, var2, 0, SHADER_VARIABLE_TYPE_FLOAT, 0, 3); 
                uint32_t att = ShaderBuilderAddFuncSub(theta, 0, SHADER_VARIABLE_TYPE_FLOAT, 0, var2, 0, SHADER_VARIABLE_TYPE_FLOAT, 0, 3); 
                att = ShaderBuilderAddFuncDiv(att, 0, SHADER_VARIABLE_TYPE_FLOAT, 0, t_att, 0, SHADER_VARIABLE_TYPE_FLOAT, 0, 3);
                att = ShaderBuilderMakeExternalFunction((uint32_t []){f_type, att, c_1, c_3}, 4, GLSLstd450FClamp);
                ShaderBuilderStoreValue((uint32_t []){ f_lightResult, att }, 2);
            }

               
            {                

                var1 = ShaderBuilderAcceptLoad(fragPos, 0);

                acc = ShaderBuilderAcceptLoad(i_iter, 0);
                var2 = ShaderBuilderAcceptAccess(lights, SHADER_VARIABLE_TYPE_VECTOR, 3, (uint32_t []){ 0, acc, 0 }, 3, true);

                res = ShaderBuilderAddFuncSub(var2, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, var1, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, 3); 

                res = ShaderBuilderMakeExternalFunction((uint32_t []){ f_type, res}, 2, GLSLstd450Length);

                uint32_t att =  ShaderBuilderAddFuncMult(res, 0, SHADER_VARIABLE_TYPE_FLOAT, 0, res, 0, SHADER_VARIABLE_TYPE_FLOAT, 0, 3); 

                var1 = ShaderBuilderAcceptAccess(lights, SHADER_VARIABLE_TYPE_FLOAT, 3, (uint32_t []){ 0, acc, 7 }, 3, true);
                
                att = ShaderBuilderAddFuncDiv(var1, 0, SHADER_VARIABLE_TYPE_FLOAT, 0, att, 0, SHADER_VARIABLE_TYPE_FLOAT, 0, 3);

                ShaderBuilderStoreValue((uint32_t []){ f_atten, att }, 2);
            }
                        
            ShaderBuilderMakeTransition(label_t_3);          
    //-------------------------------------------------------------------------------
            ShaderBuilderSetCurrentLabel(sel_merg);

            acc = ShaderBuilderAcceptLoad(f_lightResult, 0);
            var1 = ShaderBuilderAcceptLoad(v_textureColor, 0);
            var2 = ShaderBuilderAcceptLoad(f_diff, 0);
            uint32_t acc3 = ShaderBuilderAcceptLoad(v_lightColor, 0);

            uint32_t amb = ShaderBuilderAddFuncMult(acc3, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, var1, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, 3); 
            uint32_t diff = ShaderBuilderAddFuncMult(var1, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, var2, 0, SHADER_VARIABLE_TYPE_FLOAT, 0, 3);
            
            uint32_t att = ShaderBuilderAcceptLoad(f_atten, 0);

            amb = ShaderBuilderAddFuncMult(amb, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, att, 0, SHADER_VARIABLE_TYPE_FLOAT, 0, 3); 
            diff = ShaderBuilderAddFuncMult(diff, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, att, 0, SHADER_VARIABLE_TYPE_FLOAT, 0, 3);
            
            amb = ShaderBuilderAddFuncMult(amb, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, acc, 0, SHADER_VARIABLE_TYPE_FLOAT, 0, 3); 
            diff = ShaderBuilderAddFuncMult(diff, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, acc, 0, SHADER_VARIABLE_TYPE_FLOAT, 0, 3);

            res = ShaderBuilderAddFuncAdd(amb, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, diff, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, 3);
                        
            acc = ShaderBuilderAcceptLoad(v_result, 0);
            res = ShaderBuilderAddFuncAdd(res, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, acc, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, 3);
            ShaderBuilderStoreValue((uint32_t []){v_result, res}, 2);

            ShaderBuilderMakeTransition(label4);

    //-------------------------------------------------------------------------------
            acc = ShaderBuilderAcceptLoad(i_iter, 0);
            cnst_2 = ShaderBuilderAddConstant(SHADER_VARIABLE_TYPE_INT, 0, 1, 1);
            res = ShaderBuilderAddFuncAdd(acc, 0, SHADER_VARIABLE_TYPE_INT, 0, cnst_2, 0, SHADER_VARIABLE_TYPE_INT, 0, 3);
            ShaderBuilderStoreValue((uint32_t []){i_iter, res}, 2);
            ShaderBuilderMakeTransition(label1);
    //-------------------------------------------------------------------------------
            ShaderBuilderSetCurrentLabel(end_label);

            acc = ShaderBuilderAcceptLoad(v_result, 0);
            
            tex_res = ShaderBuilderGetTexture(texture, fragTexCoord, indx_mat_num);
            extr = ShaderBuilderGetElemenets(SHADER_VARIABLE_TYPE_FLOAT, tex_res, 4, 3, 1);  
            
            VectorExtract extr2 = ShaderBuilderGetElemenets(SHADER_VARIABLE_TYPE_FLOAT, acc, 3, 0, 3); 

            uint32_t type_vec4 = ShaderBuilderAddVector(4, NULL); 

            res =  ShaderBuilderCompositeConstruct((uint32_t []){ type_vec4, extr2.elems[0], extr2.elems[1], extr2.elems[2], extr.elems[0] }, 5);
            ShaderBuilderStoreValue((uint32_t []){outColor, res}, 2);

            ShaderBuilderMakeTransition(ret_label);
    //-------------------------------------------------------------------------------
            
}

void ShadersMakeDeafult3DShaderWithLight(ShaderBuilder *vert, ShaderBuilder *frag, bool hasTexture, bool hasNormal, bool hasSpecular){
    memset(vert, 0, sizeof(ShaderBuilder));
    memset(frag, 0, sizeof(ShaderBuilder));


//------------------------------------------------------
    ShaderBuilderInit(vert, SHADER_TYPE_VERTEX);

    ShaderStructConstr uniform_arr[] = {
        {SHADER_VARIABLE_TYPE_MATRIX, 4, 0, "model", NULL, 0, NULL},
        {SHADER_VARIABLE_TYPE_MATRIX, 4, 0, "view" , NULL, 0, NULL},
        {SHADER_VARIABLE_TYPE_MATRIX, 4, 0, "porl" , NULL, 0, NULL},
    };

    uint32_t uniform = ShaderBuilderAddUniform(uniform_arr, 3, "ModelBufferObjects", 0, 1);

    uint32_t posit = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "position", 0, 0);
    uint32_t nrm_indx = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "normal", 1, 0);
    uint32_t clr_indx = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "color", 2, 0);
    uint32_t txt_indx = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 2, "inTexCoord", 3, 0);

    uint32_t frg_pos = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 3, "fragPos", 0, 0);
    uint32_t frg_norm = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 3, "fragNormal", 1, 0);
    uint32_t clr_dst = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 3, "fragColor", 2, 0);
    uint32_t txt_dst = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 2, "fragTexCoord", 3, 0);

    {
        uint32_t res = ShaderBuilderAddFuncMult(uniform, 1, SHADER_VARIABLE_TYPE_MATRIX, 4, uniform, 0, SHADER_VARIABLE_TYPE_MATRIX, 4, 4);
        res = ShaderBuilderAddFuncMult(uniform, 2,  SHADER_VARIABLE_TYPE_MATRIX, 4, res, 0, SHADER_VARIABLE_TYPE_MATRIX, 4, 4);
        res = ShaderBuilderAddFuncMult(res, 0,  SHADER_VARIABLE_TYPE_MATRIX, 4, posit, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, 4);
        
        uint32_t glPos = ShaderBuilderAcceptAccess(vert->gl_struct_indx, SHADER_VARIABLE_TYPE_VECTOR, 4, (uint32_t []){ 0 }, 1, false);
        ShaderBuilderStoreValue((uint32_t []){ glPos, res}, 2);

        res = ShaderBuilderAddFuncMult(uniform, 0, SHADER_VARIABLE_TYPE_MATRIX, 4, posit, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, 4);

        VectorExtract extr = ShaderBuilderGetElemenets(SHADER_VARIABLE_TYPE_FLOAT, res, 4, 0, 4);

        uint32_t v3_type = ShaderBuilderAddVector(3, NULL);
        res = ShaderBuilderCompositeConstruct((uint32_t []){ v3_type, extr.elems[0], extr.elems[1], extr.elems[2] }, 4);
        res = ShaderBuilderAddFuncDiv(res, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, extr.elems[3], 0, SHADER_VARIABLE_TYPE_FLOAT, 0, 3);
        
        uint32_t arr[] = {frg_pos, res};
        ShaderBuilderStoreValue(arr, sizeof(arr));
        
        uint32_t acc = ShaderBuilderAcceptAccess(uniform, SHADER_VARIABLE_TYPE_MATRIX, 4, (uint32_t []){ 0 }, 1, true);

        VectorExtract extr2 = ShaderBuilderGetElemenets(SHADER_VARIABLE_TYPE_VECTOR, acc, 4, 0, 3);

        uint32_t args[3];
        for(int i=0; i < 3;i++){

            extr = ShaderBuilderGetElemenets(SHADER_VARIABLE_TYPE_FLOAT, extr2.elems[i], 4, 0, 3);

            args[i] = ShaderBuilderCompositeConstruct((uint32_t []){ v3_type, extr.elems[0], extr.elems[1], extr.elems[2]}, 4);
        }

        uint32_t mat3_type = ShaderBuilderAddMatrix(3, NULL);
        
        res = ShaderBuilderCompositeConstruct((uint32_t []){ mat3_type, args[0], args[1], args[2] }, 4);
        res = ShaderBuilderMakeExternalFunction((uint32_t []){ mat3_type, res }, 2, GLSLstd450MatrixInverse);
        res = ShaderBuilderAddOperand((uint32_t []){ mat3_type, res }, 2, SHADER_OPERAND_TYPE_TRANSPOSE);
        res = ShaderBuilderAddFuncMult(res, 0, SHADER_VARIABLE_TYPE_MATRIX, 3, nrm_indx, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, 3);
        ShaderBuilderStoreValue((uint32_t []){ frg_norm, res }, 2);

        ShaderBuilderAddFuncMove(clr_indx, 3, clr_dst, 3);
        ShaderBuilderAddFuncMove(txt_indx, 2, txt_dst, 2);
    }

    ShaderBuilderMake();
//----------------------------------------
    ShaderBuilderInit(frag, SHADER_TYPE_FRAGMENT);

    ShaderStructConstr light_str[] = {
        {SHADER_VARIABLE_TYPE_VECTOR, 3, 0, "position", NULL, 0, NULL},
        {SHADER_VARIABLE_TYPE_VECTOR, 3, 0, "color" , NULL, 0, NULL},
        {SHADER_VARIABLE_TYPE_VECTOR, 3, 0, "direction" , NULL, 0, NULL},
        {SHADER_VARIABLE_TYPE_FLOAT , 0, 0, "intensity" , NULL, 0, NULL},
        {SHADER_VARIABLE_TYPE_INT   , 0, 0, "type" , NULL, 0, NULL},
        {SHADER_VARIABLE_TYPE_FLOAT , 0, 0, "cutoff" , NULL, 0, NULL},
        {SHADER_VARIABLE_TYPE_FLOAT , 0, 0, "outCutOff" , NULL, 0, NULL},
        {SHADER_VARIABLE_TYPE_FLOAT , 0, 0, "radius" , NULL, 0, NULL},
    };

    ShaderStructConstr light_arr[] = {
        {SHADER_VARIABLE_TYPE_ARRAY, 10, 0, "lights", light_str, 8, "LightsStruct"},
        {SHADER_VARIABLE_TYPE_INT, 0, 0, "num_lights" , NULL, 0, NULL},
        {SHADER_VARIABLE_TYPE_INT, 0, 0, "light_enable" , NULL, 0, NULL},
    };

    uint32_t lights = ShaderBuilderAddUniform(light_arr, 3, "LightBufferObject", 0, 2);

    uint32_t fragPos = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "fragPos", 0, 0);
    uint32_t fragNormal = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "fragNormal", 1, 0);
    uint32_t fragColor = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "fragColor", 2, 0);
    uint32_t fragTexCoord = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 2, "fragTexCoord", 3, 0);
    
    uint32_t outColor = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 4, "outColor", 0, 0);

    uint32_t texture = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_IMAGE, SHADER_DATA_FLAG_UNIFORM_CONSTANT, NULL, 0, "diffuse", 0, 3);
    uint32_t normal = 0, specular = 0;
    
    if(hasNormal)
        normal = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_IMAGE, SHADER_DATA_FLAG_UNIFORM_CONSTANT, NULL, 0, "normal", 0, 4);

    if(hasSpecular)
        specular = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_IMAGE, SHADER_DATA_FLAG_UNIFORM_CONSTANT, NULL, 0, "specular", 0, 5);

    if(!hasTexture)
    {
        uint32_t res = ShaderBuilderAcceptLoad(fragColor, 0);

        res = ShaderBuilderMutateVector(res, 3, 4);

        ShaderBuilderStoreValue((uint32_t []){outColor, res}, 2);

    }else{

        MakeBaseLightCode(texture, normal, 0, fragNormal, fragTexCoord, fragPos, lights, outColor);
        
    }

    ShaderBuilderMake();
}

void ShadersMakeDefault3DModelShader(ShaderBuilder *vert, ShaderBuilder *frag, uint32_t count_texture){

    memset(vert, 0, sizeof(ShaderBuilder));
    memset(frag, 0, sizeof(ShaderBuilder));

    //------------------------------------------------------
    {
        ShaderBuilderInit(vert, SHADER_TYPE_VERTEX);

        ShaderStructConstr uniform_arr[] = {
            {SHADER_VARIABLE_TYPE_MATRIX, 4, 0, "model", NULL, 0, NULL},
            {SHADER_VARIABLE_TYPE_MATRIX, 4, 0, "view", NULL, 0, NULL},
            {SHADER_VARIABLE_TYPE_MATRIX, 4, 0, "proj", NULL, 0, NULL},
        };

        uint32_t uniform = ShaderBuilderAddUniform(uniform_arr, 3, "ModelBufferObjects", 0, 1);

        ShaderStructConstr mat_ptr[] = {
            {SHADER_VARIABLE_TYPE_MATRIX, 4, 0, "mat4", NULL, 0, NULL},
        };

        ShaderStructConstr uniform_arr2[] = {
            {SHADER_VARIABLE_TYPE_ARRAY, 128, 0, "mats", mat_ptr, 1, NULL},
            {SHADER_VARIABLE_TYPE_INT, 4, 0, "size", NULL, 0, NULL}
        };

        uint32_t bones = ShaderBuilderAddUniform(uniform_arr2, 2, "InvMatBuffer", 0, 2);

        uint32_t posit = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "position", 0, 0);
        uint32_t norm = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "normal", 1, 0);
        uint32_t col = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "color", 2, 0);
        uint32_t txt_indx = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 2, "inTexCoord", 3, 0);
        uint32_t mat_indx = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_INT, 0, NULL, 0, "material_indx", 4, 0);
        uint32_t joints = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 4, "joints", 5, 0);
        uint32_t weight = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 4, "weight", 6, 0);

        uint32_t clr_dst = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 3, "fragColor", 0, 0);
        uint32_t frg_norm = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 3, "fragNormal", 1, 0);
        uint32_t txt_dst = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 2, "fragTexCoord", 2, 0);
        uint32_t mat_dst = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_INT, SHADER_DATA_FLAG_OUTPUT, NULL, 0, "fragMatIndx", 3, 0);

        uint32_t vec_type = ShaderBuilderAddPointer(SHADER_VARIABLE_TYPE_VECTOR, 4, SHADER_DATA_FLAG_FUNCTION);
        uint32_t mat_type = ShaderBuilderAddPointer(SHADER_VARIABLE_TYPE_MATRIX, 4, SHADER_DATA_FLAG_FUNCTION);

        uint32_t v_model = ShaderBuilderAddOperand((uint32_t []){ vec_type, 7 /*Function*/ }, 2, SHADER_OPERAND_TYPE_VARIABLE);
        uint32_t m_skinMat = ShaderBuilderAddOperand((uint32_t []){ mat_type, 7 /*Function*/ }, 2, SHADER_OPERAND_TYPE_VARIABLE);

        uint32_t cond_label = ShaderBuilderNextLabel(false, 1);
        uint32_t end_label = ShaderBuilderNextLabel(true, 2);

        uint32_t res = ShaderBuilderAddFuncMult(uniform, 0, SHADER_VARIABLE_TYPE_MATRIX, 4, posit, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, 4);
        ShaderBuilderStoreValue((uint32_t []){v_model, res}, 2);

        uint32_t res_arr[4];

        for(int i=0; i < 4;i++){

            uint32_t res1 = ShaderBuilderAcceptAccess(weight, SHADER_VARIABLE_TYPE_FLOAT, 4, (uint32_t []){ i }, 1, true);
            uint32_t res2 = ShaderBuilderAcceptAccess(joints, SHADER_VARIABLE_TYPE_FLOAT, 4, (uint32_t []){ i }, 1, true);
            res2 = ShaderBuilderConvertFToS(res2);

            res2 = ShaderBuilderAcceptAccess(bones, SHADER_VARIABLE_TYPE_MATRIX, 4, (uint32_t []){ 0, res2 }, 2, true);

            res_arr[i] = ShaderBuilderAddFuncMult(res2, 0, SHADER_VARIABLE_TYPE_MATRIX, 4, res1, 0, SHADER_VARIABLE_TYPE_FLOAT, 0, 4);
        }

        res = ShaderBuilderAddFuncAdd(res_arr[0], 0, SHADER_VARIABLE_TYPE_MATRIX, 4, res_arr[1], 0, SHADER_VARIABLE_TYPE_MATRIX, 4, 4);
        res = ShaderBuilderAddFuncAdd(res, 0, SHADER_VARIABLE_TYPE_MATRIX, 4, res_arr[2], 0, SHADER_VARIABLE_TYPE_MATRIX, 4, 4);
        res = ShaderBuilderAddFuncAdd(res, 0, SHADER_VARIABLE_TYPE_MATRIX, 4, res_arr[3], 0, SHADER_VARIABLE_TYPE_MATRIX, 4, 4);
        ShaderBuilderStoreValue((uint32_t []){m_skinMat, res}, 2);
            
        uint32_t acc = ShaderBuilderAcceptAccess(bones, SHADER_VARIABLE_TYPE_INT, 1, (uint32_t []){1}, 1, true);

        uint32_t cst = ShaderBuilderAddConstant(SHADER_VARIABLE_TYPE_INT, 0, 0, 1);

        ShaderBuilderMakeBranchConditional(SHADER_CONDITIONAL_TYPE_SGREAT_THAN, (uint32_t []){acc, cst}, 2, end_label, cond_label, end_label);

        ShaderBuilderSetCurrentLabel(cond_label);

        res = ShaderBuilderAcceptLoad(m_skinMat, 0);

        res = ShaderBuilderAddFuncMult(res, 0, SHADER_VARIABLE_TYPE_MATRIX, 4, posit, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, 4);

        res = ShaderBuilderAddFuncMult(uniform, 0, SHADER_VARIABLE_TYPE_MATRIX, 4, res, 0, SHADER_VARIABLE_TYPE_VECTOR, 4, 4);

        ShaderBuilderStoreValue((uint32_t []){v_model, res}, 2);

        ShaderBuilderMakeTransition(end_label);
        
        res = ShaderBuilderAcceptLoad(v_model, 0);

        res = ShaderBuilderAddFuncMult(uniform, 1, SHADER_VARIABLE_TYPE_MATRIX, 4, res, 0, SHADER_VARIABLE_TYPE_VECTOR, 4, 4);
        res = ShaderBuilderAddFuncMult(uniform, 2, SHADER_VARIABLE_TYPE_MATRIX, 4, res, 0, SHADER_VARIABLE_TYPE_VECTOR, 4, 4);

        uint32_t gl_Pos = ShaderBuilderAcceptAccess(vert->gl_struct_indx, SHADER_VARIABLE_TYPE_VECTOR, 4, (uint32_t []){ 0 }, 1, false);
        ShaderBuilderStoreValue((uint32_t []){ gl_Pos, res}, 2);

        ShaderBuilderAddFuncMove(col, 3, clr_dst, 3);
        ShaderBuilderAddFuncMove(txt_indx, 2, txt_dst, 2);

        res = ShaderBuilderAcceptLoad(mat_indx, 0);

        ShaderBuilderStoreValue((uint32_t []){mat_dst, res}, 2);

        ShaderBuilderMake();
    }
    //----------------------------------------
    {
        ShaderBuilderInit(frag, SHADER_TYPE_FRAGMENT);

        uint32_t fragColor = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "fragColor", 0, 0);
        uint32_t fragNormal = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "fragNormal", 1, 0);
        uint32_t fragTexCoord = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 2, "fragTexCoord", 2, 0);
        uint32_t fragMatIndx = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_INT, 0, NULL, 0, "fragMatIndx", 3, 0);

        uint32_t outColor = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 4, "outColor", 0, 0);

        uint32_t diffuse = 0;
        
        if(count_texture > 1){
            
            ShaderStructConstr arr_textures[] = {                
                {SHADER_VARIABLE_TYPE_IMAGE, 0, 0, "textures", NULL, 0, NULL},
            };

            diffuse = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_ARRAY, SHADER_DATA_FLAG_UNIFORM_CONSTANT, arr_textures, count_texture, "diffuse", 0, 3);
        }else
            diffuse = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_IMAGE, SHADER_DATA_FLAG_UNIFORM_CONSTANT, NULL, 0, "diffuse", 0, 3);

        uint32_t normal = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_IMAGE, SHADER_DATA_FLAG_UNIFORM_CONSTANT, NULL, 0, "normal", 0, 4);
        uint32_t specular = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_IMAGE, SHADER_DATA_FLAG_UNIFORM_CONSTANT, NULL, 0, "specular", 0, 5);


        if(count_texture > 0){
            
            uint32_t res = ShaderBuilderAcceptLoad(fragMatIndx, 0);
            
            res = ShaderBuilderGetTexture(diffuse, fragTexCoord, res);

            uint32_t l_kill = ShaderBuilderNextLabel(false, 1);

            uint32_t end_label = ShaderBuilderNextLabel(true, 2);
            
            float f = 0.1f;
            uint32_t c = 0;
            memcpy(&c, &f, sizeof(uint32_t));
            uint32_t cons = ShaderBuilderAddConstant(SHADER_VARIABLE_TYPE_FLOAT, 0, c, 1);

            VectorExtract extr = ShaderBuilderGetElemenets(SHADER_VARIABLE_TYPE_FLOAT, res, 4, 3, 1);

            ShaderBuilderMakeBranchConditional(SHADER_CONDITIONAL_TYPE_FLESS_THAN, (uint32_t []){ extr.elems[0], cons}, 2, end_label, l_kill, end_label);

            ShaderBuilderSetCurrentLabel(l_kill);

            ShaderBuilderMakeKill();

            ShaderBuilderSetCurrentLabel(end_label);
            
            uint32_t arr2[] = {outColor, res};
            
            ShaderBuilderStoreValue(arr2, sizeof(arr2));

        }else{
            uint32_t res = ShaderBuilderAcceptLoad(fragColor, 0);

            res = ShaderBuilderMutateVector(res, 3, 4);
            
            uint32_t arr2[] = {outColor, res};
            
            ShaderBuilderStoreValue(arr2, sizeof(arr2));
        }

        ShaderBuilderMake();
    }
}

void ShadersMakeDeafult3DModelShaderWithLight(ShaderBuilder *vert, ShaderBuilder *frag, uint32_t count_texture, uint32_t count_normal, uint32_t count_specular){

    memset(vert, 0, sizeof(ShaderBuilder));
    memset(frag, 0, sizeof(ShaderBuilder));

    //------------------------------------------------------
    {
        ShaderBuilderInit(vert, SHADER_TYPE_VERTEX);

        ShaderStructConstr uniform_arr[] = {
            {SHADER_VARIABLE_TYPE_MATRIX, 4, 0, "model", NULL, 0, NULL},
            {SHADER_VARIABLE_TYPE_MATRIX, 4, 0, "view", NULL, 0, NULL},
            {SHADER_VARIABLE_TYPE_MATRIX, 4, 0, "proj", NULL, 0, NULL},
        };

        uint32_t uniform = ShaderBuilderAddUniform(uniform_arr, 3, "ModelBufferObjects", 0, 1);

        ShaderStructConstr mat_ptr[] = {
            {SHADER_VARIABLE_TYPE_MATRIX, 4, 0, "mat4", NULL, 0, NULL},
        };

        ShaderStructConstr uniform_arr2[] = {
            {SHADER_VARIABLE_TYPE_ARRAY, 128, 0, "mats", mat_ptr, 1, NULL},
            {SHADER_VARIABLE_TYPE_INT, 4, 0, "size", NULL, 0, NULL}
        };

        uint32_t bones = ShaderBuilderAddUniform(uniform_arr2, 2, "InvMatBuffer", 0, 2);

        uint32_t posit = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "position", 0, 0);
        uint32_t norm = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "normal", 1, 0);
        uint32_t col = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "color", 2, 0);
        uint32_t txt_indx = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 2, "inTexCoord", 3, 0);
        uint32_t mat_indx = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_INT, 0, NULL, 0, "material_indx", 4, 0);
        uint32_t joints = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 4, "joints", 5, 0);
        uint32_t weight = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 4, "weight", 6, 0);

        uint32_t frg_pos = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 3, "fragPos", 0, 0);
        uint32_t frg_norm = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 3, "fragNormal", 1, 0);
        uint32_t clr_dst = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 3, "fragColor", 2, 0);
        uint32_t txt_dst = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 2, "fragTexCoord", 3, 0);
        uint32_t mat_dst = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_INT, SHADER_DATA_FLAG_OUTPUT, NULL, 0, "fragMatIndx", 4, 0);

        uint32_t vec_type = ShaderBuilderAddPointer(SHADER_VARIABLE_TYPE_VECTOR, 4, SHADER_DATA_FLAG_FUNCTION);
        uint32_t mat_type = ShaderBuilderAddPointer(SHADER_VARIABLE_TYPE_MATRIX, 4, SHADER_DATA_FLAG_FUNCTION);

        uint32_t v_model = ShaderBuilderAddOperand((uint32_t []){ vec_type, 7 /*Function*/ }, 2, SHADER_OPERAND_TYPE_VARIABLE);
        uint32_t m_skinMat = ShaderBuilderAddOperand((uint32_t []){ mat_type, 7 /*Function*/ }, 2, SHADER_OPERAND_TYPE_VARIABLE);

        uint32_t cond_label = ShaderBuilderNextLabel(false, 1);
        uint32_t end_label = ShaderBuilderNextLabel(true, 2);

        uint32_t res = ShaderBuilderAddFuncMult(uniform, 0, SHADER_VARIABLE_TYPE_MATRIX, 4, posit, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, 4);
        ShaderBuilderStoreValue((uint32_t []){v_model, res}, 2);

        uint32_t res_arr[4];

        for(int i=0; i < 4;i++){

            uint32_t res1 = ShaderBuilderAcceptAccess(weight, SHADER_VARIABLE_TYPE_FLOAT, 4, (uint32_t []){ i }, 1, true);
            uint32_t res2 = ShaderBuilderAcceptAccess(joints, SHADER_VARIABLE_TYPE_FLOAT, 4, (uint32_t []){ i }, 1, true);
            res2 = ShaderBuilderConvertFToS(res2);

            res2 = ShaderBuilderAcceptAccess(bones, SHADER_VARIABLE_TYPE_MATRIX, 4, (uint32_t []){ 0, res2 }, 2, true);

            res_arr[i] = ShaderBuilderAddFuncMult(res2, 0, SHADER_VARIABLE_TYPE_MATRIX, 4, res1, 0, SHADER_VARIABLE_TYPE_FLOAT, 0, 4);
        }

        res = ShaderBuilderAddFuncAdd(res_arr[0], 0, SHADER_VARIABLE_TYPE_MATRIX, 4, res_arr[1], 0, SHADER_VARIABLE_TYPE_MATRIX, 4, 4);
        res = ShaderBuilderAddFuncAdd(res, 0, SHADER_VARIABLE_TYPE_MATRIX, 4, res_arr[2], 0, SHADER_VARIABLE_TYPE_MATRIX, 4, 4);
        res = ShaderBuilderAddFuncAdd(res, 0, SHADER_VARIABLE_TYPE_MATRIX, 4, res_arr[3], 0, SHADER_VARIABLE_TYPE_MATRIX, 4, 4);
        ShaderBuilderStoreValue((uint32_t []){m_skinMat, res}, 2);
            
        uint32_t acc = ShaderBuilderAcceptAccess(bones, SHADER_VARIABLE_TYPE_INT, 1, (uint32_t []){1}, 1, true);

        uint32_t cst = ShaderBuilderAddConstant(SHADER_VARIABLE_TYPE_INT, 0, 0, 1);

        ShaderBuilderMakeBranchConditional(SHADER_CONDITIONAL_TYPE_SGREAT_THAN, (uint32_t []){acc, cst}, 2, end_label, cond_label, end_label);

        ShaderBuilderSetCurrentLabel(cond_label);

        res = ShaderBuilderAcceptLoad(m_skinMat, 0);

        res = ShaderBuilderAddFuncMult(res, 0, SHADER_VARIABLE_TYPE_MATRIX, 4, posit, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, 4);

        res = ShaderBuilderAddFuncMult(uniform, 0, SHADER_VARIABLE_TYPE_MATRIX, 4, res, 0, SHADER_VARIABLE_TYPE_VECTOR, 4, 4);

        ShaderBuilderStoreValue((uint32_t []){v_model, res}, 2);

        ShaderBuilderMakeTransition(end_label);
        
        res = ShaderBuilderAcceptLoad(v_model, 0);

        res = ShaderBuilderAddFuncMult(uniform, 1, SHADER_VARIABLE_TYPE_MATRIX, 4, res, 0, SHADER_VARIABLE_TYPE_VECTOR, 4, 4);
        res = ShaderBuilderAddFuncMult(uniform, 2, SHADER_VARIABLE_TYPE_MATRIX, 4, res, 0, SHADER_VARIABLE_TYPE_VECTOR, 4, 4);

        uint32_t gl_Pos = ShaderBuilderAcceptAccess(vert->gl_struct_indx, SHADER_VARIABLE_TYPE_VECTOR, 4, (uint32_t []){ 0 }, 1, false);
        ShaderBuilderStoreValue((uint32_t []){ gl_Pos, res}, 2);

        uint32_t v3_type = ShaderBuilderAddVector(3, NULL);
        
        uint32_t acc_1 = ShaderBuilderAcceptAccess(uniform, SHADER_VARIABLE_TYPE_MATRIX, 4, (uint32_t []){ 0 }, 1, true);

        VectorExtract extr2 = ShaderBuilderGetElemenets(SHADER_VARIABLE_TYPE_VECTOR, acc_1, 4, 0, 3);

        uint32_t args[3];
        for(int i=0; i < 3;i++){

            VectorExtract extr = ShaderBuilderGetElemenets(SHADER_VARIABLE_TYPE_FLOAT, extr2.elems[i], 4, 0, 3);

            args[i] = ShaderBuilderCompositeConstruct((uint32_t []){ v3_type, extr.elems[0], extr.elems[1], extr.elems[2]}, 4);
        }

        uint32_t mat3_type = ShaderBuilderAddMatrix(3, NULL);
        
        res = ShaderBuilderCompositeConstruct((uint32_t []){ mat3_type, args[0], args[1], args[2] }, 4);
        res = ShaderBuilderMakeExternalFunction((uint32_t []){ mat3_type, res }, 2, GLSLstd450MatrixInverse);
        res = ShaderBuilderAddOperand((uint32_t []){ mat3_type, res }, 2, SHADER_OPERAND_TYPE_TRANSPOSE);
        res = ShaderBuilderAddFuncMult(res, 0, SHADER_VARIABLE_TYPE_MATRIX, 3, norm, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, 3);
        ShaderBuilderStoreValue((uint32_t []){ frg_norm, res }, 2);
        
        res = ShaderBuilderAddFuncMult(uniform, 0, SHADER_VARIABLE_TYPE_MATRIX, 4, posit, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, 4);

        VectorExtract extr = ShaderBuilderGetElemenets(SHADER_VARIABLE_TYPE_FLOAT, res, 4, 0, 4);

        res = ShaderBuilderCompositeConstruct((uint32_t []){ v3_type, extr.elems[0], extr.elems[1], extr.elems[2] }, 4);
        uint32_t d_res = ShaderBuilderCompositeConstruct((uint32_t []){ v3_type, extr.elems[3], extr.elems[3], extr.elems[3] }, 4);

        res = ShaderBuilderAddFuncDiv(res, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, d_res, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, 3);
        
        ShaderBuilderStoreValue((uint32_t []){frg_pos, res}, 2);
        
        ShaderBuilderAddFuncMove(col, 3, clr_dst, 3);
        ShaderBuilderAddFuncMove(txt_indx, 2, txt_dst, 2);

        res = ShaderBuilderAcceptLoad(mat_indx, 0);

        ShaderBuilderStoreValue((uint32_t []){mat_dst, res}, 2);

        ShaderBuilderMake();
    }
    //----------------------------------------
    {
        ShaderBuilderInit(frag, SHADER_TYPE_FRAGMENT);

        ShaderStructConstr light_str[] = {
            {SHADER_VARIABLE_TYPE_VECTOR, 3, 0, "position", NULL, 0, NULL},
            {SHADER_VARIABLE_TYPE_VECTOR, 3, 0, "color" , NULL, 0, NULL},
            {SHADER_VARIABLE_TYPE_VECTOR, 3, 0, "direction" , NULL, 0, NULL},
            {SHADER_VARIABLE_TYPE_FLOAT , 0, 0, "intensity" , NULL, 0, NULL},
            {SHADER_VARIABLE_TYPE_INT   , 0, 0, "type" , NULL, 0, NULL},
            {SHADER_VARIABLE_TYPE_FLOAT , 0, 0, "cutoff" , NULL, 0, NULL},
            {SHADER_VARIABLE_TYPE_FLOAT , 0, 0, "outCutOff" , NULL, 0, NULL},
            {SHADER_VARIABLE_TYPE_FLOAT , 0, 0, "radius" , NULL, 0, NULL},
        };

        ShaderStructConstr light_arr[] = {
            {SHADER_VARIABLE_TYPE_ARRAY, 10, 0, "lights", light_str, 8, "LightsStruct"},
            {SHADER_VARIABLE_TYPE_INT, 0, 0, "num_lights" , NULL, 0, NULL},
            {SHADER_VARIABLE_TYPE_INT, 0, 0, "light_enable" , NULL, 0, NULL},
        };

        uint32_t lights = ShaderBuilderAddUniform(light_arr, 3, "LightBufferObject", 0, 3);

        uint32_t fragPos = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "fragPos", 0, 0);
        uint32_t fragNormal = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "fragNormal", 1, 0);
        uint32_t fragColor = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "fragColor", 2, 0);
        uint32_t fragTexCoord = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 2, "fragTexCoord", 3, 0);
        uint32_t fragMatIndx = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_INT, 0, NULL, 0, "fragMatIndx", 4, 0);

        uint32_t outColor = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 4, "outColor", 0, 0);

        uint32_t diffuse = 0, normal = 0, specular = 0;
        
        if(count_texture > 1){
            
            ShaderStructConstr arr_textures[] = {                
                {SHADER_VARIABLE_TYPE_IMAGE, 0, 0, "textures", NULL, 0, NULL},
            };

            diffuse = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_ARRAY, SHADER_DATA_FLAG_UNIFORM_CONSTANT, arr_textures, count_texture, "diffuse", 0, 4);
        }else
            diffuse = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_IMAGE, SHADER_DATA_FLAG_UNIFORM_CONSTANT, NULL, 0, "diffuse", 0, 4);

        if(count_normal > 1){
            
            ShaderStructConstr arr_textures[] = {                
                {SHADER_VARIABLE_TYPE_IMAGE, 0, 0, "textures", NULL, 0, NULL},
            };
            normal = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_IMAGE, SHADER_DATA_FLAG_UNIFORM_CONSTANT, arr_textures, count_normal, "normal", 0, 5);
        }else if(count_normal > 0)
            normal = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_IMAGE, SHADER_DATA_FLAG_UNIFORM_CONSTANT, NULL, 0, "normal", 0, 5);

            
        if(count_specular > 1){
            
            ShaderStructConstr arr_textures[] = {                
                {SHADER_VARIABLE_TYPE_IMAGE, 0, 0, "textures", NULL, 0, NULL},
            };
            specular = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_IMAGE, SHADER_DATA_FLAG_UNIFORM_CONSTANT, arr_textures, count_specular, "specular", 0, 6);
        }else if(count_specular > 0)
            specular = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_IMAGE, SHADER_DATA_FLAG_UNIFORM_CONSTANT, NULL, 0, "specular", 0, 6);

        /*if(count_texture > 0){
            
            uint32_t res = ShaderBuilderAcceptLoad(fragMatIndx, 0);
            
            res = ShaderBuilderGetTexture(diffuse, fragTexCoord, res);

            uint32_t l_kill = ShaderBuilderNextLabel(false, 1);

            uint32_t end_label = ShaderBuilderNextLabel(true, 2);
            
            float f = 0.1f;
            uint32_t c = 0;
            memcpy(&c, &f, sizeof(uint32_t));
            uint32_t cons = ShaderBuilderAddConstant(SHADER_VARIABLE_TYPE_FLOAT, 0, c, 1);

            VectorExtract extr = ShaderBuilderGetElemenets(SHADER_VARIABLE_TYPE_FLOAT, res, 4, 3, 1);

            ShaderBuilderMakeBranchConditional(SHADER_CONDITIONAL_TYPE_FLESS_THAN, (uint32_t []){ extr.elems[0], cons}, 2, end_label, l_kill, end_label);

            ShaderBuilderSetCurrentLabel(l_kill);

            ShaderBuilderMakeKill();

            ShaderBuilderSetCurrentLabel(end_label);
            
            uint32_t arr2[] = {outColor, res};
            
            ShaderBuilderStoreValue(arr2, sizeof(arr2));

        }else{
            uint32_t res = ShaderBuilderAcceptLoad(fragColor, 0);

            res = ShaderBuilderMutateVector(res, 3, 4);
            
            uint32_t arr2[] = {outColor, res};
            
            ShaderBuilderStoreValue(arr2, sizeof(arr2));
        }*/

        if(count_texture == 0)
        {
            uint32_t res = ShaderBuilderAcceptLoad(fragColor, 0);

            res = ShaderBuilderMutateVector(res, 3, 4);

            ShaderBuilderStoreValue((uint32_t []){outColor, res}, 2);

        }else{
            
            MakeBaseLightCode(diffuse, normal, fragMatIndx, fragNormal, fragTexCoord, fragPos, lights, outColor);
            
        }


        ShaderBuilderMake();
    }
}

void ShaderMakeDefaultParticle3DShader(ShaderBuilder *vert, ShaderBuilder *frag){

    memset(vert, 0, sizeof(ShaderBuilder));
    memset(frag, 0, sizeof(ShaderBuilder));

//------------------------------------------------------
    {
        ShaderBuilderInit(vert, SHADER_TYPE_VERTEX);
        
        ShaderStructConstr uniform_arr[] = {
            {SHADER_VARIABLE_TYPE_MATRIX, 4, 0, "model", NULL, 0, NULL},
            {SHADER_VARIABLE_TYPE_MATRIX, 4, 0, "view", NULL, 0, NULL},
            {SHADER_VARIABLE_TYPE_MATRIX, 4, 0, "porl", NULL, 0, NULL},
        };

        uint32_t uniform = ShaderBuilderAddUniform(uniform_arr, 3, "TransformBufferObjects", 0, 1);

        uint32_t posit = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "position", 0, 0);
        uint32_t p_size = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_FLOAT, 0, NULL, 0, "size", 1, 0);
        uint32_t clr_indx = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "color", 2, 0);
        
        uint32_t clr_dst = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 3, "fragColor", 0, 0);

        float f = 200.0f;
        uint32_t c = 0;
        memcpy(&c, &f, sizeof(uint32_t));
        uint32_t cons = ShaderBuilderAddConstant(SHADER_VARIABLE_TYPE_FLOAT, 0, c, 1);

        uint32_t res = ShaderBuilderAddFuncMult(cons, 0, SHADER_VARIABLE_TYPE_FLOAT, 0, p_size, 0, SHADER_VARIABLE_TYPE_FLOAT, 0, 0);
        uint32_t acc = ShaderBuilderAcceptAccess(vert->gl_struct_indx, SHADER_VARIABLE_TYPE_FLOAT, 0, (uint32_t []){ 1 }, 1, false);
        ShaderBuilderStoreValue((uint32_t []){ acc,  res}, 2);

        res = ShaderBuilderAddFuncMult(uniform, 1, SHADER_VARIABLE_TYPE_MATRIX, 4, uniform, 0, SHADER_VARIABLE_TYPE_MATRIX, 4, 4);
        res = ShaderBuilderAddFuncMult(uniform, 2,  SHADER_VARIABLE_TYPE_MATRIX, 4, res, 0, SHADER_VARIABLE_TYPE_MATRIX, 4, 4);
        res = ShaderBuilderAddFuncMult(res, 0,  SHADER_VARIABLE_TYPE_MATRIX, 4, posit, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, 4);
        acc = ShaderBuilderAcceptAccess(vert->gl_struct_indx, SHADER_VARIABLE_TYPE_VECTOR, 4, (uint32_t []){ 0 }, 1, false);
        ShaderBuilderStoreValue((uint32_t []){ acc,  res}, 2);

        ShaderBuilderAddFuncMove(clr_indx, 3, clr_dst, 3);

        ShaderBuilderMake();
    }
//----------------------------------------
    {
        ShaderBuilderInit(frag, SHADER_TYPE_FRAGMENT);
        
        uint32_t fragColor = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "fragColor", 0, 0);
        uint32_t outColor = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 4, "outColor", 0, 0);
        
        uint32_t gl_Point_Coord = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_SYSTEM, NULL, 2, "gl_PointCoord", 0, 16);

        uint32_t texture = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_IMAGE, SHADER_DATA_FLAG_UNIFORM_CONSTANT, NULL, 0, "Texture2D", 0, 2);
        
        uint32_t res = ShaderBuilderGetTexture(texture, gl_Point_Coord, 0);

        uint32_t l_kill = ShaderBuilderNextLabel(false, 1);

        uint32_t end_label = ShaderBuilderNextLabel(true, 2);
        
        float f = 0.09f;
        uint32_t c = 0;
        memcpy(&c, &f, sizeof(uint32_t));
        uint32_t cons = ShaderBuilderAddConstant(SHADER_VARIABLE_TYPE_FLOAT, 0, c, 1);

        VectorExtract extr = ShaderBuilderGetElemenets(SHADER_VARIABLE_TYPE_FLOAT, res, 4, 3, 1);

        ShaderBuilderMakeBranchConditional(SHADER_CONDITIONAL_TYPE_FLESS_THAN, (uint32_t []){ extr.elems[0], cons}, 2, end_label, l_kill, end_label);

        ShaderBuilderSetCurrentLabel(l_kill);

        ShaderBuilderMakeKill();

        ShaderBuilderSetCurrentLabel(end_label);

        res = ShaderBuilderAddFuncMult(res, 0, SHADER_VARIABLE_TYPE_VECTOR, 4, fragColor, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, 4);

        ShaderBuilderStoreValue((uint32_t []){outColor, res}, 2);

        ShaderBuilderMake();
    }
}

void ShadersMakeClear2DShader(ShaderBuilder *vert, ShaderBuilder *frag){

    memset(vert, 0, sizeof(ShaderBuilder));
    memset(frag, 0, sizeof(ShaderBuilder));


    //------------------------------------------------------
    {
        ShaderBuilderInit(vert, SHADER_TYPE_VERTEX);
        
        uint32_t posit = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 2, "position", 0, 0);
        uint32_t clr_indx = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "color", 1, 0);
        uint32_t txt_indx = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 2, "inTexCoord", 2, 0);

        uint32_t clr_dst = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 3, "fragColor", 0, 0);
        uint32_t txt_dst = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 2, "fragTexCoord", 1, 0);


        uint32_t gl_Pos = ShaderBuilderAcceptAccess(vert->gl_struct_indx, SHADER_VARIABLE_TYPE_VECTOR, 4, (uint32_t []){ 0 }, 1, false);

        uint32_t res = ShaderBuilderMutateVector(posit, 2, 4);

        ShaderBuilderStoreValue((uint32_t []){ gl_Pos, res}, 2);

        ShaderBuilderAddFuncMove(clr_indx, 3, clr_dst, 3);
        ShaderBuilderAddFuncMove(txt_indx, 2, txt_dst, 2);

        ShaderBuilderMake();   
    }
    //----------------------------------------
    {
        ShaderBuilderInit(frag, SHADER_TYPE_FRAGMENT);

        uint32_t fragColor = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "fragColor", 0, 0);
        uint32_t fragTexCoord = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 2, "fragTexCoord", 1, 0);

        uint32_t outColor = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 4, "outColor", 0, 0);

        uint32_t texture = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_IMAGE, SHADER_DATA_FLAG_UNIFORM_CONSTANT, NULL, 0, "Texture2D", 0, 1);

        uint32_t res = ShaderBuilderGetTexture(texture, fragTexCoord, 0);

        uint32_t l_kill = ShaderBuilderNextLabel(false, 1);

        uint32_t end_label = ShaderBuilderNextLabel(true, 2);
        
        float f = 0.01f;
        uint32_t c = 0;
        memcpy(&c, &f, sizeof(uint32_t));
        uint32_t cons = ShaderBuilderAddConstant(SHADER_VARIABLE_TYPE_FLOAT, 0, c, 1);

        VectorExtract extr = ShaderBuilderGetElemenets(SHADER_VARIABLE_TYPE_FLOAT, res, 4, 0, 1);

        ShaderBuilderMakeBranchConditional(SHADER_CONDITIONAL_TYPE_FLESS_THAN, (uint32_t []){ extr.elems[0], cons}, 2, end_label, l_kill, end_label);

        ShaderBuilderSetCurrentLabel(l_kill);

        ShaderBuilderMakeKill();

        ShaderBuilderSetCurrentLabel(end_label);

        uint32_t get_frag_color = ShaderBuilderAcceptLoad(fragColor, 0);

        VectorExtract extr2 = ShaderBuilderGetElemenets(SHADER_VARIABLE_TYPE_FLOAT, get_frag_color, 3, 0, 3);

        uint32_t vec_type = ShaderBuilderAddVector(4, NULL);

        res =  ShaderBuilderCompositeConstruct((uint32_t []){vec_type, extr2.elems[0], extr2.elems[1], extr2.elems[2], extr.elems[0]}, 5);

        ShaderBuilderStoreValue((uint32_t []){ outColor, res}, 2);

        ShaderBuilderMake();
    }
}

void ShadersMakeDefault2DTextShader(ShaderBuilder *vert, ShaderBuilder *frag){

    memset(vert, 0, sizeof(ShaderBuilder));
    memset(frag, 0, sizeof(ShaderBuilder));

    //------------------------------------------------------
    {
        ShaderBuilderInit(vert, SHADER_TYPE_VERTEX);

        ShaderStructConstr uniform_arr[] = {
            {SHADER_VARIABLE_TYPE_VECTOR, 2, 0,"position", NULL, 0, NULL},
            {SHADER_VARIABLE_TYPE_VECTOR, 2, 0,"rotation", NULL, 0, NULL},
            {SHADER_VARIABLE_TYPE_VECTOR, 2, 0,"scale", NULL, 0, NULL},
        };

        uint32_t uniform = ShaderBuilderAddUniform(uniform_arr, 3, "TransformBufferObjects", 0, 1);

        uint32_t posit = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 2, "position", 0, 0);
        uint32_t clr_indx = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "color", 1, 0);
        uint32_t txt_indx = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 2, "inTexCoord", 2, 0);

        uint32_t clr_dst = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 3, "fragColor", 0, 0);
        uint32_t txt_dst = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 2, "fragTexCoord", 1, 0);

        uint32_t res = ShaderBuilderAddFuncMult(uniform, 2, SHADER_VARIABLE_TYPE_VECTOR, 2, posit, 0, SHADER_VARIABLE_TYPE_VECTOR, 2, 2);
        res = ShaderBuilderAddFuncAdd(uniform, 0, SHADER_VARIABLE_TYPE_VECTOR, 2, res, 0, SHADER_VARIABLE_TYPE_VECTOR, 2, 2);

        uint32_t acc = ShaderBuilderAcceptAccess(vert->gl_struct_indx, SHADER_VARIABLE_TYPE_VARIABLE, 4, (uint32_t []){ 0 }, 1, false);
        ShaderBuilderStoreValue((uint32_t []){ acc, res }, 2);

        ShaderBuilderAddFuncMove(clr_indx, 3, clr_dst, 3);
        ShaderBuilderAddFuncMove(txt_indx, 2, txt_dst, 2);

        ShaderBuilderMake();
    }
    //----------------------------------------
    {
        ShaderBuilderInit(frag, SHADER_TYPE_FRAGMENT);

        uint32_t fragColor = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "fragColor", 0, 0);
        uint32_t fragTexCoord = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 2, "fragTexCoord", 1, 0);
        uint32_t outColor = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 4, "outColor", 0, 0);


        uint32_t texture = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_IMAGE, SHADER_DATA_FLAG_UNIFORM_CONSTANT, NULL, 0, "Texture2D", 0, 2);

        uint32_t res = ShaderBuilderMutateVector(fragColor, 3, 4);

        uint32_t arr[] = { outColor, res} ;
        ShaderBuilderStoreValue(arr, 2);

        ShaderBuilderMake();
    }
}

void ShadersMakeTerrainShader(ShaderBuilder *vert, ShaderBuilder *tesc, ShaderBuilder *tese, ShaderBuilder *frag){
    
    memset(vert, 0, sizeof(ShaderBuilder));
    memset(tesc, 0, sizeof(ShaderBuilder));
    memset(tese, 0, sizeof(ShaderBuilder));
    memset(frag, 0, sizeof(ShaderBuilder));

    //------------------------------------------------------
    {
        ShaderBuilderInit(vert, SHADER_TYPE_VERTEX);

        uint32_t posit = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "position", 0, 0);
        uint32_t nrm_indx = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "normal", 1, 0);
        uint32_t clr_indx = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "color", 2, 0);
        uint32_t txt_indx = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 2, "inTexCoord", 3, 0);

        uint32_t nrm_dst = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 3, "outNormal", 0, 0);
        uint32_t txt_dst = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 2, "outUv", 1, 0);

        uint32_t res = ShaderBuilderAcceptLoad(posit, 0);

        res = ShaderBuilderMutateVector(res, 3, 4);
        uint32_t acc = ShaderBuilderAcceptAccess(vert->gl_struct_indx, SHADER_VARIABLE_TYPE_VARIABLE, 4, (uint32_t []){ 0 }, 1, false);
        ShaderBuilderStoreValue((uint32_t []){ acc, res }, 2);

        ShaderBuilderAddFuncMove(nrm_indx, 3, nrm_dst, 3);
        ShaderBuilderAddFuncMove(txt_indx, 2, txt_dst, 2);

        ShaderBuilderMake();
    }
    //------------------------------------------------------
    {
        ShaderBuilderInit(tesc, SHADER_TYPE_TESELLATION_CONTROL);

        ShaderStructConstr uniform_arr[] = {
            {SHADER_VARIABLE_TYPE_MATRIX, 4, 0, "model", NULL, 0, NULL},
            {SHADER_VARIABLE_TYPE_MATRIX, 4, 0, "view" , NULL, 0, NULL},
            {SHADER_VARIABLE_TYPE_MATRIX, 4, 0, "porl" , NULL, 0, NULL},
        };

        uint32_t uniform = ShaderBuilderAddUniform(uniform_arr, 3, "ModelBufferObjects", 0, 1);

        ShaderStructConstr sub_arr[] = {
            {SHADER_VARIABLE_TYPE_VECTOR, 4, 0, "vec4", NULL, 0, NULL},
        };

        ShaderStructConstr uniform_arr2[] = {
            {SHADER_VARIABLE_TYPE_VECTOR, 4, 0, "lightPos", NULL, 0, NULL},
            {SHADER_VARIABLE_TYPE_ARRAY, 6, 0, "frustumPlanes" , sub_arr, 1, NULL},
            {SHADER_VARIABLE_TYPE_FLOAT, 0, 0, "displacementFactor" , NULL, 0, NULL},
            {SHADER_VARIABLE_TYPE_FLOAT, 0, 0, "tessellationFactor" , NULL, 0, NULL},
            {SHADER_VARIABLE_TYPE_VECTOR, 2, 0, "viewportDim" , NULL, 0, NULL},
            {SHADER_VARIABLE_TYPE_FLOAT, 0, 0, "tessellatedEdgeSize" , NULL, 0, NULL},
        };

        uint32_t uniform2 = ShaderBuilderAddUniform(uniform_arr2, 6, "TesselationBuffer", 0, 2);

        
        ShaderStructConstr float_str[] = {
            {SHADER_VARIABLE_TYPE_FLOAT, 0, 0, NULL, NULL, 0, NULL}
        };

        ShaderStructConstr gl_str[] = {
            {SHADER_VARIABLE_TYPE_VECTOR, 4, 0, "gl_Position", NULL, 0, NULL},
            {SHADER_VARIABLE_TYPE_FLOAT, 32, 1, "gl_PointSize", NULL, 0, NULL},
            {SHADER_VARIABLE_TYPE_ARRAY, 1,  3, "gl_ClipDistance", &float_str, 1, NULL},
            {SHADER_VARIABLE_TYPE_ARRAY, 1,  4, "gl_CullDistance", &float_str, 1, NULL}
        };

        ShaderStructConstr gl_arr[] = {
            {SHADER_VARIABLE_TYPE_ARRAY, 4,  0, "gl_srt", &gl_str, 4, NULL},
        };
        
        uint32_t gl_in = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_STRUCT, SHADER_DATA_FLAG_SYSTEM, gl_arr, 1, "gl_in", 0, 0);
        uint32_t gl_out = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_STRUCT, SHADER_DATA_FLAG_SYSTEM, gl_arr, 1, "gl_out", 0, 0);

        uint32_t gl_Inv = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_INT, SHADER_DATA_FLAG_SYSTEM, NULL, 1, "gl_InvocationID", 0, 8);

        ShaderStructConstr gl_tes_level_in[] = {
            {SHADER_VARIABLE_TYPE_FLOAT, 0,  0, "float", NULL, 0, NULL},
        };
        
        uint32_t gl_t_l_in = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_ARRAY, SHADER_DATA_FLAG_SYSTEM, gl_tes_level_in, 2, "gl_TessLevelInner", 0, 12);

        ShaderStructConstr gl_tes_level_out[] = {
            {SHADER_VARIABLE_TYPE_FLOAT, 0,  0, "float", NULL, 0, NULL},
        };

        uint32_t gl_t_l_out = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_ARRAY, SHADER_DATA_FLAG_SYSTEM, gl_tes_level_out, 2, "gl_TessLevelOuter", 0, 11);
        
        uint32_t diffuse = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_IMAGE, SHADER_DATA_FLAG_UNIFORM_CONSTANT, NULL, 0, "diffuse", 0, 3);
        uint32_t height_map = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_IMAGE, SHADER_DATA_FLAG_UNIFORM_CONSTANT, NULL, 0, "samplerHeight", 0, 4);

        ShaderStructConstr norm_str[] = {
            {SHADER_VARIABLE_TYPE_VECTOR, 3,  0, "vec3", NULL, 0, NULL},
        };
        ShaderStructConstr uv_str[] = {
            {SHADER_VARIABLE_TYPE_VECTOR, 2,  0, "vec2", NULL, 0, NULL},
        };

        uint32_t nrm_dst = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_ARRAY, 0, norm_str, 32, "inNormal", 0, 0);
        uint32_t txt_dst = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_ARRAY, 0, uv_str, 32, "inUv", 1, 0);
        
        uint32_t nrm_out = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_ARRAY, SHADER_DATA_FLAG_OUTPUT, norm_str, 4, "outNormal", 0, 0);
        uint32_t txt_out = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_ARRAY, SHADER_DATA_FLAG_OUTPUT, uv_str, 4, "outUv", 1, 0);


        ShaderBuilderMake();
    }
    
    //------------------------------------------------------
    {
        ShaderBuilderInit(tese, SHADER_TYPE_TESELLATION_EVALUATION);


        ShaderBuilderMake();
    }
    
    //------------------------------------------------------
    {
        ShaderBuilderInit(frag, SHADER_TYPE_FRAGMENT);


        ShaderBuilderMake();
    }
}