#include <iostream>
#include<vector>
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
    
    // 導き（園を右回転）
    void rotateZone(int x, int y, int size) {
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
                cout << Ent;
                repeat++;
                if (repeat != max) {
                    cout << " ";
                }
            }
            cout << endl;
        }
    }
    
    // void display() const {
    //     for (const auto& Row : entity) {
    //         for (const auto& Ent : Row) {
    //             cout << Ent << " ";
    //         }
    //         cout << endl;
    //     }
    // }
    // 一応残していますが、別に使わなくてもおｋ

    int getMoveCount() const { return moveCount; }


};

// int main() {
//     //例として4*4のフィールドfieldを初期化
//     Field field(4, 4);
    
//     //適当に配置
//     vector<vector<int>> model = {
//         {0,1,2,3},
//         {4,5,6,7},
//         {0,1,2,3},
//         {4,5,6,7}
//     };

    
//     field.entity = model;
//     field.display();
//     cout << endl;
//     field.rotateZone(0, 0, 2);
//     field.display();

//     // cout << "modelフィールド" << endl;
    
// }


int main(){
    Field field(5, 5);

    vector<vector<int>> model = {
        {0,1,2,3,4},
        {5,6,7,8,9},
        {10,11,12,13,14},
        {15,16,17,18,19},
        {20,21,22,23,24}
    };

    field.entity = model;

    
    // 初期値設定（テスト用）
    int num = 1;
    for (int i = 0; i < 5; ++i)
    for (int j = 0; j < 5; ++j)
    field.rotateZone(0, 0, 5);  // 回転して変化を見るため仮に使用
    
    field.display();

        // 2×2の園を (1,1) を左上として回転
    field.rotateZone(1, 1, 2);

    cout << "=== 回転後 ===" << endl;
    field.display();

    cout << "手数: " << field.getMoveCount() << endl;

    
    
}
