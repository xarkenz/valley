/* ================
 * src/valley.c
 * VALLEY LANGUAGE COMPILER
 * by xarkenz
 * 3/18/2022
 * ================
 */

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "../include/valley.h"


// ---- FUNCTIONS ---- //

void vlPrintToken(VLToken token) {
    switch (token.kind) {
        case VL_TOKEN_EOF:      printf("<EOF>"); break;
        case VL_TOKEN_NAME:     printf("%s", token.stringValue.first); break;
        case VL_TOKEN_STR:      printf("\"%s\"", token.stringValue.first); break;
        case VL_TOKEN_CHAR:     printf("\'%c\'", token.charValue); break;
        case VL_TOKEN_BYTE:     printf("%db", token.byteValue); break;
        case VL_TOKEN_SHORT:    printf("%ds", token.shortValue); break;
        case VL_TOKEN_INT:      printf("%d", token.intValue); break;
        case VL_TOKEN_LONG:     printf("%ldl", token.longValue); break;
        case VL_TOKEN_FLOAT:    printf("%ff", token.floatValue); break;
        case VL_TOKEN_DOUBLE:   printf("%f", token.doubleValue); break;
        case VL_TOKEN_BOOL:     printf(token.boolValue ? "TRUE" : "FALSE"); break;
        case VL_KW_IS:          printf("IS"); break;
        case VL_KW_IF:          printf("IF"); break;
        case VL_KW_ELIF:        printf("ELIF"); break;
        case VL_KW_ELSE:        printf("ELSE"); break;
        case VL_KW_FOR:         printf("FOR"); break;
        case VL_KW_WHILE:       printf("WHILE"); break;
        case VL_KW_DO:          printf("DO"); break;
        case VL_KW_BREAK:       printf("BREAK"); break;
        case VL_KW_CONTINUE:    printf("CONTINUE"); break;
        case VL_KW_SWITCH:      printf("SWITCH"); break;
        case VL_KW_CASE:        printf("CASE"); break;
        case VL_KW_DEFAULT:     printf("DEFAULT"); break;
        case VL_KW_WITH:        printf("WITH"); break;
        case VL_KW_TRY:         printf("TRY"); break;
        case VL_KW_CATCH:       printf("CATCH"); break;
        case VL_KW_FINALLY:     printf("FINALLY"); break;
        case VL_KW_THROW:       printf("THROW"); break;
        case VL_KW_RETURN:      printf("RETURN"); break;
        case VL_KW_FINAL:       printf("FINAL"); break;
        case VL_KW_PUBLIC:      printf("PUBLIC"); break;
        case VL_KW_PROTECTED:   printf("PROTECTED"); break;
        case VL_KW_PRIVATE:     printf("PRIVATE"); break;
        case VL_KW_STATIC:      printf("STATIC"); break;
        case VL_KW_IMPORT:      printf("IMPORT"); break;
        case VL_SYM_ADD:        printf("+"); break;
        case VL_SYM_SUB:        printf("-"); break;
        case VL_SYM_MUL:        printf("*"); break;
        case VL_SYM_DIV:        printf("/"); break;
        case VL_SYM_MOD:        printf("%%"); break;
        case VL_SYM_EXP:        printf("**"); break;
        case VL_SYM_NOT:        printf("~"); break;
        case VL_SYM_AND:        printf("&"); break;
        case VL_SYM_XOR:        printf("^"); break;
        case VL_SYM_OR:         printf("|"); break;
        case VL_SYM_LSHIFT:     printf("<<"); break;
        case VL_SYM_RSHIFT:     printf(">>"); break;
        case VL_SYM_LNOT:       printf("!"); break;
        case VL_SYM_LAND:       printf("&&"); break;
        case VL_SYM_LXOR:       printf("^^"); break;
        case VL_SYM_LOR:        printf("||"); break;
        case VL_SYM_EQ:         printf("=="); break;
        case VL_SYM_NEQ:        printf("!="); break;
        case VL_SYM_LT:         printf("<"); break;
        case VL_SYM_GT:         printf(">"); break;
        case VL_SYM_LTEQ:       printf("<="); break;
        case VL_SYM_GTEQ:       printf(">="); break;
        case VL_SYM_SAME:       printf("==="); break;
        case VL_SYM_NSAME:      printf("!=="); break;
        case VL_SYM_INC:        printf("++"); break;
        case VL_SYM_DEC:        printf("--"); break;
        case VL_SYM_PUT:        printf("="); break;
        case VL_SYM_ADD_PUT:    printf("+="); break;
        case VL_SYM_SUB_PUT:    printf("-="); break;
        case VL_SYM_MUL_PUT:    printf("*="); break;
        case VL_SYM_DIV_PUT:    printf("/="); break;
        case VL_SYM_MOD_PUT:    printf("%%="); break;
        case VL_SYM_EXP_PUT:    printf("**="); break;
        case VL_SYM_AND_PUT:    printf("&="); break;
        case VL_SYM_XOR_PUT:    printf("^="); break;
        case VL_SYM_OR_PUT:     printf("|="); break;
        case VL_SYM_LSHIFT_PUT: printf("<<="); break;
        case VL_SYM_RSHIFT_PUT: printf(">>="); break;
        case VL_SYM_COLON:      printf(":"); break;
        case VL_SYM_SEMICOLON:  printf(";"); break;
        case VL_SYM_COMMA:      printf(","); break;
        case VL_SYM_L_CURLY:    printf("{"); break;
        case VL_SYM_R_CURLY:    printf("}"); break;
        case VL_SYM_COND:       printf("?"); break;
        case VL_SYM_L_PAREN:    printf("("); break;
        case VL_SYM_R_PAREN:    printf(")"); break;
        case VL_SYM_L_SQUARE:   printf("["); break;
        case VL_SYM_R_SQUARE:   printf("]"); break;
        case VL_SYM_DOT:        printf("."); break;
        case VL_SYM_ARROW:      printf("->"); break;
        case VL_SYM_ELLIPSIS:   printf("..."); break;
        default:                printf("<UNKNOWN>"); break;
    }
    printf(" ");
}


