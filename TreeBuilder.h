#ifndef TREEBUILDER_H
#define TREEBUILDER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <climits>

#include "LinkedTree.h"

// Holds data parsed from one line of the input file before tree construction
struct NodeEntry {
    int level;
    int preorderPos;
    std::string edgeLabel;
    std::string content;
};

class TreeBuilder {
public:
    // Parse one line with format: "level preorderPos [edgeLabel] content"
    // Root lines (level 0) have no edge label field.
    static bool parseLine(const std::string& line, NodeEntry& entry) {
        std::istringstream iss(line);
        if (!(iss >> entry.level >> entry.preorderPos)) return false;

        entry.edgeLabel = "";
        if (entry.level > 0) {
            if (!(iss >> entry.edgeLabel)) return false;
        }

        std::getline(iss, entry.content);
        size_t start = entry.content.find_first_not_of(" \t");
        entry.content = (start == std::string::npos) ? "" : entry.content.substr(start);

        return !entry.content.empty();
    }

    // Read a tree file and construct a LinkedTree<string>.
    // Lines are sorted by preorder position before building so that
    // the simple "most recent ancestor at level L-1" parent rule works
    // even when the file lists nodes out of strict preorder sequence.
    static LinkedTree<std::string>* buildFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Cannot open file " << filename << std::endl;
            return nullptr;
        }

        std::vector<NodeEntry> entries;
        std::string line;
        while (std::getline(file, line)) {
            // Strip Windows-style carriage return if present
            if (!line.empty() && line.back() == '\r')
                line.pop_back();
            if (line.empty()) continue;

            NodeEntry entry;
            if (parseLine(line, entry))
                entries.push_back(entry);
        }
        file.close();

        if (entries.empty()) return nullptr;

        // Sort ascending by preorder position so parent tracking is correct
        std::sort(entries.begin(), entries.end(),
            [](const NodeEntry& a, const NodeEntry& b) {
                return a.preorderPos < b.preorderPos;
            });

        LinkedTree<std::string>* tree = new LinkedTree<std::string>();
        // Maps each level to the most recently inserted node at that level
        std::map<int, Node<std::string>*> lastAtLevel;

        for (const NodeEntry& e : entries) {
            Node<std::string>* newNode = new Node<std::string>(e.content, e.edgeLabel);

            if (e.level == 0) {
                tree->setRoot(newNode);
            } else {
                auto parentIt = lastAtLevel.find(e.level - 1);
                if (parentIt != lastAtLevel.end())
                    parentIt->second->addChild(newNode);
            }
            lastAtLevel[e.level] = newNode;
        }

        return tree;
    }

    // ----------------------------------------------------------------
    // Visualization
    // ----------------------------------------------------------------

    // Recursively print the tree using dash-based indentation.
    // Level n uses 2*n dashes, followed by [edgeLabel] and node content.
    // Root (level 0) is printed without dashes or edge-label brackets.
    static void printVisualization(Node<std::string>* node, int level,
                                   std::ostream& out) {
        if (!node) return;

        if (level == 0) {
            out << node->data << "\n";
        } else {
            out << std::string(2 * level, '-')
                << "[" << node->edge_label << "] "
                << node->data << "\n";
        }

        for (Node<std::string>* child : node->children)
            printVisualization(child, level + 1, out);
    }

    // ----------------------------------------------------------------
    // Node collection helpers (preorder)
    // ----------------------------------------------------------------

    // Collect internal nodes (nodes that have at least one child) in preorder
    static void collectInternalNodes(Node<std::string>* node,
                                     std::vector<std::string>& result) {
        if (!node) return;
        if (!node->children.empty())
            result.push_back(node->data);
        for (Node<std::string>* child : node->children)
            collectInternalNodes(child, result);
    }

    // Collect external (leaf) nodes in preorder
    static void collectExternalNodes(Node<std::string>* node,
                                     std::vector<std::string>& result) {
        if (!node) return;
        if (node->children.empty())
            result.push_back(node->data);
        for (Node<std::string>* child : node->children)
            collectExternalNodes(child, result);
    }

    // ----------------------------------------------------------------
    // Binary tree classification helpers
    // ----------------------------------------------------------------

    // Returns true if every node has at most 2 children
    static bool isBinaryTree(Node<std::string>* node) {
        if (!node) return true;
        if (node->children.size() > 2) return false;
        for (Node<std::string>* child : node->children)
            if (!isBinaryTree(child)) return false;
        return true;
    }

    // Returns true if every internal node has exactly 2 children (proper/full binary tree)
    static bool isProperTree(Node<std::string>* node) {
        if (!node) return true;
        if (!node->children.empty() && node->children.size() != 2) return false;
        for (Node<std::string>* child : node->children)
            if (!isProperTree(child)) return false;
        return true;
    }

    // Edge-based height: number of edges on the longest root-to-leaf path.
    // Returns 0 for a leaf node.
    static int subtreeHeight(Node<std::string>* node) {
        if (!node || node->children.empty()) return 0;
        int maxH = 0;
        for (Node<std::string>* child : node->children)
            maxH = std::max(maxH, subtreeHeight(child) + 1);
        return maxH;
    }

    // Returns true if the binary tree is perfect:
    // all internal nodes have exactly 2 children and all leaves are at expectedLeafLevel.
    static bool isPerfectTree(Node<std::string>* node, int expectedLeafLevel,
                               int currentLevel) {
        if (!node) return true;
        if (node->children.empty())
            return currentLevel == expectedLeafLevel;
        if (node->children.size() != 2) return false;
        return isPerfectTree(node->children[0], expectedLeafLevel, currentLevel + 1) &&
               isPerfectTree(node->children[1], expectedLeafLevel, currentLevel + 1);
    }

    // Returns true if the tree is height-balanced:
    // for every node the difference between its tallest and shortest child subtree
    // heights is at most 1.
    static bool isBalancedTree(Node<std::string>* node) {
        if (!node || node->children.empty()) return true;

        int minH = INT_MAX, maxH = 0;
        for (Node<std::string>* child : node->children) {
            int h = subtreeHeight(child);
            if (h < minH) minH = h;
            if (h > maxH) maxH = h;
        }
        if (maxH - minH > 1) return false;

        for (Node<std::string>* child : node->children)
            if (!isBalancedTree(child)) return false;
        return true;
    }

    // ----------------------------------------------------------------
    // Node lookup
    // ----------------------------------------------------------------

    // Find a node by its preorder position (1-based).
    // currentPos must be initialised to 0 before the first call.
    static Node<std::string>* findByPosition(Node<std::string>* node,
                                              int targetPos, int& currentPos) {
        if (!node) return nullptr;
        currentPos++;
        if (currentPos == targetPos) return node;
        for (Node<std::string>* child : node->children) {
            Node<std::string>* found = findByPosition(child, targetPos, currentPos);
            if (found) return found;
        }
        return nullptr;
    }

    // Return one sibling of node (the first sibling in the parent's child list
    // that is not node itself), or nullptr if none exists.
    static Node<std::string>* getSibling(Node<std::string>* node) {
        if (!node || !node->parent) return nullptr;
        for (Node<std::string>* sibling : node->parent->children)
            if (sibling != node) return sibling;
        return nullptr;
    }

    // ----------------------------------------------------------------
    // Report generation
    // ----------------------------------------------------------------

    // Write about_tree.txt containing:
    //   A. Tree Visualization
    //   B. Tree Properties
    //   C. Binary Tree Analysis
    static void generateReport(LinkedTree<std::string>* tree,
                                const std::string& outputFile) {
        Node<std::string>* root = tree->getRoot();
        if (!root) {
            std::cerr << "Error: Tree is empty, cannot generate report." << std::endl;
            return;
        }

        std::ofstream out(outputFile);
        if (!out.is_open()) {
            std::cerr << "Error: Cannot write to " << outputFile << std::endl;
            return;
        }

        // A. Tree Visualization
        out << "=== Tree Visualization ===\n";
        printVisualization(root, 0, out);
        out << "\n";

        // B. Tree Properties
        int totalNodes   = tree->countNodes(root);
        int leafCount    = tree->countLeaves(root);
        int internalCount = totalNodes - leafCount;
        int treeHeight   = subtreeHeight(root);

        std::vector<std::string> internalNodes, externalNodes;
        collectInternalNodes(root, internalNodes);
        collectExternalNodes(root, externalNodes);

        out << "=== Tree Properties ===\n";
        out << "Root node: " << root->data << "\n";
        out << "Number of internal nodes: " << internalCount << "\n";
        out << "Number of external (leaf) nodes: " << leafCount << "\n";
        out << "Tree height: " << treeHeight << "\n";

        out << "Internal nodes (preorder): ";
        for (size_t i = 0; i < internalNodes.size(); i++) {
            if (i > 0) out << ", ";
            out << internalNodes[i];
        }
        out << "\n";

        out << "External nodes (preorder): ";
        for (size_t i = 0; i < externalNodes.size(); i++) {
            if (i > 0) out << ", ";
            out << externalNodes[i];
        }
        out << "\n\n";

        // C. Binary Tree Analysis
        out << "=== Binary Tree Analysis ===\n";
        bool binary = isBinaryTree(root);
        out << "Is it a Binary Tree? " << (binary ? "Yes" : "No") << "\n";

        if (binary) {
            bool proper   = isProperTree(root);
            bool perfect  = isPerfectTree(root, treeHeight, 0);
            bool balanced = isBalancedTree(root);
            out << "Proper Tree: "   << (proper   ? "Yes" : "No") << "\n";
            out << "Perfect Tree: "  << (perfect  ? "Yes" : "No") << "\n";
            out << "Balanced Tree: " << (balanced ? "Yes" : "No") << "\n";
        }

        out.close();
        std::cout << "Report written to: " << outputFile << "\n";
    }

    // ----------------------------------------------------------------
    // Interactive exploration
    // ----------------------------------------------------------------

    // Prompt the user to enter a preorder position and display
    // the node's content, one ancestor, one descendant, and one sibling.
    static void interactiveExplore(LinkedTree<std::string>* tree) {
        Node<std::string>* root = tree->getRoot();
        if (!root) return;

        int totalNodes = tree->countNodes(root);
        std::string input;

        while (true) {
            std::cout << "\nWhich node would you like to explore (enter position or \"exit\"): ";
            std::cin >> input;

            if (input == "exit") {
                std::cout << "Goodbye!" << std::endl;
                break;
            }

            // Validate: input must be a non-empty string of digits
            bool isNumber = !input.empty();
            for (char c : input)
                if (!isdigit(c)) { isNumber = false; break; }

            if (!isNumber) {
                std::cout << "Invalid input. Please try again." << std::endl;
                continue;
            }

            int pos = std::stoi(input);
            if (pos < 1 || pos > totalNodes) {
                std::cout << "Invalid input. Please try again." << std::endl;
                continue;
            }

            int currentPos = 0;
            Node<std::string>* node = findByPosition(root, pos, currentPos);
            if (!node) {
                std::cout << "Invalid input. Please try again." << std::endl;
                continue;
            }

            std::cout << "Node's content: " << node->data << "\n";

            std::cout << "Ancestor: ";
            if (node->parent) std::cout << node->parent->data;
            else std::cout << "None (this is the root)";
            std::cout << "\n";

            std::cout << "Descendant: ";
            if (!node->children.empty()) std::cout << node->children[0]->data;
            else std::cout << "None (this is a leaf)";
            std::cout << "\n";

            std::cout << "Sibling: ";
            Node<std::string>* sibling = getSibling(node);
            if (sibling) std::cout << sibling->data;
            else std::cout << "None";
            std::cout << "\n";
        }
    }
};

#endif
