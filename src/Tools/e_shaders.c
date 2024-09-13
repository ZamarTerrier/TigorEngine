#include "Tools/e_shaders.h"

void ShadersMakeDefault2DShader(ShaderBuilder *vert, ShaderBuilder *frag, bool hasTexture){

    memset(vert, 0, sizeof(ShaderBuilder));
    memset(frag, 0, sizeof(ShaderBuilder));

//------------------------------------------------------
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

    ShaderBuilderAddFuncAdd(uniform, posit, 2, vert->gl_struct_indx);
    ShaderBuilderAddFuncMove(clr_indx, 3, clr_dst, 3);
    ShaderBuilderAddFuncMove(txt_indx, 2, txt_dst, 2);

    ShaderBuilderMake();
//----------------------------------------
    ShaderBuilderInit(frag, SHADER_TYPE_FRAGMENT);

    ShaderStructConstr uniform_arr_2[] = {
        {SHADER_VARIABLE_TYPE_VECTOR, 2, 0, "offset", NULL, 0, NULL},
        {SHADER_VARIABLE_TYPE_VECTOR, 2, 0, "scale", NULL, 0, NULL},
        {SHADER_VARIABLE_TYPE_VECTOR, 2, 0, "rotation", NULL, 0, NULL},
    };

    uint32_t uniform2 = ShaderBuilderAddUniform(uniform_arr_2, 3, "ImageBufferObjects", 0, 2);

    uint32_t fragColor = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "fragColor", 0, 0);
    uint32_t fragTexCoord = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 2, "fragTexCoord", 1, 0);
    uint32_t outColor = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 4, "outColor", 0, 0);


    uint32_t texture = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_IMAGE, SHADER_DATA_FLAG_UNIFORM_CONSTANT, NULL, 0, "Texture2D", 0, 3);

    uint32_t res = ShaderBuilderAddFuncMult(uniform2, 1, SHADER_VARIABLE_TYPE_VECTOR, 2, fragTexCoord, 0, SHADER_VARIABLE_TYPE_VECTOR, 2, 2);

    ShaderBuilderAddFuncSetTexure(texture, res, outColor, 4);

    ShaderBuilderMake();
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
    ShaderBuilderAddFuncMoveToGL(res, 4, vert->gl_struct_indx);
    
    ShaderBuilderAddFuncMove(clr_indx, 3, clr_dst, 3);
    ShaderBuilderAddFuncMove(txt_indx, 2, txt_dst, 2);

    ShaderBuilderMake();
