#include "Tools/shader_builder.h"

#include "Core/e_blue_print.h"

#include "spirv.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>

ShaderBuilder *curr_builder;
ShaderLabel *curr_label;

uint32_t ShaderBuilderAddImage();
uint32_t ShaderBuilderAddFloat();
uint32_t ShaderBuilderAddInt(uint32_t sign);
uint32_t ShaderBuilderAddVector(uint32_t size, char *name);
uint32_t ShaderBuilderAddMatrix(uint32_t size, char *name);
uint32_t ShaderBuilderAddStruct(ShaderStructConstr *struct_arr, uint32_t count, char *name );
uint32_t ShaderBuilderAddConstant(ShaderVariableType var_type, ShaderDataFlags flags,  uint32_t valu, uint32_t sign);

void ShaderBuilderSetCurrent(ShaderBuilder *builder){
    curr_builder = builder;
}

ShaderLabel *ShaderBuilderFindLabel(uint32_t label_indx){

    ShaderLabel *label = NULL;

    for(int i=0;i < curr_builder->main_point_index->num_labels;i++){
        if(curr_builder->main_point_index->labels[i].index == label_indx)
            label = &curr_builder->main_point_index->labels[i];
    }

    return label;
}

void ShaderBuilderSetCurrentLabel(uint32_t label_indx){

    ShaderLabel *label = ShaderBuilderFindLabel(label_indx);

    if(label == NULL)
        return;

    curr_label = label;
}

ShaderVariable *ShaderBuilderAllocateVariabel(ShaderBuilder *builder){

    ShaderVariable *variable = calloc(1, sizeof(ShaderVariable));

     if(builder->alloc_head->node == NULL){
        builder->alloc_head->next = calloc(1, sizeof(ChildStack));
        builder->alloc_head->node = variable;
    }
    else{

        ChildStack *child = builder->alloc_head;

        while(child->next != NULL)
        {
            child = child->next;
        }

        child->next = calloc(1, sizeof(ChildStack));
        child->node = variable;
    }

    return variable;
}

uint32_t ShaderBuilderGetVariablesCount(ShaderBuilder *builder){
    ChildStack *child = builder->alloc_head;

    uint32_t counter = 0;    
    if(child == NULL)
        return counter;
           
    while(child != NULL){    
        
        if(child->node != NULL){
            counter ++;
        }

        child = child->next;
    }

    return counter;
}

void ShaderBuilderDeleteVariabel(ShaderBuilder *builder, ShaderVariable *variable){
    ShaderVariable *curr = NULL;

    ChildStack *child = builder->alloc_head;
    ChildStack *before = NULL;

    while(child != NULL)
    {
        curr = child->node;

        if(curr == variable)
            break;

        before = child;
        child = child->next;
    }

    if(curr == NULL){
        printf("Can't find this memory 0x%x\n", variable);
        return;
    }

    if(child->next != NULL){

        if(child->node != NULL)
            free(child->node);

        if(before != NULL)
            before->next = child->next;
        else
            builder->alloc_head = child->next;

        free(child);
        child = NULL;

    }else{
        
        if(before != NULL){
            free(child);
            child = NULL;  
        } 
    }
}

void ShaderBuilderClear(ShaderBuilder *builder){
    ChildStack *child = builder->alloc_head;
    
    if(child == NULL)
        return;

    ChildStack *next = NULL;
    ChildStack *before = NULL;
           
    while(child != NULL){    
        
        if(child->node != NULL){
            free(child->node);
        }

        before = child;  
        child = child->next;

        free(before);
        before = NULL;
    }
    
    builder->alloc_head = NULL;
}

uint32_t ShaderBuilderCheckVariable(ShaderVariableType variable_type, uint32_t *vals,  uint32_t size){

    ShaderVariable *variable = NULL;
    
    ChildStack *child = curr_builder->alloc_head;

    int find = 0;
    while(child != NULL){
        variable = child->node;

        if(variable != NULL){
            find = 1;

            if(variable->type == variable_type){
                for(int j=0;j < size;j++){
                    if(variable->values[j] != vals[j]){
                        find = 0;
                        break;
                    }
                }

                if(find)
                    return variable->indx;
            }
        }

        child = child->next;
    }

    return 0;
}

ShaderVariable *ShaderBuilderFindVar(uint32_t indx){

    ShaderVariable *variable = NULL;
    
    ChildStack *child = curr_builder->alloc_head;

    int find = 0;
    while(child != NULL){
        variable = child->node;

        if(variable != NULL){
            if(variable->indx == indx){
                return variable;
            }
        }

        child = child->next;
    }

    return NULL;
}


uint32_t ShaderBuilderGetType(ShaderVariableType type, uint32_t type_arg, char *name){

    uint32_t type_indx = 0;

    switch(type){
        case SHADER_VARIABLE_TYPE_FLOAT:
            type_indx = ShaderBuilderAddFloat();
            break;
        case SHADER_VARIABLE_TYPE_INT:
            type_indx = ShaderBuilderAddInt(type_arg);
            break;
        case SHADER_VARIABLE_TYPE_VECTOR:
            type_indx = ShaderBuilderAddVector(type_arg, name);
            break;
        case SHADER_VARIABLE_TYPE_MATRIX:
            type_indx = ShaderBuilderAddMatrix(type_arg, NULL);
            break;
        case SHADER_VARIABLE_TYPE_IMAGE:
            type_indx = ShaderBuilderAddImage();
            break;
    }

    return type_indx;
}

void ShaderBuilderAddDecor(uint32_t var_indx, ShaderDecorType type, uint32_t *args, uint32_t size){
    
    curr_builder->decors[curr_builder->num_decorations].indx = var_indx;
    curr_builder->decors[curr_builder->num_decorations].type = type;   

    memcpy(curr_builder->decors[curr_builder->num_decorations].args, args, size * sizeof(uint32_t));

    curr_builder->num_decorations++;

}

ShaderDecoration *ShaderBuilderFindDecor(uint32_t var_indx, ShaderDecorType type){

    uint32_t counter = 0;
    for(int i=0; i <  curr_builder->num_decorations;i++){
        if(curr_builder->decors[i].indx == var_indx && curr_builder->decors[i].type == type)
            return &curr_builder->decors[i];
    }

    return NULL;
}

void ShaderBuilderAddOp(uint32_t code, uint32_t value){

    curr_builder->code[curr_builder->size] = (value << 16) | code;

    curr_builder->size++;
}

void ShaderBuilderAddValue(uint32_t value){

    curr_builder->code[curr_builder->size] = value;

    curr_builder->size++;
}

void ShaderBuilderAddString(char* string){
    uint32_t len = strlen(string);

    char *point = (char *)&curr_builder->code[curr_builder->size];

    memcpy(point, string, len);


    while(len % sizeof(uint32_t)){
        curr_builder->infos[curr_builder->num_debug_infos].name[len] = 0;
        len ++;
    }

    curr_builder->size += len / sizeof(uint32_t);

    /*if(string[len + 1] != 0)
        ShaderBuilderAddValue(0x0 );*/
}

uint32_t ShaderBuilderAddVariableF(ShaderVariableType type, uint32_t flags, uint32_t *args, uint32_t num_args, uint32_t *vals,  uint32_t num_vals){

    ShaderVariable *variable = ShaderBuilderAllocateVariabel(curr_builder);

    variable->type = type;
    variable->indx = curr_builder->current_index + 1;

    if(num_args > 0)
    {
        if(args == NULL)
            return 0;

        for(int i=0; i  < num_args;i++)
            variable->args[i] = args[i];

    }

    variable->num_args = num_args;

    if(num_vals > 0)
    {
        if(vals == NULL)
            return 0;

        for(int i=0; i  < num_vals;i++)
            variable->values[i] = vals[i];

    }

    variable->num_values = num_vals;
    variable->flags = flags;

    curr_builder->current_index++;
    
    return curr_builder->current_index;
}

uint32_t ShaderBuilderAddVariable(ShaderVariableType type, uint32_t flags, uint32_t *args, uint32_t num_args, uint32_t *vals,  uint32_t num_vals){

    ShaderVariable *variable = NULL;

    ChildStack *child = curr_builder->alloc_head;

    int find = 0;
    while(child != NULL){
        variable = child->node;

        if(variable != NULL){
            if(flags == variable->flags && type == variable->type)
                find = 1;
            else{
                child = child->next;
                continue;
            }

            if(variable->num_args != num_args || variable->num_values != num_vals){
                find = 0;
                child = child->next;
                continue;
            }

            for(int j=0; j < num_args;j++){
                if(variable->args[j] != args[j])
                    find = 0;
            }

            for(int j=0; j < num_vals;j++){
                if(variable->values[j] != vals[j])
                    find = 0;
            }

            if(find)
                break;
        }

        child = child->next;
    }

    if(find){
        return variable->indx;
    }
    
    return ShaderBuilderAddVariableF(type, flags, args, num_args, vals,  num_vals);
}

uint32_t ShaderBuilderAddFloat(){

    uint32_t arr[] = { 32, 1 };
    uint32_t res = ShaderBuilderCheckVariable(SHADER_VARIABLE_TYPE_FLOAT, arr, 2);

    if(!res){
        res = ShaderBuilderAddVariable(SHADER_VARIABLE_TYPE_FLOAT, 0, NULL, 0, arr, 2);
    }

    return res;
}

uint32_t ShaderBuilderAddInt(uint32_t sign){

    uint32_t arr[] = { 32, sign };
    uint32_t res = ShaderBuilderCheckVariable(SHADER_VARIABLE_TYPE_INT, arr, 2);

    if(!res){
        res = ShaderBuilderAddVariable(SHADER_VARIABLE_TYPE_INT, 0, NULL, 0, arr, 2);
    }

    return res;
}

uint32_t ShaderBuilderCheckConstans(uint32_t type_indx, uint32_t valu){

    ShaderVariable *variable = NULL;

    ChildStack *child = curr_builder->alloc_head;

    int find = 0;
    while(child != NULL){
        variable = child->node;

        if(variable != NULL){
            if(variable->type == SHADER_VARIABLE_TYPE_CONSTANT && variable->args[0] == type_indx && variable->values[0] == valu)
                return variable->indx;

        }

        child = child->next;
    }

    return 0;
}

uint32_t ShaderBuilderAddConstant(ShaderVariableType var_type, ShaderDataFlags flags,  uint32_t valu, uint32_t sign){

    uint32_t arr[] = { 32, sign };
    uint32_t type_indx = ShaderBuilderCheckVariable(var_type, arr, 2);

    if(!type_indx){

        switch (var_type) {
            case SHADER_VARIABLE_TYPE_INT:
                type_indx = ShaderBuilderAddInt(sign);
                break;
            case SHADER_VARIABLE_TYPE_FLOAT:
                type_indx = ShaderBuilderAddFloat();
                break;
        }
    }

    uint32_t res = ShaderBuilderCheckConstans(type_indx, valu);

    if(!res){
        ShaderVariable *variable = ShaderBuilderAllocateVariabel(curr_builder);

        variable->type = SHADER_VARIABLE_TYPE_CONSTANT;
        variable->indx = curr_builder->current_index + 1;
        variable->args[0] = type_indx;
        variable->values[0] = valu;

        variable->num_args = 1;
        variable->num_values = 1;
        variable->flags = flags;

        curr_builder->current_index++;

        return curr_builder->current_index;
    }

    return res;
}

uint32_t ShaderBuilderAddConstantComposite(ShaderVariableType var_type, uint32_t size, uint32_t value){

    uint32_t type_indx = 0;

    switch (var_type) {
        case SHADER_VARIABLE_TYPE_VECTOR:
            type_indx = ShaderBuilderAddVector(size, NULL);
            break;
    }

    uint32_t cnst = ShaderBuilderAddConstant(SHADER_VARIABLE_TYPE_FLOAT, 0, value, 1);

    uint32_t arr[size];
    for(int i=0; i < size;i++){
        arr[i] = cnst;
    }
    
    uint32_t res = ShaderBuilderAddVariable(SHADER_VARIABLE_TYPE_CONSTANT_COMPOSITE, 0, &type_indx, 1, arr, size);

    return res;
}

uint32_t ShaderBuilderCheckArray(uint32_t type_indx, uint32_t const_indx){

    ShaderVariable *variable = NULL;

    ChildStack *child = curr_builder->alloc_head;

    int find = 0;
    while(child != NULL){
        variable = child->node;

        if(variable != NULL){

            if(variable->type == SHADER_VARIABLE_TYPE_ARRAY)
                if(variable->args[0] == type_indx && variable->args[1] == const_indx)
                    return variable->indx;
        }

        child = child->next;
    }

    return 0;
}

uint32_t ShaderBuilderAddMatrix(uint32_t size, char *name){

    uint32_t arr[] = { size };
    uint32_t res = ShaderBuilderCheckVariable(SHADER_VARIABLE_TYPE_MATRIX, arr, 1);

    if(!res){
        uint32_t vec_type = ShaderBuilderAddVector(size, NULL);

        uint32_t arr[] = { vec_type };
        uint32_t arr2[] = { size };
        res = ShaderBuilderAddVariable(SHADER_VARIABLE_TYPE_MATRIX, 0, arr, 1, arr2, 1);
    }

    if(name != NULL){
        uint32_t len = strlen(name);

        memcpy(curr_builder->infos[curr_builder->num_debug_infos].name, name, len);
        curr_builder->infos[curr_builder->num_debug_infos].indx = res;

        curr_builder->num_debug_infos++;
    }

    return res;
}

