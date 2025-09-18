#include <bits/stdc++.h>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
using namespace std;

using json = nlohmann::json;

string http_post_json(const string& url, const string& payload, long timeout_sec=15){
    CURL* curl = curl_easy_init();
    if (!curl) throw runtime_error("curl_easy_init failed");
    string body;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "field-client/1.0");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout_sec);

    CURLcode rc = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    if (rc != CURLE_OK){
        string msg = string("curl POST error: ") + curl_easy_strerror(rc);
        curl_easy_cleanup(curl);
        throw runtime_error(msg);
    }
    long code=0; curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
    curl_easy_cleanup(curl);
    if (code < 200 || code >= 300) throw runtime_error("HTTP POST status " + to_string(code));
    return body;
}