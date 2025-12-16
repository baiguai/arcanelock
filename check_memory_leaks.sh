#!/bin/bash

# Script to monitor memory usage of the ArcaneLock application for potential leaks.

# Configuration
APP_PATH="./build/arcanelock"  # Path to your ArcaneLock executable
LOG_FILE="memory_usage_$(date +%Y%m%d_%H%M%S).log"
MONITOR_DURATION_SECONDS=60 # Total duration to monitor in seconds
SAMPLE_INTERVAL_SECONDS=1   # How often to sample memory in seconds

# --- Functions ---

log_message() {
    echo "$(date +'%Y-%m-%d %H:%M:%S') - $1" | tee -a "$LOG_FILE"
}

cleanup() {
    log_message "Cleaning up..."
    if [ -n "$ARCANELOCK_PID" ] && ps -p "$ARCANELOCK_PID" > /dev/null; then
        log_message "Killing ArcaneLock process (PID: $ARCANELOCK_PID)..."
        kill "$ARCANELOCK_PID"
        wait "$ARCANELOCK_PID" 2>/dev/null
        log_message "ArcaneLock process killed."
    else
        log_message "ArcaneLock process not found or already terminated."
    fi
    log_message "Memory monitoring complete. Log saved to $LOG_FILE"
    
    # Robust summary of memory usage (RSS in KB)
    MEM_DATA=$(grep "RSS:" "$LOG_FILE" | awk '{print $NF}' | sed 's/MB$//') # Remove MB suffix for calculation
    if [ -n "$MEM_DATA" ]; then
        echo "$MEM_DATA" | sort -n | \
            awk 'BEGIN{sum=0; count=0; min=10000000; max=0} {sum+=$1; count++; if($1<min)min=$1; if($1>max)max=$1} END{print "Min: "min"MB, Max: "max"MB, Avg: "sum/count"MB"}'
    else
        echo "No memory data collected."
    fi
}

# --- Main Script ---

# Ensure cleanup is called on exit or interruption
trap cleanup EXIT INT TERM

log_message "Starting memory leak check for ArcaneLock..."
log_message "Application: $APP_PATH"
log_message "Log File: $LOG_FILE"
log_message "Monitoring Duration: $MONITOR_DURATION_SECONDS seconds"
log_message "Sample Interval: $SAMPLE_INTERVAL_SECONDS seconds"
log_message "----------------------------------------------------"

# Check if application exists
if [ ! -f "$APP_PATH" ]; then
    log_message "Error: ArcaneLock executable not found at $APP_PATH. Please build the project first."
    exit 1
fi

# Start ArcaneLock in the background using nohup to prevent SIGHUP
log_message "Launching ArcaneLock in background using nohup..."
nohup "$APP_PATH" > /dev/null 2>&1 &
ARCANELOCK_PID=$!
log_message "ArcaneLock launched with PID: $ARCANELOCK_PID"

# Wait a bit longer for the application to start up and stabilize
sleep 10 # Increased sleep duration

# Check if the process is still running
if ! ps -p "$ARCANELOCK_PID" > /dev/null; then
    log_message "Error: ArcaneLock process (PID: $ARCANELOCK_PID) terminated unexpectedly after launch. Exiting."
    exit 1
fi

log_message "ArcaneLock is running. Please interact with the GUI application now to generate memory usage patterns."
log_message "Monitoring memory for $MONITOR_DURATION_SECONDS seconds..."

END_TIME=$((SECONDS + MONITOR_DURATION_SECONDS))
while [ "$SECONDS" -lt "$END_TIME" ]; do
    if ps -p "$ARCANELOCK_PID" > /dev/null; then
        RSS_MB=$(ps -p "$ARCANELOCK_PID" -o rss= | awk '{print $1/1024}') # RSS in MB
        log_message "PID: $ARCANELOCK_PID, RSS: ${RSS_MB}MB"
    else
        log_message "ArcaneLock process (PID: $ARCANELOCK_PID) no longer running. Stopping monitoring."
        break
    fi
    sleep "$SAMPLE_INTERVAL_SECONDS"
done

log_message "Monitoring finished."
# Cleanup function will be called automatically by trap