uint32_t ShaderBuilderAddArray(uint32_t count, ShaderStructConstr *str, uint32_t size, char *name){

    uint32_t res = 0, type_indx = 0;

    if(size > 1){
        type_indx = ShaderBuilderAddStruct(str, size, name);
    }else {
        switch(str[0].var_type){
            case SHADER_VARIABLE_TYPE_FLOAT:
                type_indx = ShaderBuilderAddFloat();
                break;
            case SHADER_VARIABLE_TYPE_INT:
                type_indx = ShaderBuilderAddInt(0);
                break;
            case SHADER_VARIABLE_TYPE_STRUCT:
                type_indx = ShaderBuilderAddStruct(str[0].str, str[0].count, name);
                break;
            case SHADER_VARIABLE_TYPE_MATRIX:
                type_indx = ShaderBuilderAddMatrix(str[0].size, NULL);
                break;
        }
    }
    

    uint32_t cnst = ShaderBuilderAddConstant(SHADER_VARIABLE_TYPE_INT, 0, count, 0);

    uint32_t check = ShaderBuilderCheckArray(type_indx, cnst);

    if(!check){
        uint32_t arr[] = { type_indx, cnst };
        res = ShaderBuilderAddVariable(SHADER_VARIABLE_TYPE_ARRAY, 0, arr, 2, NULL, 0 );
    }else
        res = check;

    /*if(name != NULL){
        uint32_t len = strlen(name);

        memcpy(builder->infos[builder->num_debug_infos].name, name, len);
        builder->infos[builder->num_debug_infos].indx = res;

        builder->num_debug_infos++;
    }*/

    return res;

}

uint32_t ShaderBuilderAddVector(uint32_t size, char *name){

    uint32_t arr[] = { size };
    uint32_t res = ShaderBuilderCheckVariable(SHADER_VARIABLE_TYPE_VECTOR, arr, 1);

    if(!res){
        res = ShaderBuilderAddFloat();

        uint32_t arr[] = { res };
        uint32_t arr2[] = { size };
        res = ShaderBuilderAddVariable(SHADER_VARIABLE_TYPE_VECTOR, 0, arr, 1, arr2, 1);
    }

    if(name != NULL){
        uint32_t len = strlen(name);

        memcpy(curr_builder->infos[curr_builder->num_debug_infos].name, name, len);
        curr_builder->infos[curr_builder->num_debug_infos].indx = res;

        curr_builder->num_debug_infos++;
    }

    return res;
}

uint32_t ShaderBuilderAddImage(){
    
    uint32_t arr[] = { 1, 0, 0, 0, 1, 0};
    uint32_t res = ShaderBuilderCheckVariable(SHADER_VARIABLE_TYPE_IMAGE, arr, 6);

    if(!res){
        res = ShaderBuilderAddFloat(curr_builder);
        
        uint32_t arr[] = { res };
        uint32_t arr2[] = { 1, 0, 0, 0, 1, 0};
        res = ShaderBuilderAddVariable(SHADER_VARIABLE_TYPE_IMAGE, 0, arr, 1, arr2, 6);

    }
    
    uint32_t arr2[] = { res };
    res = ShaderBuilderAddVariable(SHADER_VARIABLE_TYPE_SAMPLED_IMAGE, 0, arr2, 1, NULL, 0);

    return res;
}

uint32_t ShaderBuilderAddStruct(ShaderStructConstr *struct_arr, uint32_t count, char *name ){

    uint32_t elem_indx[count];

    ShaderVariable *temp = NULL;

    for(int i = 0;i < count;i++){

         switch(struct_arr[i].var_type){
             case SHADER_VARIABLE_TYPE_INT:
                 elem_indx[i] = ShaderBuilderAddInt(1);
                 break;
             case SHADER_VARIABLE_TYPE_FLOAT:
                 elem_indx[i] = ShaderBuilderAddFloat();
                 break;
             case SHADER_VARIABLE_TYPE_VECTOR:
                 elem_indx[i] = ShaderBuilderAddVector(struct_arr[i].size, NULL);
                 break;
             case SHADER_VARIABLE_TYPE_MATRIX:
                 elem_indx[i] = ShaderBuilderAddMatrix(struct_arr[i].size, NULL);
                 break;
             case SHADER_VARIABLE_TYPE_ARRAY:
                 elem_indx[i] = ShaderBuilderAddArray(struct_arr[i].size, struct_arr[i].str, struct_arr[i].count, struct_arr[i].name_str);
                 break;
             case SHADER_VARIABLE_TYPE_STRUCT:
                 elem_indx[i] = ShaderBuilderAddStruct(struct_arr[i].str, struct_arr[i].count, struct_arr[i].name_str);
                 break;
         }
    }

    ShaderVariable *variable = ShaderBuilderAllocateVariabel(curr_builder);

    memcpy(variable->args, elem_indx, sizeof(uint32_t) * count);

    variable->type = SHADER_VARIABLE_TYPE_STRUCT;
    variable->indx = curr_builder->current_index + 1;
    variable->num_args = count;
    variable->num_values = 0;
    variable->flags = 0;

    uint32_t len = 0;

    if(name == NULL)
        name  = "Temp Struct";

    len = strlen(name);

    memcpy(curr_builder->infos[curr_builder->num_debug_infos].name, name, len);
    curr_builder->infos[curr_builder->num_debug_infos].indx = curr_builder->current_index + 1;

    for(int i=0;i < count;i++){
        len = strlen(struct_arr[i].name);
        memcpy(curr_builder->infos[curr_builder->num_debug_infos].child_name[i], struct_arr[i].name, len);
    }

    curr_builder->infos[curr_builder->num_debug_infos].num_childs = count;

    curr_builder->num_debug_infos++;
    curr_builder->current_index++;

    return curr_builder->current_index;
}

uint32_t ShaderBuilderAddUniform(ShaderStructConstr *struct_arr, uint32_t count, char *name, uint32_t location, uint32_t binding){

    uint32_t res = ShaderBuilderAddStruct(struct_arr, count, name);

    curr_builder->decors[curr_builder->num_decorations].indx = res;
    curr_builder->decors[curr_builder->num_decorations].str_point = struct_arr;
    curr_builder->decors[curr_builder->num_decorations].str_size = count;
    curr_builder->num_decorations++;

    uint32_t orig_indx = res;
    {
        uint32_t arr[] = { res };
        res = ShaderBuilderAddVariableF(SHADER_VARIABLE_TYPE_POINTER, SHADER_DATA_FLAG_UNIFORM, arr, 1, NULL, 0);
    }

    {
        uint32_t arr[] = { res };
        res = ShaderBuilderAddVariableF(SHADER_VARIABLE_TYPE_VARIABLE, SHADER_DATA_FLAG_UNIFORM, arr, 1, NULL, 0);
    }


    curr_builder->ioData[curr_builder->num_io_data].type = SHADER_VARIABLE_TYPE_UNIFORM;
    curr_builder->ioData[curr_builder->num_io_data].indx = res;
    curr_builder->ioData[curr_builder->num_io_data].flags = SHADER_DATA_FLAG_UNIFORM;
    curr_builder->ioData[curr_builder->num_io_data].orig_indx = orig_indx;
    curr_builder->ioData[curr_builder->num_io_data].location = location;
    curr_builder->ioData[curr_builder->num_io_data].binding = binding;
    curr_builder->num_io_data ++;

    curr_builder->decors[curr_builder->num_decorations].indx = res;
    curr_builder->num_decorations++;

    return res;
}

uint32_t ShaderBuilderCheckPointer(uint32_t type_indx, uint32_t flags){

    ShaderVariable *variable = NULL;

    ChildStack *child = curr_builder->alloc_head;

    int find = 0;
    while(child != NULL){
        variable = child->node;

        if(variable != NULL){
            if(variable->type == SHADER_VARIABLE_TYPE_POINTER && variable->args[0] == type_indx && (variable->flags & flags))
                return variable->indx;
        }

        child = child->next;
    }
    
    return 0;
}

uint32_t ShaderBuilderAddPointer(ShaderVariableType point_type, uint32_t size, ShaderDataFlags flags){

    uint32_t type_indx = ShaderBuilderGetType(point_type, size, NULL);

    uint32_t res = ShaderBuilderCheckPointer(type_indx, flags);

    if(!res){
        uint32_t arr[] = { type_indx };
        res = ShaderBuilderAddVariable(SHADER_VARIABLE_TYPE_POINTER, flags, arr, 1, NULL, 0);
    }

    return res;
}

uint32_t ShaderBuilderAddIOData(ShaderVariableType type, ShaderDataFlags flags, ShaderStructConstr *struct_arr, uint32_t size, char *name, uint32_t location, uint32_t binding){

    uint32_t res = 0;

    ShaderVariable *temp = NULL;

    switch(type){
        case SHADER_VARIABLE_TYPE_FLOAT:
            res = ShaderBuilderAddFloat();
            break;
        case SHADER_VARIABLE_TYPE_INT:
            res = ShaderBuilderAddInt(1);
            break;
        case SHADER_VARIABLE_TYPE_VECTOR:
            res = ShaderBuilderAddVector(size, NULL );
            ShaderBuilderAddDecor(res, SHADER_DECOR_TYPE_BUILTIN, (uint32_t []){binding}, 1);
            break;
        case SHADER_VARIABLE_TYPE_STRUCT:
            res = ShaderBuilderAddStruct(struct_arr, size, name);

            curr_builder->decors[curr_builder->num_decorations].indx = res;
            curr_builder->decors[curr_builder->num_decorations].str_point = struct_arr;
            curr_builder->decors[curr_builder->num_decorations].str_size = size;
            curr_builder->num_decorations++;
            break;
        case SHADER_VARIABLE_TYPE_IMAGE:
            res = ShaderBuilderAddImage();
            break;
    }

    if(type == SHADER_VARIABLE_TYPE_STRUCT){
        for(int i=0;i<size;i++){
            ShaderBuilderAddDecor(res, SHADER_DECOR_TYPE_BUILTIN, (uint32_t []){struct_arr[i].built_in}, 1);
        }
    }

    uint32_t orig_indx = res;

    {
        uint32_t arr[] = { res };
        res = ShaderBuilderAddVariableF(SHADER_VARIABLE_TYPE_POINTER, flags, arr, 1, NULL, 0);
    }

    {
        uint32_t arr[] = { res };
        res = ShaderBuilderAddVariableF(SHADER_VARIABLE_TYPE_VARIABLE, flags, arr, 1, NULL, 0);
    }


    if(type != SHADER_VARIABLE_TYPE_IMAGE){
        if(type != SHADER_VARIABLE_TYPE_STRUCT){
            uint32_t len = strlen(name);

            memcpy(curr_builder->infos[curr_builder->num_debug_infos].name, name, len);
            curr_builder->infos[curr_builder->num_debug_infos].indx = res;

            curr_builder->num_debug_infos++;
        }else{

            memset(curr_builder->infos[curr_builder->num_debug_infos].name, 0, 64);
            curr_builder->infos[curr_builder->num_debug_infos].indx = res;

            curr_builder->num_debug_infos++;
        }
    }

    ShaderDataFlags tflags = 0;

    curr_builder->ioData[curr_builder->num_io_data].type = type == SHADER_VARIABLE_TYPE_IMAGE ? SHADER_VARIABLE_TYPE_UNIFORM_CONSTANT : type;
    curr_builder->ioData[curr_builder->num_io_data].indx = res;
    curr_builder->ioData[curr_builder->num_io_data].orig_indx = orig_indx;
    curr_builder->ioData[curr_builder->num_io_data].location = location;
    curr_builder->ioData[curr_builder->num_io_data].binding = binding;
    curr_builder->ioData[curr_builder->num_io_data].flags =  flags & SHADER_DATA_FLAG_SYSTEM ? SHADER_DATA_FLAG_SYSTEM : 0;

    if(type == SHADER_VARIABLE_TYPE_IMAGE){
        curr_builder->ioData[curr_builder->num_io_data].descr_set = curr_builder->curr_descr_set;
        curr_builder->curr_descr_set ++;
    }
    curr_builder->num_io_data ++;

    return res;

}

InputOutputData *ShaderBuilderFindIOData(uint32_t indx){

    InputOutputData *res = NULL;

    for(int i=0;i < curr_builder->num_io_data;i++){
        if(curr_builder->ioData[i].indx == indx)
        {
            res = &curr_builder->ioData[i];
            break;
        }
    }

    return res;

}

void ShaderBuilderAddOperandF(uint32_t *indexes, uint32_t count, ShaderOperandType operand){

    ShaderOperand *oper = &curr_label->operands[curr_label->num_operands];

    memcpy(oper->var_indx, indexes, sizeof(uint32_t) * count);

    oper->num_vars = count;
    oper->op_type = operand;
    oper->indx = curr_builder->current_index + 1;

    curr_label->num_operands++;
}

uint32_t ShaderBuilderAddOperand(uint32_t *indexes, uint32_t count, ShaderOperandType operand){

    ShaderOperand *oper = &curr_label->operands[curr_label->num_operands];

    memcpy(oper->var_indx, indexes, sizeof(uint32_t) * count);

    oper->num_vars = count;
    oper->op_type = operand;
    oper->indx = curr_builder->current_index + 1;

    curr_label->num_operands++;
    curr_builder->current_index++;

    return curr_builder->current_index;
}

