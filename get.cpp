#include <bits/stdc++.h>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
using namespace std;

using json = nlohmann::json;

// 構造体の定義
struct Field { int size; vector<vector<int>> entities; };
struct Problem { Field field; };
struct Input { long long startsAt; Problem problem; };

// 構造体の変換
void from_json(const json& j, Field& f) {
    f.size = j.at("size").get<int>();
    f.entities = j.at("entities").get<vector<vector<int>>>();
}
void from_json(const json& j, Problem& p) { p.field = j.at("field").get<Field>(); }
void from_json(const json& j, Input& in) {
    in.startsAt = j.at("startsAt").get<long long>();
    in.problem  = j.at("problem").get<Problem>();
}

//libcurl 送受信コールバック(HTTP -> string)
static size_t write_cb(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* buf = static_cast<string*>(userdata);
    buf->append(ptr, size * nmemb);
    return size * nmemb;
}

// ---- HTTP GET ----
string http_get(const string& url, long timeout_sec = 15) {
    CURL* curl = curl_easy_init();
    if (!curl) throw runtime_error("curl_easy_init failed");

    string body;
    //接続先指定
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    //
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    //レスポンス保存先設定
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "field-client/1.0");
    //タイムアウト設定
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout_sec);

    //接続
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

// get()関数
string baseURL = 
string 
string

json get(const string& path){
    string fullURL = baseURL+path+token;
    string text = http_get(fullURL);
    return json::parse(text);
}

// main
int main() {
    try {
        // 1) GET
        string text = http_get(url);

        // 文字列 -> JSON
        json j = json::parse(text);

        //　JSON -> C++構造体
        Input data = j.get<Input>();
        const auto& g = data.problem.field.entities;

        // 出力
        cout << "startsAt: " << data.startsAt << "\n";
        cout << "size: "     << data.problem.field.size << "\n";
        if (!g.empty()) {
            cout << "rows: " << g.size()
                      << ", cols: " << g[0].size() << "\n";
            cout << "entities:\n";
            for (const auto& row : g) {
                for (size_t x = 0; x < row.size(); ++x) {
                    cout << row[x] << (x + 1 == row.size() ? '\n' : ' ');
                }
            }
        }
    } catch (const exception& e) {
        cerr << "ERROR: " << e.what() << "\n";
        return 1;
    }
    return 0;
}