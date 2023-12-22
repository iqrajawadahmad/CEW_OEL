#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <json-c/json.h>
#include <time.h>

// Struct to hold the HTTP response
struct MemoryStruct {
    char *memory;
    size_t size;
};

// Callback function to write the received data to a memory buffer
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    mem->memory = realloc(mem->memory, mem->size + realsize + 1);
    if (mem->memory == NULL) {
        // Out of memory
        fprintf(stderr, "Not enough memory (realloc returned NULL)\n");
        return 0;
    }

    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

// Function to fetch data from the API and overwrite the file with new data
int fetch_data_and_overwrite_file(const char *api_url, const char *filename) {
    CURL *curl;
    CURLcode res;

    // Initialize the libcurl library
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

    // Set the URL for the request
    curl_easy_setopt(curl, CURLOPT_URL, api_url);

    // Set the callback function to handle the response data
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    // Set timeout to 10 seconds to prevent hanging on DNS resolution
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);

    // Set connect timeout to 3 seconds (adjust as needed)
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);

    // Perform the request
    //res = curl_easy_perform(curl);

    // Check for errors
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

    // Write the new data to the file
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Failed to open file %s for writing\n", filename);
        free(chunk.memory);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return -1;
    }

    fwrite(chunk.memory, 1, chunk.size, file);
    fclose(file);
    printf("Data overwritten in %s\n", filename);

    free(chunk.memory);

    // Clean up
    curl_easy_cleanup(curl);
    // Cleanup the libcurl library
    curl_global_cleanup();

    return 0;
}

// Function to extract information from a JSON file and save desired data to the log file
int extract_and_save_to_log(const char *filename) {
    // Read the content of the file
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Failed to open file %s for reading\n", filename);
        return -1;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *file_content = malloc(file_size + 1);
    if (file_content == NULL) {
        fprintf(stderr, "Failed to allocate memory for file content\n");
        fclose(file);
        return -1;
    }

    fread(file_content, 1, file_size, file);
    fclose(file);

    file_content[file_size] = '\0';  // Null-terminate the string

    // Parse JSON data
    json_object *root = json_tokener_parse(file_content);

    // Check for JSON parsing errors
    if (root == NULL) {
        fprintf(stderr, "Failed to parse JSON data\n");
        free(file_content);
        return -1;
    }

    // Extract information from JSON
    json_object *visibility, *sys, *sunset, *sunrise, *timezone;
    if (json_object_object_get_ex(root, "visibility", &visibility) &&
        json_object_object_get_ex(root, "sys", &sys) &&
        json_object_object_get_ex(sys, "sunset", &sunset) &&
        json_object_object_get_ex(sys, "sunrise", &sunrise) &&
        json_object_object_get_ex(root, "timezone", &timezone)) {
        // ... continue with processing
    } else {
        fprintf(stderr, "Failed to extract data from JSON object\n");
        json_object_put(root);
        free(file_content);
        return -1;
    }

    // Get the current time
    time_t current_time;
    struct tm *timeinfo;
    char timestamp[64];  // Adjust the size as needed

    time(&current_time);
    timeinfo = localtime(&current_time);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);

    // Open the log file in append mode
    FILE *logFile = fopen("weather_log.txt", "a");
    if (logFile == NULL) {
        fprintf(stderr, "Failed to open log file for writing\n");
        json_object_put(root);
        free(file_content);
        return -1;
    }

    // Convert timestamps to human-readable form
    time_t sunrise_time = (time_t)json_object_get_int(sunrise);
    time_t sunset_time = (time_t)json_object_get_int(sunset);

    char formatted_sunrise[64];  // Adjust the size as needed
    char formatted_sunset[64];   // Adjust the size as needed

    strftime(formatted_sunrise, sizeof(formatted_sunrise), "%Y-%m-%d %H:%M:%S", localtime(&sunrise_time));
    strftime(formatted_sunset, sizeof(formatted_sunset), "%Y-%m-%d %H:%M:%S", localtime(&sunset_time));

    // Save the desired information to the log file with the current timestamp
    fprintf(logFile, "Timestamp: %s\n", timestamp);
    fprintf(logFile, "Visibility: %d\n", json_object_get_int(visibility));
    fprintf(logFile, "Timezone: %d\n", json_object_get_int(timezone));
    fprintf(logFile, "Sunset: %s\n", formatted_sunset);
    fprintf(logFile, "Sunrise: %s\n\n", formatted_sunrise);

    fclose(logFile);
    printf("Data saved to log file: weather_log.txt\n");

    // Cleanup
    json_object_put(root);
    free(file_content);

    return 0;
}

int main() {
    const char *api_url = "https://api.openweathermap.org/data/2.5/weather?q=Karachi&appid=0d22435c10bb9be4b4e18e9e1f6559c6";

    // Specify the filename for saving and extracting the data
    const char *filename = "karachi_weather.json";

    // Fetch data from the API and overwrite the file with new data
    if (fetch_data_and_overwrite_file(api_url, filename) == -1) {
        return -1;
    }

    // Extract information from the saved JSON file and save desired data to the log file
    if (extract_and_save_to_log(filename) == -1) {
        return -1;
    }

    return 0;
}
