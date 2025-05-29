#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <curl/curl.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>

const std::string API_KEY = "A0YyUPYTdTv7dPY9B71LRjMnnA3Kp26hvcJb8VlYbne5V7hz2fgpFNnbJmhK2OrG";
const std::string API_SECRET = "3PNJPuSOEha4SJ3M82xeCvfLvcDocvsc3f1DGucxQaV9PebN5MlZzHObQmNoNmHW";

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void check_public_latency() {
    CURL* curl = curl_easy_init();
    if (curl) {
        std::string readBuffer;
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.binance.com/api/v3/ping");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        auto start = std::chrono::high_resolution_clock::now();
        CURLcode res = curl_easy_perform(curl);
        auto end = std::chrono::high_resolution_clock::now();

        if (res == CURLE_OK) {
            auto latency = std::chrono::duration<double, std::milli>(end - start).count();
            std::cout << "Public API Latency: " << latency << " ms" << std::endl;
        } else {
            std::cerr << "Failed to reach public API: " << curl_easy_strerror(res) << std::endl;
        }
        curl_easy_cleanup(curl);
    }
}

std::string hmac_sha256(const std::string& key, const std::string& data) {
    unsigned char* digest;
    digest = HMAC(EVP_sha256(), key.c_str(), key.length(),
                  reinterpret_cast<const unsigned char*>(data.c_str()), data.length(), NULL, NULL);

    char mdString[65];
    for (int i = 0; i < 32; i++)
        sprintf(&mdString[i * 2], "%02x", (unsigned int)digest[i]);
    mdString[64] = 0;

    return std::string(mdString);
}

void check_private_latency() {
    CURL* curl = curl_easy_init();
    if (curl) {
        std::string readBuffer;

        long long timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        std::string query_string = "timestamp=" + std::to_string(timestamp);
        std::string signature = hmac_sha256(API_SECRET, query_string);
        std::string full_url = "https://api.binance.com/api/v3/account?" + query_string + "&signature=" + signature;

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, ("X-MBX-APIKEY: " + API_KEY).c_str());

        curl_easy_setopt(curl, CURLOPT_URL, full_url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        auto start = std::chrono::high_resolution_clock::now();
        CURLcode res = curl_easy_perform(curl);
        auto end = std::chrono::high_resolution_clock::now();

        if (res == CURLE_OK) {
            auto latency = std::chrono::duration<double, std::milli>(end - start).count();
            std::cout << "Private API Latency: " << latency << " ms" << std::endl;
        } else {
            std::cerr << "Failed to reach private API: " << curl_easy_strerror(res) << std::endl;
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
}

int main() {
    std::cout << "Measuring latency to Binance API..." << std::endl;
    while (true) {
        check_public_latency();
        check_private_latency();
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
    return 0;
}