void vlGrabNameToken(VLParser* parser) {
    size_t pos = parser->pos;
    char* name = calloc(1, sizeof(char));
    size_t len = 0;

    int c = VL_READ();
    while (!VL_EOF() && (isalpha(c) || isdigit(c) || c == '_')) {
        name = realloc(name, len + sizeof(c) + sizeof(char));
        if (!name) {
            parser->status = VL_STATUS_OUT_OF_MEM;
            return;
        }
        strncat(name, (const char*) &c, 1);
        ++len;
        c = VL_READ();
    }
    VL_UNREAD(c);

    VL_CHECK_KW("is", IS);
    VL_CHECK_KW("if", IF);
    VL_CHECK_KW("elif", ELIF);
    VL_CHECK_KW("else", ELSE);
    VL_CHECK_KW("for", FOR);
    VL_CHECK_KW("while", WHILE);
    VL_CHECK_KW("do", DO);
    VL_CHECK_KW("break", BREAK);
    VL_CHECK_KW("continue", CONTINUE);
    VL_CHECK_KW("switch", SWITCH);
    VL_CHECK_KW("case", CASE);
    VL_CHECK_KW("default", DEFAULT);
    VL_CHECK_KW("with", WITH);
    VL_CHECK_KW("try", TRY);
    VL_CHECK_KW("catch", CATCH);
    VL_CHECK_KW("finally", FINALLY);
    VL_CHECK_KW("throw", THROW);
    VL_CHECK_KW("return", RETURN);
    VL_CHECK_KW("final", FINAL);
    VL_CHECK_KW("public", PUBLIC);
    VL_CHECK_KW("protected", PROTECTED);
    VL_CHECK_KW("private", PRIVATE);
    VL_CHECK_KW("static", STATIC);
    VL_CHECK_KW("import", IMPORT);

    VLString string = {name, len};
    VLToken token = {.kind = VL_TOKEN_NAME, .pos = pos, .stringValue = string};
    parser->token = token;
}


