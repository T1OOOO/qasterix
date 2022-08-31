#ifndef ASTERIXTEST_H
#define ASTERIXTEST_H

#include <gtest/gtest.h>
#include <qasterix/AsterixDecoder>
#include <qasterix/AsterixEncoder>

// The fixture for testing class Foo.
class AsterixTest : public ::testing::Test {
protected:
  // You can remove any or all of the following functions if their bodies would
  // be empty.

  AsterixTest() {
    // You can do set-up work for each test here.
  }

  ~AsterixTest() override {
    // You can do clean-up work that doesn't throw exceptions here.
  }

  // If the constructor and destructor are not enough for setting up
  // and cleaning up each test, you can define the following methods:

  void SetUp() override {
    encoder.clear();
    decoder.clear();
    // Code here will be called immediately after the constructor (right
    // before each test).
  }

  void TearDown() override {
    encoder.clear();
    decoder.clear();
    // Code here will be called immediately after each test (right
    // before the destructor).
  }

  // Class members declared here can be used by all tests in the test suite
  // for Foo.
  qasterix::AsterixEncoder encoder;
  qasterix::AsterixDecoder decoder;
};

#endif // ASTERIXTEST_H
