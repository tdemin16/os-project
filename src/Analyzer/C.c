#include "../lib/lib.h"

int main(int argc, char const *argv[]) {
    
    int value_return = err_args();
    int nfiles;

    if(argc < 2) {
        value_return = err_args();
    } else {
        nfiles = atoi(argv[1]);
        printf("%d\n", nfiles);
    }

    return value_return;
}
