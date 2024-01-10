#include <stdio.h>        // Standard Input/Output functions
#include <stdlib.h>       // General utilities library
#include <string.h>       // String manipulation functions
#include <curl/curl.h>    // libcurl library for making HTTP requests
#include <json-c/json.h>  // JSON-C library for JSON parsing and manipulation
#include <time.h>         // Time-related functions
#include <unistd.h>       // Standard symbolic constants and types (e.g., STDIN_FILENO)
#include <sys/ioctl.h>    // Input/Output control operations (e.g., ioctl for getting console dimensions)
#include <sys/types.h>    // Data types used in system calls (e.g., size_t)
#include <sys/wait.h>     // Waiting for process termination functions

// ANSI escape codes for text formatting
#define ANSI_BOLD "\033[1m"
#define ANSI_RESET "\033[0m"
#define ANSI_COLOR_YELLOW "\033[33m"
#define ANSI_COLOR_CYAN "\033[36m"
#define ANSI_COLOR_MAGENTA "\033[35m"
#define ANSI_COLOR_GREEN "\033[32m"
#define ANSI_COLOR_RED "\033[31m"

// Structure to store memory data while fetching from the API
struct MemoryStruct {
    char *memory;
    size_t size;
};

// Callback function for writing memory data while fetching from the API
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

// Function to fetch data from API and overwrite a file
int fetch_data_and_overwrite_file(const char *api_url, const char *filename) {
    // ... (implementation for fetching data)
}

// Function to check visibility condition and append to a log file
const char *check_and_append_visibility(const char *filename, const char *log_filename) {
    // ... (implementation for checking visibility and appending to log)
}

// Function to extract weather data and save to a log file
double extract_and_save_to_log(const char *filename, const char *logFilename) {
    // ... (implementation for extracting and saving to log)
}

// Function to send warning and threat email for poor visibility
void send_poor_visibility_warning_and_threat(const char *recipient, const char *sender, const char *logFilename, const char *visibility_condition, double visibility_value) {
    // ... (implementation for sending email warning and threat)
}

// Function to get console dimensions
void get_console_dimensions(int *rows, int *cols) {
    // ... (implementation for getting console dimensions)
}

// Function to print the last weather data with formatting
void print_last_weather_data(const char *log_filename, int num_lines, const char *visibility_condition) {
    // ... (implementation for printing last weather data)
}

// Main function
int main(void) {
    // Constants for file paths
    const char *api_url = "https://api.openweathermap.org/data/2.5/weather?q=Karachi&appid=0d22435c10bb9be4b4e18e9e1f6559c6";
    const char *karachi_filename = "karachi_weather.json";
    const char *visibility_log_filename = "visibility_log.txt";
    const char *weather_data_log_filename = "weather_data_log.txt";
    const char *warning_file = "warning_log.txt";
    
    // Fetch data from API and handle errors
    if (fetch_data_and_overwrite_file(api_url, karachi_filename) != 0) {
        fprintf(stderr, "Failed to fetch data from API: %s\n", api_url);
        return EXIT_FAILURE;
    }

    // Check and append visibility information
    const char *visibility_condition = check_and_append_visibility(karachi_filename, visibility_log_filename);
    if (visibility_condition == NULL) {
        fprintf(stderr, "Failed to check and append visibility for %s\n", karachi_filename);
        return EXIT_FAILURE;
    }

    // Extract and save weather data to log
    double visibility_value = extract_and_save_to_log(karachi_filename, weather_data_log_filename);

    if (visibility_value < 0) {
        fprintf(stderr, "Error occurred while processing the file: %s\n", karachi_filename);
        // Handle error if needed
    } else {
        // Check for good visibility and send warning/threat
        if (strcmp(visibility_condition, "Poor Visibility") == 0) {
            const char *recipient_email = "iqrajawad45591@gmail.com";
            const char *sender_email = "iqrajawad4559@gmail.com";
            send_poor_visibility_warning_and_threat(recipient_email, sender_email, warning_file, visibility_condition, visibility_value);
        }
    }

    // Number of lines to print
    int num_lines_to_print = 5;

    // Print the last weather data
    print_last_weather_data(weather_data_log_filename, num_lines_to_print, visibility_condition);

    return EXIT_SUCCESS;
}
