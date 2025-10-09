#include <bits/stdc++.h>
#include <nlohmann/json.hpp>
#include <random>
using namespace std;
using json = nlohmann::json;

// 座標を表す構造体
struct Point {
    int x, y;
    Point(int x = 0, int y = 0) : x(x), y(y) {
    }
};

//回転履歴をまとめる構造体
struct performance {
  int x;//x座標
  int y;//y座標
  int n;//園の大きさ
  
  // constructor
  performance(int x, int y, int n)
      : x(x), y(y), n(n){}
};


//フィールドクラスの定義
class Field {

    
    //園の指定
    //chatGPT曰く
    public:
    int row, col;
    vector<vector<int>> entity;
    int moveCount = 0;  // 手数カウント用
    vector<performance> history; //回転履歴を格納する配列
    
    public:
    Field(int r, int c) : row(r), col(c) {
        entity.resize(row, vector<int>(col, 0));
    }

    public:
    vector<vector<pair<int,int>>> Position;

    // 便利関数：全てペアなら処理終了
    bool stopIfPaired() {
        if (check(row)) {
            finalField();
            return true;
        }
        return false;
    }
    
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
            cout << "えらー" << endl;
            exit(0);
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

        cout << "rotate(" << y << "," << x <<"," << size << ")"<<endl; 
        history.push_back(performance(y,x,size));  
        display();
    }

    //フィールドを表示する関数
    void display() const {
        for (const auto& Row : entity) {
            int max = Row.size();
            int repeat = 0;
            for (const auto& Ent : Row) {
                printf("%3d",Ent);
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
    bool check(int size) const {
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
                return false;
            }
        }
    }
    cout << "すべてのエンティティがペアになっています" << endl;
    return true;
    
}

    //手数をカウントする関数
    int getMoveCount() const { return moveCount; }

    //相方データの座標を取得
Point findTarget(int target, int excludeX, int excludeY) {
    for (int y = 0; y < row; y++) {
        for (int x = 0; x < col; x++) {
            if (entity[y][x] == target) {
                // 自分自身の座標はスキップ
                if (x == excludeX && y == excludeY) continue;
                 cout << "target(" << x << "," << y << ")=" << target <<endl;
                return Point(x, y);
               
            }
        }
    }
    return Point(-1, -1);
}