int ShaderBuilderFindOperand( ShaderOperandType op_type){

    for(int i=0;i < curr_label->num_operands;i++){
        if(curr_label->operands[i].op_type == op_type){
            return i;
        }
    }

    return -1;
}

ShaderOperand *ShaderBuilderFindOperandByIndex(uint32_t index){

    for(int i=0;i < curr_builder->main_point_index->labels[0].num_operands;i++){
        if(curr_builder->main_point_index->labels[0].operands[i].indx == index){
            return &curr_builder->main_point_index->labels[0].operands[i];
        }
    }

    return NULL;
}

void ShaderBuilderStoreValue(uint32_t *arr, uint32_t size){

    ShaderOperand *oper = &curr_label->operands[curr_label->num_operands];

    memcpy(oper->var_indx, arr, size * sizeof(uint32_t));

    oper->num_vars = size;
    oper->op_type = SHADER_OPERAND_TYPE_STORE;

    curr_label->num_operands++;

}

uint32_t ShaderBuilderAcceptAccess(uint32_t val_indx, ShaderVariableType var_type, uint32_t type_arg, uint32_t *chain, uint32_t size, int with_load){

    uint32_t arr[size + 2];

    InputOutputData *data = ShaderBuilderFindIOData(val_indx);
    
    if(data != NULL){
        if(data->flags & SHADER_DATA_FLAG_UNIFORM)
            arr[0] = ShaderBuilderAddPointer(var_type, type_arg, SHADER_DATA_FLAG_UNIFORM);
        else if(data->flags & SHADER_DATA_FLAG_SYSTEM){
            arr[0] = ShaderBuilderAddPointer(var_type, type_arg, SHADER_DATA_FLAG_OUTPUT);
        }else{        
            arr[0] = ShaderBuilderAddPointer(var_type, type_arg, 0);
        }
    }else{
        arr[0] = ShaderBuilderGetType(var_type, type_arg, NULL);
    }

    uint32_t load_type = ShaderBuilderGetType(var_type, type_arg, NULL);;
    
    arr[1] = val_indx;

    for(int i=0;i < size;i++){
        if(chain[i] > 10)
            arr[i + 2] = ShaderBuilderAcceptLoad(chain[i], 0);
        else
            arr[i + 2] = ShaderBuilderAddConstant(SHADER_VARIABLE_TYPE_INT, 0, chain[i], 1);
    }


    uint32_t res = ShaderBuilderAddOperand(arr, size + 2,  SHADER_OPERAND_TYPE_ACCESS);

    if(with_load)
        res = ShaderBuilderAddOperand((uint32_t []){load_type, res}, 2, SHADER_OPERAND_TYPE_LOAD);

    return res;
}

ShaderVariable *ShaderBuilderCheckType(uint32_t val, uint32_t indx){
    ShaderVariable *variable = ShaderBuilderFindVar(val);
    
    InputOutputData *data = ShaderBuilderFindIOData(val);
    if(data != NULL){
        ShaderVariable *var = ShaderBuilderFindVar(data->orig_indx);

        
        if(var->type == SHADER_VARIABLE_TYPE_STRUCT){
            ShaderVariable *res_var = ShaderBuilderFindVar(var->args[indx]);

            return res_var;
        }else{
            return var;
        }

    }else{
        return variable;
    }
}

uint32_t ShaderBuilderAcceptLoad(uint32_t val_indx, uint32_t struct_indx){
    
    uint32_t res = 0;
    
    ShaderVariable *variable = ShaderBuilderFindVar(val_indx);

    if(variable != NULL)
        if(variable->type == SHADER_VARIABLE_TYPE_CONSTANT)
            return val_indx;
    
    InputOutputData *data = ShaderBuilderFindIOData(val_indx);
    if(data != NULL){

        ShaderVariable *var = ShaderBuilderFindVar(data->orig_indx);


        if(var->type == SHADER_VARIABLE_TYPE_STRUCT){
            res = ShaderBuilderAddVariable(SHADER_VARIABLE_TYPE_POINTER, SHADER_DATA_FLAG_UNIFORM, &var->args[struct_indx], 1, 0, 0);

            uint32_t cnst = ShaderBuilderAddConstant(SHADER_VARIABLE_TYPE_INT, 0, struct_indx, 1);

            uint32_t arr[] = {res, data->indx, cnst};
            res = ShaderBuilderAddOperand(arr, 3, SHADER_OPERAND_TYPE_ACCESS);

            uint32_t arr2[] = {var->args[struct_indx], res};
            res = ShaderBuilderAddOperand(arr2, 2, SHADER_OPERAND_TYPE_LOAD);


        }else{
            uint32_t arr[] = {data->orig_indx, val_indx};
            res = ShaderBuilderAddOperand(arr, 2, SHADER_OPERAND_TYPE_LOAD);
        }
        
    }else{

        if(variable == NULL){
            ShaderOperand *op = ShaderBuilderFindOperandByIndex(val_indx);
          
            if(op == NULL)
                return val_indx;

            if(op->op_type != SHADER_OPERAND_TYPE_VARIABLE)
                return val_indx;

            ShaderVariable *point = ShaderBuilderFindVar(op->var_indx[0]);

            ShaderVariable *type = ShaderBuilderFindVar(point->args[0]);
            
            uint32_t arr[] = {type->indx, val_indx};
            res = ShaderBuilderAddOperand(arr, 2, SHADER_OPERAND_TYPE_LOAD);

            return res;
        }

        uint32_t res_type = 0;

        switch (variable->type)
        {
            case  SHADER_VARIABLE_TYPE_VECTOR:
                res_type = ShaderBuilderAddVector(variable->values[0], NULL );
                break;
            
            default:
                break;
        }
        
        uint32_t arr[] = {res_type, val_indx};
        res = ShaderBuilderAddOperand(arr, 2, SHADER_OPERAND_TYPE_LOAD);

    }

    return res;
}


uint32_t ShaderBuilderAddFuncMove(uint32_t src_indx, uint32_t src_size, uint32_t dest_indx, uint32_t dest_size){

    uint32_t res = ShaderBuilderAcceptLoad(src_indx, 0);

    if(src_size != dest_size)
    {       
        uint32_t res_type = ShaderBuilderAddVector(4, NULL);

        uint32_t arr_extract[src_size]; 

        uint32_t float_point = ShaderBuilderAddFloat();

        for(int i=0;i < src_size;i++){
            uint32_t arr[] = {float_point, res,  i};
            arr_extract[i] = ShaderBuilderAddOperand(arr, sizeof(arr), SHADER_OPERAND_TYPE_COMPOSITE_EXTRACT);
        }
        
        float v_f = 1.0f;
        uint32_t v_u = 0;
        memcpy(&v_u, &v_f, sizeof(uint32_t));        
        uint32_t cnst2 = ShaderBuilderAddConstant(SHADER_VARIABLE_TYPE_FLOAT, 0, v_u, 1);
        uint32_t arr[] = {res_type, arr_extract[0], arr_extract[1], arr_extract[2], cnst2};
        res = ShaderBuilderAddOperand(arr, 5, SHADER_OPERAND_TYPE_COMPOSITE_CONSTRUCT);
    }

    uint32_t arr[] = {dest_indx, res};
    
    ShaderBuilderStoreValue(arr, sizeof(arr));

    return res;
}

uint32_t ShaderBuilderGetTexture(uint32_t texture_indx, uint32_t uv_indx){

    uint32_t res = ShaderBuilderAcceptLoad(texture_indx, 0);
    
    uint32_t res2 = ShaderBuilderAcceptLoad(uv_indx, 0);

    uint32_t res_type = ShaderBuilderAddVector(4, NULL);

    uint32_t arr[] = {res_type, res, res2};

    res = ShaderBuilderAddOperand(arr, sizeof(arr), SHADER_OPERAND_TYPE_IMAGE_SAMLE_IMPLICIT_LOD);

    return res;
}

VectorExtract ShaderBuilderGetElemenets(uint32_t src_type, uint32_t src_index, uint32_t src_size, uint32_t start_indx, uint32_t size){

    uint32_t res = 0;   

    VectorExtract extract;
    memset(&extract, 0, sizeof(VectorExtract)); 

    uint32_t var_type = ShaderBuilderGetType(src_type, src_size, NULL);  

    for(int i=0;i < size;i++){
        uint32_t arr[] = {var_type, src_index,  i + start_indx};
        extract.elems[i] = ShaderBuilderAddOperand(arr, 3, SHADER_OPERAND_TYPE_COMPOSITE_EXTRACT);
    }   

    extract.size = size;

    return extract;
}

uint32_t ShaderBuilderConvertFToS(uint32_t val_indx){

    uint32_t int_type = ShaderBuilderAddInt(1);

    return ShaderBuilderAddOperand((uint32_t []){int_type, val_indx}, 2, SHADER_OPERAND_TYPE_CONVERT_FTOS);
}

uint32_t ShaderBuilderMakeVectorConstruct(float *vals, uint32_t num_vals){
    uint32_t arg[num_vals + 1];

    uint32_t vec_type = ShaderBuilderAddVector(num_vals, NULL);
    
    uint32_t temp = 0;

    arg[0] = vec_type;
    for(int i=0;i < num_vals;i++){
        memcpy(&temp, &vals[i], sizeof(uint32_t));
        arg[i + 1] = ShaderBuilderAddConstant(SHADER_VARIABLE_TYPE_FLOAT, 0, temp, 1);
    }
    
    uint32_t res = ShaderBuilderAddOperand(arg, num_vals + 1, SHADER_OPERAND_TYPE_COMPOSITE_CONSTRUCT);

    return res;
}

uint32_t ShaderBuilderMutateVector(uint32_t val_indx, uint32_t val_size, uint32_t res_size){

    uint32_t acc = ShaderBuilderAcceptLoad(val_indx, 0);

    uint32_t razn = res_size - val_size;

    uint32_t res = 0;
    
    if(razn){
        float v_f = 1.0f;
        uint32_t v_u = 0;
        memcpy(&v_u, &v_f, sizeof(uint32_t));        
        uint32_t cnst = ShaderBuilderAddConstant(SHADER_VARIABLE_TYPE_FLOAT, 0, v_u, 1);

        VectorExtract extr = ShaderBuilderGetElemenets(SHADER_VARIABLE_TYPE_FLOAT, acc, val_size, 0,  val_size);

        if(res_size == 4){

            uint32_t cnst2 = ShaderBuilderAddConstant(SHADER_VARIABLE_TYPE_FLOAT, 0, 0, 1);

            if(razn == 1){

                uint32_t res_vec_type = ShaderBuilderAddVector(res_size, NULL);

                uint32_t arr[] = { res_vec_type, extr.elems[0], extr.elems[1], extr.elems[2], cnst};
                res = ShaderBuilderCompositeConstruct(arr, 5);
            }else if(razn == 2){

                uint32_t res_vec_type = ShaderBuilderAddVector(4, NULL);

                uint32_t arr[] = { res_vec_type, extr.elems[0], extr.elems[1], cnst2, cnst};
                res = ShaderBuilderCompositeConstruct(arr, 5);
            }
        }else if(res_size == 3){
            if(razn == 1){

                uint32_t res_vec_type = ShaderBuilderAddVector(res_size, NULL);

                uint32_t arr[] = { res_vec_type, extr.elems[0], extr.elems[1], cnst};
                res = ShaderBuilderCompositeConstruct(arr, 4);
            }
        }
    }else{
        res = acc;
    }

    return res;

}

uint32_t ShaderBuilderCompositeConstruct(uint32_t *arr_arg, uint32_t size_arr){
        
    uint32_t res = ShaderBuilderAddOperand(arr_arg, size_arr, SHADER_OPERAND_TYPE_COMPOSITE_CONSTRUCT);

    return res;
}

uint32_t ShaderBuilderAddFuncDiv(uint32_t val_1, uint32_t indx_1, uint32_t type_1, uint32_t size_1,  uint32_t val_2, uint32_t indx_2, uint32_t type_2, uint32_t size_2, uint32_t res_size){
    uint32_t res = 0;

    uint32_t acc_1 = ShaderBuilderAcceptLoad(val_1, indx_1);
    uint32_t acc_2 = ShaderBuilderAcceptLoad(val_2, indx_2);

    if(type_1 == SHADER_VARIABLE_TYPE_MATRIX && type_2 == SHADER_VARIABLE_TYPE_MATRIX){
        uint32_t mat_type = ShaderBuilderAddMatrix(size_1, NULL);
        uint32_t vec_type = ShaderBuilderAddVector(size_2, NULL);

        uint32_t arr[] = { mat_type, acc_1, acc_2 };
        res = ShaderBuilderAddOperand(arr, 3, SHADER_OPERAND_TYPE_MATTIMEMAT);
    }else if(type_1 == SHADER_VARIABLE_TYPE_MATRIX && type_2 == SHADER_VARIABLE_TYPE_VECTOR){
        uint32_t mat_type = ShaderBuilderAddMatrix(size_1, NULL);
        uint32_t vec_type = ShaderBuilderAddVector(size_2, NULL);
        uint32_t res_vec_type = ShaderBuilderAddVector(res_size, NULL);

        float v_f = 1.0f;
        uint32_t v_u = 0;
        memcpy(&v_u, &v_f, sizeof(uint32_t));        
        uint32_t cnst = ShaderBuilderAddConstant(SHADER_VARIABLE_TYPE_FLOAT, 0, v_u, 1);
        uint32_t cnst2 = ShaderBuilderAddConstant(SHADER_VARIABLE_TYPE_FLOAT, 0, 0, 1);

        if(size_2 == 2){
                        
            VectorExtract extr = ShaderBuilderGetElemenets(SHADER_VARIABLE_TYPE_FLOAT, acc_2, size_2, 0, size_2);

            uint32_t arr[] = { res_vec_type, extr.elems[0], extr.elems[1], cnst2, cnst};
            res = ShaderBuilderCompositeConstruct(arr, 5);

        }else if(size_2 == 3){

            VectorExtract extr = ShaderBuilderGetElemenets(SHADER_VARIABLE_TYPE_FLOAT, acc_2, size_2, 0, size_2);

            uint32_t arr[] = { res_vec_type, extr.elems[0], extr.elems[1], extr.elems[2], cnst};
            res = ShaderBuilderCompositeConstruct(arr, 5);
        }else{
            uint32_t arr[] = { res_vec_type, acc_1, acc_2 };
            res = ShaderBuilderAddOperand(arr, 3, SHADER_OPERAND_TYPE_MATTIMEVEC);

            return res;
        }
        
        
        uint32_t arr[] = { res_vec_type, acc_1, res };
        res = ShaderBuilderAddOperand(arr, 3, SHADER_OPERAND_TYPE_MATTIMEVEC);

    }else if(type_1 == SHADER_VARIABLE_TYPE_VECTOR && type_2 == SHADER_VARIABLE_TYPE_VECTOR){

        uint32_t res_vec_type = ShaderBuilderAddVector(res_size, NULL);

        uint32_t arr[] = { res_vec_type, acc_1, acc_2 };
        res = ShaderBuilderAddOperand(arr, 3, SHADER_OPERAND_TYPE_DIV);
    }


    return res;
}

