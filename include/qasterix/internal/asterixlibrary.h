#ifndef QASTERIXLIBRARY_H
#define QASTERIXLIBRARY_H

#include <QResource>

/**
 * @brief The QAsterixLibrary class Class for init and info
 */
class QAsterixLibrary {
public:
  /**
   * @brief init Initialize library resources
   */
  static void init() { Q_INIT_RESOURCE(config); }

  /**
   * @brief version Get library version
   * @return
   */
  static QString version();

  /**
   * @brief versionMajor Get library major version
   * @return
   */
  static int versionMajor();

  /**
   * @brief versionMinor Get library minor version
   * @return
   */
  static int versionMinor();

  /**
   * @brief versionPatch Get library patch version
   * @return
   */
  static int versionPatch();
};

#endif // QASTERIXLIBRARY_H
