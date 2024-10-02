#include "Tools/e_shaders.h"

void ShadersMakeDefault2DShader(ShaderBuilder *vert, ShaderBuilder *frag, bool hasTexture){

    memset(vert, 0, sizeof(ShaderBuilder));
    memset(frag, 0, sizeof(ShaderBuilder));

    //------------------------------------------------------
    {
        ShaderBuilderInit(vert, SHADER_TYPE_VERTEX);

        ShaderStructConstr uniform_arr[] = {
            {SHADER_VARIABLE_TYPE_VECTOR, 2, 0, "position", NULL, 0, NULL},
            {SHADER_VARIABLE_TYPE_VECTOR, 2, 0, "rotation", NULL, 0, NULL},
            {SHADER_VARIABLE_TYPE_VECTOR, 2, 0, "scale", NULL, 0, NULL},
        };

        uint32_t uniform = ShaderBuilderAddUniform(uniform_arr, 3, "TransformBufferObjects", 0, 1);

        uint32_t posit = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 2, "position", 0, 0);
        uint32_t clr_indx = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "color", 1, 0);
        uint32_t txt_indx = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 2, "inTexCoord", 2, 0);

        uint32_t clr_dst = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 3, "fragColor", 0, 0);
        uint32_t txt_dst = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 2, "fragTexCoord", 1, 0);

        uint32_t res = ShaderBuilderAddFuncMult(uniform, 2, SHADER_VARIABLE_TYPE_VECTOR, 2, posit, 0, SHADER_VARIABLE_TYPE_VECTOR, 2, 2);
        res = ShaderBuilderAddFuncAdd(uniform, 0, SHADER_VARIABLE_TYPE_VECTOR, 2, res, 0, SHADER_VARIABLE_TYPE_VECTOR, 2, 2);

        res = ShaderBuilderMutateVector(res, 2, 4);
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
            {SHADER_VARIABLE_TYPE_VECTOR, 2, 0, 0, "offset", NULL, 0, NULL},
            {SHADER_VARIABLE_TYPE_VECTOR, 2, 0, 0, "scale", NULL, 0, NULL},
            {SHADER_VARIABLE_TYPE_VECTOR, 2, 0, 0, "rotation", NULL, 0, NULL},
        };

        uint32_t uniform2 = ShaderBuilderAddUniform(uniform_arr_2, 3, "ImageBufferObjects", 0, 2);

        uint32_t fragColor = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "fragColor", 0, 0);
        uint32_t fragTexCoord = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 2, "fragTexCoord", 1, 0);
        uint32_t outColor = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 4, "outColor", 0, 0);

        if(hasTexture){
            uint32_t texture = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_IMAGE, SHADER_DATA_FLAG_UNIFORM_CONSTANT, NULL, 0, "Texture2D", 0, 3);

            uint32_t res = ShaderBuilderAddFuncMult(uniform2, 1, SHADER_VARIABLE_TYPE_VECTOR, 2, fragTexCoord, 0, SHADER_VARIABLE_TYPE_VECTOR, 2, 2);

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
            {SHADER_VARIABLE_TYPE_VECTOR, 2, 0, "position", NULL, 0, NULL},
            {SHADER_VARIABLE_TYPE_VECTOR, 2, 0, "rotation", NULL, 0, NULL},
            {SHADER_VARIABLE_TYPE_VECTOR, 2, 0, "scale", NULL, 0, NULL},
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

        res = ShaderBuilderAddFuncMult(uniform, 2, SHADER_VARIABLE_TYPE_VECTOR, 2, posit, 0, SHADER_VARIABLE_TYPE_VECTOR, 2, 2);
        res = ShaderBuilderAddFuncAdd(uniform, 0, SHADER_VARIABLE_TYPE_VECTOR, 2, res, 0, SHADER_VARIABLE_TYPE_VECTOR, 2, 2);
        res = ShaderBuilderMutateVector(res, 2, 4);
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

        uint32_t l_kill = ShaderBuilderNextLabel(false);

        uint32_t end_label = ShaderBuilderNextLabel(true);
        
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
        {SHADER_VARIABLE_TYPE_MATRIX, 4, 0, "porl", NULL, 0, NULL},
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
        uint32_t txt_dst = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 2, "fragTexCoord", 1, 0);
        uint32_t mat_dst = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_INT, SHADER_DATA_FLAG_OUTPUT, NULL, 0, "fragMatIndx", 2, 0);

        uint32_t vec_type = ShaderBuilderAddPointer(SHADER_VARIABLE_TYPE_VECTOR, 4, SHADER_DATA_FLAG_FUNCTION);
        uint32_t mat_type = ShaderBuilderAddPointer(SHADER_VARIABLE_TYPE_MATRIX, 4, SHADER_DATA_FLAG_FUNCTION);

        uint32_t v_model = ShaderBuilderAddOperand((uint32_t []){ vec_type, 7 /*Function*/ }, 2, SHADER_OPERAND_TYPE_VARIABLE);
        uint32_t m_skinMat = ShaderBuilderAddOperand((uint32_t []){ mat_type, 7 /*Function*/ }, 2, SHADER_OPERAND_TYPE_VARIABLE);

        uint32_t cond_label = ShaderBuilderNextLabel(false);
        uint32_t end_label = ShaderBuilderNextLabel(true);

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
        uint32_t fragTexCoord = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 2, "fragTexCoord", 1, 0);
        uint32_t fragMatIndx = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_INT, 0, NULL, 0, "fragMatIndx", 2, 0);

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

            uint32_t l_kill = ShaderBuilderNextLabel(false);

            uint32_t end_label = ShaderBuilderNextLabel(true);
            
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

        uint32_t l_kill = ShaderBuilderNextLabel(false);

        uint32_t end_label = ShaderBuilderNextLabel(true);
        
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

