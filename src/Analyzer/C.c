#include "../lib/lib.h"

int main(int argc, char const *argv[]) {
    
    int value_return = 0;
    int nfiles = 0; //number of files to retreive from pipe
    int n = 3;
    int m = 4;
    node files; //list of paths
    int i;

    
    //Parsing arguments------------------------------------------------------------------------------------------
    if(argc % 2 == 0 || argc < 2) { //if number of arguments is even or less than 1, surely it's a wrong input
        value_return = err_args_C();
    }
    else {
        for(i = 1; i< argc; i += 2) {
            if(!strcmp(argv[i], "-nfiles")) { //Check if the argument is equal to -nfiles
                nfiles = atoi(argv[i + 1]);
                if(nfiles == 0) value_return = err_args_C();
            } 
            else if(!strcmp(argv[i], "-setn")) {//Check if argument is equal to -setn
                n = atoi(argv[i + 1]);
                if(n == 0) value_return = err_args_C();
            }
            else if(!strcmp(argv[i], "-setm")) { //Check if argument is equal to -setm
                m = atoi(argv[i+1]);
                if(m == 0) value_return = err_args_C();
            }
            else {
                value_return = err_args_C();
            }
        }
        if(nfiles == 0 && value_return == 0) value_return = err_args_C(); //CHeck if nfiles is setted, if not gives an error (value_return used to avoid double messages)
    }


    printf("C says: nfiles=%d n=%d m=%d\n", nfiles, n, m);

    return value_return;
}
