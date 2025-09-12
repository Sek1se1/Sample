#include <bits/stdc++.h>
using namespace std;

// 座標を表す構造体
struct Point {
    int x, y;
    Point(int x = 0, int y = 0) : x(x), y(y) {
    }
};

//フィールドクラスの定義
class Field {

    
    //園の指定
    //chatGPT曰く
    public:
    int row, col;
    vector<vector<int>> entity;
    int moveCount = 0;  // 手数カウント用
    
    public:
    Field(int r, int c) : row(r), col(c) {
        entity.resize(row, vector<int>(col, 0));
    }

    public:
    vector<vector<pair<int,int>>> Position;

    void init(int x, int y) {
        vector<pair<int, int>> K;
        for (int i = 0; i < x * y / 2; i++) {
            Position.push_back(K);
        }

        for (int i = 0; i < x; i++) {
            for (int j = 0; j < y; j++) {
                Position.at(entity.at(i).at(j)).push_back(make_pair(i, j));
            }
        }
    }
    
    // 導き（園を右回転）
    void rotateZone(int y, int x, int size) {
        // 境界チェック
        if (x + size > row || y + size > col) {
            cout << "範囲外です" << endl;
            return;
        }
        
        // 回転前の部分をコピー
        vector<vector<int>> zone(size, vector<int>(size));
        for (int i = 0; i < size; ++i)
        for (int j = 0; j < size; ++j)
        zone[i][j] = entity[x + i][y + j];
        
        // 右回転してフィールドに戻す
        for (int i = 0; i < size; ++i)
        for (int j = 0; j < size; ++j)
        entity[x + j][y + size - 1 - i] = zone[i][j];
        
        moveCount++;  // 手数カウント
    }
    //フィールドを表示する関数
    void display() const {
        for (const auto& Row : entity) {
            int max = Row.size();
            int repeat = 0;
            for (const auto& Ent : Row) {
                printf("%2d",Ent);
                //cout << Ent;
                repeat++;
                if (repeat != max) {
                    cout << " ";
                }
            }
            cout << endl;
        }
    }

    //すべてペアになっているか確認する関数
    //引数はフィールドの大きさ
    void check(int size) const {
    vector<pair<int,int>> dirs = {{1,0},{-1,0},{0,1},{0,-1}};
    for (int x = 0; x < size; x++) {
        for (int y = 0; y < size; y++) {
            int cpa = entity[x][y];
            bool judge = false;
            for (auto [dx, dy] : dirs) {
                int nx = x + dx, ny = y + dy;
                if (0 <= nx && nx < size && 0 <= ny && ny < size) {
                    if (entity[nx][ny] == cpa) {
                        judge = true;
                        break;
                    }
                }
            }
            if (!judge) {
                cout << "ペアになっていないエンティティがあります。" << endl;
                return;
            }
        }
    }
    cout << "すべてのエンティティがペアになっています" << endl;
}

    //手数をカウントする関数
    int getMoveCount() const { return moveCount; }

    // targetの位置を探す
vector<Point> findTarget(const Field& field, int target) {
    vector<Point> res;
    for (int i = 0; i < field.row; i++) {
        for (int j = 0; j < field.col; j++) {
            if (field.entity[i][j] == target) {
                res.push_back(Point(i,j));
            }
        }
    }
    return res;
}

// targetを隣接させる
void bringTogether(Field& field, int target) {
    auto pos = findTarget(field, target);
    if (pos.size() < 2) {
        cout << "対象が2つ未満です" << endl;
        return;
    }

    Point base = pos[0]; // 最初のターゲットを基準にする

    while (true) {
        pos = findTarget(field, target);
        // 隣同士になっていれば終了
        for (auto &p1 : pos) {
            for (auto &p2 : pos) {
                if (p1.x == p2.x && abs(p1.y - p2.y) == 1) {
                    cout << "横に隣接しました！" << endl;
                    return;
                }
                if (p1.y == p2.y && abs(p1.x - p2.x) == 1) {
                    cout << "縦に隣接しました！" << endl;
                    return;
                }
            }
        }

        // 基準以外の1つを基準の近くに動かす（右か下方向）
        Point targetPos = pos[1];
        if (targetPos.x > base.x && targetPos.y > base.y) {
            field.rotateZone(targetPos.y-1, targetPos.x-1, 2);
        } else if (targetPos.x > base.x) {
            field.rotateZone(max(0,targetPos.y-1), targetPos.x-1, 2);
        } else if (targetPos.y > base.y) {
            field.rotateZone(targetPos.y-1, max(0,targetPos.x-1), 2);
        } else {
            // 動かしづらい場合は少しランダムに動かす
            field.rotateZone(0,0,2);
        }

        field.display();
        cout << "---" << endl;
        }
    }

};


int main(){
    
    //テスト
    //フィールド1
    /*
    Field field(5, 5);
    vector<vector<int>> model = {
        {0,1,2,3,4},
        {5,6,7,8,9},
        {10,11,12,13,14},
        {15,16,17,18,19},
        {20,21,22,23,24}
    };
    */

    //フィールド2
    
    Field field(4,4);
    vector<vector<int>> model = {
        {0,1,2,3},
        {0,2,6,3},
        {4,1,5,7},
        {4,5,6,7},
    };
    

    field.entity = model;

    
    // 初期値設定
    //パターン1
    /*
    field.display();
    cout << "=== 回転後 ===" << endl;
    field.rotateZone(0, 1, 2); 
    field.display();

    field.check(4);

    cout << "手数: " << field.getMoveCount() << endl;
    */

    //パターン2
    field.init(4, 4);
    for (auto i : field.Position) {
        for (auto [a, b] : i) {
            cout << a << " " << b << " | ";
        }
        cout << endl;
    }
    field.display();
    cout << "=== 回転後 ===" << endl;
    field.rotateZone(1, 2, 2); 
    field.display();

    field.check(4);

    // cout << "手数: " << field.getMoveCount() << endl;
    
}
