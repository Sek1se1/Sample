#include <bits/stdc++.h>
using namespace std;

struct Point {
    int x, y;
    Point(int x_ = 0, int y_ = 0) : x(x_), y(y_) {}
    bool operator==(const Point& o) const { return x==o.x && y==o.y; }
};

class Field {
public:
    int row, col;
    vector<vector<int>> entity;
    int moveCount = 0;
    vector<Point> fixedCells; // 固定済みセル

    Field(int r, int c) : row(r), col(c) {
        entity.assign(row, vector<int>(col, 0));
    }

    void display() const {
        for (int i = 0; i < row; ++i) {
            for (int j = 0; j < col; ++j) {
                printf("%2d", entity[i][j]);
                if (j+1 != col) cout << " ";
            }
            cout << "\n";
        }
        cout << "\n";
    }

    // 2x2 右回転（左上が(r,c)）
    static void rotateBoard2x2(vector<vector<int>>& b, int r, int c) {
        int tmp = b[r][c];
        b[r][c] = b[r+1][c];
        b[r+1][c] = b[r+1][c+1];
        b[r+1][c+1] = b[r][c+1];
        b[r][c+1] = tmp;
    }

    bool canRotateBlock(int r, int c) const {
        for (auto &p : fixedCells) {
            if (p.x >= r && p.x <= r+1 && p.y >= c && p.y <= c+1) return false;
        }
        return true;
    }

    vector<Point> locate(int target, const vector<vector<int>>& b) const {
        vector<Point> res;
        for (int i=0;i<row;i++) for (int j=0;j<col;j++) if (b[i][j]==target) res.emplace_back(i,j);
        return res;
    }

    static string serializeBoard(const vector<vector<int>>& b) {
        string s;
        for (auto &r : b) for (auto v : r) { s += to_string(v); s.push_back(','); }
        return s;
    }

    // ---------- ヒューリスティック関数 ----------
    // 2個のタイルのマンハッタン距離 D に対して隣接までに必要な最短回転数の下界:
    // h = ceil((D-1)/2) が安全だが簡潔に D/2 (整数除算) を使う (同等)
    static int heuristicPairToAdjacent(const vector<Point>& pos) {
        if (pos.size() < 2) return 0;
        int D = abs(pos[0].x - pos[1].x) + abs(pos[0].y - pos[1].y);
        return D / 2; // 整数除算：ceil((D-1)/2) と同等の過小評価しない簡易式
    }

    // ゴール (2セル) へ移動する際のヒューリスティック:
    // 2つのタイルと 2つのゴール座標の最小割当の合計マンハッタン距離を計算し、
    // 1回の回転で合計距離を最大2変えられることを考慮して ceil(sum/2)
    static int heuristicPairToGoal(const vector<Point>& pos, const pair<Point,Point>& goal) {
        if (pos.size() != 2) return 0;
        int d1 = abs(pos[0].x - goal.first.x) + abs(pos[0].y - goal.first.y)
               + abs(pos[1].x - goal.second.x) + abs(pos[1].y - goal.second.y);
        int d2 = abs(pos[0].x - goal.second.x) + abs(pos[0].y - goal.second.y)
               + abs(pos[1].x - goal.first.x) + abs(pos[1].y - goal.first.y);
        int best = min(d1, d2);
        return (best + 1) / 2; // ceil(best/2)
    }

    // ---------- A*: target を隣接にする ----------
    bool bringTogetherAstar(int target, int max_expand = 200000) {
        struct Node {
            vector<vector<int>> board;
            vector<pair<int,int>> moves;
            int g, f;
            // f = g + h
            bool operator<(const Node& other) const { return f > other.f; } // for min-heap
        };

        auto initial = entity;
        string initKey = serializeBoard(initial);

        priority_queue<Node> pq;
        unordered_map<string,int> bestG; // visited best g

        int h0 = 0;
        auto pos0 = locate(target, initial);
        if (pos0.size() >= 2) h0 = heuristicPairToAdjacent(pos0);

        pq.push({initial, {}, 0, 0 + h0});
        bestG[initKey] = 0;

        int expanded = 0;
        while (!pq.empty()) {
            Node cur = pq.top(); pq.pop();
            expanded++;
            if (expanded > max_expand) {
                cerr << "bringTogetherAstar: expand limit reached\n";
                return false;
            }

            // goal?
            if (boardIsAdjacent(cur.board, target)) {
                entity = cur.board;
                moveCount += cur.g;
                cout << "bringTogetherAstar: 成功 moves=" << cur.g << "\n";
                for (auto &m : cur.moves) cout << " rotate(" << m.first << "," << m.second << ")\n";
                cout << "\n";
                return true;
            }

            // expand neighbors
            for (int r = 0; r < row-1; ++r) {
                for (int c = 0; c < col-1; ++c) {
                    if (!canRotateBlock(r, c)) continue;
                    auto nb = cur.board;
                    rotateBoard2x2(nb, r, c);
                    string key = serializeBoard(nb);
                    int ng = cur.g + 1;
                    if (bestG.find(key) != bestG.end() && bestG[key] <= ng) continue;
                    bestG[key] = ng;

                    auto pos = locate(target, nb);
                    int h = heuristicPairToAdjacent(pos);
                    Node nxt{nb, cur.moves, ng, ng + h};
                    nxt.moves.emplace_back(r, c);
                    pq.push(nxt);
                }
            }
        }

        cout << "bringTogetherAstar: 解なし\n";
        return false;
    }

