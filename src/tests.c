#include "collector.h"
#include "tests.h"

int main(int argc, char* argv[]) {
    stream_tests();
    tokenize_tests();
    parse_tests();
    gc_close();
    return 0;
}