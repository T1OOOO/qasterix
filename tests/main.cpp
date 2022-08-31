#include <gtest/gtest.h>

#include <qasterix/AsterixLibrary>

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);

  QAsterixLibrary::init();

  return RUN_ALL_TESTS();
}
