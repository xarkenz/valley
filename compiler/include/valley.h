#ifndef VALLEY_H
#define VALLEY_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// ---- MACROS ---- //

#define VL_READ() getc(parser->stream); ++parser->pos
#define VL_UNREAD(c) ungetc(c, parser->stream); --parser->pos
#define VL_EOF() feof(parser->stream)
#define VL_CHECK_KW(str, value) if (strcmp(name, str) == 0) { VLToken token = {.kind = VL_KW_##value, .pos = pos}; parser->token = token; return; }

#define VL_ANSI_RED     "\x1b[31m"
#define VL_ANSI_GREEN   "\x1b[32m"
#define VL_ANSI_YELLOW  "\x1b[33m"
#define VL_ANSI_BLUE    "\x1b[34m"
#define VL_ANSI_MAGENTA "\x1b[35m"
#define VL_ANSI_CYAN    "\x1b[36m"
#define VL_ANSI_RESET   "\x1b[0m"

// ---- TYPEDEFS ---- //

typedef struct {
    const char* first;
    size_t len;
} VLString;

typedef char VLChar;
typedef int8_t VLByte;
typedef int16_t VLShort;
typedef int32_t VLInt;
typedef int64_t VLLong;
typedef float VLFloat;
typedef double VLDouble;
typedef bool VLBool;

typedef enum VLTokenKind {
    VL_TOKEN_EOF,
    VL_TOKEN_NAME,

    // Primitive literals
    VL_TOKEN_STR,
    VL_TOKEN_CHAR,
    VL_TOKEN_BYTE,
    VL_TOKEN_SHORT,
    VL_TOKEN_INT,
    VL_TOKEN_LONG,
    VL_TOKEN_FLOAT,
    VL_TOKEN_DOUBLE,
    VL_TOKEN_BOOL,

    // Keywords
    VL_KW_IS,
    VL_KW_IF,
    VL_KW_ELIF,
    VL_KW_ELSE,
    VL_KW_FOR,
    VL_KW_WHILE,
    VL_KW_DO,
    VL_KW_BREAK,
    VL_KW_CONTINUE,
    VL_KW_SWITCH,
    VL_KW_CASE,
    VL_KW_DEFAULT,
    VL_KW_WITH,
    VL_KW_TRY,
    VL_KW_CATCH,
    VL_KW_FINALLY,
    VL_KW_THROW,
    VL_KW_RETURN,
    VL_KW_FINAL,
    VL_KW_PUBLIC,
    VL_KW_PROTECTED,
    VL_KW_PRIVATE,
    VL_KW_STATIC,
    VL_KW_IMPORT,

    // Special symbols
    VL_SYM_ADD,
    VL_SYM_SUB,
    VL_SYM_MUL,
    VL_SYM_DIV,
    VL_SYM_MOD,
    VL_SYM_EXP,
    VL_SYM_NOT,
    VL_SYM_AND,
    VL_SYM_XOR,
    VL_SYM_OR,
    VL_SYM_LSHIFT,
    VL_SYM_RSHIFT,
    VL_SYM_LNOT,
    VL_SYM_LAND,
    VL_SYM_LXOR,
    VL_SYM_LOR,
    VL_SYM_EQ,
    VL_SYM_NEQ,
    VL_SYM_LT,
    VL_SYM_GT,
    VL_SYM_LTEQ,
    VL_SYM_GTEQ,
    VL_SYM_SAME,
    VL_SYM_NSAME,
    VL_SYM_INC,
    VL_SYM_DEC,
    VL_SYM_PUT,
    VL_SYM_ADD_PUT,
    VL_SYM_SUB_PUT,
    VL_SYM_MUL_PUT,
    VL_SYM_DIV_PUT,
    VL_SYM_MOD_PUT,
    VL_SYM_EXP_PUT,
    VL_SYM_AND_PUT,
    VL_SYM_XOR_PUT,
    VL_SYM_OR_PUT,
    VL_SYM_LSHIFT_PUT,
    VL_SYM_RSHIFT_PUT,
    VL_SYM_COLON,
    VL_SYM_SEMICOLON,
    VL_SYM_COMMA,
    VL_SYM_L_CURLY,
    VL_SYM_R_CURLY,
    VL_SYM_COND,
    VL_SYM_L_PAREN,
    VL_SYM_R_PAREN,
    VL_SYM_L_SQUARE,
    VL_SYM_R_SQUARE,
    VL_SYM_DOT,
    VL_SYM_ARROW,
    VL_SYM_ELLIPSIS,
//    VL_SYM_AT,
//    VL_SYM_BACKSLASH,
//    VL_SYM_HASH,
//    VL_SYM_DOLLAR,
} VLTokenKind;

typedef enum VLExprKind {
    VL_EXPR_NAME,
    VL_EXPR_STR,
    VL_EXPR_CHAR,
    VL_EXPR_BYTE,
    VL_EXPR_SHORT,
    VL_EXPR_INT,
    VL_EXPR_LONG,
    VL_EXPR_FLOAT,
    VL_EXPR_DOUBLE,
    VL_EXPR_BOOL,
    VL_EXPR_UNARY,
    VL_EXPR_BINARY,
    VL_EXPR_TERNARY,
    VL_EXPR_MULTI,
} VLExprKind;

