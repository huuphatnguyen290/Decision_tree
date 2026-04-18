#ifndef LINKEDTREE_H
#define LINKEDTREE_H

#include <iostream>
// Read file and put them into the stream
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

#include "Node.h"

template <typename T> class LinkedTree {
    private:
        Node<T>* root;
    public:
        LinkedTree() : root(nullptr) {}

        // Destructor: deletes root which recursively deletes all children via Node destructor
        ~LinkedTree() { delete root; }

        // Set the root of the tree
        void setRoot(Node<T>* r) { root = r; }

        // Get the root of the tree
        Node<T>* getRoot() const { return root; }

        /*  Preorder Tree Traversal for multiple nodes
            - O(n) time complexities */ 
        void preorder(Node<T>* node) {
            // If node is nullptr, return nothing
            if (!node)
                return;

            // Else, print out node's data
            std::cout << node->data << " ";

            // Traverse to that node's children, from left to right
            for (int i = 0; i < node->children.size(); i++) {
                preorder(node->children[i]);
            }
        }

        // Count the number of nodes: root, internal/external nodes
        int countNodes(Node<T>* node) {
            if (!node) 
                return 0;
            
            int count = 1;
            for (int i = 0; i < node->children.size(); i++)
                count += countNodes(node->children[i]);

            return count;
        }
        // Count only external nodes (nodes with no children)
        int countLeaves(Node<T>* node) {
            if (!node) 
                return 0;
            if (node->children.size() == 0)
                return 1;

            
            int count = 0;
            for (int i = 0; i < node->children.size(); i++) {
                count += countLeaves(node->children[i]);
                
            }

            return count;
        }
        // Count the height of the tree
        int height(Node<T>* node) {
            // Invalid, return -1
            if (!node)
                return -1;
            
            // If there is a node, but no edge, return 0 
            int maxHeight = 0;
            for (int i = 0; i < node->children.size(); i++) {
                maxHeight = std::max(maxHeight, height(node->children[i]));
            }

            return maxHeight + 1;
        }
};

#endif