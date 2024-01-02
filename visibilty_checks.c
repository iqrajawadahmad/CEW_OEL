#include "weather.h"




const char *check_and_append_visibility(const char *filename, const char *log_filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Failed to open file %s for reading\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *json_data = (char *)malloc(file_size + 1);
    if (json_data == NULL) {
        fprintf(stderr, "Failed to allocate memory for JSON data\n");
        fclose(file);
        return NULL;
    }

    fread(json_data, 1, file_size, file);
    fclose(file);

    json_data[file_size] = '\0';

    json_object *root;
    enum json_tokener_error jerr;
    root = json_tokener_parse_verbose(json_data, &jerr);

    // Check for parsing errors
    if (root == NULL || jerr != json_tokener_success) {
        fprintf(stderr, "JSON parsing error: %s\n", json_tokener_error_desc(jerr));
        free(json_data);
        if (root != NULL) {
            json_object_put(root);
        }
        return NULL;
    }

    const char *visibility_condition;

    // Default visibility value if key is missing
    double visibility = -1.0;

    json_object *visibility_obj;
    if (json_object_object_get_ex(root, "visibility", &visibility_obj)) {
        visibility = json_object_get_double(visibility_obj);
    }

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

    FILE *log_file = fopen(log_filename, "a");
    if (log_file == NULL) {
        fprintf(stderr, "Failed to open log file %s for appending\n", log_filename);
        free(json_data);
        json_object_put(root);
        return NULL;
    }

    time_t current_time = time(NULL);
    struct tm *local_time = localtime(&current_time);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", local_time);

    fprintf(log_file, "Visibility: %.2f meters, Condition: %s, Timestamp: %s\n", visibility, visibility_condition, timestamp);

    fclose(log_file);

    free(json_data);
    json_object_put(root);

    return visibility_condition;
}
double extract_and_save_to_log(const char *filename, const char *logFilename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Failed to open file %s for reading\n", filename);
        return -1.0;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *file_content = malloc(file_size + 1);
    if (file_content == NULL) {
        fprintf(stderr, "Failed to allocate memory for file content\n");
        fclose(file);
        return -1.0;
    }

    fread(file_content, 1, file_size, file);
    fclose(file);
    file_content[file_size] = '\0';

    json_object *root = json_tokener_parse(file_content);

    if (root == NULL) {
        fprintf(stderr, "Failed to parse JSON data\n");
        free(file_content);
        return -1.0;
    }

    FILE *logFile = fopen(logFilename, "a");
    if (logFile == NULL) {
        fprintf(stderr, "Failed to open log file for writing\n");
        json_object_put(root);
        free(file_content);
        return -1.0;
    }

    double visibility_meters = -1.0;

    json_object *visibility = json_object_object_get(root, "visibility");
    json_object *sys = json_object_object_get(root, "sys");
    json_object *sunset = json_object_object_get(sys, "sunset");
    json_object *sunrise = json_object_object_get(sys, "sunrise");
    json_object *timezone = json_object_object_get(root, "timezone");

    if (visibility != NULL && sys != NULL && sunset != NULL && sunrise != NULL && timezone != NULL) {
        time_t current_time;
        struct tm *timeinfo;
        char timestamp[64];
        time(&current_time);
        timeinfo = localtime(&current_time);
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);

        char formatted_timezone[64];
        strftime(formatted_timezone, sizeof(formatted_timezone), "%z %Z", timeinfo);

        visibility_meters = json_object_get_double(visibility);

        time_t sunrise_time = (time_t)json_object_get_int(sunrise);
        time_t sunset_time = (time_t)json_object_get_int(sunset);
        char formatted_sunrise[64];
        char formatted_sunset[64];
        strftime(formatted_sunrise, sizeof(formatted_sunrise), "%Y-%m-%d %H:%M:%S", localtime(&sunrise_time));
        strftime(formatted_sunset, sizeof(formatted_sunset), "%Y-%m-%d %H:%M:%S", localtime(&sunset_time));

        fprintf(logFile, "Timestamp: %s\n", timestamp);
        fprintf(logFile, "Sunset: %s\n", formatted_sunset);
        fprintf(logFile, "Sunrise: %s\n", formatted_sunrise);
        fprintf(logFile, "Timezone: %s\n", formatted_timezone);
        fprintf(logFile, "Visibility: %.2f meters\n\n", visibility_meters);
    } else {
        fprintf(stderr, "Failed to extract data from JSON object\n");
    }

    fclose(logFile);
    json_object_put(root);
    free(file_content);

    return visibility_meters;
}
void send_poor_visibility_warning_and_threat(const char *recipient, const char *sender, const char *logFilename, const char *visibility_condition, double visibility_value) {
    // Open the log file for appending
    FILE *logFile = fopen(logFilename, "a");
    if (logFile == NULL) {
        fprintf(stderr, "Failed to open log file %s for writing\n", logFilename);
        perror("fopen");
        return;
    }

    // Get the current timestamp
    time_t current_time;
    struct tm *timeinfo;
    char timestamp[64];
    time(&current_time);
    timeinfo = localtime(&current_time);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);

    // Append visibility information to the log file
    fprintf(logFile, "Timestamp: %s\n", timestamp);
    fprintf(logFile, "Visibility: %.2f meters\n", visibility_value);
    fprintf(logFile, "Visibility Condition: %s\n\n", visibility_condition);

    // Close the log file
    fclose(logFile);

    // Prepare the command to execute the threat email script
    const char *threat_email_script_path = "./send_threat_email.sh";
    char *command = (char *)malloc(512);  // Adjust the size accordingly
    if (command == NULL) {
        fprintf(stderr, "Failed to allocate memory for the command\n");
        return;
    }

    snprintf(command, 512, "%s '%s' '%s' '%s'", threat_email_script_path, recipient, sender, logFilename);

    // Execute the threat email script
    int result = system(command);

    // Check the result of the system command
    if (result == -1) {
        fprintf(stderr, "Failed to execute the threat email script\n");
        perror("system");
        free(command);  // Free allocated memory
        return;
    } else if (WIFEXITED(result) && WEXITSTATUS(result) != 0) {
        fprintf(stderr, "Email script exited with non-zero status: %d\n", WEXITSTATUS(result));
        // You may want to log or handle this case appropriately
    }

    // Free allocated memory
    free(command);
}

