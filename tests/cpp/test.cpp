// -!- C++ -!- 

// used to check the commands file-lint and file-exec for errors

#include <iostream>

// char is an error and should show-up in file-exec and file-lint
char main(int argc, char *argv[]) {
    for (int i = 0; i < 8; i++) {
        auto j = 0;
        std::cout << i << std::endl;
    }
    return (0);
}