void ShadersMakeDeafult3DShaderWithLight(ShaderBuilder *vert, ShaderBuilder *frag, bool hasTexture){
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
    uint32_t clr_indx = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "color", 1, 0);
    uint32_t txt_indx = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 2, "inTexCoord", 2, 0);

    uint32_t frg_pos = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 4, "fragPos", 0, 0);
    uint32_t clr_dst = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 3, "fragColor", 1, 0);
    uint32_t txt_dst = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 2, "fragTexCoord", 2, 0);

    {
        uint32_t res = ShaderBuilderAddFuncMult(uniform, 1, SHADER_VARIABLE_TYPE_MATRIX, 4, uniform, 0, SHADER_VARIABLE_TYPE_MATRIX, 4, 4);
        res = ShaderBuilderAddFuncMult(uniform, 2,  SHADER_VARIABLE_TYPE_MATRIX, 4, res, 0, SHADER_VARIABLE_TYPE_MATRIX, 4, 4);
        res = ShaderBuilderAddFuncMult(res, 0,  SHADER_VARIABLE_TYPE_MATRIX, 4, posit, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, 4);
        
        uint32_t glPos = ShaderBuilderAcceptLoad(vert->gl_struct_indx, 0);
        ShaderBuilderStoreValue((uint32_t []){ glPos, res}, 2);

        res = ShaderBuilderAddFuncMult(uniform, 0, SHADER_VARIABLE_TYPE_MATRIX, 4, posit, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, 4);
        
        uint32_t arr[] = {frg_pos, res};
        ShaderBuilderStoreValue(arr, sizeof(arr));
        
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
        {SHADER_VARIABLE_TYPE_FLOAT , 3, 0, "intensity" , NULL, 0, NULL},
        {SHADER_VARIABLE_TYPE_INT   , 3, 0, "type" , NULL, 0, NULL},
        {SHADER_VARIABLE_TYPE_FLOAT , 3, 0, "cutoff" , NULL, 0, NULL},
    };

    ShaderStructConstr light_arr[] = {
        {SHADER_VARIABLE_TYPE_ARRAY, 10, 0, "lights", light_str, 6, "LightsStruct"},
        {SHADER_VARIABLE_TYPE_INT, 0, 0, "numLights" , NULL, 0, NULL},
    };

    uint32_t lights = ShaderBuilderAddUniform(light_arr, 2, "LightBufferObject", 0, 1);

    uint32_t fragPos = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 4, "fragPos", 0, 0);
    uint32_t fragColor = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "fragColor", 1, 0);
    uint32_t fragTexCoord = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 2, "fragTexCoord", 2, 0);
    uint32_t outColor = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 4, "outColor", 0, 0);

    uint32_t texture = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_IMAGE, SHADER_DATA_FLAG_UNIFORM_CONSTANT, NULL, 0, "Texture2D", 0, 2);
    uint32_t normal = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_IMAGE, SHADER_DATA_FLAG_UNIFORM_CONSTANT, NULL, 0, "Normal", 0, 3);

    {
        float v = 0.5f;
        uint32_t c = 0;
        memcpy(&c, &v, sizeof(uint32_t));
        uint32_t c_c1 = ShaderBuilderAddConstantComposite(SHADER_VARIABLE_TYPE_VECTOR, 3, c);
        uint32_t c_c2 = ShaderBuilderAddConstantComposite(SHADER_VARIABLE_TYPE_VECTOR, 3, 0);
        
        uint32_t c_1 = ShaderBuilderAddConstant(SHADER_VARIABLE_TYPE_FLOAT, 0, 0, 1);
        uint32_t c_2 = ShaderBuilderAddConstant(SHADER_VARIABLE_TYPE_INT, 0, 0, 1);
        
        v = 1.0f;
        memcpy(&c, &v, sizeof(uint32_t));
        uint32_t c_3 = ShaderBuilderAddConstant(SHADER_VARIABLE_TYPE_FLOAT, 0, c, 1);

        uint32_t pointer_vec3 = ShaderBuilderAddPointer(SHADER_VARIABLE_TYPE_VECTOR, 3, SHADER_DATA_FLAG_FUNCTION /*Function*/);

        uint32_t arr[] = { pointer_vec3,  7 /*Function*/};
        uint32_t v_result = ShaderBuilderAddOperand(arr, 2, SHADER_OPERAND_TYPE_VARIABLE);
        uint32_t v_ambient = ShaderBuilderAddOperand(arr, 2, SHADER_OPERAND_TYPE_VARIABLE);
        uint32_t v_some_vec = ShaderBuilderAddOperand(arr, 2, SHADER_OPERAND_TYPE_VARIABLE);
        uint32_t v_normal = ShaderBuilderAddOperand(arr, 2, SHADER_OPERAND_TYPE_VARIABLE);
        uint32_t v_lightDir = ShaderBuilderAddOperand(arr, 2, SHADER_OPERAND_TYPE_VARIABLE);
        uint32_t v_textureColor = ShaderBuilderAddOperand(arr, 2, SHADER_OPERAND_TYPE_VARIABLE);
        uint32_t v_lightColor = ShaderBuilderAddOperand(arr, 2, SHADER_OPERAND_TYPE_VARIABLE);
        
        uint32_t pointer_int = ShaderBuilderAddPointer(SHADER_VARIABLE_TYPE_INT, 0, SHADER_DATA_FLAG_FUNCTION /*Function*/);
        uint32_t pointer_float = ShaderBuilderAddPointer(SHADER_VARIABLE_TYPE_FLOAT, 0, SHADER_DATA_FLAG_FUNCTION /*Function*/);

        uint32_t arr2[] = { pointer_int,  7 /*Function*/};
        uint32_t i_iter = ShaderBuilderAddOperand(arr2, 2, SHADER_OPERAND_TYPE_VARIABLE);
        ShaderBuilderStoreValue((uint32_t []){ i_iter,  c_2}, 2);
        
        uint32_t arr3[] = { pointer_float,  7 /*Function*/};
        uint32_t f_diff = ShaderBuilderAddOperand(arr3, 2, SHADER_OPERAND_TYPE_VARIABLE);
        uint32_t f_theta = ShaderBuilderAddOperand(arr3, 2, SHADER_OPERAND_TYPE_VARIABLE);
        ShaderBuilderStoreValue((uint32_t []){ f_diff,  c_1}, 2);

        uint32_t arr4[] = { v_result,  c_c2};
        ShaderBuilderStoreValue(arr4, 2);
        
        uint32_t arr5[] = { v_ambient,  c_c1};
        ShaderBuilderStoreValue(arr5, 2);

        uint32_t type_vec3 = ShaderBuilderAddVector(3, NULL);

        uint32_t res = ShaderBuilderAcceptLoad(fragPos, 0);

        uint32_t arr6[] = { type_vec3,  res, res, 0, 1, 2};
        uint32_t v_shuffle =  ShaderBuilderAddOperand(arr6, 6, SHADER_OPERAND_TYPE_VECSHUFFLE);

        VectorExtract extr = ShaderBuilderGetElemenets(SHADER_VARIABLE_TYPE_VECTOR, res, 3, 3, 1);

        // uint32_t c_c3 = ShaderBuilderAddConstant(SHADER_VARIABLE_TYPE_INT, 0, 3, 0);
        // uint32_t inp_float = ShaderBuilderAddPointer(SHADER_VARIABLE_TYPE_FLOAT, 0, 0);

        // uint32_t arr7[] = { inp_float,  fragPos, c_c3};
        // res = ShaderBuilderAddOperand(arr7, 3, SHADER_OPERAND_TYPE_ACCESS);

        // res = ShaderBuilderAcceptLoad(res, 0);

        res = ShaderBuilderCompositeConstruct((uint32_t []){ type_vec3,  extr.elems[0], extr.elems[0], extr.elems[0]}, 4);
        
        res = ShaderBuilderAddFuncDiv(v_shuffle, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, res, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, 3);
        
        ShaderBuilderStoreValue((uint32_t []){ v_some_vec,  res}, 2);

        res = ShaderBuilderGetTexture(normal, fragTexCoord, 0);

        VectorExtract extr2 = ShaderBuilderGetElemenets(SHADER_VARIABLE_TYPE_VECTOR, res, 4, 0, 3);
        
        res = ShaderBuilderCompositeConstruct((uint32_t []){ type_vec3,  extr2.elems[0], extr2.elems[1], extr2.elems[2]}, 4);

        res = ShaderBuilderGetTexture(texture, fragTexCoord, 0);

        VectorExtract extr3 = ShaderBuilderGetElemenets(SHADER_VARIABLE_TYPE_VECTOR, res, 4, 0, 3);
        
        res = ShaderBuilderCompositeConstruct((uint32_t []){ type_vec3,  extr3.elems[0], extr3.elems[1], extr3.elems[2]}, 4);
                
        ShaderBuilderStoreValue((uint32_t []){ v_textureColor,  res}, 2);

        uint32_t begin_label = ShaderBuilderNextLabel(false);    

        uint32_t loop_label = ShaderBuilderNextLabel(false);  

        uint32_t end_label = ShaderBuilderNextLabel(true);
        
        ShaderBuilderAddOperand((uint32_t []){end_label, loop_label}, 2, SHADER_OPERAND_TYPE_LOOP);  

        uint32_t inner_block = ShaderBuilderNextLabel(false);

        uint32_t cond_block = ShaderBuilderNextLabel(false);
        ShaderBuilderMakeTransition(cond_block);

        uint32_t temp_var = ShaderBuilderAcceptLoad(i_iter, 0);

        res = ShaderBuilderAcceptAccess(lights, SHADER_VARIABLE_TYPE_INT, 1, (uint32_t []){ 1 }, 1, true);

        ShaderBuilderMakeBranchConditional(0, (uint32_t []){ temp_var, res }, 2, 0, inner_block, end_label);

        ShaderBuilderSetCurrentLabel(end_label);
        
        res = ShaderBuilderAcceptLoad(v_result, 0);

        VectorExtract extr4 = ShaderBuilderGetElemenets(SHADER_VARIABLE_TYPE_VECTOR, res, 3, 0, 3);
        
        res = ShaderBuilderCompositeConstruct((uint32_t []){ type_vec3,  extr4.elems[0], extr4.elems[1], extr4.elems[2], c_3}, 5);
        
        ShaderBuilderStoreValue((uint32_t []){ outColor,  res}, 2);
    }

    /*if(hasTexture){
        ShaderBuilderAddFuncSetTexure(texture, fragTexCoord, outColor, 4);
    }else{

        uint32_t res = ShaderBuilderAcceptLoad(fragColor, 0);

        VectorExtract extr = ShaderBuilderGetElemenets(res, 0, 3);

        uint32_t vec_type = ShaderBuilderAddVector(4, NULL);

        float v_f = 1.0f;
        uint32_t v_u = 0;
        memcpy(&v_u, &v_f, sizeof(uint32_t));        
        uint32_t cnst = ShaderBuilderAddConstant(SHADER_VARIABLE_TYPE_FLOAT, 0, v_u, 1);

        uint32_t arr[] = { vec_type, extr.elems[0], extr.elems[1], extr.elems[2], cnst };
        res = ShaderBuilderCompositeConstruct(arr, 5);
        
        uint32_t arr2[] = {outColor, res};
        
        ShaderBuilderStoreValue(arr2, sizeof(arr2));
    }*/

    ShaderBuilderMake();
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

        uint32_t l_kill = ShaderBuilderNextLabel(false);

        uint32_t end_label = ShaderBuilderNextLabel(true);
        
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