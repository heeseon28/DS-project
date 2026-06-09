#ifndef SCORE_TREE_H
#define SCORE_TREE_H

#include <string>

struct ScoreRecord {
    std::string playerName;
    int score;

    ScoreRecord(const std::string& playerName = "", int score = 0)
        : playerName(playerName), score(score) {}
};

class ScoreTree {
private:
    struct Node {
        ScoreRecord record;
        Node* left;
        Node* right;

        Node(const ScoreRecord& record) : record(record), left(nullptr), right(nullptr) {}
    };

    Node* root;
    int count;

    Node* insertRecursive(Node* current, const ScoreRecord& record);
    bool containsRecursive(Node* current, const std::string& playerName) const;
    void printDescendingRecursive(Node* current) const;
    void clearRecursive(Node* current);

    int countHigherRecursive(Node* current, int score) const;
    int countAllRecursive(Node* current) const;
    Node* findHighestNode(Node* current) const;
    Node* findLowestNode(Node* current) const;
    void printAboveScoreRecursive(Node* current, int threshold) const;

    ScoreTree(const ScoreTree& other) = delete;
    ScoreTree& operator=(const ScoreTree& other) = delete;

public:
    ScoreTree();
    ~ScoreTree();

    void insert(const ScoreRecord& record);
    bool containsPlayer(const std::string& playerName) const;
    void printDescending() const;
    int size() const;
    bool isEmpty() const;
    void clear();

    // score보다 높은 기록 수 + 1. O(h) 평균, 단 right 서브트리 전체 카운트 시 O(n) 최악.
    int getRank(int score) const;
    // 가장 높은/낮은 점수 레코드 반환. O(h).
    ScoreRecord findHighest() const;
    ScoreRecord findLowest() const;
    // threshold 이상 기록만 내림차순 출력. threshold 미만 left 서브트리는 가지치기. O(k + h).
    void printAboveScore(int threshold) const;
};

#endif
