#include "weather.h"

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