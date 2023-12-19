#include <stdio.h>
#include <string.h>
#include <curl/curl.h>

// Callback function to handle the API response
size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    // Print the response directly to the console
    printf("%.*s\n", (int)(size * nmemb), (char *)contents);
    return size * nmemb;
}

int main(void) {
    CURL *curl;
    CURLcode response;

    char city[100];
    printf("Enter the city name: ");
    scanf("%s", city);

    // Construct the API URL with the provided city name
    char url[200];
    sprintf(url, "https://api.openweathermap.org/data/2.5/weather?q=%s&appid=21baa6ecb037e1083560ed99909326de", city);

    curl_global_init(CURL_GLOBAL_ALL);

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);

        // Set the write callback function to handle the API response
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

        response = curl_easy_perform(curl);

        if (response != CURLE_OK) {
            fprintf(stderr, "Request failed: %s\n", curl_easy_strerror(response));
        }

        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    return 0;
}