uint32_t ShaderBuilderAddFuncMult(uint32_t val_1, uint32_t indx_1, uint32_t type_1, uint32_t size_1,  uint32_t val_2, uint32_t indx_2, uint32_t type_2, uint32_t size_2, uint32_t res_size){

    uint32_t res = 0;

    uint32_t acc_1 = ShaderBuilderAcceptLoad(val_1, indx_1);
    uint32_t acc_2 = ShaderBuilderAcceptLoad(val_2, indx_2);

    if(type_1 == SHADER_VARIABLE_TYPE_MATRIX && type_2 == SHADER_VARIABLE_TYPE_MATRIX){
        uint32_t mat_type = ShaderBuilderAddMatrix(size_1, NULL);
        uint32_t vec_type = ShaderBuilderAddVector(size_2, NULL);

        uint32_t arr[] = { mat_type, acc_1, acc_2 };
        res = ShaderBuilderAddOperand(arr, 3, SHADER_OPERAND_TYPE_MATTIMEMAT);
    }else if(type_1 == SHADER_VARIABLE_TYPE_MATRIX && type_2 == SHADER_VARIABLE_TYPE_VECTOR){
        uint32_t mat_type = ShaderBuilderAddMatrix(size_1, NULL);
        uint32_t vec_type = ShaderBuilderAddVector(size_2, NULL);
        uint32_t res_vec_type = ShaderBuilderAddVector(res_size, NULL);

        float v_f = 1.0f;
        uint32_t v_u = 0;
        memcpy(&v_u, &v_f, sizeof(uint32_t));        
        uint32_t cnst = ShaderBuilderAddConstant(SHADER_VARIABLE_TYPE_FLOAT, 0, v_u, 1);
        uint32_t cnst2 = ShaderBuilderAddConstant(SHADER_VARIABLE_TYPE_FLOAT, 0, 0, 1);

        if(size_1 > size_2){

            if(size_2 == 2){
                            
                VectorExtract extr = ShaderBuilderGetElemenets(SHADER_VARIABLE_TYPE_FLOAT, acc_2, size_2, 0, size_2);

                uint32_t arr[] = { res_vec_type, extr.elems[0], extr.elems[1], cnst2, cnst};
                res = ShaderBuilderCompositeConstruct(arr, 5);

            }else if(size_2 == 3){

                VectorExtract extr = ShaderBuilderGetElemenets(SHADER_VARIABLE_TYPE_FLOAT, acc_2, size_2, 0, size_2);

                uint32_t arr[] = { res_vec_type, extr.elems[0], extr.elems[1], extr.elems[2], cnst};
                res = ShaderBuilderCompositeConstruct(arr, 5);
            }else{
                uint32_t arr[] = { res_vec_type, acc_1, acc_2 };
                res = ShaderBuilderAddOperand(arr, 3, SHADER_OPERAND_TYPE_MATTIMEVEC);

                return res;
            }
            
            
            uint32_t arr[] = { res_vec_type, acc_1, res };
            res = ShaderBuilderAddOperand(arr, 3, SHADER_OPERAND_TYPE_MATTIMEVEC);
        }else{
            uint32_t arr[] = { res_vec_type, acc_1, acc_2 };
            res = ShaderBuilderAddOperand(arr, 3, SHADER_OPERAND_TYPE_MATTIMEVEC);

            return res;
        }

    }else if((type_1 == SHADER_VARIABLE_TYPE_MATRIX && type_2 == SHADER_VARIABLE_TYPE_INT) || (type_1 == SHADER_VARIABLE_TYPE_MATRIX && type_2 == SHADER_VARIABLE_TYPE_FLOAT)){
        
        uint32_t mat_type = ShaderBuilderAddMatrix(size_1, NULL);

        uint32_t arr[] = { mat_type, acc_1, acc_2 };
        res = ShaderBuilderAddOperand(arr, 3, SHADER_OPERAND_TYPE_MATTIMESCAL);
    }else if(type_1 == SHADER_VARIABLE_TYPE_VECTOR && type_2 == SHADER_VARIABLE_TYPE_VECTOR){

        uint32_t res_vec_type = ShaderBuilderAddVector(res_size, NULL);

        float v_f = 1.0f;
        uint32_t v_u = 0;
        memcpy(&v_u, &v_f, sizeof(uint32_t));        
        uint32_t cnst = ShaderBuilderAddConstant(SHADER_VARIABLE_TYPE_FLOAT, 0, v_u, 1);
        uint32_t cnst2 = ShaderBuilderAddConstant(SHADER_VARIABLE_TYPE_FLOAT, 0, 0, 1);
        
        if(size_1 > size_2){

            if(size_2 == 2){                        
                VectorExtract extr = ShaderBuilderGetElemenets(SHADER_VARIABLE_TYPE_FLOAT, acc_2, size_2, 0, size_2);

                uint32_t arr[] = { res_vec_type, extr.elems[0], extr.elems[1], cnst2, cnst};
                res = ShaderBuilderCompositeConstruct(arr, 5);

            }
            if(size_2 == 3){
                VectorExtract extr = ShaderBuilderGetElemenets(SHADER_VARIABLE_TYPE_FLOAT, acc_2, size_2, 0, size_2);

                uint32_t arr[] = { res_vec_type, extr.elems[0], extr.elems[1], extr.elems[2], cnst};
                res = ShaderBuilderCompositeConstruct(arr, 5);
            }else{
                uint32_t arr[] = { res_vec_type, acc_1, acc_2 };
                res = ShaderBuilderAddOperand(arr, 3, SHADER_OPERAND_TYPE_MUL);

                return res;
            }

            uint32_t arr[] = { res_vec_type, acc_1, res };
            res = ShaderBuilderAddOperand(arr, 3, SHADER_OPERAND_TYPE_MUL);
        }else{
            uint32_t arr[] = { res_vec_type, acc_1, acc_2 };
            res = ShaderBuilderAddOperand(arr, 3, SHADER_OPERAND_TYPE_MUL);

            return res;
        }
    }else if(type_1 == SHADER_VARIABLE_TYPE_FLOAT && type_2 == SHADER_VARIABLE_TYPE_FLOAT){
        
        uint32_t res_vec_type = ShaderBuilderAddFloat();

        uint32_t arr[] = { res_vec_type, acc_1, acc_2 };
        res = ShaderBuilderAddOperand(arr, 3, SHADER_OPERAND_TYPE_MUL);
    }

    return res;
}

uint32_t ShaderBuilderAddFuncAdd(uint32_t val_1, uint32_t indx_1, uint32_t type_1, uint32_t size_1,  uint32_t val_2, uint32_t indx_2, uint32_t type_2, uint32_t size_2, uint32_t res_size){

    uint32_t res = 0;

    uint32_t acc_1 = ShaderBuilderAcceptLoad(val_1, indx_1);
    uint32_t acc_2 = ShaderBuilderAcceptLoad(val_2, indx_2);
    
    if(type_1 == SHADER_VARIABLE_TYPE_MATRIX && type_2 == SHADER_VARIABLE_TYPE_MATRIX){
        uint32_t vec_type = ShaderBuilderAddVector(4, NULL);
        uint32_t mat_type = ShaderBuilderAddMatrix(4, NULL);

        uint32_t arr[4];
        for(int i=0;i < res_size;i++){
            VectorExtract extr = ShaderBuilderGetElemenets(SHADER_VARIABLE_TYPE_VECTOR, acc_1, 4, i, 1);
            VectorExtract extr2 = ShaderBuilderGetElemenets(SHADER_VARIABLE_TYPE_VECTOR, acc_2, 4, i, 1);

            arr[i] = ShaderBuilderAddOperand((uint32_t []){vec_type, extr.elems[0], extr2.elems[0]}, 3, SHADER_OPERAND_TYPE_ADD);
        }

        res = ShaderBuilderCompositeConstruct((uint32_t []){ mat_type, arr[0], arr[1], arr[2], arr[3] }, 5);

    }if(type_1 == SHADER_VARIABLE_TYPE_VECTOR && type_2 == SHADER_VARIABLE_TYPE_VECTOR){

        uint32_t res_vec_type = ShaderBuilderAddVector(res_size, NULL);

        float v_f = 1.0f;
        uint32_t v_u = 0;
        memcpy(&v_u, &v_f, sizeof(uint32_t));        
        uint32_t cnst = ShaderBuilderAddConstant(SHADER_VARIABLE_TYPE_FLOAT, 0, v_u, 1);
        uint32_t cnst2 = ShaderBuilderAddConstant(SHADER_VARIABLE_TYPE_FLOAT, 0, 0, 1);
        
        if(size_1 > size_2){

            if(size_2 == 2){                        
                VectorExtract extr = ShaderBuilderGetElemenets(SHADER_VARIABLE_TYPE_FLOAT, acc_2, size_2, 0, size_2);

                uint32_t arr[] = { res_vec_type, extr.elems[0], extr.elems[1], cnst2, cnst};
                res = ShaderBuilderCompositeConstruct(arr, 5);

            }
            if(size_2 == 3){
                VectorExtract extr = ShaderBuilderGetElemenets(SHADER_VARIABLE_TYPE_FLOAT, acc_2, size_2, 0, size_2);

                uint32_t arr[] = { res_vec_type, extr.elems[0], extr.elems[1], extr.elems[2], cnst};
                res = ShaderBuilderCompositeConstruct(arr, 5);
            }else{
                uint32_t arr[] = { res_vec_type, acc_1, acc_2 };
                res = ShaderBuilderAddOperand(arr, 3, SHADER_OPERAND_TYPE_MUL);

                return res;
            }

            uint32_t arr[] = { res_vec_type, acc_1, res };
            res = ShaderBuilderAddOperand(arr, 3, SHADER_OPERAND_TYPE_ADD);
        }else{
            uint32_t arr[] = { res_vec_type, acc_1, acc_2 };
            res = ShaderBuilderAddOperand(arr, 3, SHADER_OPERAND_TYPE_ADD);

            return res;
        }
    }else if(type_1 == SHADER_VARIABLE_TYPE_FLOAT && type_2 == SHADER_VARIABLE_TYPE_FLOAT){
        
        uint32_t res_vec_type = ShaderBuilderAddFloat();

        uint32_t arr[] = { res_vec_type, acc_1, acc_2 };
        res = ShaderBuilderAddOperand(arr, 3, SHADER_OPERAND_TYPE_ADD);
    }

    return res;
}

uint32_t ShaderBuilderAddFuncMultS(uint32_t val_indx, uint32_t single_indx, uint32_t size){

    uint32_t res = 0;

    uint32_t func_indx = ShaderBuilderAddPointer(SHADER_VARIABLE_TYPE_FLOAT, 0, SHADER_DATA_FLAG_FUNCTION);

    uint32_t vec3_indx = ShaderBuilderAddVector(3, NULL);

    uint32_t arr[] = {func_indx, 7 /*Function*/};
    uint32_t variable = ShaderBuilderAddOperand(arr, 2, SHADER_OPERAND_TYPE_VARIABLE);

    VectorExtract vector = ShaderBuilderGetElemenets(SHADER_VARIABLE_TYPE_FLOAT, single_indx, 4, 0, 1);

    uint32_t arr2[] = { variable, vector.elems[0] };
    ShaderBuilderStoreValue(arr2, 2);

    uint32_t var_type = ShaderBuilderGetType(SHADER_VARIABLE_TYPE_FLOAT, 0, NULL);

    uint32_t val1 = ShaderBuilderAcceptLoad(vec3_indx, val_indx);
    uint32_t val2 = ShaderBuilderAcceptLoad(var_type, variable);

    uint32_t arr3[] = { vec3_indx, val1, val2};
    res = ShaderBuilderAddOperand(arr3, 3, SHADER_OPERAND_TYPE_VECTIMES);
    
    return res;
}