void vlGrabNumberToken(VLParser* parser) {
    size_t pos = parser->pos;
    char* numStr = calloc(1, sizeof(char));
    size_t len = 0;
    bool isFloating = false;

    int c = VL_READ();
    while (!VL_EOF() && (isdigit(c) || c == '.')) {
        if (c == '.') {
            if (isFloating) {
                parser->status = VL_STATUS_UNEXPECTED;
                parser->what = ".";
                return;
            }
            isFloating = true;
        }
        numStr = realloc(numStr, len + sizeof(c) + sizeof(char));
        if (!numStr) {
            parser->status = VL_STATUS_OUT_OF_MEM;
            break;
        }
        strncat(numStr, (const char*) &c, 1);
        ++len;
        c = VL_READ();
    }

    if (isFloating) {
        VLDouble num = strtod(numStr, NULL);
        if (c == 'f' || c == 'F') {
            VLToken token = {.kind = VL_TOKEN_FLOAT, .pos = pos, .floatValue = (VLFloat) num};
            parser->token = token;
        } else {
            if (c != 'd' && c != 'D') { VL_UNREAD(c); }
            VLToken token = {.kind = VL_TOKEN_DOUBLE, .pos = pos, .doubleValue = num};
            parser->token = token;
        }
    } else {
        VLLong num = strtoll(numStr, NULL, 10);
        if (c == 'b' || c == 'B') {
            VLToken token = {.kind = VL_TOKEN_BYTE, .pos = pos, .byteValue = (VLByte) num};
            parser->token = token;
        } else if (c == 's' || c == 'S') {
            VLToken token = {.kind = VL_TOKEN_SHORT, .pos = pos, .shortValue = (VLShort) num};
            parser->token = token;
        } else if (c == 'l' || c == 'L') {
            VLToken token = {.kind = VL_TOKEN_LONG, .pos = pos, .longValue = num};
            parser->token = token;
        } else if (c == 'f' || c == 'F') {
            VLToken token = {.kind = VL_TOKEN_FLOAT, .pos = pos, .floatValue = (VLFloat) num};
            parser->token = token;
        } else if (c == 'd' || c == 'D') {
            VLToken token = {.kind = VL_TOKEN_BYTE, .pos = pos, .doubleValue = (VLDouble) num};
            parser->token = token;
        } else {
            if (c != 'i' && c != 'I') { VL_UNREAD(c); }
            VLToken token = {.kind = VL_TOKEN_INT, .pos = pos, .intValue = (VLInt) num};
            parser->token = token;
        }
    }
}


void vlGrabStringToken(VLParser* parser) {
    size_t pos = parser->pos;
    char* rawStr = calloc(1, sizeof(char));
    size_t len = 0;

    int c = VL_READ();
    while (!VL_EOF()) {
        if (c == '\\') {
            c = VL_READ();
            if (VL_EOF()) break;
            switch (c) {
                case 'n': c = '\n'; break;
                case 'r': c = '\r'; break;
                case 't': c = '\t'; break;
                default: break;
            }
        } else if (c == '"') {
            VLString string = {rawStr, len};
            VLToken token = {.kind = VL_TOKEN_STR, .pos = pos, .stringValue = string};
            parser->token = token;
            return;
        } else if (c == '\n' || c == '\r' || c == '\t') {
            break;
        }

        rawStr = realloc(rawStr, len + sizeof(c) + sizeof(char));
        if (!rawStr) {
            parser->status = VL_STATUS_OUT_OF_MEM;
            break;
        }
        strncat(rawStr, (const char*) &c, 1);
        ++len;
        c = VL_READ();
    }

    parser->pos = pos;
    parser->status = VL_STATUS_UNCLOSED;
    parser->what = "\"";
}


