#!/bin/bash
# Run BrewOS Marketing Site in development mode
# Usage: ./scripts/run.sh [--build|--preview]

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WEB_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

cd "$WEB_DIR"

# Install dependencies if needed
if [ ! -d "node_modules" ]; then
    echo "📦 Installing dependencies..."
    npm install
fi

# Check for flags
if [ "$1" == "--build" ]; then
    echo "🔨 Building site for production..."
    npm run build
    echo ""
    echo "✅ Build complete! Output in: $WEB_DIR/dist"
    echo "   Run 'npm run preview' to preview the build"
elif [ "$1" == "--preview" ]; then
    echo "🔨 Building and previewing..."
    npm run build
    echo ""
    echo "🌐 Starting preview server..."
    npm run preview
else
    echo "🌐 Starting Astro dev server..."
    echo "   Site will be available at http://localhost:4321"
    echo ""
    npm run dev
fi

