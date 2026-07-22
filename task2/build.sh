#!/usr/bin/env bash
set -euo pipefail

make all
make python

printf '\nBuild completed. Run tests from the project root.\n'
