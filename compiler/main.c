#include <stdio.h>

#include "include/valley.h"

int main() {
    VLParser parser = {fopen("test.vl", "r"), -1, {}};
    vlNextToken(&parser);
    while (parser.token.kind != VL_TOKEN_EOF) {
        printf("%s\n", parser.token.stringValue.first);
        vlNextToken(&parser);
    }
    return 0;
}
