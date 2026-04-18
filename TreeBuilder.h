#ifndef TREEBUILDER_H
#define TREEBUILDER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

#include "LinkedTree.h"

// Holds data for one node parsed from the input file
struct NodeEntry {
    int level;
    int pos;
    std::string edgeLabel;
    std::string content;
};

class TreeBuilder {
public:
    // Read tree from file and build a LinkedTree
    static LinkedTree<std::string>* buildFromFile(std::string filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cout << "Error: Could not open file " << filename << std::endl;
            return nullptr;
        }

        std::vector<NodeEntry> entries;
        std::string line;

        while (std::getline(file, line)) {
            if (line.empty()) continue;
            // Remove Windows carriage return if present
            if (line.back() == '\r') line.pop_back();

            NodeEntry e;
            std::istringstream iss(line);
            iss >> e.level >> e.pos;

            if (e.level > 0)
                iss >> e.edgeLabel;
            else
                e.edgeLabel = "";

            std::getline(iss, e.content);
            // Remove leading space before content
            if (!e.content.empty() && e.content[0] == ' ')
                e.content = e.content.substr(1);

            if (!e.content.empty())
                entries.push_back(e);
        }
        file.close();

        // Sort by preorder position so parent tracking works correctly
        std::sort(entries.begin(), entries.end(), [](NodeEntry a, NodeEntry b) {
            return a.pos < b.pos;
        });

        LinkedTree<std::string>* tree = new LinkedTree<std::string>();
        std::map<int, Node<std::string>*> lastAtLevel;

        for (int i = 0; i < entries.size(); i++) {
            NodeEntry e = entries[i];
            Node<std::string>* newNode = new Node<std::string>(e.content, e.edgeLabel);

            if (e.level == 0) {
                tree->setRoot(newNode);
            } else {
                lastAtLevel[e.level - 1]->addChild(newNode);
            }
            lastAtLevel[e.level] = newNode;
        }

