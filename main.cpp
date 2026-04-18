#include <iostream>
#include <string>

#include "TreeBuilder.h"

int main() {
    std::string filename;
    std::cout << "Enter tree file name: ";
    std::cin >> filename;

    LinkedTree<std::string>* tree = TreeBuilder::buildFromFile(filename);
    if (!tree) {
        std::cerr << "Failed to build tree from file: " << filename << std::endl;
        return 1;
    }

    // Generate the analysis report
    TreeBuilder::generateReport(tree, "about_tree.txt");

    // Interactive node exploration
    TreeBuilder::interactiveExplore(tree);

    delete tree;
    return 0;
}
