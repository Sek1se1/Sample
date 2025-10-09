#include <iostream>
#include <bits/stdc++.h>
#include <vector>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <chrono>
#include "algo.cpp"
using namespace std;
using namespace chrono;
using json = nlohmann::json;


string baseURL = "http://localhost:";
string token = "?token=player1";
string path = "3000";

struct HttpResult {
    long status = -1;
    string body;
    json json_body;
    string error;
};

// =============================
// 構造体定義
// =============================
struct FieldData {
    int size;
    vector<vector<int>> entities;
};

struct Problem {
    FieldData field;
};

struct Input {
    long long startsAt;
    Problem problem;
};

// =============================
// JSON → 構造体 変換関数
// =============================
void from_json(const json& j, FieldData& f) {
    f.size = j.at("size").get<int>();
    f.entities = j.at("entities").get<vector<vector<int>>>();
}

void from_json(const json& j, Problem& p) {
    p.field = j.at("field").get<FieldData>();
}

void from_json(const json& j, Input& in) {
    in.startsAt = j.at("startsAt").get<long long>();
    in.problem  = j.at("problem").get<Problem>();
}

// =============================
// libcurl コールバック
// =============================
static size_t write_cb(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* buf = static_cast<string*>(userdata);
    buf->append(ptr, size * nmemb);
    return size * nmemb;
}

// =============================
// HTTP GET関数
// =============================
string http_get(const string& url, long timeout_sec = 15) {
    CURL* curl = curl_easy_init();
    if (!curl) throw runtime_error("curl_easy_init failed");

    string body;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "field-client/1.0");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout_sec);

    CURLcode rc = curl_easy_perform(curl);
    if (rc != CURLE_OK) {
        string msg = string("curl GET error: ") + curl_easy_strerror(rc);
        curl_easy_cleanup(curl);
        throw runtime_error(msg);
    }

    long code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
    curl_easy_cleanup(curl);
    if (code < 200 || code >= 300) {
        throw runtime_error("HTTP GET status " + to_string(code));
    }
    return body;
}

// =============================
// get() 関数
// =============================


json get(const string& path) {
    string fullURL = baseURL + path + token;
    string text = http_get(fullURL);
    cout << text << endl;
    return json::parse(text);
}

// ====== POST ======
HttpResult post_json_soft(const json& payload, long timeout_sec = 15) {
    HttpResult r;

    CURL* curl = curl_easy_init();
    if (!curl) {
        r.error = "curl_easy_init failed";
        return r;
    }

    string fullURL = baseURL + path + token;
    string body    = payload.dump();

    curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    cout << fullURL << endl;
    //接続先設定
    curl_easy_setopt(curl, CURLOPT_URL, fullURL.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    //ヘッダー設定
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    //タイムアウト設定
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout_sec);
    //送信データ設定
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());    
    //おまじない
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    //レスポンス取得設定
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &r.body);

    //post
    CURLcode rc = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &r.status);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (rc != CURLE_OK) {
        r.error = string("curl POST error: ") + curl_easy_strerror(rc);
        return r;
    }

    try {
        if (!r.body.empty()) r.json_body = json::parse(r.body);
    } catch (const std::exception& e) {
        r.error = string("JSON parse error: ") + e.what();
    }

    return r;
}

void posting(json ans){
    // 3. サーバへPOST
    HttpResult r = post_json_soft(ans);

    // 4. レスポンス確認
    cout << "HTTP status: " << r.status << "\n";
    if (!r.error.empty()) {
        cerr << "Error: " << r.error << "\n";
    }
    if (!r.json_body.is_null()) {
        cout << "Response JSON:\n" << r.json_body.dump(2) << "\n";
    }

    return;
}

bool limitedProcess(int limitSeconds ,time_point<steady_clock> start) {
    auto now = steady_clock::now();
    auto elapsed = duration_cast<seconds>(now - start).count();
    if(limitSeconds < elapsed){
        cout << limitSeconds << "秒経過 → 処理を終了します。\n";
        return false;
    }else{
        this_thread::sleep_for(milliseconds(500)); // 負荷軽減
        return true;
    }  
}

// =============================
// main()
// =============================
int main() {
auto start = steady_clock::now();  // 開始時刻
int limit = 60;
Field field(1, 1); // 仮の初期化。あとで本物のサイズで上書き。
vector<vector<int>> model;
int bestAnswer = INT_MAX; //いい回答の手数を保持
vector<performance> bestHistory; //いい回答の内容を保持

try {
    // 1) GETでJSONを取得
    json j = get(path);

    // 2) JSON → 構造体
    Input data = j.get<Input>();
    const auto& g = data.problem.field.entities;

    // 3) 出力
    cout << "startsAt: " << data.startsAt << "\n";
    cout << "size: "     << data.problem.field.size << "\n";

    int size = data.problem.field.size;
    model = data.problem.field.entities;

    // 4) ここでFieldを作り直す
    field = Field(size, size);
    field.entity = model;

    // デバッグ出力（任意）
    cout << "rows: " << g.size() << ", cols: " << g[0].size() << "\n";
    cout << "entities:\n";
    for (const auto& row : g) {
        for (size_t x = 0; x < row.size(); ++x) {
            cout << row[x] << (x + 1 == row.size() ? '\n' : ' ');
        }
    }
}
catch (const exception& e) {
    cerr << "ERROR: " << e.what() << "\n";
}

    field.mainMove(); 
    if(field.stopIfPaired() == false){
        cout << "答えなし" << endl;
        exit(0);
    }
    bestAnswer = field.moveCount;
    bestHistory = field.history;
    posting(field.exportHistoryToJson(bestHistory));

    int tryTimes = 2;

while(limitedProcess(limit,start)){
    field.entity = model;
    field.moveCount = 0;
    field.history.clear();
    cout << "===" << tryTimes <<  "回目開始===\n";
    field.display();
    field.nextMove();
    field.mainMove();
    if(field.stopIfPaired() == false){
        cout << "答えなし" << endl;
        exit(0);
    }
    if(field.moveCount < bestAnswer){
        bestHistory = field.history;
        bestAnswer = field.moveCount;
    }
    tryTimes++;
}
cout << tryTimes << endl;
cout << bestAnswer << endl;
posting(field.exportHistoryToJson(bestHistory));
    
}