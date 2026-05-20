#include <iostream>
#include <string>
#include <vector>
#include <ctime>

#ifdef __has_include
# if __has_include(<curl/curl.h>)
# include <curl/curl.h>
# define HAVE_CURL
#endif
#endif

#include "json.hpp"

using namespace std;
using json = nlohmann::json;


size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* output) {
    output->append((char*)contents, size * nmemb);
    return size * nmemb;
}

int main() {
    

    string Search;
    cout << "Podaj nazwe gry: ";
    getline(cin, Search);

#ifndef HAVE_CURL
    cerr << "Brak biblioteki curl!" << endl;
    return 1;
#else
    CURL* curl = curl_easy_init();
    if (!curl) {
        cerr << "Nie udalo sie zainicjalizowac curl!" << endl;
        return 1;
    }

    
    char* encoded = curl_easy_escape(curl, Search.c_str(), Search.length());

    string url = "https://api.rawg.io/api/games?key=522678a8a5bc4864ae723d1c7f1207dd&search="
    + string(encoded) 
    + "&page_size=50";

    curl_free(encoded);
    

    string response;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        cerr << "Blad curl: " << curl_easy_strerror(res) << endl;
        curl_easy_cleanup(curl);
        return 1;
    }

    curl_easy_cleanup(curl);

    
    json dane;
    try {
        dane = json::parse(response);
    } catch (...) {
        cerr << "Blad parsowania JSON!" << endl;
        return 1;
    }

    if (!dane.contains("results")) {
        cerr << "Brak wynikow w API!" << endl;
        return 1;
    }

    cout << "Znaleziono: " 
        << dane["results"].size()
        << " gier" << endl;
    int licznik = 0;
    
    for (auto& gra : dane["results"]) {
        if (licznik++ >= 50) break;

        cout << "Nazwa: " << gra["name"] << endl;
        
        if (gra.contains("rating"))
            cout << "Ocena: " << gra["rating"] << endl;

        if (gra.contains("released") && !gra["released"].is_null())
            cout << "Data wydania: " << gra["released"] << endl;

        
        if (gra.contains("platforms")) {
            cout << "Platformy: ";
            for (auto& p : gra["platforms"]) {
                cout << p["platform"]["name"] << ", ";
            }
            cout << endl;
        }

        
        

        cout << "----------------------" << endl;
    }

    return 0;
#endif
}