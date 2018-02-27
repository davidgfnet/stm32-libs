
STM32-LIBS
==========

This project is an attempt to create a collection of well written, fast and
tiny sources targeting STM32 (and in general ARM embedded platforms) so they
can be used in larger projects. The aim is to have well validated sources,
ideally they should be testable on any Linux machine.

Benchmarking
------------

Make benchmark-img.bin image and flash it on your device. By default it assumes
there's a 6KB DFU bootloader, but one can easily fix that by updating
APP_ADDRESS to match.

Once the device is ready one can benchmark the device using the cli-tool. For
example:

  ./cli-tool md5 0  # Benchmark the number of md5_transforms per second

  ./cli-tool md5 2  # Benchmark the number of md5sum ops/sec (256 byte block)


Tests
-----

Running `make test` will build and run tests on your platform and generate
coverage reports. Requirements are lcov and genhtml for the coverage to be
calculated. Report is generated at coverage_report/index.html

