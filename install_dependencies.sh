#!/usr/bin/env bash
set -e

sudo apt-get update
sudo apt-get install -y build-essential cmake \
    qtbase5-dev qtbase5-dev-tools qttools5-dev-tools \
    libbz2-dev zlib1g-dev libexpat1-dev \
    libproj-dev libsqlite3-dev libsqlitecpp-dev \
    libgeos-dev libgeos++-dev libosmium2-dev \
    catch2