void get_console_dimensions(int *rows, int *cols) {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    *rows = w.ws_row;
    *cols = w.ws_col;
}

void print_last_weather_data(const char *log_filename, int num_lines, const char *visibility_condition) {
    FILE *log_file = fopen(log_filename, "r");
    if (log_file == NULL) {
        fprintf(stderr, "Failed to open log file %s for reading\n", log_filename);
        return;
    }

    char lines[num_lines][256];
    int count = 0;

    // Read the last 'num_lines' lines from the file
    while (fgets(lines[count % num_lines], sizeof(lines[0]), log_file) != NULL) {
        count++;
    }

    // Get console dimensions
    int rows, cols;
    get_console_dimensions(&rows, &cols);

    // Calculate left margin to center the content
    int left_margin = (cols - 32) / 2;  // Assuming the widest line is 32 characters

    // Print the last 'num_lines' lines with centered and colored formatting
    printf("\n");
    for (int i = 0; i < rows / 2 - 5; i++) {
        printf("\n");
    }
    printf("%*s%s========================================%s\n", left_margin, "", ANSI_BOLD ANSI_COLOR_CYAN, ANSI_RESET);
    printf("%*s%s              Weather Data              %s\n", left_margin, "", ANSI_COLOR_CYAN, ANSI_RESET);
    printf("%*s%s========================================%s\n", left_margin, "", ANSI_BOLD ANSI_RESET, ANSI_RESET);
    for (int i = 0; i < num_lines; i++) {
        printf("%*s%s   %s%s", left_margin, "", ANSI_RESET ANSI_COLOR_GREEN, lines[(count + i) % num_lines], ANSI_RESET);
    }

    // Display the visibility condition and timestamp with color
    time_t current_time = time(NULL);
    struct tm *local_time = localtime(&current_time);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", local_time);

    printf("\n%*s%s   Visibility Condition: %s%s\n", left_margin, "", ANSI_RESET ANSI_COLOR_MAGENTA, visibility_condition, ANSI_RESET);
    printf("%*s%s   Current Time: %s%s\n", left_margin, "", ANSI_RESET ANSI_COLOR_MAGENTA, timestamp, ANSI_RESET);

    printf("%*s%s=======================================%s\n", left_margin, "", ANSI_BOLD ANSI_RESET, ANSI_RESET);

    fclose(log_file);
}

