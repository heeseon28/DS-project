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

// score보다 높은 노드 수를 셈. BST 성질 활용:
//   current->score > target  → right 서브트리는 전부 > target이므로 비교 없이 전체 카운트,
//                              left 서브트리는 일부만 해당할 수 있으므로 재귀.
//   current->score <= target → left 서브트리는 전부 <= target이므로 가지치기(탐색 불필요),
//                              right 서브트리만 재귀.
// 시간복잡도: 평균 O(h), right 전체 카운트 포함 최악 O(n).
int ScoreTree::countHigherRecursive(Node* current, int target) const {
    if (current == nullptr) return 0;
    if (current->record.score > target) {
        return 1 + countAllRecursive(current->right) + countHigherRecursive(current->left, target);
    }
    return countHigherRecursive(current->right, target);
}

// 서브트리의 전체 노드 수. O(n).
int ScoreTree::countAllRecursive(Node* current) const {
    if (current == nullptr) return 0;
    return 1 + countAllRecursive(current->left) + countAllRecursive(current->right);
}

// BST에서 가장 오른쪽 노드 = 최고 점수. O(h).
ScoreTree::Node* ScoreTree::findHighestNode(Node* current) const {
    if (current == nullptr || current->right == nullptr) return current;
    return findHighestNode(current->right);
}

// BST에서 가장 왼쪽 노드 = 최저 점수. O(h).
ScoreTree::Node* ScoreTree::findLowestNode(Node* current) const {
    if (current == nullptr || current->left == nullptr) return current;
    return findLowestNode(current->left);
}

// threshold 이상인 노드만 내림차순(right→root→left) 출력.
// current->score < threshold이면 left 서브트리 전부 < threshold → 가지치기. O(k + h).
void ScoreTree::printAboveScoreRecursive(Node* current, int threshold) const {
    if (current == nullptr) return;
    printAboveScoreRecursive(current->right, threshold);
    if (current->record.score >= threshold) {
        std::cout << current->record.playerName << ": " << current->record.score << "\n";
        printAboveScoreRecursive(current->left, threshold);
    }
}

int ScoreTree::getRank(int score) const {
    return countHigherRecursive(root, score) + 1;
}

ScoreRecord ScoreTree::findHighest() const {
    Node* node = findHighestNode(root);
    return node ? node->record : ScoreRecord();
}

ScoreRecord ScoreTree::findLowest() const {
    Node* node = findLowestNode(root);
    return node ? node->record : ScoreRecord();
}

void ScoreTree::printAboveScore(int threshold) const {
    if (root == nullptr) {
        std::cout << "기록된 점수가 없습니다.\n";
        return;
    }
    printAboveScoreRecursive(root, threshold);
}