typedef enum VLOperation {
    VL_OP_POS,
    VL_OP_NEG,
    VL_OP_ADD,
    VL_OP_SUB,
    VL_OP_MUL,
    VL_OP_DIV,
    VL_OP_MOD,
    VL_OP_EXP,
    VL_OP_NOT,
    VL_OP_AND,
    VL_OP_XOR,
    VL_OP_OR,
    VL_OP_LSHIFT,
    VL_OP_RSHIFT,
    VL_OP_LNOT,
    VL_OP_LAND,
    VL_OP_LXOR,
    VL_OP_LOR,
    VL_OP_EQ,
    VL_OP_NEQ,
    VL_OP_LT,
    VL_OP_GT,
    VL_OP_LTEQ,
    VL_OP_GTEQ,
    VL_OP_SAME,
    VL_OP_NSAME,
    VL_OP_IS,
    VL_OP_INC_BEF,
    VL_OP_INC_AFT,
    VL_OP_DEC_BEF,
    VL_OP_DEC_AFT,
    VL_OP_PUT,
    VL_OP_ADD_PUT,
    VL_OP_SUB_PUT,
    VL_OP_MUL_PUT,
    VL_OP_DIV_PUT,
    VL_OP_MOD_PUT,
    VL_OP_EXP_PUT,
    VL_OP_AND_PUT,
    VL_OP_XOR_PUT,
    VL_OP_OR_PUT,
    VL_OP_LSHIFT_PUT,
    VL_OP_RSHIFT_PUT,
    VL_OP_CAST,
    VL_OP_MEMBER,
    VL_OP_COND,
    VL_OP_LIST,
    VL_OP_INDEX,
    VL_OP_CALL,
    VL_OP_ARR_INIT,
    VL_OP_DECLARE,
    VL_OP_DECLARE_FINAL,
    VL_OP_EXTEND,
} VLOperation;

typedef enum VLPrecedence {
    VL_PREC_ACCESS,
    VL_PREC_POSTFIX,
    VL_PREC_PREFIX,
    VL_PREC_DECLARATION,
    VL_PREC_EXPONENTIAL,
    VL_PREC_MULTIPLICATIVE,
    VL_PREC_ADDITIVE,
    VL_PREC_SHIFT,
    VL_PREC_RELATIONAL,
    VL_PREC_EQUALITY,
    VL_PREC_BITWISE_AND,
    VL_PREC_BITWISE_XOR,
    VL_PREC_BITWISE_OR,
    VL_PREC_LOGICAL_AND,
    VL_PREC_LOGICAL_XOR,
    VL_PREC_LOGICAL_OR,
    VL_PREC_CAST,
    VL_PREC_CONDITIONAL,
    VL_PREC_ASSIGNMENT,
    VL_PREC_STRUCTURAL,
} VLPrecedence;

typedef enum VLDataType {
    VL_TYPE_VOID,
    VL_TYPE_STR,
    VL_TYPE_CHAR,
    VL_TYPE_BYTE,
    VL_TYPE_SHORT,
    VL_TYPE_INT,
    VL_TYPE_LONG,
    VL_TYPE_FLOAT,
    VL_TYPE_DOUBLE,
    VL_TYPE_BOOL,
    VL_TYPE_OBJECT,
    VL_TYPE_ARRAY,
    VL_TYPE_TYPENAME,
    VL_TYPE_FUNCTION,
} VLDataType;

typedef enum VLStatus {
    VL_STATUS_OK,
    VL_STATUS_OUT_OF_MEM,
    VL_STATUS_UNEXPECTED,
    VL_STATUS_EXPECTED,
    VL_STATUS_UNCLOSED,
} VLStatus;

typedef struct VLToken {
    VLTokenKind kind;
    size_t pos;
    union {
        VLString stringValue;
        VLChar charValue;
        VLByte byteValue;
        VLShort shortValue;
        VLInt intValue;
        VLLong longValue;
        VLFloat floatValue;
        VLDouble doubleValue;
        VLBool boolValue;
    };
} VLToken;

typedef struct VLExpression {
    VLExprKind kind;
    size_t pos;
    union {
        VLString stringValue;
        VLChar charValue;
        VLByte byteValue;
        VLShort shortValue;
        VLInt intValue;
        VLLong longValue;
        VLFloat floatValue;
        VLDouble doubleValue;
        VLBool boolValue;
        struct {
            VLOperation operation;
            struct VLExpression* child;
        } unaryOp;
        struct {
            VLOperation operation;
            struct VLExpression* first;
            struct VLExpression* second;
        } binaryOp;
        struct {
            VLOperation operation;
            struct VLExpression* first;
            struct VLExpression* second;
            struct VLExpression* third;
        } ternaryOp;
        struct {
            VLOperation operation;
            struct VLExpression* children;
            size_t count;
        } multiOp;
    };
} VLExpression;

typedef struct VLParser {
    FILE* stream;
    size_t pos;
    VLToken token;
    VLStatus status;
    const char* what;
} VLParser;

// ---- FUNCTION PROTOTYPES ---- //

void vlPrintToken(VLToken token);

void vlGrabNameToken(VLParser* parser);
void vlGrabNumberToken(VLParser* parser);
void vlGrabStringToken(VLParser* parser);
void vlGrabSymbolToken(VLParser* parser);

void vlSkipLineComment(VLParser* parser);
void vlSkipBlockComment(VLParser* parser);

void vlGrabToken(VLParser* parser);
bool vlNextToken(VLParser* parser);

VLOperation vlGetOperation(VLTokenKind kind, bool prefix);
VLPrecedence vlGetPrecedence(VLOperation operation);
bool vlIsLeftToRightAssociative(VLPrecedence precedence);

VLExpression* vlParseExpr(VLParser* parser, VLDataType type, bool lvalue, bool allowComma, bool allowEmpty);

#endif /* VALLEY_H */