void vlGrabSymbolToken(VLParser* parser) {
    size_t pos = parser->pos;
    VLTokenKind sym = VL_TOKEN_EOF;
    int c0, c1, c2;

    c0 = VL_READ();
    switch (c0) {
        case '+':
            c1 = VL_READ();
            switch (c1) {
                case '+': sym = VL_SYM_INC; break;
                case '=': sym = VL_SYM_ADD_PUT; break;
                default: sym = VL_SYM_ADD; VL_UNREAD(c1); break;
            }
            break;
        case '-':
            c1 = VL_READ();
            switch (c1) {
                case '-': sym = VL_SYM_DEC; break;
                case '=': sym = VL_SYM_SUB_PUT; break;
                case '>': sym = VL_SYM_ARROW; break;
                default: sym = VL_SYM_SUB; VL_UNREAD(c1); break;
            }
            break;
        case '*':
            c1 = VL_READ();
            switch (c1) {
                case '*':
                    c2 = VL_READ();
                    switch (c2) {
                        case '=': sym = VL_SYM_EXP_PUT; break;
                        default: sym = VL_SYM_EXP; VL_UNREAD(c2); break;
                    }
                    break;
                case '=': sym = VL_SYM_MUL_PUT; break;
                default: sym = VL_SYM_MUL; VL_UNREAD(c1); break;
            }
            break;
        case '/':
            c1 = VL_READ();
            switch (c1) {
                case '=': sym = VL_SYM_DIV_PUT; break;
                default: sym = VL_SYM_DIV; VL_UNREAD(c1); break;
            }
            break;
        case '%':
            c1 = VL_READ();
            switch (c1) {
                case '=': sym = VL_SYM_MOD_PUT; break;
                default: sym = VL_SYM_MOD; VL_UNREAD(c1); break;
            }
            break;
        case '~': sym = VL_SYM_NOT; break;
        case '&':
            c1 = VL_READ();
            switch (c1) {
                case '&': sym = VL_SYM_LAND; break;
                case '=': sym = VL_SYM_AND_PUT; break;
                default: sym = VL_SYM_AND; VL_UNREAD(c1); break;
            }
            break;
        case '^':
            c1 = VL_READ();
            switch (c1) {
                case '^': sym = VL_SYM_LXOR; break;
                case '=': sym = VL_SYM_XOR_PUT; break;
                default: sym = VL_SYM_XOR; VL_UNREAD(c1); break;
            }
            break;
        case '|':
            c1 = VL_READ();
            switch (c1) {
                case '|': sym = VL_SYM_LOR; break;
                case '=': sym = VL_SYM_OR_PUT; break;
                default: sym = VL_SYM_OR; VL_UNREAD(c1); break;
            }
            break;
        case '<':
            c1 = VL_READ();
            switch (c1) {
                case '<':
                    c2 = VL_READ();
                    switch (c2) {
                        case '=': sym = VL_SYM_LSHIFT_PUT; break;
                        default: sym = VL_SYM_LSHIFT; VL_UNREAD(c2); break;
                    }
                    break;
                case '=': sym = VL_SYM_LTEQ; break;
                default: sym = VL_SYM_LT; VL_UNREAD(c1); break;
            }
            break;
        case '>':
            c1 = VL_READ();
            switch (c1) {
                case '>':
                    c2 = VL_READ();
                    switch (c2) {
                        case '=': sym = VL_SYM_RSHIFT_PUT; break;
                        default: sym = VL_SYM_RSHIFT; VL_UNREAD(c2); break;
                    }
                    break;
                case '=': sym = VL_SYM_GTEQ; break;
                default: sym = VL_SYM_GT; VL_UNREAD(c1); break;
            }
            break;
        case '!':
            c1 = VL_READ();
            switch (c1) {
                case '=':
                    c2 = VL_READ();
                    switch (c2) {
                        case '=': sym = VL_SYM_NSAME; break;
                        default: sym = VL_SYM_NEQ; VL_UNREAD(c2); break;
                    }
                    break;
                default: sym = VL_SYM_LNOT; VL_UNREAD(c1); break;
            }
            break;
        case '=':
            c1 = VL_READ();
            switch (c1) {
                case '=':
                    c2 = VL_READ();
                    switch (c2) {
                        case '=': sym = VL_SYM_SAME; break;
                        default: sym = VL_SYM_EQ; VL_UNREAD(c2); break;
                    }
                    break;
                default: sym = VL_SYM_PUT; VL_UNREAD(c1); break;
            }
            break;
        case ':': sym = VL_SYM_COLON; break;
        case ';': sym = VL_SYM_SEMICOLON; break;
        case ',': sym = VL_SYM_COMMA; break;
        case '{': sym = VL_SYM_L_CURLY; break;
        case '}': sym = VL_SYM_R_CURLY; break;
        case '?': sym = VL_SYM_COND; break;
        case '(': sym = VL_SYM_L_PAREN; break;
        case ')': sym = VL_SYM_R_PAREN; break;
        case '[': sym = VL_SYM_L_SQUARE; break;
        case ']': sym = VL_SYM_R_SQUARE; break;
        case '.':
            c1 = VL_READ();
            switch (c1) {
                case '.':
                    c2 = VL_READ();
                    switch (c2) {
                        case '.': sym = VL_SYM_ELLIPSIS; break;
                        default: sym = VL_SYM_DOT; VL_UNREAD(c2); VL_UNREAD(c1); break;
                    }
                    break;
                default: sym = VL_SYM_DOT; VL_UNREAD(c1); break;
            }
            break;
        default:
            parser->status = VL_STATUS_UNEXPECTED;
            parser->what = (const char*) &c0;
            return;
    }

    VLToken token = {.kind = sym, .pos = pos};
    parser->token = token;
}