//----------------------------------------
    ShaderBuilderInit(frag, SHADER_TYPE_FRAGMENT);

    uint32_t fragColor = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "fragColor", 0, 0);
    uint32_t fragTexCoord = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 2, "fragTexCoord", 1, 0);
    uint32_t outColor = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 4, "outColor", 0, 0);

    uint32_t texture = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_IMAGE, SHADER_DATA_FLAG_UNIFORM_CONSTANT, NULL, 0, "Texture2D", 0, 2);

    if(hasTexture){
        ShaderBuilderAddFuncSetTexure(texture, fragTexCoord, outColor, 4);
    }else{

        uint32_t res = ShaderBuilderAcceptLoadL(fragColor, 0);

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
    }

    ShaderBuilderMake();
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
        ShaderBuilderAddFuncMoveToGL(res, 4, vert->gl_struct_indx);

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
        {SHADER_VARIABLE_TYPE_ARRAY, 10, SHADER_VARIABLE_TYPE_STRUCT, "lights", light_str, 6, "LightsStruct"},
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

        uint32_t res = ShaderBuilderAcceptLoadL(fragPos, 0);

        uint32_t arr6[] = { type_vec3,  res, res, 0, 1, 2};
        uint32_t v_shuffle =  ShaderBuilderAddOperand(arr6, 6, SHADER_OPERAND_TYPE_VECSHUFFLE);

        VectorExtract extr = ShaderBuilderGetElemenets(res, 3, 1);

        // uint32_t c_c3 = ShaderBuilderAddConstant(SHADER_VARIABLE_TYPE_INT, 0, 3, 0);
        // uint32_t inp_float = ShaderBuilderAddPointer(SHADER_VARIABLE_TYPE_FLOAT, 0, 0);

        // uint32_t arr7[] = { inp_float,  fragPos, c_c3};
        // res = ShaderBuilderAddOperand(arr7, 3, SHADER_OPERAND_TYPE_ACCESS);

        // res = ShaderBuilderAcceptLoadL(res, 0);

        res = ShaderBuilderCompositeConstruct((uint32_t []){ type_vec3,  extr.elems[0], extr.elems[0], extr.elems[0]}, 4);
        
        res = ShaderBuilderAddFuncDiv(v_shuffle, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, res, 0, SHADER_VARIABLE_TYPE_VECTOR, 3, 3);
        
        ShaderBuilderStoreValue((uint32_t []){ v_some_vec,  res}, 2);

        res = ShaderBuilderGetTexture(normal, fragTexCoord);

        VectorExtract extr2 = ShaderBuilderGetElemenets(res, 0, 3);
        
        res = ShaderBuilderCompositeConstruct((uint32_t []){ type_vec3,  extr2.elems[0], extr2.elems[1], extr2.elems[2]}, 4);

        res = ShaderBuilderGetTexture(texture, fragTexCoord);

        VectorExtract extr3 = ShaderBuilderGetElemenets(res, 0, 3);
        
        res = ShaderBuilderCompositeConstruct((uint32_t []){ type_vec3,  extr3.elems[0], extr3.elems[1], extr3.elems[2]}, 4);
                
        ShaderBuilderStoreValue((uint32_t []){ v_textureColor,  res}, 2);

        uint32_t begin_label = ShaderBuilderNextLabel(true, 0, false);    

        uint32_t loop_label = ShaderBuilderNextLabel(false, begin_label, false);  

        uint32_t end_label = ShaderBuilderNextLabel(false, 0, true);
        
        ShaderBuilderAddOperand((uint32_t []){end_label, loop_label}, 2, SHADER_OPERAND_TYPE_LOOP);  

        uint32_t inner_block = ShaderBuilderNextLabel(false, 0, false);

        uint32_t cond_block = ShaderBuilderNextLabel(true, 0, false);

        uint32_t temp_var = ShaderBuilderAcceptLoadL(i_iter, 0);

        res = ShaderBuilderAcceptAccess(lights, SHADER_VARIABLE_TYPE_INT, 1, (uint32_t []){ 1 }, 1);

        ShaderBuilderMakeBranchConditional(0, (uint32_t []){ temp_var, res }, 2, inner_block, end_label);

        ShaderBuilderSetCurrentLabel(end_label);
        
        res = ShaderBuilderAcceptLoadL(v_result, 0);

        VectorExtract extr4 = ShaderBuilderGetElemenets(res, 0, 3);
        
        res = ShaderBuilderCompositeConstruct((uint32_t []){ type_vec3,  extr4.elems[0], extr4.elems[1], extr4.elems[2], c_3}, 5);
        
        ShaderBuilderStoreValue((uint32_t []){ outColor,  res}, 2);
    }

    /*if(hasTexture){
        ShaderBuilderAddFuncSetTexure(texture, fragTexCoord, outColor, 4);
    }else{

        uint32_t res = ShaderBuilderAcceptLoadL(fragColor, 0);

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
    ShaderBuilderInit(vert, SHADER_TYPE_VERTEX);

    ShaderStructConstr uniform_arr[] = {
        {SHADER_VARIABLE_TYPE_VECTOR, 2, 0, "position", NULL, 0, NULL},
        {SHADER_VARIABLE_TYPE_VECTOR, 2, 0, "rotation", NULL, 0, NULL},
        {SHADER_VARIABLE_TYPE_VECTOR, 2, 0, "scale", NULL, 0, NULL},
    };
    
    uint32_t posit = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 2, "position", 0, 0);
    uint32_t clr_indx = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "color", 1, 0);
    uint32_t txt_indx = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 2, "inTexCoord", 2, 0);

    uint32_t clr_dst = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 3, "fragColor", 0, 0);
    uint32_t txt_dst = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 2, "fragTexCoord", 1, 0);

    ShaderBuilderAddFuncMoveToGL(posit, 2, vert->gl_struct_indx);
    ShaderBuilderAddFuncMove(clr_indx, 3, clr_dst, 3);
    ShaderBuilderAddFuncMove(txt_indx, 2, txt_dst, 2);

    ShaderBuilderMake();    
//----------------------------------------
    ShaderBuilderInit(frag, SHADER_TYPE_FRAGMENT);

    uint32_t fragColor = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "fragColor", 0, 0);
    uint32_t fragTexCoord = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 2, "fragTexCoord", 1, 0);
    uint32_t outColor = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 4, "outColor", 0, 0);

    uint32_t texture = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_IMAGE, SHADER_DATA_FLAG_UNIFORM_CONSTANT, NULL, 0, "Texture2D", 0, 1);

    uint32_t tExtract = ShaderBuilderGetTexture(texture, fragTexCoord);

    uint32_t res = ShaderBuilderAddFuncSetColor4(fragColor, tExtract, 1);

    uint32_t arr[] = { outColor, res} ;
    ShaderBuilderStoreValue(arr, 2);

    ShaderBuilderMake();
}

void ShadersMakeDefault2DTextShader(ShaderBuilder *vert, ShaderBuilder *frag){

    memset(vert, 0, sizeof(ShaderBuilder));
    memset(frag, 0, sizeof(ShaderBuilder));


//------------------------------------------------------
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

    ShaderBuilderAddFuncAdd(uniform, posit, 2, vert->gl_struct_indx);
    ShaderBuilderAddFuncMove(clr_indx, 3, clr_dst, 3);
    ShaderBuilderAddFuncMove(txt_indx, 2, txt_dst, 2);

    ShaderBuilderMake();
//----------------------------------------
    ShaderBuilderInit(frag, SHADER_TYPE_FRAGMENT);

    uint32_t fragColor = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 3, "fragColor", 0, 0);
    uint32_t fragTexCoord = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, 0, NULL, 2, "fragTexCoord", 1, 0);
    uint32_t outColor = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_VECTOR, SHADER_DATA_FLAG_OUTPUT, NULL, 4, "outColor", 0, 0);


    uint32_t texture = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_IMAGE, SHADER_DATA_FLAG_UNIFORM_CONSTANT, NULL, 0, "Texture2D", 0, 2);

    uint32_t tExtract = ShaderBuilderGetTexture(texture, fragTexCoord);

    uint32_t res = ShaderBuilderAddFuncSetColor4(fragColor, tExtract, 1);

    uint32_t arr[] = { outColor, res} ;
    ShaderBuilderStoreValue(arr, 2);

    ShaderBuilderMake();
}