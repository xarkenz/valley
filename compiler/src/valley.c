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
    VL_UNREAD(c);

    VLString string = {name, len};
    VLToken token = {.kind = VL_TOKEN_NAME, .pos = pos, .stringValue = string};
    parser->token = token;
}


void vlGrabNumberToken(VLParser* parser) {
    return; // placeholder

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
    VL_UNREAD(c);

    // parse numStr
}


void vlGrabStringToken(VLParser* parser) {
    return; // placeholder
}


void vlGrabSymbolToken(VLParser* parser) {
    return; // placeholder
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
        if (c == EOF) {
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
            //return;
            VL_READ(); c = VL_READ(); continue; // placeholder
        } else if (ispunct(c)) {
            if (c == '/') {
                int c1 = VL_READ();
                if (c == '/') {
                    vlSkipLineComment(parser);
                    c = VL_READ();
                    continue;
                } else if (c == '*') {
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
                    //return;
                    VL_READ(); c = VL_READ(); continue; // placeholder
                }
            } else if (c == '"') {
                vlGrabStringToken(parser);
                //return;
                c = VL_READ(); continue; // placeholder
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