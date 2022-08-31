#include "qasterix/internal/asterixlibrary.h"

QString QAsterixLibrary::version() {
  return QStringLiteral(QASTERIX_VERSION_STR);
}

int QAsterixLibrary::versionMajor() { return QASTERIX_VERSION_MAJOR; }

int QAsterixLibrary::versionMinor() { return QASTERIX_VERSION_MINOR; }

int QAsterixLibrary::versionPatch() { return QASTERIX_VERSION_PATCH; }
