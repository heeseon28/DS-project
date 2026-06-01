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
};

#endif
