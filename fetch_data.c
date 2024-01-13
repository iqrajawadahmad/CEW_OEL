#include "weather.h"



struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;
    mem->memory = realloc(mem->memory, mem->size + realsize + 1);
    if (mem->memory == NULL) {
        fprintf(stderr, "Not enough memory (realloc returned NULL)\n");
        return 0;
    }
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    return realsize;
}

int fetch_data_and_overwrite_file(const char *api_url, const char *filename) {
    CURL *curl;
    CURLcode res;

    if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) {
        fprintf(stderr, "Failed to initialize libcurl\n");
        return -1;
    }

    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Failed to initialize libcurl easy handle\n");
        curl_global_cleanup();
        return -1;
    }

    struct MemoryStruct chunk = {NULL, 0};

    curl_easy_setopt(curl, CURLOPT_URL, api_url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10);

    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        if (res == CURLE_OPERATION_TIMEDOUT) {
            fprintf(stderr, "curl_easy_perform() timed out: %s\n", curl_easy_strerror(res));
        } else if (res == CURLE_COULDNT_RESOLVE_HOST) {
            fprintf(stderr, "curl_easy_perform() failed to resolve host: %s\n", curl_easy_strerror(res));
        } else {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
        free(chunk.memory);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return -1;
    }

    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Failed to open file %s for writing\n", filename);
        free(chunk.memory);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return -1;
    }

    size_t written = fwrite(chunk.memory, 1, chunk.size, file);
    if (written != chunk.size) {
        fprintf(stderr, "Failed to write the entire chunk to file\n");
        free(chunk.memory);
        fclose(file);
        remove(filename); // Remove the partially written file
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return -1;
    }

    fclose(file);
    free(chunk.memory);

    curl_easy_cleanup(curl);
    curl_global_cleanup();

    return 0;
}