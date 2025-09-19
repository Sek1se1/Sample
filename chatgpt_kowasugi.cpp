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

    // 固定済みセル（以後の回転で触ってはいけない）
    vector<Point> fixedCells;

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

    // 2x2 を右回転 (左上が (r,c) )
    // r: row index, c: col index
    bool rotateZone(int r, int c, int size = 2) {
        if (r < 0 || c < 0 || r + size > row || c + size > col) return false;
        // fixed cells に触れるかチェック（外部で呼ぶ場合は canRotateBlock を使うこと推奨）
        vector<vector<int>> zone(size, vector<int>(size));
        for (int i = 0; i < size; ++i)
            for (int j = 0; j < size; ++j)
                zone[i][j] = entity[r + i][c + j];
        for (int i = 0; i < size; ++i)
            for (int j = 0; j < size; ++j)
                entity[r + j][c + size - 1 - i] = zone[i][j];
        moveCount++;
        return true;
    }

    // 2x2ブロックに固定セルが含まれていれば回せない
    bool canRotateBlock(int r, int c) const {
        for (auto &p : fixedCells) {
            if (p.x >= r && p.x <= r+1 && p.y >= c && p.y <= c+1) return false;
        }
        return true;
    }

    // フィールド内の target の位置を返す
    vector<Point> locate(int target) const {
        vector<Point> res;
        for (int i = 0; i < row; ++i)
            for (int j = 0; j < col; ++j)
                if (entity[i][j] == target) res.emplace_back(i, j);
        return res;
    }

    // シリアライズ（BFSの visited 用）
    static string serializeBoard(const vector<vector<int>>& b) {
        string s;
        for (auto &r : b)
            for (auto v : r) {
                s += to_string(v);
                s.push_back(',');
            }
        return s;
    }

    // board 上で target が隣接（マンハッタン距離 = 1）なら true
    bool boardIsAdjacent(const vector<vector<int>>& b, int target) const {
        vector<Point> pos;
        for (int i = 0; i < row; ++i)
            for (int j = 0; j < col; ++j)
                if (b[i][j] == target) pos.emplace_back(i,j);
        if (pos.size() < 2) return false;
        for (size_t i = 0; i < pos.size(); ++i)
            for (size_t j = i+1; j < pos.size(); ++j) {
                int dx = abs(pos[i].x - pos[j].x);
                int dy = abs(pos[i].y - pos[j].y);
                if (dx + dy == 1) return true;
            }
        return false;
    }

    // ---------- BFS で target を隣接にする ----------
    bool bringTogether(int target) {
        struct Node {
            vector<vector<int>> board;
            vector<pair<int,int>> moves; // 回転履歴
            int heuristic;               // 評価値（距離）
        };

        auto heuristicFunc = [&](const vector<vector<int>> &b) {
            vector<Point> pos;
            for (int r = 0; r < row; ++r)
                for (int c = 0; c < col; ++c)
                    if (b[r][c] == target) pos.push_back(Point(r,c));
            if (pos.size() < 2) return 0; // ない or 1個だけなら意味なし
            return abs(pos[0].x - pos[1].x) + abs(pos[0].y - pos[1].y);
        };

        vector<Node> beam;
        unordered_set<string> visited;

        Node start{entity, {}, heuristicFunc(entity)};
        beam.push_back(start);
        visited.insert(serializeBoard(entity));

        const int BEAM_WIDTH = 50;
        int step = 0;

        while (!beam.empty() && step < 500) { // 安全のため最大手数制限
            ++step;
            vector<Node> next;

            for (auto &cur : beam) {
                if (boardIsAdjacent(cur.board, target)) {
                    // 成功
                    entity = cur.board;
                    moveCount += (int)cur.moves.size();
                    cout << "bringTogether(beam): 隣接達成 手順数=" << cur.moves.size() << "\n";
                    for (auto &m : cur.moves) cout << " rotate(" << m.first << "," << m.second << ")\n";
                    cout << "\n";
                    return true;
                }

                // 回転を試す
                for (int r = 0; r < row-1; ++r) {
                    for (int c = 0; c < col-1; ++c) {
                        if (!canRotateBlock(r, c)) continue;

                        auto nb = cur.board;
                        int tmp = nb[r][c];
                        nb[r][c] = nb[r+1][c];
                        nb[r+1][c] = nb[r+1][c+1];
                        nb[r+1][c+1] = nb[r][c+1];
                        nb[r][c+1] = tmp;

                        string key = serializeBoard(nb);
                        if (visited.count(key)) continue;
                        visited.insert(key);

                        Node nxt{nb, cur.moves, 0};
                        nxt.moves.emplace_back(r,c);
                        nxt.heuristic = heuristicFunc(nb);
                        next.push_back(std::move(nxt));
                    }
                }
            }

            // ヒューリスティックでソート（距離が小さいほど優先）
            sort(next.begin(), next.end(), [](auto &a, auto &b) {
                return a.heuristic < b.heuristic;
            });

            if ((int)next.size() > BEAM_WIDTH) next.resize(BEAM_WIDTH);
            beam = std::move(next);
        }

        cout << "bringTogether(beam): 隣接できませんでした\n";
        return false;
    }

    // ---------- ペア配置スロットの列挙（右上から左へ、行ごとに下へ） ----------
    vector<pair<Point,Point>> enumeratePairGoals() const {
        vector<pair<Point,Point>> goals;
        // row: 0..row-1, col: 0..col-1
        // 各行について右端から左へ2つずつ取る: (row, col-2),(row,col-1) then (row,col-4),(row,col-3) ...
        for (int r = 0; r < row; ++r) {
            for (int c = col - 2; c >= 0; c -= 2) {
                goals.push_back({Point(r, c), Point(r, c+1)});
            }
        }
        return goals;
    }

    // ---------- board 上で pairGoal に target が入っているか（順不同） ----------
    bool boardHasPairAt(const vector<vector<int>>& b, int target, pair<Point,Point> goal) const {
        Point a = goal.first, bb = goal.second;
        vector<Point> pos;
        for (int i=0;i<row;i++) for (int j=0;j<col;j++) if (b[i][j] == target) pos.emplace_back(i,j);
        if (pos.size() != 2) return false;
        // 両方が goal のいずれかと一致していれば OK
        bool pa = (pos[0] == a && pos[1] == bb) || (pos[1] == a && pos[0] == bb);
        return pa;
    }

    bool canRotateBlock(int x, int y) {
        for (auto &c : fixedCells) {
            if (c.x >= x && c.x <= x+1 && c.y >= y && c.y <= y+1)
                return false;
        }
        return true;
    }

    // ---------- 指定ターゲットのペアを右上の次の空きスロットに詰める ----------
    bool movePairToTopRight(int target) {
        // --- まず target を隣接にする ---
        auto curPos = locate(target);
        if (curPos.size() < 2) {
            cout << "movePairToTopRight: target が2個未満です\n";
            return false;
        }
        if (! (abs(curPos[0].x - curPos[1].x) + abs(curPos[0].y - curPos[1].y) == 1) ) {
            bool ok = bringTogether(target);
            if (!ok) return false;
        }

        // --- ゴールスロットを決める ---
        auto goals = enumeratePairGoals();
        pair<Point,Point> chosenGoal;
        bool foundGoal = false;
        for (auto &g : goals) {
            bool occupied = false;
            for (auto &fc : fixedCells)
                if (fc == g.first || fc == g.second) { occupied = true; break; }
            if (!occupied) { chosenGoal = g; foundGoal = true; break; }
        }
        if (!foundGoal) {
            cout << "movePairToTopRight: 空きゴールスロットがありません\n";
            return false;
        }

        // --- ビームサーチ ---
        struct Node {
            vector<vector<int>> board;
            vector<pair<int,int>> moves;
            int score; // ヒューリスティック: 2個のtargetのマンハッタン距離合計
        };
        auto heuristic = [&](const vector<vector<int>> &b)->int {
            vector<Point> pos;
            for (int i=0;i<row;i++) for (int j=0;j<col;j++)
                if (b[i][j]==target) pos.emplace_back(i,j);
            if (pos.size()!=2) return 1e9;
            int d1 = abs(pos[0].x - chosenGoal.first.x) + abs(pos[0].y - chosenGoal.first.y)
                + abs(pos[1].x - chosenGoal.second.x) + abs(pos[1].y - chosenGoal.second.y);
            int d2 = abs(pos[1].x - chosenGoal.first.x) + abs(pos[1].y - chosenGoal.first.y)
                + abs(pos[0].x - chosenGoal.second.x) + abs(pos[0].y - chosenGoal.second.y);
            return min(d1,d2);
        };

        int beamWidth = 200;       // ★ビーム幅（調整可能）
        int maxDepth  = 100;       // ★手数制限

        vector<Node> beam;
        beam.push_back({entity, {}, heuristic(entity)});

        unordered_set<string> visited;
        visited.insert(serializeBoard(entity));

        for (int depth = 0; depth < maxDepth; ++depth) {
            vector<Node> next;

            for (auto &cur : beam) {
                if (boardHasPairAt(cur.board, target, chosenGoal)) {
                    entity = cur.board;
                    fixedCells.push_back(chosenGoal.first);
                    fixedCells.push_back(chosenGoal.second);
                    moveCount += (int)cur.moves.size();
                    cout << "movePairToTopRight(beam): 配置成功 手順数=" << cur.moves.size() << "\n";
                    for (auto &m : cur.moves) cout << " rotate(" << m.first << "," << m.second << ")\n";
                    cout << "\n";
                    return true;
                }

                // 次の回転候補
                for (int r = 0; r < row-1; ++r) {
                    for (int c = 0; c < col-1; ++c) {
                        if (!canRotateBlock(r,c)) continue;

                        auto nb = cur.board;
                        int tmp = nb[r][c];
                        nb[r][c] = nb[r+1][c];
                        nb[r+1][c] = nb[r+1][c+1];
                        nb[r+1][c+1] = nb[r][c+1];
                        nb[r][c+1] = tmp;

                        string key = serializeBoard(nb);
                        if (visited.count(key)) continue;
                        visited.insert(key);

                        auto nm = cur.moves;
                        nm.emplace_back(r,c);
                        next.push_back({nb, nm, heuristic(nb)});
                    }
                }
            }

            if (next.empty()) break;
            // スコア順にソートし、上位 beamWidth 件だけ残す
            sort(next.begin(), next.end(), [](auto &a, auto &b){return a.score < b.score;});
            if ((int)next.size() > beamWidth) next.resize(beamWidth);
            beam.swap(next);
        }

        cout << "movePairToTopRight(beam): 解が見つかりませんでした\n";
        return false;
    }

    // ペアが全部揃っているか確認
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
    // 2x2回転できるか（固定セルを壊さないか）
    
};


int main() {
    Field field(6,6);
    vector<vector<int>> model = {
        { 15,  6,  0,  3, 11,  7},
        { 16,  2,  8,  4, 17,  1},
        { 13,  5,  7, 14, 10,  3},
        {  6, 16, 12,  0,  9,  2},
        {  1,  9,  4, 13, 15,  8},
        { 12, 14, 17, 10,  5, 11}
    };
    field.entity = model;

    cout << "=== 初期状態 ===\n";
    field.display();

    // 例: 数字 1 のペアを作って右上へ詰める（成功すれば固定される）
    for (int i = 0; i < 13; i++) {
        if (field.bringTogether(i)) {
            field.movePairToTopRight(i);
        } else {
            cout << i << " のペア化に失敗\n";
        }
        cout << "=== 中間状態 ===\n";
        field.display();
    }

    cout << "=== 最終状態 ===\n";
    field.display();

    field.checkAllPaired();
    cout << "総手数: " << field.moveCount << "\n";

    return 0;
}
