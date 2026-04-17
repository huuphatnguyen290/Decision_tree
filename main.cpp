#include <iostream>

#include "LinkedTree.h"

void print() {
    std::cout << std::endl;
}

template<typename T, typename... Args> void print(T first, Args... args) {
    std::cout << first << " ";
    print(args...);
}

int main() {
    print("One","Two","Three");

    return 0;
}