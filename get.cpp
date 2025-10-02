#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
using namespace std;
using json = nlohmann::json;

// ===== 構造体 =====
struct Field { int size; vector<vector<int>> entities; };
struct Problem { Field field; };
struct Input { long long startsAt; Problem problem; };

// ===== JSON -> 構造体 変換 =====
void from_json(const json& j, Field& f) {
    f.size     = j.at("size").get<int>();
    f.entities = j.at("entities").get<vector<vector<int>>>();
}
void from_json(const json& j, Problem& p) { p.field = j.at("field").get<Field>(); }
void from_json(const json& j, Input& in) {
    in.startsAt = j.at("startsAt").get<long long>();
    in.problem  = j.at("problem").get<Problem>();
}

// ===== libcurl コールバック =====
static size_t write_cb(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* buf = static_cast<string*>(userdata);
    buf->append(ptr, size * nmemb);
    return size * nmemb;
}

// ===== HTTP GET（文字列で返す） =====
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
    if (code < 200 || code >= 300) throw runtime_error("HTTP GET status " + to_string(code));
    return body;
}

// ===== ここを埋める（接続先と get() 関数）=====
// サーバのベースURL（procon36_server の -listen に合わせる）
static const string baseURL = "http://127.0.0.1:3000";
// トークンの付け方（クエリで渡す場合）
static const string token_qs = "?token=player1";
// 取得するパス（スクショでは GET "/"）
static const string problem_path = "/";

// JSON を返す get()（path は "/","/status" 等を想定）
json get(const string& path) {
    // クエリで token を付ける。既にクエリがあるなら "&token=..." にしてね
    const string fullURL = baseURL + path + token_qs;
    string text = http_get(fullURL);
    return json::parse(text);
}

// ===== main =====
int main() {
    try {
        // 1) GET "/"
        json j = get(problem_path);

        // 2) JSON -> C++ 構造体
        Input data = j.get<Input>();
        const auto& g = data.problem.field.entities;

        // 3) 出力（確認用）
        cout << "startsAt: " << data.startsAt << "\n";
        cout << "size: "     << data.problem.field.size << "\n";
        if (!g.empty()) {
            cout << "rows: " << g.size()
                 << ", cols: " << g[0].size() << "\n";
            cout << "entities:\n";
            for (const auto& row : g) {
                for (size_t x = 0; x < row.size(); ++x)
                    cout << row[x] << (x + 1 == row.size() ? '\n' : ' ');
            }
        }
    } catch (const exception& e) {
        cerr << "ERROR: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