void vlSkipLineComment(VLParser* parser) {
    bool skipNewline = false;

    int c = VL_READ();
    while (!VL_EOF()) {
        if (!skipNewline && c == '\n') return;

        if (skipNewline) {
            if (!isblank(c)) skipNewline = false;
        } else if (c == '\\') skipNewline = true;

        c = VL_READ();
    }
}


void vlSkipBlockComment(VLParser* parser) {
    bool checkEnd = false;

    int c = VL_READ();
    while (!VL_EOF()) {
        if (checkEnd) {
            if (c == '/') return;
            checkEnd = false;
        }

        if (c == '*') checkEnd = true;

        c = VL_READ();
    }
}


void vlGrabToken(VLParser* parser) {
    int c = VL_READ();
    while (true) {
        if (VL_EOF()) {
            VLToken token = {.kind = VL_TOKEN_EOF, .pos = parser->pos};
            parser->token = token;
            return;
        } else if (isspace(c)) {
            c = VL_READ();
            continue;
        } else if (isalpha(c) || c == '_') {
            VL_UNREAD(c);
            vlGrabNameToken(parser);
            return;
        } else if (isdigit(c)) {
            VL_UNREAD(c);
            vlGrabNumberToken(parser);
            return;
        } else if (ispunct(c)) {
            if (c == '/') {
                int c1 = VL_READ();
                if (c1 == '/') {
                    vlSkipLineComment(parser);
                    c = VL_READ();
                    continue;
                } else if (c1 == '*') {
                    vlSkipBlockComment(parser);
                    c = VL_READ();
                    continue;
                }
                VL_UNREAD(c1);
            } else if (c == '.') {
                int c1 = VL_READ();
                VL_UNREAD(c1);
                if (isdigit(c1)) {
                    VL_UNREAD(c);
                    vlGrabNumberToken(parser);
                    return;
                }
            } else if (c == '"') {
                vlGrabStringToken(parser);
                return;
            }
            VL_UNREAD(c);
            vlGrabSymbolToken(parser);
            return;
        }

        // If c isn't accounted for, skip it
        c = VL_READ();
    }
}