//動かす
void moveTarget(Point target, int x,int y,int count){
    //1.すでに目的のペアの形
    if((target.x==x&&target.y==y+1)){
        cout << "目的のペアの形です" << endl;
        cout << "ペアにするまでの総手数:" << count << endl;
        count = 0;
        return;
    }//2.すでに目的のペアの形の一手前
    else if(target.x==x-1&&target.y==y){
        cout << "目的のペアの一手前の形です" << endl;
        rotateZone(target.x,target.y,2);
        count++;
        cout << "目的のペアの形です" << endl;
        cout << "ペアにするまでの総手数:" << count << endl;
        count = 0;
        return;
    }//3.targetのxが相方のx-2以下かつtargetのyが相方のyのとき
    else if(target.x < x-1 && target.y == y){
        if((x-target.x)<=(row-target.y)) rotateZone(target.x,target.y,(x-target.x));
        else rotateZone(target.x,target.y,(row - y));
        count++;
        if(stopIfPaired()) return;
        moveTarget(findTarget(entity[y][x],x,y),x,y,count);
    }//4.targetのxが相方のx-1かつtargetのyが相方のy未満のとき
    else if(target.x == x-1 && target.y < y){
        if((target.x+1) >= (y-target.y+1)) rotateZone(target.x-(y-target.y),target.y,((y-target.y)+1));
        else rotateZone(0,target.y,target.x+1);
        count++;
        if(stopIfPaired()) return;
        moveTarget(findTarget(entity[y][x],x,y),x,y,count);
    }//5.targetのxが相方のx-1未満かつtargetのyが相方のy未満
    else if(target.x < x-1&&target.y<y){
        if((x-target.x)<=(row-target.y)) rotateZone(target.x,target.y,(x-target.x));
        else rotateZone(target.x,target.y,(row-target.y));
        count++;
        if(stopIfPaired()) return;
        moveTarget(findTarget(entity[y][x],x,y),x,y,count);
    }//6.targetのxが相方のx未満かつtargetのyが相方のy+1
    else if(target.x < x && target.y == y+1){
        if(row-y>2){
            if((x-target.x+1)<=(row-y-1)) rotateZone(target.x,target.y,(x-target.x+1));
            else rotateZone(target.x,target.y,(row-y-1));
            count++;
            if(stopIfPaired()) return;
            moveTarget(findTarget(entity[y][x],x,y),x,y,count);
        }
        else{
            if(target.x==x-1&&target.y==y+1){
                rotateZone(target.x-1,target.y-1,2);
                count++;
                if(stopIfPaired()) return;
                moveTarget(findTarget(entity[y][x],x,y),x,y,count);
            }
            else{
                rotateZone(target.x,target.y-(x-target.x)+1,x-target.x);
                count++;
                if(stopIfPaired()) return;
                moveTarget(findTarget(entity[y][x],x,y),x,y,count);
            }
        }
    }//7.targetのxが相方のxかつtargetのyが相方のy+2以上の時
    else if(target.x==x&&target.y > y+1){
        if(target.x+1>=target.y-y) rotateZone(x-(target.y-y)+1,y+1,target.y-y);
        else rotateZone(0,target.y-target.x,target.x+1);
        count++;
        if(stopIfPaired()) return;
        moveTarget(findTarget(entity[y][x],x,y),x,y,count);
    }//8.targetのxが相方のx未満かつtargetのyが相方のy+2以上の時
    else if(target.x<x&&target.y>y+1){
        if(x-target.x+1 <= target.y-y) rotateZone(target.x,target.y-(x-target.x),x-target.x+1);
        else rotateZone(target.x,y+1,target.y-y);
        count++;
        if(stopIfPaired()) return;
        moveTarget(findTarget(entity[y][x],x,y),x,y,count);
    }
    else{
        cout << "なんかえらー" << endl;
        cout << target.x << "," << target.y << "," << y << "," << x << endl; 
    }
}

