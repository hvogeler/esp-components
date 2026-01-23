#!/bin/bash

if [ -z "$1" ]; then
    echo "Usage: $0 <version>"
    echo "Example: $0 1.0.4"
    exit 1
fi

VERSION="$1"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

for manifest in "$SCRIPT_DIR"/hvo__*/idf_component.yml; do
    if [ -f "$manifest" ]; then
        sed -i '' "s/^version: .*/version: \"$VERSION\"/" "$manifest"
        echo "Updated: $manifest"
    fi
done

git tag ${VERSION}
git push origin ${VERSION}

echo "All components updated to version $VERSION"