bool vlNextToken(VLParser* parser) {
    vlGrabToken(parser);

    switch (parser->status) {
        case VL_STATUS_OK:
            return true;
        case VL_STATUS_OUT_OF_MEM:
            printf(VL_ANSI_RED "Error: Ran out of available memory." VL_ANSI_RESET "\n");
            return false;
        case VL_STATUS_UNEXPECTED:
            printf(VL_ANSI_RED "Error: Encountered unexpected '%s'." VL_ANSI_RESET "\n", parser->what);
            return false;
        case VL_STATUS_EXPECTED:
            printf(VL_ANSI_RED "Error: Expected '%s'." VL_ANSI_RESET "\n", parser->what);
            return false;
        case VL_STATUS_UNCLOSED:
            printf(VL_ANSI_RED "Error: Unable to find a matching '%s'." VL_ANSI_RESET "\n", parser->what);
            return false;
        default:
            return false;
    }
}


VLOperation vlGetOperation(VLTokenKind kind, bool prefix) {
    switch (kind) {
        case VL_SYM_ADD: return prefix ? VL_OP_POS : VL_OP_ADD;
        case VL_SYM_SUB: return prefix ? VL_OP_NEG : VL_OP_SUB;
        case VL_SYM_MUL: return VL_OP_MUL;
        case VL_SYM_DIV: return VL_OP_DIV;
        case VL_SYM_MOD: return VL_OP_MOD;
        case VL_SYM_EXP: return VL_OP_EXP;
        case VL_SYM_NOT: return VL_OP_NOT;
        case VL_SYM_AND: return VL_OP_AND;
        case VL_SYM_XOR: return VL_OP_XOR;
        case VL_SYM_OR: return VL_OP_OR;
        case VL_SYM_LSHIFT: return VL_OP_LSHIFT;
        case VL_SYM_RSHIFT: return VL_OP_RSHIFT;
        case VL_SYM_LNOT: return VL_OP_LNOT;
        case VL_SYM_LAND: return VL_OP_LAND;
        case VL_SYM_LXOR: return VL_OP_LXOR;
        case VL_SYM_LOR: return VL_OP_LOR;
        case VL_SYM_EQ: return VL_OP_EQ;
        case VL_SYM_NEQ: return VL_OP_NEQ;
        case VL_SYM_LT: return VL_OP_LT;
        case VL_SYM_GT: return VL_OP_GT;
        case VL_SYM_LTEQ: return VL_OP_LTEQ;
        case VL_SYM_GTEQ: return VL_OP_GTEQ;
        case VL_SYM_SAME: return VL_OP_SAME;
        case VL_SYM_NSAME: return VL_OP_NSAME;
        case VL_SYM_INC: return prefix ? VL_OP_INC_BEF : VL_OP_INC_AFT;
        case VL_SYM_DEC: return prefix ? VL_OP_DEC_BEF : VL_OP_DEC_AFT;
        case VL_SYM_PUT: return VL_OP_PUT;
        case VL_SYM_ADD_PUT: return VL_OP_ADD_PUT;
        case VL_SYM_SUB_PUT: return VL_OP_SUB_PUT;
        case VL_SYM_MUL_PUT: return VL_OP_MUL_PUT;
        case VL_SYM_DIV_PUT: return VL_OP_DIV_PUT;
        case VL_SYM_MOD_PUT: return VL_OP_MOD_PUT;
        case VL_SYM_EXP_PUT: return VL_OP_EXP_PUT;
        case VL_SYM_AND_PUT: return VL_OP_AND_PUT;
        case VL_SYM_XOR_PUT: return VL_OP_XOR_PUT;
        case VL_SYM_OR_PUT: return VL_OP_OR_PUT;
        case VL_SYM_LSHIFT_PUT: return VL_OP_LSHIFT_PUT;
        case VL_SYM_RSHIFT_PUT: return VL_OP_RSHIFT_PUT;
        case VL_SYM_COMMA: return VL_OP_LIST;
        case VL_SYM_COND: return VL_OP_COND;
        case VL_SYM_L_PAREN: return VL_OP_CALL;
        case VL_SYM_L_SQUARE: return VL_OP_INDEX;
        case VL_SYM_DOT: return VL_OP_MEMBER;
        case VL_SYM_ARROW: return VL_OP_CAST;
        case VL_SYM_ELLIPSIS: return VL_OP_EXTEND;
        case VL_KW_IS: return VL_OP_IS;
        default: return 0;
    }
}


