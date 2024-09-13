#ifndef SHADER_BUILDER_H
#define SHADER_BUILDER_H

#include <stdint.h>

#define EXTEND_IMPORT_POINT_INDEX 0x1

#define SHADER_MAX_LENGTH 2048
#define SHADER_MAX_IODATA 64
#define SHADER_MAX_DEBUG_NAMES 64
#define SHADER_MAX_FUNCTIONS 64
#define SHADER_MAX_DECORATIONS 64

typedef enum{
    SHADER_TYPE_FRAGMENT,
    SHADER_TYPE_VERTEX,
    SHADER_TYPE_COMPUTED,
    SHADER_TYPE_GEOMETRY,
    SHADER_TYPE_TESELLATION_CONTROL,
    SHADER_TYPE_TESELLATION_EVALUATION,
} ShaderType;

typedef enum{
    SHADER_VARIABLE_TYPE_VOID,
    SHADER_VARIABLE_TYPE_BOOL,
    SHADER_VARIABLE_TYPE_INT,
    SHADER_VARIABLE_TYPE_FLOAT,
    SHADER_VARIABLE_TYPE_VECTOR,
    SHADER_VARIABLE_TYPE_MATRIX,
    SHADER_VARIABLE_TYPE_ARRAY,
    SHADER_VARIABLE_TYPE_IMAGE,
    SHADER_VARIABLE_TYPE_SAMPLED_IMAGE,
    SHADER_VARIABLE_TYPE_STRUCT,
    SHADER_VARIABLE_TYPE_VARIABLE,
    SHADER_VARIABLE_TYPE_CONSTANT,
    SHADER_VARIABLE_TYPE_CONSTANT_COMPOSITE,
    SHADER_VARIABLE_TYPE_POINTER,
    SHADER_VARIABLE_TYPE_FUNCTION,
    SHADER_VARIABLE_TYPE_UNIFORM,
    SHADER_VARIABLE_TYPE_UNIFORM_CONSTANT,
    SHADER_VARIABLE_TYPE_EXTENDED_IMPORT,
} ShaderVariableType;

typedef struct ShaderStructConstr{
    ShaderVariableType var_type;
    uint32_t size;
    uint32_t type;
    char name[32];
    struct ShaderStructConstr *str;
    uint32_t count;
    char name_str[32];
} ShaderStructConstr;

typedef enum{
    SHADER_OPERAND_TYPE_ACCESS,
    SHADER_OPERAND_TYPE_LOAD,
    SHADER_OPERAND_TYPE_STORE,
    SHADER_OPERAND_TYPE_BRANCH,
    SHADER_OPERAND_TYPE_BRANCH_CONDITIONAL,
    SHADER_OPERAND_TYPE_LOOP,
    SHADER_OPERAND_TYPE_COMPOSITE_EXTRACT,
    SHADER_OPERAND_TYPE_COMPOSITE_CONSTRUCT,
    SHADER_OPERAND_TYPE_IMAGE_SAMLE_IMPLICIT_LOD,
    SHADER_OPERAND_TYPE_ADD,
    SHADER_OPERAND_TYPE_SUB,
    SHADER_OPERAND_TYPE_MUL,
    SHADER_OPERAND_TYPE_DIV,
    SHADER_OPERAND_TYPE_MATTIMEMAT,
    SHADER_OPERAND_TYPE_MATTIMEVEC,
    SHADER_OPERAND_TYPE_VECTIMES,
    SHADER_OPERAND_TYPE_VECSHUFFLE,
    SHADER_OPERAND_TYPE_VARIABLE,
    SHADER_OPERAND_TYPE_SLESSTHAN,
} ShaderOperandType;

typedef enum{
    SHADER_DATA_FLAG_UNSIGNED = 0x1,
    SHADER_DATA_FLAG_OUTPUT = 0x2,
    SHADER_DATA_FLAG_UNIFORM= 0x4,
    SHADER_DATA_FLAG_UNIFORM_CONSTANT = 0x8,
    SHADER_DATA_FLAG_SYSTEM = 0x10,
    SHADER_DATA_FLAG_FUNCTION = 0x20,
} ShaderDataFlags;

typedef struct{
    ShaderVariableType type;
    uint32_t indx;
    uint32_t orig_indx;
    uint32_t binding;
    uint32_t descr_set;
    uint32_t location;
    ShaderDataFlags flags;
} InputOutputData;

typedef struct{
    uint32_t indx;
    uint32_t res_store_indx;
    ShaderOperandType op_type;
    uint32_t var_indx[16];
    uint32_t num_vars;
} ShaderOperand;

typedef struct ShaderVariable{
    ShaderVariableType type;
    uint32_t indx;
    uint32_t args[32];
    uint32_t num_args;
    uint32_t values[32];
    uint32_t num_values;
    ShaderDataFlags flags;
    uint32_t result_type_indx;
} ShaderVariable;

typedef struct VectorExtract{
    uint32_t elems[8];
    uint32_t size;
} VectorExtract;

typedef struct{
    ShaderVariableType type;
    uint32_t index;
} ShaderFuncParam;

typedef struct{
    uint32_t index;
    ShaderOperand operands[256];
    uint32_t num_operands;
    int will_return;
} ShaderLabel;