ShaderLabel *ShaderBuilderAddLabel(uint32_t func_indx, int will_return){

    uint32_t num_labels = curr_builder->functions[func_indx].num_labels;

    curr_builder->functions[func_indx].labels[num_labels].index =  curr_builder->current_index + 1;
    curr_builder->functions[func_indx].labels[num_labels].will_return = will_return;
    curr_builder->functions[func_indx].num_labels++;
    curr_builder->current_index++;

    return &curr_builder->functions[func_indx].labels[num_labels];
}

uint32_t ShaderBuilderMakeTransition(uint32_t indx_label){

    ShaderOperand *oper = NULL;
    
    oper = &curr_label->operands[curr_label->num_operands];

    oper->num_vars = 1;
    oper->op_type = SHADER_OPERAND_TYPE_BRANCH;
    oper->var_indx[0] = indx_label;

    curr_label->num_operands++;

    ShaderBuilderSetCurrentLabel(indx_label);
}

uint32_t ShaderBuilderNextLabel(int will_return){
    ShaderLabel *label = ShaderBuilderAddLabel(0, will_return);

    return label->index;
}

void ShaderBuilderMakeKill(){
    ShaderBuilderAddOperandF(NULL, 0, SHADER_OPERAND_TYPE_KILL);
}

void ShaderBuilderMakeBranchConditional(ConditionalType cond_type, uint32_t *vals, uint32_t size, uint32_t sel_merge, uint32_t true_label, uint32_t false_label){
    uint32_t type_bool = ShaderBuilderAddVariable(SHADER_VARIABLE_TYPE_BOOL, 0, NULL, 0, NULL, 0);

    uint32_t res = 0;
    
    switch(cond_type){
        case SHADER_CONDITIONAL_TYPE_SLESS_THAN:
            res = ShaderBuilderAddOperand((uint32_t []){type_bool, vals[0], vals[1]}, 3, SHADER_OPERAND_TYPE_SLESSTHAN);
            break;
        case SHADER_CONDITIONAL_TYPE_FLESS_THAN:
            res = ShaderBuilderAddOperand((uint32_t []){type_bool, vals[0], vals[1]}, 3, SHADER_OPERAND_TYPE_FLESSTHAN);
            break;
        case SHADER_CONDITIONAL_TYPE_SGREAT_THAN:
            res = ShaderBuilderAddOperand((uint32_t []){type_bool, vals[0], vals[1]}, 3, SHADER_OPERAND_TYPE_SGREATTHAN);
            break;
        case SHADER_CONDITIONAL_TYPE_FGREAT_THAN:
            res = ShaderBuilderAddOperand((uint32_t []){type_bool, vals[0], vals[1]}, 3, SHADER_OPERAND_TYPE_FGREATTHAN);
            break;
    }
    
    if(sel_merge)
        ShaderBuilderAddOperandF((uint32_t []){sel_merge, 0}, 2, SHADER_OPERAND_TYPE_SELECTION_MERGE);

    ShaderBuilderAddOperandF((uint32_t []){res, true_label, false_label}, 3, SHADER_OPERAND_TYPE_BRANCH_CONDITIONAL);
}

ShaderFunc *ShaderBuilderAddFunction(ShaderVariableType output, char *name){

    uint32_t void_type, func_type;

    int res = ShaderBuilderCheckVariable(SHADER_VARIABLE_TYPE_FUNCTION, NULL, 0);

    if(!res){

        res = ShaderBuilderCheckVariable(output,  NULL, 0);

        if(!res)
            res = ShaderBuilderAddVariable(output, 0, NULL, 0, NULL, 0);

        void_type = res;

        uint32_t arr[] = {res};

        res = ShaderBuilderAddVariable(SHADER_VARIABLE_TYPE_FUNCTION, 0, arr, 1, NULL, 0);
    }

    func_type = res;

    curr_builder->functions[curr_builder->num_functions].result_type_indx = void_type;
    curr_builder->functions[curr_builder->num_functions].func_type_indx = func_type;
    curr_builder->functions[curr_builder->num_functions].indx = curr_builder->current_index + 1;

    uint32_t len = strlen(name);

    memcpy(curr_builder->infos[curr_builder->num_debug_infos].name, name, len);
    curr_builder->infos[curr_builder->num_debug_infos].indx = curr_builder->current_index + 1;

    curr_builder->num_debug_infos++;
    curr_builder->num_functions++;
    curr_builder->current_index++;

    ShaderBuilderAddLabel(curr_builder->num_functions - 1, false);

    return &curr_builder->functions[curr_builder->num_functions - 1];

}

void ShaderBuilderInit(ShaderBuilder *builder, ShaderType type){

    memset(builder, 0, sizeof(ShaderBuilder));

    ShaderBuilderSetCurrent(builder);

    curr_builder->alloc_head = calloc(1, sizeof(ChildStack));

    ShaderBuilderAddValue(SpvMagicNumber);// Magic number
    ShaderBuilderAddValue(0x00010000);//SpvVersion);
    ShaderBuilderAddValue(0x000D000B);//Generator version
    ShaderBuilderAddValue(0);//Lines code
    ShaderBuilderAddValue(0);

    ShaderBuilderAddOp(SpvOpCapability, 2);
    ShaderBuilderAddValue(SpvCapabilityShader);

    curr_builder->type = type;

    ShaderBuilderAddVariable(SHADER_VARIABLE_TYPE_EXTENDED_IMPORT, 0, NULL, 0, NULL, 0);

    curr_builder->main_point_index = ShaderBuilderAddFunction(SHADER_VARIABLE_TYPE_VOID, "main");

    ShaderBuilderSetCurrentLabel(curr_builder->main_point_index->labels[0].index);

    if(type == SHADER_TYPE_VERTEX){

        ShaderStructConstr float_str[] = {
            {SHADER_VARIABLE_TYPE_FLOAT, 0, 0, NULL, NULL, 0, NULL}
        };

        ShaderStructConstr struct_arr[] = {
            {SHADER_VARIABLE_TYPE_VECTOR, 4, 0, "gl_Position", NULL, 0, NULL},
            {SHADER_VARIABLE_TYPE_FLOAT, 32, 1, "gl_PointSize", NULL, 0, NULL},
            {SHADER_VARIABLE_TYPE_ARRAY, 1,  3, "gl_ClipDistance", &float_str, 1, NULL},
            {SHADER_VARIABLE_TYPE_ARRAY, 1,  4, "gl_CullDistance", &float_str, 1, NULL}
        };

        curr_builder->gl_struct_indx = ShaderBuilderAddIOData(SHADER_VARIABLE_TYPE_STRUCT, SHADER_DATA_FLAG_OUTPUT | SHADER_DATA_FLAG_SYSTEM, struct_arr, 4, "gl_PerVertex", 0, 0);
    }

}


void ShaderBuilderWriteFuncType(ShaderVariable *variable){

    ShaderBuilderAddOp(SpvOpTypeFunction, 3);

    ShaderBuilderAddValue(variable->indx);

    for(int i=0;i < variable->num_args;i++)
        ShaderBuilderAddValue(variable->args[i]);

}

uint32_t ReturnSizeStruct(ShaderVariable *str_elm);

uint32_t DecoratingUniforStruct(ShaderVariable *str_elm, uint32_t *offset){

    for(int j=0;j < str_elm->num_args;j++){
        ShaderVariable *var_elm = ShaderBuilderFindVar(str_elm->args[j]);

        if(var_elm->type == SHADER_VARIABLE_TYPE_STRUCT){
            
            ShaderBuilderAddOp(SpvOpMemberDecorate, 5);
            ShaderBuilderAddValue(str_elm->indx );
            ShaderBuilderAddValue(j );
            ShaderBuilderAddValue(SpvDecorationOffset );
            ShaderBuilderAddValue(*offset);

            *offset += DecoratingUniforStruct(var_elm, offset);
        }else if(var_elm->type == SHADER_VARIABLE_TYPE_ARRAY){
            ShaderVariable *arr_elm = ShaderBuilderFindVar(var_elm->args[0]);
            ShaderVariable *count_elm = ShaderBuilderFindVar(var_elm->args[1]);

            if(arr_elm->type == SHADER_VARIABLE_TYPE_STRUCT){
                uint32_t t_off = 0;
                DecoratingUniforStruct(arr_elm, &t_off);
            }
            
            uint32_t size = ReturnSizeStruct(var_elm);
            ShaderBuilderAddOp(SpvOpDecorate, 4);
            ShaderBuilderAddValue(var_elm->indx );
            ShaderBuilderAddValue(SpvDecorationArrayStride );
            ShaderBuilderAddValue(size);

            if(arr_elm->type == SHADER_VARIABLE_TYPE_MATRIX){
                ShaderBuilderAddOp(SpvOpMemberDecorate, 4);
                ShaderBuilderAddValue(str_elm->indx );
                ShaderBuilderAddValue(j );
                ShaderBuilderAddValue(SpvDecorationColMajor );
            }

            ShaderBuilderAddOp(SpvOpMemberDecorate, 5);
            ShaderBuilderAddValue(str_elm->indx );
            ShaderBuilderAddValue(j );
            ShaderBuilderAddValue(SpvDecorationOffset );
            ShaderBuilderAddValue(*offset );

            
            if(arr_elm->type == SHADER_VARIABLE_TYPE_MATRIX){
                ShaderBuilderAddOp(SpvOpMemberDecorate, 5);
                ShaderBuilderAddValue(str_elm->indx );
                ShaderBuilderAddValue(j );
                ShaderBuilderAddValue(SpvDecorationMatrixStride );
                ShaderBuilderAddValue(16);
            }

            *offset += size * count_elm->values[0] /*count*/;

        }else if(var_elm->type == SHADER_VARIABLE_TYPE_MATRIX){
            
            ShaderBuilderAddOp(SpvOpMemberDecorate, 4);
            ShaderBuilderAddValue(str_elm->indx );
            ShaderBuilderAddValue(j );
            ShaderBuilderAddValue(SpvDecorationColMajor );

            ShaderBuilderAddOp(SpvOpMemberDecorate, 5);
            ShaderBuilderAddValue(str_elm->indx );
            ShaderBuilderAddValue(j );
            ShaderBuilderAddValue(SpvDecorationOffset );
            ShaderBuilderAddValue(*offset );
            
            ShaderBuilderAddOp(SpvOpMemberDecorate, 5);
            ShaderBuilderAddValue(str_elm->indx );
            ShaderBuilderAddValue(j );
            ShaderBuilderAddValue(SpvDecorationMatrixStride );
            ShaderBuilderAddValue(16);

            *offset +=64;
        }else if(var_elm->type == SHADER_VARIABLE_TYPE_VECTOR){
            
            ShaderBuilderAddOp(SpvOpMemberDecorate, 5);
            ShaderBuilderAddValue(str_elm->indx );
            ShaderBuilderAddValue(j );
            ShaderBuilderAddValue(SpvDecorationOffset );
            ShaderBuilderAddValue(*offset);

            *offset += 4 * var_elm->values[0]/*size*/;
        }else if(var_elm->type == SHADER_VARIABLE_TYPE_INT || var_elm->type == SHADER_VARIABLE_TYPE_FLOAT){
            
            ShaderBuilderAddOp(SpvOpMemberDecorate, 5);
            ShaderBuilderAddValue(str_elm->indx );
            ShaderBuilderAddValue(j );
            ShaderBuilderAddValue(SpvDecorationOffset );
            ShaderBuilderAddValue(*offset);

            *offset += 4;
        }
    }
}

