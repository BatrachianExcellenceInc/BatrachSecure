#!/usr/bin/env bash

# ./client.sh 1 --help
#
# This script creates conf dirs to help with development and testing

set -eu

export QT_FATAL_WARNINGS=1
export LD_LIBRARY_PATH=../libcurve25519/:../libaxolotl-qt5/:$LD_LIBRARY_PATH

client_num=$1
shift 1

exec ./BtxSecure http://localhost:8080 --conf-dir btxsec${client_num}/ ${@}

