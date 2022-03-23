#include <stdio.h>

#include "include/valley.h"

int main() {
    const char* path = "test.vl";
    FILE* stream = fopen(path, "r");
    if (!stream) {
        printf("Unable to load file '%s'.\n", path);
        return 0;
    }
    VLParser parser = {stream, -1, {.kind = VL_TOKEN_EOF, -1}};
    vlNextToken(&parser);
    printf("---- TOKENS ----\n");
    while (parser.token.kind != VL_TOKEN_EOF) {
        printf("VL_TOKEN_NAME:  %s\n", parser.token.stringValue.first);
        vlNextToken(&parser);
    }
    fclose(stream);
    return 0;
}