    // 補助: board 上で target が隣接しているか
    bool boardIsAdjacent(const vector<vector<int>>& b, int target) const {
        vector<Point> pos;
        for (int i=0;i<row;i++) for (int j=0;j<col;j++) if (b[i][j]==target) pos.emplace_back(i,j);
        if (pos.size() < 2) return false;
        for (size_t i=0;i<pos.size(); ++i)
            for (size_t j=i+1;j<pos.size(); ++j) {
                int D = abs(pos[i].x - pos[j].x) + abs(pos[i].y - pos[j].y);
                if (D == 1) return true;
            }
        return false;
    }

    // ---------- ペアゴール列挙（右上から左へ、行ごとに下へ） ----------
    vector<pair<Point,Point>> enumeratePairGoals() const {
        vector<pair<Point,Point>> goals;
        for (int r = 0; r < row; ++r) {
            for (int c = col - 2; c >= 0; c -= 2) {
                goals.push_back({Point(r, c), Point(r, c+1)});
            }
        }
        return goals;
    }

    bool boardHasPairAt(const vector<vector<int>>& b, int target, const pair<Point,Point>& goal) const {
        vector<Point> pos;
        for (int i=0;i<row;i++) for (int j=0;j<col;j++) if (b[i][j]==target) pos.emplace_back(i,j);
        if (pos.size() != 2) return false;
        auto a = goal.first, bb = goal.second;
        return (pos[0]==a && pos[1]==bb) || (pos[1]==a && pos[0]==bb);
    }

    // ---------- A* : ペアを指定ゴール (右上の次の空き) に移動 ----------
    bool movePairToTopRightAstar(int target, int max_expand = 500000) {
        // ensure pair adjacent first
        auto curPos = locate(target, entity);
        if (curPos.size() < 2) {
            cerr << "movePairToTopRightAstar: target fewer than 2\n";
            return false;
        }
        if (!(abs(curPos[0].x - curPos[1].x) + abs(curPos[0].y - curPos[1].y) == 1)) {
            if (!bringTogetherAstar(target, max_expand/4)) return false;
        }

        // find an empty goal pair slot (not overlapping fixedCells)
        auto goals = enumeratePairGoals();
        pair<Point,Point> chosen;
        bool found = false;
        for (auto &g : goals) {
            bool occupied = false;
            for (auto &fc : fixedCells) {
                if (fc == g.first || fc == g.second) { occupied = true; break; }
            }
            if (occupied) continue;
            chosen = g; found = true; break;
        }
        if (!found) { cerr << "movePairToTopRightAstar: no free goal\n"; return false; }

        // A* search: state = board
        struct Node {
            vector<vector<int>> board;
            vector<pair<int,int>> moves;
            int g, f;
            bool operator<(const Node& o) const { return f > o.f; }
        };

        priority_queue<Node> pq;
        unordered_map<string,int> bestG;
        string initKey = serializeBoard(entity);
        int h0 = heuristicPairToGoal(locate(target, entity), chosen);
        pq.push({entity, {}, 0, 0 + h0});
        bestG[initKey] = 0;

        int expanded = 0;
        while (!pq.empty()) {
            Node cur = pq.top(); pq.pop();
            expanded++;
            if (expanded > max_expand) { cerr << "movePairToTopRightAstar: expand limit\n"; return false; }

            if (boardHasPairAt(cur.board, target, chosen)) {
                // success: adopt board, fix cells
                entity = cur.board;
                fixedCells.push_back(chosen.first);
                fixedCells.push_back(chosen.second);
                moveCount += cur.g;
                cout << "movePairToTopRightAstar: 成功 moves=" << cur.g << "\n";
                for (auto &m : cur.moves) cout << " rotate(" << m.first << "," << m.second << ")\n";
                cout << "\n";
                return true;
            }

            for (int r = 0; r < row-1; ++r) {
                for (int c = 0; c < col-1; ++c) {
                    if (!canRotateBlock(r, c)) continue;
                    auto nb = cur.board;
                    rotateBoard2x2(nb, r, c);
                    string key = serializeBoard(nb);
                    int ng = cur.g + 1;
                    if (bestG.find(key) != bestG.end() && bestG[key] <= ng) continue;
                    bestG[key] = ng;
                    int h = heuristicPairToGoal(locate(target, nb), chosen);
                    Node nxt{nb, cur.moves, ng, ng + h};
                    nxt.moves.emplace_back(r, c);
                    pq.push(nxt);
                }
            }
        }

        cerr << "movePairToTopRightAstar: 解なし\n";
        return false;
    }

    // 全体チェック
    void checkAllPaired() const {
        vector<pair<int,int>> dirs = {{1,0},{-1,0},{0,1},{0,-1}};
        for (int x = 0; x < row; ++x) {
            for (int y = 0; y < col; ++y) {
                int v = entity[x][y];
                bool ok = false;
                for (auto [dx,dy] : dirs) {
                    int nx = x + dx, ny = y + dy;
                    if (0 <= nx && nx < row && 0 <= ny && ny < col) {
                        if (entity[nx][ny] == v) { ok = true; break; }
                    }
                }
                if (!ok) {
                    cout << "ペアになっていないエンティティがあります。\n";
                    return;
                }
            }
        }
        cout << "すべてのエンティティがペアになっています\n";
    }
};

int main() {
    Field field(4,4);
    vector<vector<int>> model = {
        {0,1,2,3},
        {0,2,6,3},
        {4,1,5,7},
        {4,5,6,7},
    };
    field.entity = model;

    cout << "=== 初期状態 ===\n";
    field.display();

    for (int i = 0; i < field.row * field.col / 2; i++) {
        if (field.bringTogetherAstar(i)) {
            field.movePairToTopRightAstar(i);
            cout << "=== 中間状態 ===\n";
            field.display();
        }
    }

    cout << "=== 最終状態 ===\n";
    field.display();

    field.checkAllPaired();
    cout << "総手数: " << field.moveCount << "\n";
    return 0;
}
