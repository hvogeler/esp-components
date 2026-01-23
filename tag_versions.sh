#!/bin/bash
if [ -z "$1" ]; then
    echo "Usage: $0 <version>"
    echo "Example: $0 1.0.4"
    exit 1
fi

VERSION="$1"
git tag ${VERSION}
git push origin ${VERSION}
