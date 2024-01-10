/ weather.h

#ifndef WEATHER_H
#define WEATHER_H

// Include necessary libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <json-c/json.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>

// ANSI escape codes for text formatting
#define ANSI_BOLD "\033[1m"
#define ANSI_RESET "\033[0m"
#define ANSI_COLOR_YELLOW "\033[33m"
#define ANSI_COLOR_CYAN "\033[36m"
#define ANSI_COLOR_MAGENTA "\033[35m"
#define ANSI_COLOR_GREEN "\033[32m"
#define ANSI_COLOR_RED "\033[31m"

// Function prototypes
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);
int fetch_data_and_overwrite_file(const char *api_url, const char *filename);
const char *check_and_append_visibility(const char *filename, const char *log_filename);
double extract_and_save_to_log(const char *filename, const char *logFilename);
void send_poor_visibility_warning_and_threat(const char *recipient, const char *sender, const char *logFilename, const char *visibility_condition, double visibility_value);
void print_last_weather_data(const char *log_filename, int num_lines, const char *visibility_condition);
void get_console_dimensions(int *rows, int *cols);

#endif
