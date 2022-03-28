#include <stdio.h>
#include <time.h>

#include "include/valley.h"

int main() {
    clock_t timer = clock();

    const char* path = "test.vl";
    FILE* stream = fopen(path, "r");
    if (!stream) {
        printf("Unable to load file '%s'.\n", path);
        return 0;
    }
    VLParser parser = {stream, -1, {.kind = VL_TOKEN_EOF, -1}, VL_STATUS_OK, NULL};
    vlNextToken(&parser);
    printf("------------ TOKENS ------------\n");
    while (parser.token.kind != VL_TOKEN_EOF) {
        vlPrintToken(parser.token);
        vlNextToken(&parser);
    }
    fclose(stream);

    timer = clock() - timer;
    printf("\n============\nTime taken: %f seconds\n", ((float) timer) / CLOCKS_PER_SEC);

    return 0;
}