typedef struct{
    uint32_t result_type_indx;
    uint32_t func_type_indx;
    uint32_t indx;
    uint32_t function_control;
    ShaderFuncParam params[16];
    uint32_t num_params;
    ShaderLabel labels[32];
    uint32_t num_labels;
} ShaderFunc;

typedef struct{
    uint32_t indx;
    char name[64];
    char child_name[16][64];
    uint32_t num_childs;
} ShaderDebugInfo;

typedef struct{
    uint32_t indx;
    uint32_t type;
    uint32_t val;
    void *str_point;
    uint32_t str_size;
} ShaderDecoration;

typedef struct{   
    uint32_t code[SHADER_MAX_LENGTH];
    uint32_t size;
    InputOutputData ioData[SHADER_MAX_IODATA];
    uint32_t num_io_data;
    ShaderFunc functions[SHADER_MAX_FUNCTIONS];
    uint32_t num_functions;
    ShaderDebugInfo infos[SHADER_MAX_DEBUG_NAMES];
    uint32_t num_debug_infos;
    ShaderDecoration decors[SHADER_MAX_DECORATIONS];
    uint32_t num_decorations;
    ShaderType type;
    uint32_t current_index;
    uint32_t curr_descr_set;
    ShaderFunc *main_point_index;
    uint32_t gl_struct_indx;
    struct ChildStack *alloc_head; 
} ShaderBuilder;

void ShaderBuilderInit(ShaderBuilder *builder, ShaderType type);
void ShaderBuilderMake();
void ShaderBuilderWriteToFile(ShaderBuilder *builder, const char *path);

void ShaderBuilderSetCurrentLabel(int label_indx);

uint32_t ShaderBuilderAddPointer(ShaderVariableType point_type, uint32_t size, ShaderDataFlags flags);
uint32_t ShaderBuilderAddConstant(ShaderVariableType var_type, ShaderDataFlags flags,  uint32_t valu, uint32_t sign);
uint32_t ShaderBuilderAddConstantComposite(ShaderVariableType var_type, uint32_t size, uint32_t value);

uint32_t ShaderBuilderAddVector(uint32_t size, char *name);
VectorExtract ShaderBuilderGetElemenets(uint32_t src_index, uint32_t start_indx, uint32_t size);
uint32_t ShaderBuilderAddOperand(uint32_t *indexes, uint32_t count, ShaderOperandType operand);
uint32_t ShaderBuilderAcceptAccess(uint32_t val_indx, ShaderVariableType var_type, uint32_t type_arg, uint32_t *chain, uint32_t size);
uint32_t ShaderBuilderAcceptLoadL(uint32_t val_indx, uint32_t struct_indx);

uint32_t ShaderBuilderNextLabel(int with_transition, uint32_t indx_label, int will_return);

uint32_t ShaderBuilderCompositeConstruct(uint32_t *arr_arg, uint32_t size_arr);

void ShaderBuilderMakeBranchConditional(uint32_t cond_type, uint32_t *vals, uint32_t size, uint32_t true_label, uint32_t false_label);

uint32_t ShaderBuilderAddFuncDiv(uint32_t val_1, uint32_t indx_1, uint32_t type_1, uint32_t size_1,  uint32_t val_2, uint32_t indx_2, uint32_t type_2, uint32_t size_2, uint32_t res_size);
uint32_t ShaderBuilderAddFuncMult(uint32_t val_1, uint32_t indx_1, uint32_t type_1, uint32_t size_1,  uint32_t val_2, uint32_t indx_2, uint32_t type_2, uint32_t size_2, uint32_t res_size);
uint32_t ShaderBuilderAddFuncSetTexure(uint32_t texture_indx, uint32_t uv_indx, uint32_t dest_indx, uint32_t dest_size);
int ShaderBuilderAddFuncAdd(uint32_t val_1, uint32_t val_2, uint32_t size, uint32_t res_store);
uint32_t ShaderBuilderAddFuncMove(uint32_t src_indx, uint32_t src_size, uint32_t dest_indx, uint32_t dest_size);
uint32_t ShaderBuilderAddFuncMultS(uint32_t val_indx, uint32_t single_indx, uint32_t size);
uint32_t ShaderBuilderAddFuncSetColor4(uint32_t val_indx, uint32_t single_indx, uint32_t size);
uint32_t ShaderBuilderGetTexture(uint32_t texture_indx, uint32_t uv_indx);

int ShaderBuilderAddFuncMoveToGL(uint32_t vec_val, uint32_t vec_size, uint32_t res_store);

void ShaderBuilderStoreValue(uint32_t *arr, uint32_t size);

uint32_t ShaderBuilderAddUniform(ShaderStructConstr *struct_arr, uint32_t count, char *name, uint32_t location, uint32_t binding);
uint32_t ShaderBuilderAddIOData(ShaderVariableType type, ShaderDataFlags flags, ShaderStructConstr *struct_arr, uint32_t size, char *name, uint32_t location, uint32_t binding);

void ShaderBuilderMakeUniformsFromShader(ShaderBuilder *builder, uint32_t *code, uint32_t size, void *blueprints, uint32_t indx_pack);
void ShaderBuilderParcingShader(uint32_t *shader, uint32_t size);

void ShaderBuilderClear(ShaderBuilder *builder);

#endif // SHADER_BUILDER_H
