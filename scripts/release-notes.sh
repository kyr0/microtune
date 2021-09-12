#!/bin/bash

cat <<- EOH
# Automated build of audioapp

For more information see https://about:blank

EOH
date
echo ""
echo "Most recent commits:" 
echo ""
git log --pretty=oneline | head -5