void ShaderBuilderCheckThatLabel(ShaderLabel *label){

    ShaderBuilderAddOp(SpvOpLabel, 2);
    ShaderBuilderAddValue(label->index);
    
    ShaderOperand *operand;

    uint32_t iter = 0;
    for(int l=0;l < label->num_operands;l++){
        operand = &label->operands[l];  

        switch(operand->op_type){                         
            case SHADER_OPERAND_TYPE_VARIABLE:
                ShaderBuilderAddOp(SpvOpVariable, 4);
                ShaderBuilderAddValue(operand->var_indx[0]);
                ShaderBuilderAddValue(operand->indx);
                ShaderBuilderAddValue(operand->var_indx[1]);
                break;
        }
    }

    for(int l=0;l < label->num_operands;l++){
        operand = &label->operands[l];

            switch(operand->op_type){
                case SHADER_OPERAND_TYPE_ACCESS:
                    ShaderBuilderAddOp(SpvOpAccessChain, 4 + operand->num_vars - 2);
                    ShaderBuilderAddValue(operand->var_indx[0]);
                    ShaderBuilderAddValue(operand->indx);
                    ShaderBuilderAddValue(operand->var_indx[1]);
                    for(iter=2;iter<operand->num_vars;iter++)
                        ShaderBuilderAddValue(operand->var_indx[iter]);
                    break;
                case SHADER_OPERAND_TYPE_COMPOSITE_EXTRACT:
                    ShaderBuilderAddOp(SpvOpCompositeExtract, 4 + 1);
                    ShaderBuilderAddValue(operand->var_indx[0]);
                    ShaderBuilderAddValue(operand->indx);
                    ShaderBuilderAddValue(operand->var_indx[1]);
                    ShaderBuilderAddValue(operand->var_indx[2]);
                    break;
                case SHADER_OPERAND_TYPE_COMPOSITE_CONSTRUCT:
                    ShaderBuilderAddOp(SpvOpCompositeConstruct, 3 + operand->num_vars - 1);
                    ShaderBuilderAddValue(operand->var_indx[0]);
                    ShaderBuilderAddValue(operand->indx);
                    for(iter = 1;iter < operand->num_vars;iter++)
                        ShaderBuilderAddValue(operand->var_indx[iter]);
                    break;
                case SHADER_OPERAND_TYPE_IMAGE_SAMLE_IMPLICIT_LOD:
                    ShaderBuilderAddOp(SpvOpImageSampleImplicitLod, 5);
                    ShaderBuilderAddValue(operand->var_indx[0]);
                    ShaderBuilderAddValue(operand->indx);
                    ShaderBuilderAddValue(operand->var_indx[1]);
                    ShaderBuilderAddValue(operand->var_indx[2]);
                    break;
                case SHADER_OPERAND_TYPE_LOAD:
                    ShaderBuilderAddOp(SpvOpLoad, 4);
                    ShaderBuilderAddValue(operand->var_indx[0]);
                    ShaderBuilderAddValue(operand->indx);
                    ShaderBuilderAddValue(operand->var_indx[1]);
                    break;
                case SHADER_OPERAND_TYPE_STORE:
                    ShaderBuilderAddOp(SpvOpStore, 3);
                    ShaderBuilderAddValue(operand->var_indx[0]);
                    ShaderBuilderAddValue(operand->var_indx[1]);
                    break;
                case SHADER_OPERAND_TYPE_BRANCH:
                    ShaderBuilderAddOp(SpvOpBranch, 2);
                    ShaderBuilderAddValue(operand->var_indx[0]);
                    break;
                case SHADER_OPERAND_TYPE_BRANCH_CONDITIONAL:
                    ShaderBuilderAddOp(SpvOpBranchConditional, 4);
                    ShaderBuilderAddValue(operand->var_indx[0]);
                    ShaderBuilderAddValue(operand->var_indx[1]);
                    ShaderBuilderAddValue(operand->var_indx[2]);
                    break;  
                case SHADER_OPERAND_TYPE_SELECTION_MERGE:
                    ShaderBuilderAddOp(SpvOpSelectionMerge, 3);
                    ShaderBuilderAddValue(operand->var_indx[0]);
                    ShaderBuilderAddValue(operand->var_indx[1]);
                    break;  
                case SHADER_OPERAND_TYPE_LOOP:
                    ShaderBuilderAddOp(SpvOpLoopMerge, 4);
                    ShaderBuilderAddValue(operand->var_indx[0]);
                    ShaderBuilderAddValue(operand->var_indx[1]);
                    ShaderBuilderAddValue(0);
                    break;
                case SHADER_OPERAND_TYPE_ADD:
                    ShaderBuilderAddOp(SpvOpFAdd, 5);
                    ShaderBuilderAddValue(operand->var_indx[0]);
                    ShaderBuilderAddValue(operand->indx);
                    ShaderBuilderAddValue(operand->var_indx[1]);
                    ShaderBuilderAddValue(operand->var_indx[2]);
                    break;
                case SHADER_OPERAND_TYPE_MUL:
                    ShaderBuilderAddOp(SpvOpFMul, 5);
                    ShaderBuilderAddValue(operand->var_indx[0]);
                    ShaderBuilderAddValue(operand->indx);
                    ShaderBuilderAddValue(operand->var_indx[1]);
                    ShaderBuilderAddValue(operand->var_indx[2]);
                    break; 
                case SHADER_OPERAND_TYPE_DIV:
                    ShaderBuilderAddOp(SpvOpFDiv, 5);
                    ShaderBuilderAddValue(operand->var_indx[0]);
                    ShaderBuilderAddValue(operand->indx);
                    ShaderBuilderAddValue(operand->var_indx[1]);
                    ShaderBuilderAddValue(operand->var_indx[2]);
                    break; 
                case SHADER_OPERAND_TYPE_MATTIMEVEC:
                    ShaderBuilderAddOp(SpvOpMatrixTimesVector, 5);
                    ShaderBuilderAddValue(operand->var_indx[0]);
                    ShaderBuilderAddValue(operand->indx);
                    ShaderBuilderAddValue(operand->var_indx[1]);
                    ShaderBuilderAddValue(operand->var_indx[2]);
                    break; 
                case SHADER_OPERAND_TYPE_MATTIMESCAL:
                    ShaderBuilderAddOp(SpvOpMatrixTimesScalar, 5);
                    ShaderBuilderAddValue(operand->var_indx[0]);
                    ShaderBuilderAddValue(operand->indx);
                    ShaderBuilderAddValue(operand->var_indx[1]);
                    ShaderBuilderAddValue(operand->var_indx[2]);
                    break; 
                case SHADER_OPERAND_TYPE_MATTIMEMAT:
                    ShaderBuilderAddOp(SpvOpMatrixTimesMatrix, 5);
                    ShaderBuilderAddValue(operand->var_indx[0]);
                    ShaderBuilderAddValue(operand->indx);
                    ShaderBuilderAddValue(operand->var_indx[1]);
                    ShaderBuilderAddValue(operand->var_indx[2]);
                    break; 
                case SHADER_OPERAND_TYPE_VECTIMES:
                    ShaderBuilderAddOp(SpvOpVectorTimesScalar, 5);
                    ShaderBuilderAddValue(operand->var_indx[0]);
                    ShaderBuilderAddValue(operand->indx);
                    ShaderBuilderAddValue(operand->var_indx[1]);
                    ShaderBuilderAddValue(operand->var_indx[2]);
                    break;   
                case SHADER_OPERAND_TYPE_VECSHUFFLE:
                    ShaderBuilderAddOp(SpvOpVectorShuffle, 5 + operand->num_vars - 3);
                    ShaderBuilderAddValue(operand->var_indx[0]);
                    ShaderBuilderAddValue(operand->indx);
                    ShaderBuilderAddValue(operand->var_indx[1]);
                    ShaderBuilderAddValue(operand->var_indx[2]);
                    for(iter=3;iter<operand->num_vars;iter++)
                        ShaderBuilderAddValue(operand->var_indx[iter]);
                    break;  
                case SHADER_OPERAND_TYPE_SLESSTHAN:
                    ShaderBuilderAddOp(SpvOpSLessThan, 5);
                    ShaderBuilderAddValue(operand->var_indx[0]);
                    ShaderBuilderAddValue(operand->indx);
                    ShaderBuilderAddValue(operand->var_indx[1]);
                    ShaderBuilderAddValue(operand->var_indx[2]);
                    break;  
                case SHADER_OPERAND_TYPE_FLESSTHAN:
                    ShaderBuilderAddOp(SpvOpFOrdLessThan, 5);
                    ShaderBuilderAddValue(operand->var_indx[0]);
                    ShaderBuilderAddValue(operand->indx);
                    ShaderBuilderAddValue(operand->var_indx[1]);
                    ShaderBuilderAddValue(operand->var_indx[2]);
                    break;   
                case SHADER_OPERAND_TYPE_SGREATTHAN:
                    ShaderBuilderAddOp(SpvOpSGreaterThan, 5);
                    ShaderBuilderAddValue(operand->var_indx[0]);
                    ShaderBuilderAddValue(operand->indx);
                    ShaderBuilderAddValue(operand->var_indx[1]);
                    ShaderBuilderAddValue(operand->var_indx[2]);
                    break;  
                case SHADER_OPERAND_TYPE_FGREATTHAN:
                    ShaderBuilderAddOp(SpvOpFOrdGreaterThan, 5);
                    ShaderBuilderAddValue(operand->var_indx[0]);
                    ShaderBuilderAddValue(operand->indx);
                    ShaderBuilderAddValue(operand->var_indx[1]);
                    ShaderBuilderAddValue(operand->var_indx[2]);
                    break;   
                case SHADER_OPERAND_TYPE_CONVERT_FTOS:
                    ShaderBuilderAddOp(SpvOpConvertFToS, 4);
                    ShaderBuilderAddValue(operand->var_indx[0]);
                    ShaderBuilderAddValue(operand->indx);
                    ShaderBuilderAddValue(operand->var_indx[1]);
                    break; 
                case SHADER_OPERAND_TYPE_KILL:
                    ShaderBuilderAddOp(SpvOpKill, 1);
                    break;  
            }
    }
}

