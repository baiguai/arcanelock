#!/bin/bash

# Exit immediately if a command exits with a non-zero status.
set -e

echo "Running build.sh..."
./build.sh

echo "Running ArcaneLock..."
./build/arcanelock