VLPrecedence vlGetPrecedence(VLOperation operation) {
    switch (operation) {
        case VL_OP_CALL:
        case VL_OP_INDEX:
        case VL_OP_MEMBER:
            return VL_PREC_ACCESS;
        case VL_OP_INC_AFT:
        case VL_OP_DEC_AFT:
        case VL_OP_EXTEND:
            return VL_PREC_POSTFIX;
        case VL_OP_POS:
        case VL_OP_NEG:
        case VL_OP_NOT:
        case VL_OP_LNOT:
        case VL_OP_INC_BEF:
        case VL_OP_DEC_BEF:
            return VL_PREC_PREFIX;
        case VL_OP_DECLARE:
        case VL_OP_DECLARE_FINAL:
            return VL_PREC_DECLARATION;
        case VL_OP_EXP:
            return VL_PREC_EXPONENTIAL;
        case VL_OP_MUL:
        case VL_OP_DIV:
        case VL_OP_MOD:
            return VL_PREC_MULTIPLICATIVE;
        case VL_OP_ADD:
        case VL_OP_SUB:
            return VL_PREC_ADDITIVE;
        case VL_OP_LSHIFT:
        case VL_OP_RSHIFT:
            return VL_PREC_SHIFT;
        case VL_OP_LT:
        case VL_OP_GT:
        case VL_OP_LTEQ:
        case VL_OP_GTEQ:
        case VL_OP_IS:
            return VL_PREC_RELATIONAL;
        case VL_OP_EQ:
        case VL_OP_NEQ:
        case VL_OP_SAME:
        case VL_OP_NSAME:
            return VL_PREC_EQUALITY;
        case VL_OP_AND:
            return VL_PREC_BITWISE_AND;
        case VL_OP_XOR:
            return VL_PREC_BITWISE_XOR;
        case VL_OP_OR:
            return VL_PREC_BITWISE_OR;
        case VL_OP_LAND:
            return VL_PREC_LOGICAL_AND;
        case VL_OP_LXOR:
            return VL_PREC_LOGICAL_XOR;
        case VL_OP_LOR:
            return VL_PREC_LOGICAL_OR;
        case VL_OP_CAST:
            return VL_PREC_CAST;
        case VL_OP_COND:
            return VL_PREC_CONDITIONAL;
        case VL_OP_PUT:
        case VL_OP_ADD_PUT:
        case VL_OP_SUB_PUT:
        case VL_OP_MUL_PUT:
        case VL_OP_DIV_PUT:
        case VL_OP_MOD_PUT:
        case VL_OP_EXP_PUT:
        case VL_OP_AND_PUT:
        case VL_OP_XOR_PUT:
        case VL_OP_OR_PUT:
        case VL_OP_LSHIFT_PUT:
        case VL_OP_RSHIFT_PUT:
            return VL_PREC_ASSIGNMENT;
        case VL_OP_ARR_INIT:
        case VL_OP_LIST:
            return VL_PREC_STRUCTURAL;
        default:
            return 0;
    }
}


bool vlIsLeftToRightAssociative(VLPrecedence precedence) {
    switch (precedence) {
        case VL_PREC_PREFIX:
        case VL_PREC_ASSIGNMENT:
            return false;
        default:
            return true;
    }
}


VLExpression* vlParseExpr(VLParser* parser, VLDataType type, bool lvalue, bool allowComma, bool allowEmpty) {
    return NULL; // temporary
}