void ShaderBuilderMake(){

    if(curr_builder->main_point_index->num_labels == 1)
        curr_builder->main_point_index->labels[0].will_return = true;

    ShaderBuilderAddOp(SpvOpExtInstImport, 6);
    ShaderBuilderAddValue(((ShaderVariable *)curr_builder->alloc_head->node)->indx);
    ShaderBuilderAddString("GLSL.std.450");
    ShaderBuilderAddValue(0x0);

    ShaderBuilderAddOp(SpvOpMemoryModel, 3);
    ShaderBuilderAddValue(SpvAddressingModelLogical);
    ShaderBuilderAddValue(SpvMemoryModelGLSL450);

    uint32_t count = 0;

    for(int i=0;i < curr_builder->num_io_data;i++)
        if(curr_builder->ioData[i].type != SHADER_VARIABLE_TYPE_UNIFORM && curr_builder->ioData[i].type != SHADER_VARIABLE_TYPE_UNIFORM_CONSTANT)
            count++;

    {
        ShaderBuilderAddOp(SpvOpEntryPoint, 4 + count + 1);

        switch(curr_builder->type)
        {
            case SHADER_TYPE_FRAGMENT:
                ShaderBuilderAddValue(SpvExecutionModelFragment);
                break;
            case SHADER_TYPE_VERTEX:
                ShaderBuilderAddValue(SpvExecutionModelVertex);
                break;
            case SHADER_TYPE_COMPUTED:
                ShaderBuilderAddValue(SpvExecutionModelGLCompute);
                break;
            case SHADER_TYPE_GEOMETRY:
                ShaderBuilderAddValue(SpvExecutionModelGeometry);
                break;
            case SHADER_TYPE_TESELLATION_CONTROL:
                ShaderBuilderAddValue(SpvExecutionModelTessellationControl);
                break;
            case SHADER_TYPE_TESELLATION_EVALUATION:
                ShaderBuilderAddValue(SpvExecutionModelTessellationEvaluation);
                break;
        }

        ShaderBuilderAddValue(curr_builder->main_point_index->indx);

        ShaderBuilderAddString(curr_builder->infos[0].name);
        ShaderBuilderAddValue(0x0);

        for(int i=0;i < curr_builder->num_io_data;i++)
            if(curr_builder->ioData[i].type != SHADER_VARIABLE_TYPE_UNIFORM && curr_builder->ioData[i].type != SHADER_VARIABLE_TYPE_UNIFORM_CONSTANT)
                ShaderBuilderAddValue(curr_builder->ioData[i].indx);

    }

    if(curr_builder->type == SHADER_TYPE_FRAGMENT){
        ShaderBuilderAddOp(SpvOpExecutionMode, 3);
        ShaderBuilderAddValue(curr_builder->main_point_index->indx );
        ShaderBuilderAddValue(SpvExecutionModeOriginUpperLeft );
    }

    //Debug informations
    {
        ShaderBuilderAddOp(SpvOpSource, 3);
        ShaderBuilderAddValue(SpvSourceLanguageGLSL );
        ShaderBuilderAddValue(450 );

        ShaderBuilderAddOp(SpvOpSourceExtension, 10);
        ShaderBuilderAddString("GL_GOOGLE_cpp_style_line_directive");
        ShaderBuilderAddOp(SpvOpSourceExtension, 8);
        ShaderBuilderAddString("GL_GOOGLE_include_directive");

        uint32_t orig_len = 0, len =  0, temp = 0;
        ShaderDebugInfo *info;
        for(int i=0;i < curr_builder->num_debug_infos;i++){
            info = &curr_builder->infos[i];

            orig_len = strlen( info->name);

            temp = orig_len;
            while(temp % sizeof(uint32_t))
                temp ++;

            len = temp / sizeof(uint32_t);

            if(orig_len % sizeof(uint32_t))
                len --;

            ShaderBuilderAddOp(SpvOpName, 3 + len);
            ShaderBuilderAddValue(info->indx );
            ShaderBuilderAddString(info->name);

            if(!(orig_len % sizeof(uint32_t)))
                ShaderBuilderAddValue(0x0);

            for(int j=0;j < info->num_childs;j++){
                orig_len = strlen(info->child_name[j]);

                temp = orig_len;
                while(temp % sizeof(uint32_t))
                    temp ++;

                len = temp / sizeof(uint32_t);

                if(orig_len % sizeof(uint32_t))
                    len --;

                ShaderBuilderAddOp(SpvOpMemberName, 4 + len);
                ShaderBuilderAddValue(info->indx );
                ShaderBuilderAddValue(j);
                ShaderBuilderAddString(info->child_name[j]);

                if(!(orig_len % sizeof(uint32_t)))
                    ShaderBuilderAddValue(0x0);
            }
        }
    }

    //Decorations
    {

        if(curr_builder->num_decorations > 0){
            for(int i=0;i < curr_builder->num_io_data;i++){

                if(curr_builder->ioData[i].type == SHADER_VARIABLE_TYPE_UNIFORM){
                    
                    ShaderBuilderAddOp(SpvOpDecorate, 3);
                    ShaderBuilderAddValue(curr_builder->ioData[i].orig_indx );
                    ShaderBuilderAddValue(SpvDecorationBlock );

                    ShaderVariable *var_orig = ShaderBuilderFindVar(curr_builder->ioData[i].orig_indx);
                    
                    uint32_t offset = 0;
                    DecoratingUniforStruct(var_orig, &offset);
                }          

            }
        }

        for(int i=0;i < curr_builder->num_io_data;i++){

            if(curr_builder->ioData[i].flags & SHADER_DATA_FLAG_SYSTEM){
                ShaderVariable *v = ShaderBuilderFindVar(curr_builder->ioData[i].orig_indx);

                if(v->type == SHADER_VARIABLE_TYPE_STRUCT){
                    
                    ShaderBuilderAddOp(SpvOpDecorate, 3);
                    ShaderBuilderAddValue(v->indx);
                    ShaderBuilderAddValue(SpvDecorationBlock );
                    
                    uint32_t iter = 0;
                    for(int j=0; j <  curr_builder->num_decorations;j++){
                        if(curr_builder->decors[j].indx == curr_builder->ioData[i].orig_indx && 
                           curr_builder->decors[j].type == SHADER_DECOR_TYPE_BUILTIN){

                            ShaderBuilderAddOp(SpvOpMemberDecorate, 5);
                            ShaderBuilderAddValue(curr_builder->ioData[i].orig_indx );
                            ShaderBuilderAddValue(iter);
                            ShaderBuilderAddValue(SpvDecorationBuiltIn);
                            
                            
                            ShaderBuilderAddValue(curr_builder->decors[j].args[0] );

                            iter++;
                        }
                    }
                }else if(v->type == SHADER_VARIABLE_TYPE_VECTOR){                    
                    ShaderBuilderAddOp(SpvOpDecorate, 4);
                    ShaderBuilderAddValue(curr_builder->ioData[i].indx );
                    ShaderBuilderAddValue(SpvDecorationBuiltIn);

                    ShaderDecoration *decor = ShaderBuilderFindDecor(v->indx, SHADER_DECOR_TYPE_BUILTIN);

                    ShaderBuilderAddValue(decor->args[0] );
                }

            }else if(curr_builder->ioData[i].binding != 0){                    
                ShaderBuilderAddOp(SpvOpDecorate, 4);
                ShaderBuilderAddValue(curr_builder->ioData[i].indx );
                ShaderBuilderAddValue(SpvDecorationBinding );
                ShaderBuilderAddValue(curr_builder->ioData[i].binding - 1 );
                ShaderBuilderAddOp(SpvOpDecorate, 4);
                ShaderBuilderAddValue(curr_builder->ioData[i].indx );
                ShaderBuilderAddValue(SpvDecorationDescriptorSet );
                ShaderBuilderAddValue(curr_builder->ioData[i].descr_set);   
            }else{         
                ShaderBuilderAddOp(SpvOpDecorate, 4);
                ShaderBuilderAddValue(curr_builder->ioData[i].indx );
                ShaderBuilderAddValue(SpvDecorationLocation );
                ShaderBuilderAddValue(curr_builder->ioData[i].location );
            }
        }
    }

    //Variables
    {

        ShaderVariable *variable;
        ChildStack *child = curr_builder->alloc_head;

        int j=0;
        while(child != NULL)
        {
            variable = child->node;
            j = 0;

            if(variable == NULL){
                child = child->next;
                continue;
            }

            if(variable->indx == 0)
                continue;

            switch(variable->type){
                case SHADER_VARIABLE_TYPE_VOID:
                    ShaderBuilderAddOp(SpvOpTypeVoid, 2);
                    ShaderBuilderAddValue(variable->indx);
                    break;
                case SHADER_VARIABLE_TYPE_BOOL:
                    ShaderBuilderAddOp(SpvOpTypeBool, 2);
                    ShaderBuilderAddValue(variable->indx);
                    break;
                case SHADER_VARIABLE_TYPE_FUNCTION:
                    ShaderBuilderWriteFuncType(variable);
                    break;
                case SHADER_VARIABLE_TYPE_INT:
                    ShaderBuilderAddOp(SpvOpTypeInt, 4);
                    ShaderBuilderAddValue(variable->indx);
                    ShaderBuilderAddValue(variable->values[0]);
                    ShaderBuilderAddValue(variable->values[1]);
                    break;
                case SHADER_VARIABLE_TYPE_FLOAT:
                    ShaderBuilderAddOp(SpvOpTypeFloat, 3);
                    ShaderBuilderAddValue(variable->indx);
                    ShaderBuilderAddValue(variable->values[0]);
                    break;
                case SHADER_VARIABLE_TYPE_ARRAY:
                    ShaderBuilderAddOp(SpvOpTypeArray, 4);
                    ShaderBuilderAddValue(variable->indx);
                    ShaderBuilderAddValue(variable->args[0]);
                    ShaderBuilderAddValue(variable->args[1]);
                    break;
                case SHADER_VARIABLE_TYPE_IMAGE:
                    ShaderBuilderAddOp(SpvOpTypeImage, 9);
                    ShaderBuilderAddValue(variable->indx);
                    ShaderBuilderAddValue(variable->args[0]);
                    ShaderBuilderAddValue(variable->values[0]);
                    ShaderBuilderAddValue(variable->values[1]);
                    ShaderBuilderAddValue(variable->values[2]);
                    ShaderBuilderAddValue(variable->values[3]);
                    ShaderBuilderAddValue(variable->values[4]);
                    ShaderBuilderAddValue(variable->values[5]);
                    break;
                case SHADER_VARIABLE_TYPE_SAMPLED_IMAGE:
                    ShaderBuilderAddOp(SpvOpTypeSampledImage, 3);
                    ShaderBuilderAddValue(variable->indx);
                    ShaderBuilderAddValue(variable->args[0]);
                    break;
                case SHADER_VARIABLE_TYPE_CONSTANT:
                    ShaderBuilderAddOp(SpvOpConstant, 4);
                    ShaderBuilderAddValue(variable->args[0]);
                    ShaderBuilderAddValue(variable->indx);
                    ShaderBuilderAddValue(variable->values[0]);
                    break;
                    
                case SHADER_VARIABLE_TYPE_CONSTANT_COMPOSITE:
                    ShaderBuilderAddOp(SpvOpConstantComposite, 3 + variable->num_values);
                    ShaderBuilderAddValue(variable->args[0]);
                    ShaderBuilderAddValue(variable->indx);
                    for(j=0;j < variable->num_values;j++)
                        ShaderBuilderAddValue(variable->values[j]);
                    break;
                case SHADER_VARIABLE_TYPE_VECTOR:
                    ShaderBuilderAddOp(SpvOpTypeVector, 4);
                    ShaderBuilderAddValue(variable->indx);
                    ShaderBuilderAddValue(variable->args[0]);
                    ShaderBuilderAddValue(variable->values[0]);
                    break;
                case SHADER_VARIABLE_TYPE_MATRIX:
                    ShaderBuilderAddOp(SpvOpTypeMatrix, 4);
                    ShaderBuilderAddValue(variable->indx);
                    ShaderBuilderAddValue(variable->args[0]);
                    ShaderBuilderAddValue(variable->values[0]);
                    break;
                case SHADER_VARIABLE_TYPE_STRUCT:
                    ShaderBuilderAddOp(SpvOpTypeStruct, 2 + variable->num_args);
                    ShaderBuilderAddValue(variable->indx);
                    for(j=0;j < variable->num_args;j++)
                        ShaderBuilderAddValue(variable->args[j]);
                    break;
                case SHADER_VARIABLE_TYPE_POINTER:
                    ShaderBuilderAddOp(SpvOpTypePointer, 4);
                    ShaderBuilderAddValue(variable->indx);
                    ShaderBuilderAddValue(variable->flags & SHADER_DATA_FLAG_UNIFORM ? SpvStorageClassUniform : (variable->flags & SHADER_DATA_FLAG_OUTPUT ? SpvStorageClassOutput : (variable->flags & SHADER_DATA_FLAG_UNIFORM_CONSTANT ? SpvStorageClassUniformConstant : (variable->flags & SHADER_DATA_FLAG_FUNCTION ? SpvStorageClassFunction : SpvStorageClassInput))));
                    ShaderBuilderAddValue(variable->args[0]);
                    break;
                case SHADER_VARIABLE_TYPE_VARIABLE:
                    ShaderBuilderAddOp(SpvOpVariable, 4);
                    ShaderBuilderAddValue(variable->args[0]);
                    ShaderBuilderAddValue(variable->indx);
                    ShaderBuilderAddValue(variable->flags & SHADER_DATA_FLAG_UNIFORM ? SpvStorageClassUniform : (variable->flags & SHADER_DATA_FLAG_OUTPUT ? SpvStorageClassOutput : (variable->flags & SHADER_DATA_FLAG_UNIFORM_CONSTANT ? SpvStorageClassUniformConstant : (variable->flags & SHADER_DATA_FLAG_FUNCTION ? SpvStorageClassFunction : SpvStorageClassInput))));
                    break;
            }

            child = child->next;
        }
    }

    //Functions
    {
        ShaderOperand *operand;
                    
        uint32_t iter = 0;

        for(int i=0;i < curr_builder->num_functions;i++)
        {
            ShaderBuilderAddOp(SpvOpFunction, 5);
            ShaderBuilderAddValue(curr_builder->functions[i].result_type_indx);
            ShaderBuilderAddValue(curr_builder->functions[i].indx);
            ShaderBuilderAddValue(curr_builder->functions[i].function_control);
            ShaderBuilderAddValue(curr_builder->functions[i].func_type_indx);

            for(int j=0;j < curr_builder->functions[i].num_labels;j++)
            {
                if(curr_builder->functions[i].labels[j].will_return)
                    continue;               

                ShaderBuilderCheckThatLabel(&curr_builder->functions[i].labels[j]);
            }

            for(int j=0;j < curr_builder->functions[i].num_labels;j++)
            {
                if(!curr_builder->functions[i].labels[j].will_return)
                    continue;

                ShaderBuilderCheckThatLabel(&curr_builder->functions[i].labels[j]);

                ShaderBuilderAddOp(SpvOpReturn, 1);
            }

            ShaderBuilderAddOp(SpvOpFunctionEnd, 1);
        }
    }

    curr_builder->code[3] = curr_builder->current_index + 1;

    ShaderBuilderClear(curr_builder);

    curr_builder = NULL;
    curr_label = NULL;
}

int iter = 0;
uint32_t val = 0, op = 0, left_val = 0;
void StartReading(){
    left_val = 0;
    iter = 15;
    val = 0;
    op = 0;
}

void NextCode(uint32_t *ptr, uint32_t offset){

    iter += offset;
    val = ptr[iter];
    left_val = val >> 16;
    op = val & 0x0000FFFF;
}

ShaderDataFlags FindDataFlags(uint32_t val){

    switch(val){
        case 0:
            return SHADER_DATA_FLAG_UNIFORM_CONSTANT;
        case 2:
            return SHADER_DATA_FLAG_UNIFORM;
        case 3:
            return SHADER_DATA_FLAG_OUTPUT;
        case 7:
            return SHADER_DATA_FLAG_FUNCTION;
    }

    return 0;
}

