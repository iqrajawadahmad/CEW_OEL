#!/bin/bash

recipient="$1"
sender="$2"
attachment="$3"

# Add a line indicating "Email Delivery Status"
echo -e "\033[1m##############################\nEmail Delivery Status:\033[0m"

# Customize threat email details here
subject="Warning: ⚠ Threat Alert: Poor Visibility"
message="Dear Citizen,

Your area is under threat due to poor visibility. Take necessary precautions immediately.

Sincerely,
Your City Weather Service"

# Use mutt to send the email with an attachment
if [ -f "$attachment" ]; then
    mutt -s "$subject" -a "$attachment" -- "$recipient" <<< "$message"
    
    # Check the exit status of the mutt command
    if [ $? -eq 0 ]; then
        # Print a success message in red
        echo -e "\033[1;32mEmail sent successfully.\033[0m"
    else
        # Print an error message in red
        echo -e "\033[1;31mFailed to send email.\033[0m"
    fi
else
    echo -e "\033[1;31mError: Attachment file not found.\033[0m"
fi

# Add a two-line gap with "#### ***" after the status
echo -e "\n\033[1;31m*****************************\033[0m"
