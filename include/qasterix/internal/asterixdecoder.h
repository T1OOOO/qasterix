#ifndef ASTERIXDECODER_H
#define ASTERIXDECODER_H

#include <QDomDocument>
#include <QDomElement>
#include <QDomNodeList>
#include <QObject>
#include <QVariantMap>

#include "qasterix/export/qasterix_export.h"

namespace qasterix {

/**
 * @brief The AsterixDecoder class Сlass for decode asterix messages
 */
class QASTERIX_EXPORT AsterixDecoder : public QObject {
  Q_OBJECT
public:
  /**
   * @brief AsterixDecoder Constructor of class for decode asterix messages
   * @param parent Parent object
   */
  explicit AsterixDecoder(QObject *parent = nullptr);

  /**
   * @brief decode Decode asterix message
   * @param data Data of asterix message
   * @return Object with asterix message description
   */
  QVariantMap decode(const QByteArray &data);

  /**
   * @brief clear Clear internal state of object
   */
  void clear();

protected:
  /**
   * @brief decode Decode asterix message
   */
  void decode();

  /**
   * @brief decode_fixed Decode fixed value of asterix message field
   * @param datafield Xml description of asterix field
   * @return Decoded field values of asterix message
   */
  QVariantMap decode_fixed(const QDomElement &datafield);

  /**
   * @brief decode_variable Decode variable value of asterix message field
   * @param datafield Xml description of asterix field
   * @return Decoded field values of asterix message
   */
  QVariantMap decode_variable(const QDomElement &datafield);

  /**
   * @brief decode_repetitive Decode repetitive value of asterix message field
   * @param datafield Xml description of asterix field
   * @return Decoded field values of asterix message
   */
  QVariantList decode_repetitive(const QDomElement &datafield);

  /**
   * @brief decode_compound Decode compound value of asterix message field
   * @param datafield Xml description of asterix field
   * @return Decoded field values of asterix message
   */
  QVariantMap decode_compound(const QDomElement &datafield);

  /**
   * @brief getBitfieldPos Get position of bit in bitfield
   * @param pos Original position of bit
   * @return New position of bit
   */
  quint16 getBitfieldPos(const quint16 pos) const;

private:
  int p;            ///< Data position
  QByteArray bytes; ///< Data
  qint32 length;    ///< Length of data

  QDomElement cat;        ///< Xml description of asterix message
  QDomNodeList dataitems; ///< Data fields of asterix message
  QDomNodeList uapitems;  ///< Uap fields of asterix message
  QVariantMap decoded;    ///< Decoded field values of asterix message
};

} // namespace qasterix

#endif // ASTERIXDECODER_H