        return tree;
    }

    // Print tree with dash indentation: level n = 2*n dashes
    static void printTree(Node<std::string>* node, int level, std::ostream& out) {
        if (node == nullptr) return;

        if (level == 0) {
            out << node->data << "\n";
        } else {
            std::string dashes(2 * level, '-');
            out << dashes << "[" << node->edge_label << "] " << node->data << "\n";
        }

        for (int i = 0; i < node->children.size(); i++)
            printTree(node->children[i], level + 1, out);
    }

    // Collect internal nodes in preorder
    static void getInternalNodes(Node<std::string>* node, std::vector<std::string>& result) {
        if (node == nullptr) return;
        if (node->children.size() > 0)
            result.push_back(node->data);
        for (int i = 0; i < node->children.size(); i++)
            getInternalNodes(node->children[i], result);
    }

    // Collect leaf nodes in preorder
    static void getLeafNodes(Node<std::string>* node, std::vector<std::string>& result) {
        if (node == nullptr) return;
        if (node->children.size() == 0)
            result.push_back(node->data);
        for (int i = 0; i < node->children.size(); i++)
            getLeafNodes(node->children[i], result);
    }

    // Check if every node has at most 2 children
    static bool isBinaryTree(Node<std::string>* node) {
        if (node == nullptr) return true;
        if (node->children.size() > 2) return false;
        for (int i = 0; i < node->children.size(); i++)
            if (!isBinaryTree(node->children[i])) return false;
        return true;
    }

    // Check if every internal node has exactly 2 children
    static bool isProperTree(Node<std::string>* node) {
        if (node == nullptr) return true;
        if (node->children.size() != 0 && node->children.size() != 2) return false;
        for (int i = 0; i < node->children.size(); i++)
            if (!isProperTree(node->children[i])) return false;
        return true;
    }

    // Get height of a subtree (number of edges, 0 for a leaf)
    static int getHeight(Node<std::string>* node) {
        if (node == nullptr || node->children.size() == 0) return 0;
        int maxH = 0;
        for (int i = 0; i < node->children.size(); i++) {
            int h = getHeight(node->children[i]) + 1;
            if (h > maxH) maxH = h;
        }
        return maxH;
    }

    // Check if all leaves are at expectedLevel (used for perfect tree check)
    static bool isPerfectTree(Node<std::string>* node, int expectedLevel, int currentLevel) {
        if (node == nullptr) return true;
        if (node->children.size() == 0)
            return currentLevel == expectedLevel;
        if (node->children.size() != 2) return false;
        return isPerfectTree(node->children[0], expectedLevel, currentLevel + 1) &&
               isPerfectTree(node->children[1], expectedLevel, currentLevel + 1);
    }

    // Check if subtree heights differ by at most 1 for every node
    static bool isBalancedTree(Node<std::string>* node) {
        if (node == nullptr || node->children.size() == 0) return true;

        int minH = getHeight(node->children[0]);
        int maxH = getHeight(node->children[0]);

        for (int i = 1; i < node->children.size(); i++) {
            int h = getHeight(node->children[i]);
            if (h < minH) minH = h;
            if (h > maxH) maxH = h;
        }

        if (maxH - minH > 1) return false;

        for (int i = 0; i < node->children.size(); i++)
            if (!isBalancedTree(node->children[i])) return false;

        return true;
    }

    // Find a node by preorder position (1-based), currentPos starts at 0
    static Node<std::string>* findByPosition(Node<std::string>* node, int targetPos, int& currentPos) {
        if (node == nullptr) return nullptr;
        currentPos++;
        if (currentPos == targetPos) return node;
        for (int i = 0; i < node->children.size(); i++) {
            Node<std::string>* found = findByPosition(node->children[i], targetPos, currentPos);
            if (found != nullptr) return found;
        }
        return nullptr;
    }

    // Get one sibling of a node (first sibling that isn't the node itself)
    static Node<std::string>* getSibling(Node<std::string>* node) {
        if (node == nullptr || node->parent == nullptr) return nullptr;
        for (int i = 0; i < node->parent->children.size(); i++) {
            if (node->parent->children[i] != node)
                return node->parent->children[i];
        }
        return nullptr;
    }

    // Write about_tree.txt with visualization, properties, and binary analysis
    static void generateReport(LinkedTree<std::string>* tree, std::string outputFile) {
        Node<std::string>* root = tree->getRoot();
        if (root == nullptr) {
            std::cout << "Error: Tree is empty." << std::endl;
            return;
        }

        std::ofstream out(outputFile);
        if (!out.is_open()) {
            std::cout << "Error: Cannot open output file." << std::endl;
            return;
        }

        // A. Tree Visualization
        out << "=== Tree Visualization ===\n";
        printTree(root, 0, out);
        out << "\n";

        // B. Tree Properties
        int total = tree->countNodes(root);
        int leaves = tree->countLeaves(root);
        int internal = total - leaves;
        int treeHeight = getHeight(root);

        std::vector<std::string> internalNodes, leafNodes;
        getInternalNodes(root, internalNodes);
        getLeafNodes(root, leafNodes);

        out << "=== Tree Properties ===\n";
        out << "Root node: " << root->data << "\n";
        out << "Number of internal nodes: " << internal << "\n";
        out << "Number of external (leaf) nodes: " << leaves << "\n";
        out << "Tree height: " << treeHeight << "\n";

        out << "Internal nodes (preorder): ";
        for (int i = 0; i < internalNodes.size(); i++) {
            if (i > 0) out << ", ";
            out << internalNodes[i];
        }
        out << "\n";

        out << "External nodes (preorder): ";
        for (int i = 0; i < leafNodes.size(); i++) {
            if (i > 0) out << ", ";
            out << leafNodes[i];
        }
        out << "\n\n";

        // C. Binary Tree Analysis
        out << "=== Binary Tree Analysis ===\n";
        bool binary = isBinaryTree(root);
        out << "Is it a Binary Tree? " << (binary ? "Yes" : "No") << "\n";

        if (binary) {
            out << "Proper Tree: " << (isProperTree(root) ? "Yes" : "No") << "\n";
            out << "Perfect Tree: " << (isPerfectTree(root, treeHeight, 0) ? "Yes" : "No") << "\n";
            out << "Balanced Tree: " << (isBalancedTree(root) ? "Yes" : "No") << "\n";
        }

        out.close();
        std::cout << "Report written to: " << outputFile << std::endl;
    }

    // Let user explore nodes by preorder position
    static void interactiveExplore(LinkedTree<std::string>* tree) {
        Node<std::string>* root = tree->getRoot();
        if (root == nullptr) return;

        int totalNodes = tree->countNodes(root);
        std::string input;

        while (true) {
            std::cout << "\nWhich node would you like to explore (enter position or \"exit\"): ";
            std::cin >> input;

            if (input == "exit") {
                std::cout << "Goodbye!" << std::endl;
                break;
            }

            // Check that input is all digits
            bool valid = true;
            for (int i = 0; i < input.size(); i++) {
                if (!isdigit(input[i])) {
                    valid = false;
                    break;
                }
            }

            if (!valid || stoi(input) < 1 || stoi(input) > totalNodes) {
                std::cout << "Invalid input. Please try again." << std::endl;
                continue;
            }

            int pos = stoi(input);
            int currentPos = 0;
            Node<std::string>* node = findByPosition(root, pos, currentPos);

            std::cout << "Node's content: " << node->data << "\n";

            std::cout << "Ancestor: ";
            if (node->parent != nullptr) std::cout << node->parent->data;
            else std::cout << "None (this is the root)";
            std::cout << "\n";

            std::cout << "Descendant: ";
            if (node->children.size() > 0) std::cout << node->children[0]->data;
            else std::cout << "None (this is a leaf)";
            std::cout << "\n";

            std::cout << "Sibling: ";
            Node<std::string>* sibling = getSibling(node);
            if (sibling != nullptr) std::cout << sibling->data;
            else std::cout << "None";
            std::cout << "\n";
        }
    }
};

#endif