void ShaderBuilderParcingShader(uint32_t *shader, uint32_t size){

    uint32_t *ptr = shader;

    int entry = 0;

    StartReading();
    NextCode(ptr, 1);

    entry = val >> 16;

    NextCode(ptr, 1);

    switch(val){
        case SpvExecutionModelFragment: 
            curr_builder->type = SHADER_TYPE_FRAGMENT;
            NextCode(ptr, 1);
            break;
        case SpvExecutionModelVertex: 
            curr_builder->type = SHADER_TYPE_VERTEX;
            NextCode(ptr, 1);
            break;
        case SpvExecutionModelGLCompute: 
            curr_builder->type = SHADER_TYPE_COMPUTED;
            NextCode(ptr, 1);
            break;
        case SpvExecutionModelGeometry: 
            curr_builder->type = SHADER_TYPE_GEOMETRY;
            NextCode(ptr, 1);
            break;
        case SpvExecutionModelTessellationControl: 
            curr_builder->type = SHADER_TYPE_TESELLATION_CONTROL;
            NextCode(ptr, 1);
            break;
        case SpvExecutionModelTessellationEvaluation: 
            curr_builder->type = SHADER_TYPE_TESELLATION_EVALUATION;
            NextCode(ptr, 1);
            break;
    }

    //curr_builder->main_point_index = val;
    
    int j = 0;

    while(val != 0x00040047){
        NextCode(ptr, 1);
    }

    ShaderVariable *variable = NULL;

    while(iter < size){

        switch(op){
                case SpvOpDecorate:
                    NextCode(ptr, 1);
                    curr_builder->decors[curr_builder->num_decorations].indx = val;
                    NextCode(ptr, 1);
                    curr_builder->decors[curr_builder->num_decorations].type = val;
                    if(val == 2){
                        NextCode(ptr, 1);
                        continue;
                    }
                    NextCode(ptr, 1);
                    curr_builder->decors[curr_builder->num_decorations].val = val;
                    curr_builder->num_decorations++;
                    NextCode(ptr, 1);
                    continue;
                case SpvOpMemberDecorate:
                    NextCode(ptr, 4);
                    break;
                case SpvOpDecorationGroup:
                    NextCode(ptr, 2);
                    break;
                case SpvOpGroupDecorate:
                    NextCode(ptr, 2);
                    break;
                case SpvOpGroupMemberDecorate:
                    NextCode(ptr, 2);
                    break;
                case SpvOpDecorateId:
                    NextCode(ptr, 3);
                    break;
                case SpvOpDecorateString:
                    NextCode(ptr, 4);
                    break;
                case SpvOpMemberDecorateString:
                    NextCode(ptr, 4);
                    break;
        }  

        if(val < SpvOpGroupLogicalXorKHR)
        {
            NextCode(ptr, 1);
            continue;
        }
        
        switch(op){
            case SpvOpTypeVoid:
                variable = ShaderBuilderAllocateVariabel(curr_builder);
                variable->type = SHADER_VARIABLE_TYPE_VOID;
                NextCode(ptr, 1);
                variable->indx = val;
                NextCode(ptr, 1);
                break;
            case SpvOpTypeFunction:
                variable = ShaderBuilderAllocateVariabel(curr_builder);
                variable->type = SHADER_VARIABLE_TYPE_FUNCTION;
                NextCode(ptr, 1);
                variable->indx = val;
                NextCode(ptr, 1);
                variable->values[0] = val;
                NextCode(ptr, 1);
                variable->num_values = 1;
                break;
            case SpvOpTypeInt:
                variable = ShaderBuilderAllocateVariabel(curr_builder);
                variable->type = SHADER_VARIABLE_TYPE_INT;
                NextCode(ptr, 1);
                variable->indx = val;
                NextCode(ptr, 1);
                variable->values[0] = val;
                NextCode(ptr, 1);
                variable->values[1] = val;
                NextCode(ptr, 1);
                variable->num_values = 2;
                break;
            case SpvOpTypeFloat:
                variable = ShaderBuilderAllocateVariabel(curr_builder);
                variable->type = SHADER_VARIABLE_TYPE_FLOAT;
                NextCode(ptr, 1);
                variable->indx = val;
                NextCode(ptr, 1);
                variable->values[0] = val;
                NextCode(ptr, 1);
                variable->num_values = 1;
                break;
            case SpvOpTypeArray:
                variable = ShaderBuilderAllocateVariabel(curr_builder);
                variable->type = SHADER_VARIABLE_TYPE_ARRAY;
                NextCode(ptr, 1);
                variable->indx = val;
                NextCode(ptr, 1);
                variable->args[0] = val;
                NextCode(ptr, 1);
                variable->args[1] = val;
                NextCode(ptr, 1);
                variable->num_args = 2;
                break;
            case SpvOpTypeImage:
                variable = ShaderBuilderAllocateVariabel(curr_builder);
                variable->type = SHADER_VARIABLE_TYPE_IMAGE;
                NextCode(ptr, 1);
                variable->indx = val;
                NextCode(ptr, 1);
                variable->args[0] = val;
                NextCode(ptr, 1);
                variable->values[0] = val;
                NextCode(ptr, 1);
                variable->values[1] = val;
                NextCode(ptr, 1);
                variable->values[2] = val;
                NextCode(ptr, 1);
                variable->values[3] = val;
                NextCode(ptr, 1);
                variable->values[4] = val;
                NextCode(ptr, 1);
                variable->values[5] = val;
                NextCode(ptr, 1);
                variable->num_values = 6;
                variable->num_args = 1;
                break;
            case SpvOpTypeSampledImage:
                variable = ShaderBuilderAllocateVariabel(curr_builder);
                variable->type = SHADER_VARIABLE_TYPE_SAMPLED_IMAGE;
                NextCode(ptr, 1);
                variable->indx = val;
                NextCode(ptr, 1);
                variable->args[0] = val;
                NextCode(ptr, 1);
                variable->num_args = 1;
                break;
            case SpvOpConstant:
                variable = ShaderBuilderAllocateVariabel(curr_builder);
                variable->type = SHADER_VARIABLE_TYPE_CONSTANT;
                NextCode(ptr, 1);
                variable->args[0] = val;
                NextCode(ptr, 1);
                variable->indx = val;
                NextCode(ptr, 1);
                variable->values[0] = val;
                NextCode(ptr, 1);
                variable->num_args = 1;
                variable->num_values = 1;
                break;
            case SpvOpTypeVector:
                variable = ShaderBuilderAllocateVariabel(curr_builder);
                variable->type = SHADER_VARIABLE_TYPE_VECTOR;
                NextCode(ptr, 1);
                variable->indx = val;
                NextCode(ptr, 1);
                variable->args[0] = val;
                NextCode(ptr, 1);
                variable->values[0] = val;
                NextCode(ptr, 1);
                variable->num_args = 1;
                variable->num_values = 1;
                break;
            case SpvOpTypeMatrix:
                variable = ShaderBuilderAllocateVariabel(curr_builder);
                variable->type = SHADER_VARIABLE_TYPE_MATRIX;
                NextCode(ptr, 1);
                variable->indx = val;
                NextCode(ptr, 1);
                variable->args[0] = val;
                NextCode(ptr, 1);
                variable->values[0] = val;
                NextCode(ptr, 1);
                variable->num_args = 1;
                variable->num_values = 1;
                break;
            case SpvOpTypeStruct:   
                variable = ShaderBuilderAllocateVariabel(curr_builder);         
                variable->type = SHADER_VARIABLE_TYPE_STRUCT;
                variable->num_args = left_val - 2;
                NextCode(ptr, 1);
                variable->indx = val;
                NextCode(ptr, 1);
                for(j=0;j < variable->num_args;j++){                    
                    variable->args[j] = val;
                    NextCode(ptr, 1);
                }
                break;
            case SpvOpTypePointer:
                variable = ShaderBuilderAllocateVariabel(curr_builder);
                variable->type = SHADER_VARIABLE_TYPE_POINTER;
                NextCode(ptr, 1);
                variable->indx = val;
                NextCode(ptr, 1);
                variable->flags = FindDataFlags(val); // Flags
                NextCode(ptr, 1);
                variable->args[0] = val;
                NextCode(ptr, 1);
                variable->num_args = 1;
                break;
            case SpvOpVariable:
                variable = ShaderBuilderAllocateVariabel(curr_builder);
                variable->type = SHADER_VARIABLE_TYPE_VARIABLE;
                NextCode(ptr, 1);
                variable->args[0] = val;
                NextCode(ptr, 1);
                variable->indx = val;
                NextCode(ptr, 1);
                variable->flags = FindDataFlags(val); //Flags
                NextCode(ptr, 1);
                variable->num_args = 1;
                break;
            case SpvOpFunction:
                return;
            default:        
                NextCode(ptr, 1);
                break;
        }
    }
}

uint32_t ReturnSizeVector(ShaderVariable *var_elm){

    ShaderVariable *elm_type = ShaderBuilderFindVar(var_elm->args[0]);

    uint32_t temp_size = 0;
    if(elm_type->type == SHADER_VARIABLE_TYPE_FLOAT || elm_type->type == SHADER_VARIABLE_TYPE_INT)
        temp_size = 4 * var_elm->values[0] /*count elem*/;

    return temp_size;
}

uint32_t ReturnSizeMatrix(ShaderVariable *mat_elm){

    uint32_t size = 0;

    ShaderVariable *elm_type = ShaderBuilderFindVar(mat_elm->args[0]); 

    if(elm_type->type == SHADER_VARIABLE_TYPE_FLOAT || elm_type->type == SHADER_VARIABLE_TYPE_INT)
        size += 4 * mat_elm->values[0] /*count elem*/;
    else if(elm_type->type == SHADER_VARIABLE_TYPE_VECTOR){
        size += ReturnSizeVector(elm_type) * mat_elm->values[0];
    }

    
    while(size % 16)
        size ++;

    return size;
}

uint32_t ReturnSizeStruct(ShaderVariable *str_elm){

    uint32_t size = 0, temp_size = 0, biggest_value = 0, const_count = 0;
    ShaderVariable *next_elm;
    for(int j=0;j < str_elm->num_args;j++){
        ShaderVariable *var_elm = ShaderBuilderFindVar(str_elm->args[j]);
        
        if(j + 1 < str_elm->num_args)
            next_elm = ShaderBuilderFindVar(str_elm->args[j + 1]);
        else
            next_elm = NULL;
        
        if(var_elm->type == SHADER_VARIABLE_TYPE_FLOAT || var_elm->type == SHADER_VARIABLE_TYPE_INT){
            temp_size = 4;
            size += 4;
            const_count++;
        }else if(var_elm->type == SHADER_VARIABLE_TYPE_VECTOR){
            
            temp_size = ReturnSizeVector(var_elm);

            if(next_elm != NULL){
                if(var_elm->values[0] /*count elem*/ == 3 && (next_elm->type == SHADER_VARIABLE_TYPE_FLOAT || next_elm->type == SHADER_VARIABLE_TYPE_INT))
                    size += temp_size;
                else{
                    temp_size += 4;
                    size += temp_size;
                }

            }else{
                if(var_elm->values[0] /*count elem*/ == 3){
                    temp_size += 4;
                    size += temp_size;
                }else{
                    size += temp_size;
                }
            }
            

        }else if(var_elm->type == SHADER_VARIABLE_TYPE_MATRIX){
            size += ReturnSizeMatrix(var_elm);
        }else if(var_elm->type == SHADER_VARIABLE_TYPE_ARRAY){
            ShaderVariable *elm_type = ShaderBuilderFindVar(var_elm->args[0]);
            ShaderVariable *elm_const = ShaderBuilderFindVar(var_elm->args[1]);        
            if(elm_type->type == SHADER_VARIABLE_TYPE_FLOAT || elm_type->type == SHADER_VARIABLE_TYPE_INT){
                size += 4 * elm_const->values[0]/*count elem*/;
            }else if(elm_type->type == SHADER_VARIABLE_TYPE_MATRIX){
                size += ReturnSizeMatrix(elm_type) * elm_const->values[0]/*count elem*/;                                
            }else if(elm_type->type == SHADER_VARIABLE_TYPE_STRUCT){
                uint32_t str_size = ReturnSizeStruct(elm_type);
                size += str_size * elm_const->values[0]/*count elem*/;
            }            
            
            while(size % 16)
                size ++;

        }else if(var_elm->type == SHADER_VARIABLE_TYPE_STRUCT){
                size += ReturnSizeStruct(var_elm);
        }
        
        if(biggest_value < temp_size)
            biggest_value = temp_size;
    }

    if(biggest_value != 0)
        while(size % biggest_value)
            size ++;

    return size;
}

void ShaderBuilderMakeUniformsFromShader(ShaderBuilder *builder, uint32_t *code, uint32_t size, void *blueprints, uint32_t indx_pack){

    ShaderBuilderSetCurrent(builder);

    size /= sizeof(uint32_t);

    if(curr_builder->alloc_head == NULL)
        curr_builder->alloc_head = calloc(1, sizeof(ChildStack));

    ShaderBuilderParcingShader(code, size);

    printf("Shader builder : Variables count %i \n", ShaderBuilderGetVariablesCount(builder));

    ShaderVariable *currVar = NULL;
    uint32_t size_buffer = 0, flags = 0;
    
    ChildStack *child = curr_builder->alloc_head;

    int find = 0;
    while(child != NULL){
        currVar = child->node;

        if(currVar != NULL){
            if(currVar->type == SHADER_VARIABLE_TYPE_VARIABLE){
                flags = currVar->flags;
                if(flags & SHADER_DATA_FLAG_UNIFORM){
                    size_buffer = 0;

                    ShaderVariable *var_point = ShaderBuilderFindVar(currVar->args[0]);

                    ShaderVariable *var_orig = ShaderBuilderFindVar(var_point->args[0]);

                    if(var_orig->type == SHADER_VARIABLE_TYPE_STRUCT){
                        size_buffer += ReturnSizeStruct(var_orig);
                        

                        uint32_t binding = 0;
                        for(int i=0;i < curr_builder->num_decorations;i++){
                            if(curr_builder->decors[i].type == SpvDecorationBinding && curr_builder->decors[i].indx == currVar->indx)
                                binding = curr_builder->decors[i].val;
                        }
                        
                        BluePrintAddUniformObjectC(blueprints, indx_pack, size_buffer, curr_builder->type == SHADER_TYPE_VERTEX ? VK_SHADER_STAGE_VERTEX_BIT : VK_SHADER_STAGE_FRAGMENT_BIT, binding);

                        printf("Shader builder : Uniform buffer size is %i\n", size_buffer);
                    }


                }else if(flags & SHADER_DATA_FLAG_UNIFORM_CONSTANT){

                    ShaderVariable *var_point = ShaderBuilderFindVar(currVar->args[0]);

                    ShaderVariable *var_orig = ShaderBuilderFindVar(var_point->args[0]);
                    
                    if(var_orig->type == SHADER_VARIABLE_TYPE_SAMPLED_IMAGE){

                        
                        uint32_t binding = 0;
                        for(int i=0;i < curr_builder->num_decorations;i++){
                            if(curr_builder->decors[i].type == SpvDecorationBinding && curr_builder->decors[i].indx == currVar->indx)
                                binding = curr_builder->decors[i].val;
                        }
                    
                        BluePrintAddTextureC(blueprints, indx_pack, curr_builder->type == SHADER_TYPE_VERTEX ? VK_SHADER_STAGE_VERTEX_BIT : VK_SHADER_STAGE_FRAGMENT_BIT, binding);

                        printf("Shader builder : Image added to blueprint\n", size_buffer);
                    }
                }
            }
        }

        child = child->next;
    }

    ShaderBuilderClear(builder);

    curr_builder = NULL;
}

void ShaderBuilderWriteToFile(ShaderBuilder *builder, const char *path){

    int somefile = open(path, O_WRONLY | O_CREAT | O_BINARY);

    write(somefile, builder->code, builder->size * sizeof(uint32_t));

    close(somefile);

}
