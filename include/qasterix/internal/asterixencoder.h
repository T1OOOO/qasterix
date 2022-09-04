#ifndef ASTERIXENCODER_H
#define ASTERIXENCODER_H

#include <QDomElement>
#include <QDomNodeList>
#include <QObject>
#include <QVariantMap>
#include <tuple>

#include "qasterix/export/qasterix_export.h"

namespace qasterix {

/**
 * @brief The AsterixEncoder class Class for encode asterix message
 */
class QASTERIX_EXPORT AsterixEncoder : public QObject {
public:
  /**
   * @brief AsterixEncoder Constructor of class for encode asterix message
   * @param parent Parent object
   */
  explicit AsterixEncoder(QObject *parent = nullptr);

  /**
   * @brief encode Encode asterix message
   * @param catNum Number of category asterix message
   * @param obj String with asterix message description
   * @return Data of asterix message
   */
  QByteArray encode(const quint8 catNum, const QString &objStr);

  /**
   * @brief encode Encode asterix message
   * @param catNum Number of category asterix message
   * @param obj Object with asterix message description
   * @return Data of asterix message
   */
  QByteArray encode(const quint8 catNum, const QVariantMap &obj);

  /**
   * @brief clear Clear internal state of object
   */
  void clear();

protected:
  /**
   * @brief encode Encode asterix message
   */
  void encode();

  /**
   * @brief encode_fixed Encode fixed value of asterix message field
   * @param data_asterix Value of asterix field
   * @param datafield Xml description of asterix field
   * @return Count of values and data of encoded asterix field
   */
  std::tuple<int, QByteArray> encode_fixed(QVariant &data_asterix,
                                           const QDomElement &datafield);

  /**
   * @brief encode_repetitive Encode variable value of asterix message field
   * @param data_asterix Value of asterix field
   * @param datafield Xml description of asterix field
   * @return Count of values and data of encoded asterix field
   */
  std::tuple<int, QByteArray> encode_repetitive(QVariant &data_asterix,
                                                const QDomElement &datafield);

  /**
   * @brief encode_variable Encode repetitive value of asterix message field
   * @param data_asterix Value of asterix field
   * @param datafield Xml description of asterix field
   * @return Count of values and data of encoded asterix field
   */
  std::tuple<int, QByteArray> encode_variable(QVariant &data_asterix,
                                              const QDomElement &datafield);

  /**
   * @brief encode_compound Encode compound value of asterix message field
   * @param data_asterix Value of asterix field
   * @param datafield Xml description of asterix field
   * @return Count of values and data of encoded asterix field
   */
  std::tuple<int, QByteArray> encode_compound(QVariant &data_asterix,
                                              const QDomElement &datafield);

  /**
   * @brief getBitfieldPos Get position of bit in bitfield
   * @param pos Original position of bit
   * @return New position of bit
   */
  quint16 getBitfieldPos(const quint16 pos) const;

  /**
   * @brief arrayNotNull Check if array has not null data
   * @param data Data
   * @return Success of operation
   */
  bool arrayNotNull(const QByteArray &data) const;

  QVariantMap parseJson(const QString &str) const;

private:
  QDomElement cat;            ///< Xml description of asterix message
  QDomNodeList dataitems;     ///< Data fields of asterix message
  QDomNodeList uapitems;      ///< Uap fields of asterix message
  QVariantMap asterix;        ///< Values of fields of asterix message
  QByteArray encorded_result; ///< Encoded data of asterix message
  QByteArray encoded;         ///< Encoded data of asterix message
};

} // namespace qasterix

#endif // ASTERIXENCODER_H