void semilastMove(Point target, int x,int y,int count){
    if(y > 3){
        if(target.x==x+1&&target.y==y ){
        cout << "目的のペアの一手前の形です" << endl;
        rotateZone(target.x-1,target.y-1,2);
        count++;
        if (stopIfPaired()) return;
        semilastMove(findTarget(entity[y][x],x,y),x,y,count);
        }
        else if(target.x==1&&target.y==col-1){
            if(x==1){
                cout << "ペアにできないのでリセット(x=1)" << endl;
                rotateZone(x-1,y-1,2);
                count++;
                if (stopIfPaired()) return;
                rotateZone(x-1,y-1,2);
                count++;
                if (stopIfPaired()) return;
                rotateZone(x-1,y-2,2);
                count++;
                if (stopIfPaired()) return;
                rotateZone(x-1,y-1,2);
                count++;
                if (stopIfPaired()) return;
                rotateZone(x-1,y-1,2);
                count++;
                if (stopIfPaired()) return;
                rotateZone(x-1,y-1,2);
                count++;
                if (stopIfPaired()) return;
                semilastMove(findTarget(entity[y][x],x,y),x,y,count);
            }else if(x==0){
                cout << "ペアにできないのでリセット(x=0)" << endl;
                rotateZone(x,y-1,2);
                count++;
                if (stopIfPaired()) return;
                rotateZone(x,y-2,2);
                count++;
                if (stopIfPaired()) return;
                rotateZone(x,y-1,2);
                count++;
                if (stopIfPaired()) return;
                rotateZone(x,y-1,2);
                count++;
                if (stopIfPaired()) return;
                rotateZone(x,y-1,2);
                count++;
                if (stopIfPaired()) return;
                semilastMove(findTarget(entity[y][x],x,y),x,y,count);
            }
        }
        else if(target.x==x&&target.y==y-1){
            cout << "目的のペアの形です" << endl;
            cout << "ペアにするまでの総手数:" << count << endl;
            count = 0;
            return;
        }else if(target.x==0){
            if(x==1&&target.y==y-1){
                rotateZone(x-1,y-2,2);
                count++;
                if (stopIfPaired()) return;
                rotateZone(x-1,y-2,2);
                count++;
                if (stopIfPaired()) return;
                rotateZone(x-1,y-2,2);
                count++;
                if (stopIfPaired()) return;
                semilastMove(findTarget(entity[y][x],x,y),x,y,count);
            }else{
                rotateZone(target.x,target.y,2);
                count++;
                if (stopIfPaired()) return;
                semilastMove(findTarget(entity[y][x],x,y),x,y,count);
            }
            
        }else if(target.x==1){
            if(x==1){
                rotateZone(target.x-1,target.y,2);
                count++;
                if (stopIfPaired()) return;
                semilastMove(findTarget(entity[y][x],x,y),x,y,count);
            }else if(x==0&&target.x==x+1&&target.y==y-1){
                rotateZone(target.x-1,target.y-1,2);
                count++;
                if (stopIfPaired()) return;
                semilastMove(findTarget(entity[y][x],x,y),x,y,count);
            }else if(x==0){
                rotateZone(target.x-1,target.y,2);
                count++;
                if (stopIfPaired()) return;
                semilastMove(findTarget(entity[y][x],x,y),x,y,count);
            }

            
        }
    }
}

void lastMove(int count){
    if(row >4){
        rotateZone(2,0,row-4);
        rotateZone(1,3,row-3);
    }
}

void final(Point target, int x,int y,int count){
    if(target.x==x+1&&target.y==y){
        cout << "目的のペアの一手前の形です" << endl;
        rotateZone(target.x-1,target.y-1,2);
        count++;
        if (stopIfPaired()) return;
        final(findTarget(entity[y][x],x,y),x,y,count);
    }else if(target.x==1&&target.y==3){
        cout << "ペアにできないのでリセット(x=1)" << endl;
                rotateZone(x-1,y-1,2);
                count++;
                if (stopIfPaired()) return;
                rotateZone(x-1,y-1,2);
                count++;
                if (stopIfPaired()) return;
                rotateZone(x-1,y-2,2);
                count++;
                if (stopIfPaired()) return;
                rotateZone(x-1,y-1,2); 
                count++;
                if (stopIfPaired()) return;
                rotateZone(x-1,y-1,2);
                count++;
                if (stopIfPaired()) return;
                rotateZone(x-1,y-1,2);
                count++;
                if (stopIfPaired()) return;
                final(findTarget(entity[y][x],x,y),x,y,count);
    }
    else if(target.x==x&&target.y==y-1){
            cout << "目的のペアの形です" << endl;
            cout << "ペアにするまでの総手数:" << count << endl;
            count = 0;
            return;
        }else if(target.x==0){
            if(x==1&&target.y==y-1){
                rotateZone(x-1,y-2,2);
                count++;
                if (stopIfPaired()) return;
                rotateZone(x-1,y-2,2);
                count++;
                if (stopIfPaired()) return;
                rotateZone(x-1,y-2,2);
                count++;
                if (stopIfPaired()) return;
                final(findTarget(entity[y][x],x,y),x,y,count);
            }else{
                rotateZone(target.x,target.y,2);
                count++;
                if (stopIfPaired()) return;
                final(findTarget(entity[y][x],x,y),x,y,count);
            }
            
        }else if(target.x==1){
            if(x==1){
                rotateZone(target.x-1,target.y,2);
                count++;
                if (stopIfPaired()) return;
                final(findTarget(entity[y][x],x,y),x,y,count);
            }else if(x==0&&target.x==x+1&&target.y==y-1){
                rotateZone(target.x-1,target.y-1,2);
                count++;
                if (stopIfPaired()) return;
                final(findTarget(entity[y][x],x,y),x,y,count);
            }else if(x==0){
                rotateZone(target.x-1,target.y,2);
                count++;
                if (stopIfPaired()) return;
                final(findTarget(entity[y][x],x,y),x,y,count);
            }

            
        }
}

