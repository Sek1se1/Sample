#include <iostream>
#include <vector>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

// ====== 設定 ======
string baseURL = "http://localhost:8080";   // コンテストのサーバURL
string token   = "?token=YOUR_TOKEN";       // 認証トークン

// ====== レスポンス格納用 ======
struct HttpResult {
    long status = -1;
    string body;
    json json_body;
    string error;
};

// ====== libcurl コールバック ======
static size_t write_cb(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* buf = static_cast<string*>(userdata);
    buf->append(ptr, size * nmemb);
    return size * nmemb;
}

// ====== POST ======
HttpResult post_json_soft(const string& path, const json& payload, long timeout_sec = 15) {
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

// ====== 解答用構造体 ======
struct Op { int x,y,n; };

void to_json(json& j, const Op& o) {
    j = json{{"x", o.x}, {"y", o.y}, {"n", o.n}};
}

// ====== main ======
int main() {
    // 1. アルゴリズムで解いた手を用意
    vector<Op> ops;
    ops.push_back({0,0,2});
    ops.push_back({2,2,2});

    // 2. JSON に変換
    json req;
    req["ops"] = ops;

    cout << "送信するJSON:\n" << req.dump(2) << "\n";

    // 3. サーバへPOST
    HttpResult r = post_json_soft("/answer", req);

    // 4. レスポンス確認
    cout << "HTTP status: " << r.status << "\n";
    if (!r.error.empty()) {
        cerr << "Error: " << r.error << "\n";
    }
    if (!r.json_body.is_null()) {
        cout << "Response JSON:\n" << r.json_body.dump(2) << "\n";
    }

    return 0;
}
