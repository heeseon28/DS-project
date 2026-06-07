#include "ds/ScoreTree.h"
#include <iostream>

ScoreTree::ScoreTree() : root(nullptr), count(0) {}

ScoreTree::~ScoreTree() {
    clear();
}

void ScoreTree::insert(const ScoreRecord& record) {
    root = insertRecursive(root, record);
}

ScoreTree::Node* ScoreTree::insertRecursive(Node* current, const ScoreRecord& record) {
    if (current == nullptr) {
        ++count;
        return new Node(record);
    }

    if (record.score < current->record.score) {
        current->left = insertRecursive(current->left, record);
    } else {
        // 중복 점수는 오른쪽에 둔다. 같은 점수를 여러 명이 받아도 기록을 보존하기 위함이다.
        current->right = insertRecursive(current->right, record);
    }

    return current;
}

bool ScoreTree::containsPlayer(const std::string& playerName) const {
    return containsRecursive(root, playerName);
}

bool ScoreTree::containsRecursive(Node* current, const std::string& playerName) const {
    if (current == nullptr) {
        return false;
    }

    if (current->record.playerName == playerName) {
        return true;
    }

    return containsRecursive(current->left, playerName) ||
           containsRecursive(current->right, playerName);
}

void ScoreTree::printDescending() const {
    if (root == nullptr) {
        std::cout << "기록된 점수가 없습니다.\n";
        return;
    }
    printDescendingRecursive(root);
}

void ScoreTree::printDescendingRecursive(Node* current) const {
    if (current == nullptr) {
        return;
    }

    printDescendingRecursive(current->right);
    std::cout << current->record.playerName << ": " << current->record.score << "\n";
    printDescendingRecursive(current->left);
}

void ScoreTree::clearRecursive(Node* current) {
    if (current == nullptr) {
        return;
    }
    clearRecursive(current->left);
    clearRecursive(current->right);
    delete current;
}

int ScoreTree::size() const {
    return count;
}

bool ScoreTree::isEmpty() const {
    return count == 0;
}

void ScoreTree::clear() {
    clearRecursive(root);
    root = nullptr;
    count = 0;
}
