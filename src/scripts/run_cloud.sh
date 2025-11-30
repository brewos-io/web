#!/bin/bash
# Run BrewOS Cloud Service in development mode
# Usage: ./run_cloud.sh

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CLOUD_DIR="$SCRIPT_DIR/../cloud"

# Check if cloud directory exists
if [ ! -d "$CLOUD_DIR" ]; then
    echo "❌ Cloud directory not found: $CLOUD_DIR"
    exit 1
fi

cd "$CLOUD_DIR"

# Check for .env file
if [ ! -f ".env" ]; then
    if [ -f "env.example" ]; then
        echo "⚠️  No .env file found. Creating from env.example..."
        cp env.example .env
        echo "   Please update .env with your configuration"
    else
        echo "⚠️  No .env file found. Create one with required variables."
    fi
fi

# Install dependencies if needed
if [ ! -d "node_modules" ]; then
    echo "📦 Installing dependencies..."
    npm install
fi

# Build TypeScript
echo "🔨 Building TypeScript..."
npm run build

# Run the server
echo "🚀 Starting cloud service..."
echo "   HTTP:   http://localhost:3001"
echo "   Device: ws://localhost:3001/ws/device"
echo "   Client: ws://localhost:3001/ws/client"
echo ""
npm start

