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
        fprintf(stderr, "Not enough memory (realloc returned NULL)\n");
        return 0;
    }

    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    // Debug print
    printf("Received data chunk (%zu bytes):\n", realsize);
    for (size_t i = 0; i < realsize; ++i) {
        printf("%02x ", ((unsigned char *)contents)[i]);
    }
    printf("\n");

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

    // Set timeout to 30 seconds to prevent hanging on DNS resolution
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);

    // Set connect timeout to 10 seconds
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10);

    // Perform the request
    res = curl_easy_perform(curl);

    // Debug print
    printf("curl_easy_perform result: %s\n", curl_easy_strerror(res));
    printf("Received data from API:\n%s\n", chunk.memory);

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

    // Debug print
    printf("Data written to file %s:\n%s\n", filename, chunk.memory);

    free(chunk.memory);

    // Clean up
    curl_easy_cleanup(curl);
    // Cleanup the libcurl library
    curl_global_cleanup();

    return 0;
}
// Function to read data from the "karachi_weather.json" file, check visibility conditions, and append to the log file
int check_and_append_visibility(const char *filename, const char *log_filename) {
    // Read data from the file
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Failed to open file %s for reading\n", filename);
        return -1;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *json_data = (char *)malloc(file_size + 1);
    if (json_data == NULL) {
        fprintf(stderr, "Failed to allocate memory for JSON data\n");
        fclose(file);
        return -1;
    }

    fread(json_data, 1, file_size, file);
    fclose(file);

    // Null-terminate the JSON data
    json_data[file_size] = '\0';

    // Parse JSON data
    json_object *root;
    enum json_tokener_error jerr;

    root = json_tokener_parse_verbose(json_data, &jerr);

    // Check for JSON parse errors
    if (jerr != json_tokener_success) {
        fprintf(stderr, "JSON parsing error: %s\n", json_tokener_error_desc(jerr));
        free(json_data);
        json_object_put(root);
        return -1;
    }

    // Extract visibility from the JSON object directly
    json_object *visibility_obj;
    if (json_object_object_get_ex(root, "visibility", &visibility_obj)) {
        double visibility = json_object_get_double(visibility_obj);

        // Determine visibility conditions
        const char *visibility_condition;
        if (visibility > 10000) {
            visibility_condition = "Excellent Visibility";
        } else if (visibility >= 5000) {
            visibility_condition = "Good Visibility";
        } else if (visibility >= 2000) {
            visibility_condition = "Moderate Visibility";
        } else if (visibility >= 1000) {
            visibility_condition = "Poor Visibility";
        } else if (visibility >= 500) {
            visibility_condition = "Very Poor Visibility";
        } else {
            visibility_condition = "Extremely Poor Visibility";
        }

        // Open log file for appending
        FILE *log_file = fopen(log_filename, "a");
        if (log_file == NULL) {
            fprintf(stderr, "Failed to open log file %s for appending\n", log_filename);
            free(json_data);
            json_object_put(root);
            return -1;
        }

        // Get the current time
        time_t current_time = time(NULL);
        struct tm *local_time = localtime(&current_time);

        // Format the timestamp
        char timestamp[20];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", local_time);

        // Write visibility status and timestamp to log file
        fprintf(log_file, "Visibility: %.2f meters, Condition: %s, Timestamp: %s\n", visibility, visibility_condition, timestamp);


        // Close log file
        fclose(log_file);

    } else {
        fprintf(stderr, "Failed to extract visibility from JSON object\n");
        free(json_data);
        json_object_put(root);
        return -1;
    }

    // Clean up
    free(json_data);
    json_object_put(root);

    return 0;
}


// Function to extract information from a JSON file and save desired data to the log file
int extract_and_save_to_log(const char *filename, const char *logFilename) {
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

        // Get the current time
        time_t current_time;
        struct tm *timeinfo;
        char timestamp[64];  // Adjust the size as needed

        time(&current_time);
        timeinfo = localtime(&current_time);
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);

        // Convert timezone to human-readable form
        char formatted_timezone[64];  // Adjust the size as needed
        strftime(formatted_timezone, sizeof(formatted_timezone), "%z %Z", timeinfo);

        // Open the log file in append mode
        FILE *logFile = fopen(logFilename, "a");
        if (logFile == NULL) {
            fprintf(stderr, "Failed to open log file for writing\n");
            json_object_put(root);
            free(file_content);
            return -1;
        }

        // Directly get visibility without converting to kilometers
        double visibility_meters = json_object_get_double(visibility);

        // Convert timestamps to human-readable form
        time_t sunrise_time = (time_t)json_object_get_int(sunrise);
        time_t sunset_time = (time_t)json_object_get_int(sunset);

        char formatted_sunrise[64];  // Adjust the size as needed
        char formatted_sunset[64];   // Adjust the size as needed

        strftime(formatted_sunrise, sizeof(formatted_sunrise), "%Y-%m-%d %H:%M:%S", localtime(&sunrise_time));
        strftime(formatted_sunset, sizeof(formatted_sunset), "%Y-%m-%d %H:%M:%S", localtime(&sunset_time));

        // Save the desired information to the log file with the current timestamp
        fprintf(logFile, "Timestamp: %s\n", timestamp);
        fprintf(logFile, "Sunset: %s\n", formatted_sunset);
        fprintf(logFile, "Sunrise: %s\n", formatted_sunrise);
        fprintf(logFile, "Timezone: %s\n", formatted_timezone);
        fprintf(logFile, "Visibility: %.2f meters\n\n", visibility_meters);

        fclose(logFile);
        printf("Data saved to log file: %s\n", logFilename);
    } else {
        fprintf(stderr, "Failed to extract data from JSON object\n");
        json_object_put(root);
        free(file_content);
        return -1;
    }

    // Cleanup
    json_object_put(root);
    free(file_content);

    return 0;
}

int main(void) {
    const char *api_url = "https://api.openweathermap.org/data/2.5/weather?q=Karachi&appid=0d22435c10bb9be4b4e18e9e1f6559c6";
    const char *karachi_filename = "karachi_weather.json";
    const char *visibility_log_filename = "visibility_log.txt";
    const char *weather_data_log_filename = "weather_data_log.txt";

    // Fetch data from the API and overwrite the file (karachi_weather.json)
    if (fetch_data_and_overwrite_file(api_url, karachi_filename) != 0) {
        fprintf(stderr, "Failed to fetch data from API\n");
        return EXIT_FAILURE;
    }

    // Check and append visibility information for Karachi to log file (visibility_log.txt)
    if (check_and_append_visibility(karachi_filename, visibility_log_filename) != 0) {
        fprintf(stderr, "Failed to check and append visibility for Karachi\n");
        return EXIT_FAILURE;
    }

    // Read data from the file (karachi_weather.json) and call extract_and_save_to_log
    if (extract_and_save_to_log(karachi_filename, weather_data_log_filename) != 0) {
        fprintf(stderr, "Failed to extract and save data to log\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
