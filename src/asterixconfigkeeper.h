#ifndef ASTERIXCONFIGKEEPER_H
#define ASTERIXCONFIGKEEPER_H

#include <QDomElement>

/**
 * @brief The AsterixConfigKeeper class Class of read xml description of asterix
 * message
 */
class AsterixConfigKeeper {
public:
  /**
   * @brief AsterixConfigKeeper
   * @param parent
   */
  explicit AsterixConfigKeeper();

  /**
   * @brief hasCategory Check xml description of asterix message
   * @param category Number of category of asterix message
   * @return Success of operation
   */
  static const bool hasCategory(const quint8 category);

  /**
   * @brief getCategory Get xml description of asterix message
   * @param category Number of category of asterix message
   * @return Xml description of asterix message
   */
  static const QDomElement getCategory(const int category);
};

#endif // ASTERIXCONFIGKEEPER_H
