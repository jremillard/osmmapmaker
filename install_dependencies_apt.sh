#!/usr/bin/env bash
set -e

sudo apt-get update
sudo apt-get install -y build-essential cmake \
    qtbase5-dev qtbase5-dev-tools qttools5-dev-tools \
    libqt5xmlpatterns5 libqt5xmlpatterns5-dev \
    qt5-doc qt5-doc-html qtbase5-doc-html qtbase5-examples \
    libbz2-dev zlib1g-dev libexpat1-dev \
    libproj-dev clang-format lcov \
    libsqlite3-dev sqlite3-doc libsqlitecpp-dev \
    libgeos-dev libgeos++-dev libgeos-doc catch2 \
    libosmium2-dev valgrind libosmium2-doc \
    libspdlog-dev

