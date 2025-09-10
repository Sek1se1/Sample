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
    void check(const int size) const {
        for(int x=0; x < size; x++){
            for(int y=0; y < size; y++){
                int cpa = entity.at(x).at(y);
                bool judge = false;
                if(x==0 && y==0){
                    if((cpa == entity.at(x+1).at(y)) || (cpa == entity.at(x).at(y+1))){
                        judge = true;
                    }
                }else if(x==0 && y==(size-1)){
                    if((cpa == entity.at(x+1).at(y)) || (cpa == entity.at(x).at(y-1))){
                        judge = true;
                    }
                }else if(x==0){
                    if((cpa == entity.at(x).at(y-1)) || (cpa == entity.at(x).at(y+1)) || (cpa == entity.at(x+1).at(y))){
                        judge = true;
                    }
                }else if(x==(size-1) && y==0){
                    if((cpa == entity.at(x-1).at(y)) || (cpa == entity.at(x).at(y+1))){
                        judge = true;
                    }
                }else if(y==0 ){
                    if((cpa == entity.at(x-1).at(y)) || (cpa == entity.at(x+1).at(y)) || (cpa == entity.at(x).at(y+1))){
                        judge = true;
                    }
                }else if(x==(size-1) && y==(size-1)){
                    if((cpa == entity.at(x).at(y-1)) || (cpa == entity.at(x-1).at(y))){
                        judge = true;
                    }
                }else if(x==(size-1)){
                    if((cpa == entity.at(x).at(y-1)) || (cpa == entity.at(x).at(y+1)) || (cpa == entity.at(x-1).at(y))){
                        judge = true;
                    }
                }else if(y==(size-1)){
                    if((cpa == entity.at(x-1).at(y)) || (cpa == entity.at(x+1).at(y)) || (cpa == entity.at(x).at(y-1))){
                        judge = true;
                    }
                }else{
                    if((cpa == entity.at(x+1).at(y)) || (cpa == entity.at(x-1).at(y)) || (cpa == entity.at(x).at(y+1)) || (cpa == entity.at(x).at(y-1))){
                        judge = true;
                    }
                }
                if(judge==false){
                    cout << "ペアになっていないエンティティがあります。" << endl;
                    return;
                }
                
            }
        }
        
        cout << "すべてのエンティティがペアになっています" << endl;

    }

    //手数をカウントする関数
    int getMoveCount() const { return moveCount; }

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
