/* ================
 * src/valley.c
 * VALLEY LANGUAGE COMPILER
 * by xarkenz, loosely inspired by Beans Shader Language compiler
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
        case VL_TOKEN_EOF:    printf("TOKEN_EOF\n"); break;
        case VL_TOKEN_NAME:   printf("TOKEN_NAME:    %s\n", token.stringValue.first); break;
        case VL_TOKEN_STR:    printf("TOKEN_STR:     \"%s\"\n", token.stringValue.first); break;
        case VL_TOKEN_CHAR:   printf("TOKEN_CHAR:    \'%c\'\n", token.charValue); break;
        case VL_TOKEN_BYTE:   printf("TOKEN_BYTE:    %db\n", token.byteValue); break;
        case VL_TOKEN_SHORT:  printf("TOKEN_SHORT:   %ds\n", token.shortValue); break;
        case VL_TOKEN_INT:    printf("TOKEN_INT:     %d\n", token.intValue); break;
        case VL_TOKEN_LONG:   printf("TOKEN_LONG:    %ldl\n", token.longValue); break;
        case VL_TOKEN_FLOAT:  printf("TOKEN_FLOAT:   %ff\n", token.floatValue); break;
        case VL_TOKEN_DOUBLE: printf("TOKEN_DOUBLE:  %f\n", token.doubleValue); break;
        case VL_TOKEN_BOOL:   printf("TOKEN_BOOL:    %s\n", token.boolValue ? "true" : "false"); break;
        default:              printf("TOKEN_UNKNOWN\n"); break;
    }
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
    VL_UNREAD(c);

    if (isFloating) {
        VLDouble num = strtod(numStr, NULL);
        if (c == 'f' || c == 'F') {
            VLToken token = {.kind = VL_TOKEN_FLOAT, .pos = pos, .floatValue = (VLFloat) num};
            parser->token = token;
        } else {
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
            VLToken token = {.kind = VL_TOKEN_INT, .pos = pos, .intValue = (VLInt) num};
            parser->token = token;
        }
    }
}


void vlGrabStringToken(VLParser* parser) {
    size_t pos = parser->pos;
    char* rawString = calloc(1, sizeof(char));
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
            VLString string = {rawString, len};
            VLToken token = {.kind = VL_TOKEN_STR, .pos = pos, .stringValue = string};
            parser->token = token;
            return;
        }

        rawString = realloc(rawString, len + sizeof(c) + sizeof(char));
        if (!rawString) break; // TODO: flag error instead
        strncat(rawString, (const char*) &c, 1);
        ++len;
        c = VL_READ();
    }

    parser->pos = pos;
    parser->status = VL_STATUS_UNCLOSED;
    parser->what = "\"";
}


void vlGrabSymbolToken(VLParser* parser) {
    // placeholder
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


void vlNextToken(VLParser* parser) {
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
            //return;
            VL_READ(); c = VL_READ(); continue; // placeholder
        }

        // If c isn't accounted for, skip it
        c = VL_READ();
    }
}