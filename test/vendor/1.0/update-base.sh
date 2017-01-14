#!/bin/bash


# WARNING: vendor-base is placed here only for testing.
# It should typically be created elsewhere.

options="-Lexport-header \
         -r tests:system/tools/hidl/test/\
         -r android.hidl:system/libhidl/transport"

hidl-gen $options \
         -o system/tools/hidl/test/vendor/1.0/vendor-base.h \
         tests.vendor@1.0