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

void vlGrabNameToken(VLParser* parser) {
    if (!parser) return;
    size_t pos = parser->pos;
    char* name = calloc(1, sizeof(char));
    size_t len = 0;
    int c = VL_READ();
    while (!VL_EOF() && (isalpha(c) || isdigit(c) || c == '_')) {
        name = realloc(name, len + sizeof(c) + sizeof(char));
        if (!name) break; // TODO: flag error instead
        strncat(name, (const char*) &c, 1);
        ++len;
        c = VL_READ();
    }
    VLString string = {name, len};
    VLToken token = {.kind = VL_TOKEN_NAME, .pos = pos, .stringValue = string};
    parser->token = token;
}

void vlGrabNumberToken(VLParser* parser) {
    if (!parser) return;
    size_t pos = parser->pos;
    char* numStr = calloc(1, sizeof(char));
    size_t len = 0;
    int c = VL_READ();
    while (!VL_EOF() && (isdigit(c) || c == '.')) {
        numStr = realloc(numStr, len + sizeof(c) + sizeof(char));
        if (!numStr) break; // TODO: flag error instead
        strncat(numStr, (const char*) &c, 1);
        ++len;
        c = VL_READ();
    }
    // etc
}

void vlNextToken(VLParser* parser) {
    if (!parser) return;
    int c = VL_READ();
    while (!VL_EOF()) {
        if (isalpha(c)) {
            VL_UNREAD(c);
            vlGrabNameToken(parser);
        } /*else if (isdigit(c)) {
            VL_UNREAD(c);
            vlGrabNumberToken(parser);
        }*/
        c = VL_READ();
    }
    VLToken token = {.kind = VL_TOKEN_EOF, .pos = parser->pos};
    parser->token = token;
}