void finalCace(int count){
    if(entity[1][2]!=entity[1][3]){
        rotateZone(2,0,2);
        count++;
        if (stopIfPaired()) return;
    }
    rotateZone(1,1,3);
    count++;
    if (stopIfPaired()) return;
}

void finalField(){
    cout << "=== 最終状態 ===\n";
    display();
    cout << "総手数:" << getMoveCount() << endl;
    return;
}

nlohmann::json exportHistoryToJson(const vector<performance>& his) {
    nlohmann::json ans;
    ans["ops"] = nlohmann::json::array();  // ops配列を作る

    for (const auto& h : his) {
        ans["ops"].push_back({
            {"x", h.x},
            {"y", h.y},
            {"n", h.n}
        });
    }
    cout << ans << endl;
    return ans;
}
void mainMove() {
    cout << "=== 初期状態 ===\n";
    if (stopIfPaired()) return;
    display();
    for (int i = row - 1; i > 1; i--) {
        for (int j = 0; j < col; j += 2) {
            moveTarget(findTarget(entity[j][i], i, j), i, j, 0);
             if (stopIfPaired()) return;
            cout << "=== 現在の状態 ===\n";
            display();
        }
    }

    cout << "=== 残り二列 ===\n";

    int semiX = 0;
    for (int j = row - 1; j > 3; j--) {
        cout << semiX << "," << j << endl;
        semilastMove(findTarget(entity[j][semiX], semiX, j), semiX, j, 0);
        if (stopIfPaired()) return;
        cout << "=== 現在の状態 ===\n";
        display();
        if (semiX == 0) {
            semiX = 1;
        } else if (semiX == 1) {
            semiX = 0;
        }
    }

    cout << "=== 残り8マス ===\n";

    lastMove(0);
    if (stopIfPaired()) return; 
    cout << "=== 現在の状態 ===\n";
    display();

    final(findTarget(entity[3][0], 0, 3), 0, 3, 0);
    if (stopIfPaired()) return;
    cout << "=== 現在の状態 ===\n";
    display();

    final(findTarget(entity[2][1], 1, 2), 1, 2, 1);
    if (stopIfPaired()) return;
    cout << "=== 現在の状態 ===\n";
    display();

    if (entity[0][0] == entity[3][1]) {
        rotateZone(0, 2, 2);
        if (stopIfPaired()) return; 
        final(findTarget(entity[3][0], 0, 3), 0, 3, 0);
        if (stopIfPaired()) return;
        cout << "=== 現在の状態 ===\n";
        display();

        final(findTarget(entity[2][1], 1, 2), 1, 2, 1);
        if (stopIfPaired()) return; 
        cout << "=== 現在の状態 ===\n";
        display();
    }

    finalCace(0);
    if (stopIfPaired()) return;
}

void nextMove() {
    mt19937 rng(random_device{}());  

    int t = max(1, row / 6);
    uniform_int_distribution<int> scaleDist(max(2, row / 3), row - 2);
    uniform_int_distribution<int> placeDist(0, row - 1);

    for (int i = 0; i < t; i++) {
        int s = scaleDist(rng);
        int pX, pY;

        // 範囲内が出るまで再生成
        while (true) {
            pX = placeDist(rng);
            pY = placeDist(rng);
            if (pX + s <= row && pY + s <= col) break;
        }

        cout << "=== " << i + 1 << " 回転目 ===\n";
        rotateZone(pX, pY, s);

        if (stopIfPaired()) return;
    }
}
};