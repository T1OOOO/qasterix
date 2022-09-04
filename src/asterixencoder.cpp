#include "qasterix/internal/asterixencoder.h"

#include <QBuffer>
#include <QDataStream>
#include <QDomDocument>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtDebug>

#include "3rdparty/bitfield-c/src/bitfield/bitfield.h"
#include "asterixconfigkeeper.h"
#include "asterixnames.h"
#include "math.h"

namespace {
constexpr int doubleZeroSize = 2;
constexpr int defaultFSPEC_len = 10;
} // namespace

namespace qasterix {

AsterixEncoder::AsterixEncoder(QObject *parent) : QObject{parent} {}

QByteArray AsterixEncoder::encode(const quint8 catNum, const QString &objStr) {
  const QVariantMap obj = parseJson(objStr);

  return encode(catNum, objStr);
}

QByteArray AsterixEncoder::encode(const quint8 catNum, const QVariantMap &obj) {
  if (obj.isEmpty()) {
    clear();

    return {};
  }

  if (!AsterixConfigKeeper::hasCategory(catNum)) {
    clear();

    return {};
  }

  cat = AsterixConfigKeeper::getCategory(catNum);

  dataitems = cat.elementsByTagName(AN::dataItemName);
  if (dataitems.isEmpty()) {
    clear();

    return {};
  }

  QDomNodeList uap_elements = cat.elementsByTagName(AN::uapName);
  if (uap_elements.isEmpty()) {
    clear();

    return {};
  }

  QDomElement uap = uap_elements.at(0).toElement();
  uapitems = uap.elementsByTagName(AN::uapItemName);
  if (uapitems.isEmpty()) {
    clear();

    return {};
  }

  encorded_result.push_back(catNum);
  asterix = obj;

  encode();

  const qint16 length = 3 + encoded.size();
  QByteArray lengthData(doubleZeroSize, static_cast<char>(0));
  QBuffer buf(&lengthData);
  buf.open(QIODevice::WriteOnly);
  QDataStream ds(&buf);
  ds.setByteOrder(QDataStream::BigEndian);
  ds << length;

  encorded_result.append(lengthData);
  encorded_result.append(encoded);

  QByteArray res = std::move(encorded_result);
  clear();

  return res;
}

void AsterixEncoder::clear() {
  cat = QDomElement();
  dataitems = QDomNodeList();
  uapitems = QDomNodeList();
  asterix = QVariantMap();
  encorded_result = QByteArray();
  encoded = QByteArray();
}

bool AsterixEncoder::arrayNotNull(const QByteArray &data) const {
  bool res = std::any_of(data.cbegin(), data.cend(),
                         [](const auto &ch) -> bool { return ch != 0; });

  return res;
}

quint16 AsterixEncoder::getBitfieldPos(const quint16 pos) const {
  const quint16 posF = (pos / CHAR_WIDTH) * CHAR_WIDTH;
  const quint16 posM = pos % CHAR_WIDTH;
  const quint16 newPos = posF + CHAR_WIDTH - 1 - posM;

  return newPos;
}

void AsterixEncoder::encode() {
  QByteArray FSPEC_bits(defaultFSPEC_len, static_cast<char>(0));
  qint16 FSPEC_bits_len = 0;

  // reversed
  for (int i = uapitems.count() - 1; i >= 0; i--) {
    const QDomElement uapitem = uapitems.at(i).toElement();

    if (FSPEC_bits_len % 8 == 0) {
      // FX field
      if (arrayNotNull(FSPEC_bits)) {
        const quint16 pos = getBitfieldPos(FSPEC_bits_len);
        set_bitfield(1, pos, 1, reinterpret_cast<uint8_t *>(FSPEC_bits.data()),
                     FSPEC_bits.size());
      } else {
        FSPEC_bits_len = 0;
      }
      FSPEC_bits_len += 1;
      continue;
    }

    const QString id = uapitem.text();
    if (asterix.contains(id)) {
      if (asterix[id].isNull()) {
        asterix.remove(id);
        continue;
      }

      const quint16 pos = getBitfieldPos(FSPEC_bits_len);
      set_bitfield(1, pos, 1, reinterpret_cast<uint8_t *>(FSPEC_bits.data()),
                   FSPEC_bits.size());
    }

    FSPEC_bits_len += 1;
  }

  const qint16 FSPEC_bits_len_r =
      std::ceil(FSPEC_bits_len / static_cast<float>(CHAR_WIDTH)) * 8;
  const int FSPEC_size = FSPEC_bits_len_r / 8;
  FSPEC_bits = FSPEC_bits.mid(0, FSPEC_size);

  std::reverse(FSPEC_bits.begin(), FSPEC_bits.end());
  encoded.append(FSPEC_bits);

  for (int i = 0; i < uapitems.count(); i++) {
    const QDomElement uapitem = uapitems.at(i).toElement();
    const QString id = uapitem.text();

    if (!asterix.contains(id)) {
      continue;
    }

    for (int i = 0; i < dataitems.count(); i++) {
      const QDomElement dataitem = dataitems.at(i).toElement();
      const QString itemid = dataitem.attribute(AN::idName);
      if (itemid == id) {
        const QDomNodeList dataitem_elements =
            dataitem.elementsByTagName(AN::dataItemFromatName);
        if (dataitem_elements.isEmpty()) {
          continue;
        }

        const QDomElement dataitemformat = dataitem_elements.at(0).toElement();

        const QDomNodeList dataitemformat_cn = dataitemformat.childNodes();
        for (int i = 0; i < dataitemformat_cn.size(); i++) {
          const QDomElement cn = dataitemformat_cn.at(i).toElement();

          QVariant &asterixItem = asterix[itemid];

          std::tuple<int, QByteArray> res;
          if (cn.nodeName() == AN::fixedName) {
            res = encode_fixed(asterixItem, cn);
          } else if (cn.nodeName() == AN::repetitiveName) {
            res = encode_repetitive(asterixItem, cn);
          } else if (cn.nodeName() == AN::variableName) {
            res = encode_variable(asterixItem, cn);
          } else if (cn.nodeName() == AN::compoundName) {
            res = encode_compound(asterixItem, cn);
          }

          const auto &[n, r] = res;

          if (!r.isEmpty()) {
            encoded.append(r);
          }
        }
      }
    }
  }
}

std::tuple<int, QByteArray>
AsterixEncoder::encode_fixed(QVariant &data_asterix,
                             const QDomElement &datafield) {
  if (data_asterix.type() != QVariant::Map) {
    return {};
  }

  QVariantMap data_asterix_map = data_asterix.toMap();

  const int length = datafield.attribute(AN::lengthName).toInt();
  const int length_b = length * CHAR_WIDTH;
  QDomNodeList bitslist = datafield.elementsByTagName(AN::bitsName);

  QByteArray encoded_bytes(length, static_cast<char>(0));
  int encoded_num = 0;
  for (int i = 0; i < bitslist.count(); i++) {
    const QDomElement bits = bitslist.at(i).toElement();
    const QDomNodeList bits_elements =
        bits.elementsByTagName(AN::bitsShortName);
    if (bits_elements.isEmpty()) {
      continue;
    }
    const QString bit_name = bits_elements.at(0).toElement().text();

    if (data_asterix_map.contains(bit_name)) {
      static const QStringList bit_names = {AN::fxName, AN::spareName};
      if (bit_names.contains(bit_name) || data_asterix_map[bit_name] == 0) {
        data_asterix_map.remove(bit_name);
        continue;
      }

      encoded_num += 1;
      const QString bit = bits.attribute(AN::bitName);
      if (bit != "") {
        const quint16 _shift = bit.toInt();
        const quint16 _shift_r = length_b - _shift;
        const quint16 pos = getBitfieldPos(_shift);
        set_bitfield(1, _shift_r, 1,
                     reinterpret_cast<uint8_t *>(encoded_bytes.data()), length);
      } else {
        qint16 _from = bits.attribute(AN::fromName).toInt();
        qint16 _to = bits.attribute(AN::toName).toInt();
        const qint32 _size = _from - _to + 1;

        if (_from < _to) {
          std::swap(_to, _from);
        }
        const qint16 _from_r = length_b - _from;

        const int _length = _from - _to + 1;
        const QVariant &v = data_asterix_map[bit_name];

        quint64 vU = 0;
        qint64 vS = 0;
        double vD = 0.0;

        bool isSigned = false;
        bool isDouble = false;
        if (v.type() == QVariant::Type::LongLong ||
            v.type() == QVariant::Type::Int ||
            v.type() == QVariant::Type::Char) {
          isSigned = true;
          vS = v.toLongLong();
        } else if (v.type() == QVariant::Type::Double) {
          isSigned = true;
          isDouble = true;
          vD = v.toDouble();
        } else if (v.type() == QVariant::Type::ULongLong ||
                   v.type() == QVariant::Type::UInt) {
          isSigned = false;
          vU = v.toULongLong();
        }

        QDomNodeList BitsUnit = bits.elementsByTagName(AN::bitsUnitName);
        if (!BitsUnit.isEmpty()) {
          QString scale_str =
              BitsUnit.at(0).toElement().attribute(AN::scaleName);
          if (scale_str == "") {
            scale_str = "1.0";
          }

          const double scale = scale_str.toDouble();
          if (isSigned) {
            if (isDouble) {
              vS = vD / scale;
            } else {
              vS = vS / scale;
            }
          } else {
            vU = vU / scale;
          }
        }

        if (isSigned) {
          vU = vS & bitmask(_length);
        }
        set_bitfield(vU, _from_r, _length,
                     reinterpret_cast<uint8_t *>(encoded_bytes.data()), length);

        data_asterix_map.remove(bit_name);
      }
    }
  }

  data_asterix = data_asterix_map;

  return std::make_tuple(encoded_num, encoded_bytes);
}

std::tuple<int, QByteArray>
AsterixEncoder::encode_variable(QVariant &data_asterix,
                                const QDomElement &datafield) {
  if (data_asterix.type() != QVariant::Map) {
    return {};
  }

  QByteArray result;
  int encoded_num = 0;

  QDomNodeList fixedList = datafield.elementsByTagName(AN::fixedName);
  for (int i = 0; i < fixedList.count(); i++) {
    const QDomElement fixed = fixedList.at(i).toElement();
    const auto [n, r] = encode_fixed(data_asterix, fixed);
    encoded_num += n;
    result.append(r);

    QVariantMap data_asterix_map = data_asterix.toMap();

    if (!data_asterix_map.isEmpty()) {
      const int last = result.size() - 1;
      result[last] = result.at(last) | 0x1;
    } else {
      break;
    }
  }

  return std::make_tuple(encoded_num, result);
}

std::tuple<int, QByteArray>
AsterixEncoder::encode_repetitive(QVariant &data_asterix,
                                  const QDomElement &datafield) {
  if (data_asterix.type() != QVariant::List) {
    return {};
  }

  QVariantList data_asterix_list = data_asterix.toList();

  QByteArray result;

  const char length = data_asterix_list.size();
  result.push_back(length);
  int encoded_num = 0;

  QDomNodeList datafield_elements = datafield.elementsByTagName(AN::fixedName);
  if (datafield_elements.isEmpty()) {
    return {};
  }

  QDomElement fixed = datafield_elements.at(0).toElement();
  for (int i = 0; i < data_asterix_list.size(); i++) {
    QVariant &subdata = data_asterix_list[i];
    const auto [n, r] = encode_fixed(subdata, fixed);
    encoded_num += n;
    result.append(r);
  }
  data_asterix = data_asterix_list;

  return std::make_tuple(encoded_num, result);
}

std::tuple<int, QByteArray>
AsterixEncoder::encode_compound(QVariant &data_asterix,
                                const QDomElement &datafield) {
  QByteArray result;
  int encoded_num = 0;

  int index = 0;
  QVector<int> indexs;

  QDomNodeList datafield_list = datafield.childNodes();
  for (int i = 0; i < datafield_list.size(); i++) {
    const QDomElement cn = datafield_list.at(i).toElement();

    static const QStringList nodeNameList = {
        AN::fixedName, AN::repetitiveName, AN::variableName, AN::compoundName};
    const QString nodeName = cn.nodeName();
    if (!nodeNameList.contains(nodeName)) {
      continue;
    }

    index += 1;

    //    if (index == 1) {
    //      continue;
    //    }

    if (index % CHAR_WIDTH == 0) {
      index += 1;
    }

    std::tuple<int, QByteArray> res;

    if (nodeName == AN::fixedName) {
      res = encode_fixed(data_asterix, cn);
    } else if (nodeName == AN::repetitiveName) {
      res = encode_repetitive(data_asterix, cn);
    } else if (nodeName == AN::variableName) {
      res = encode_variable(data_asterix, cn);
    } else if (nodeName == AN::compoundName) {
      res = encode_compound(data_asterix, cn);
    }

    const auto &[n, r] = res;

    if (n == 0) {
      continue;
    }
    encoded_num += n;
    result.append(r);
    indexs.push_back(index);
  }

  if (indexs.isEmpty()) {
    return {};
  }

  const quint64 maxindex = indexs.last();
  const quint64 indicator_bytes = (maxindex + 7) / CHAR_WIDTH;
  // const quint64 indicator_bytes_b = indicator_bytes * CHAR_WIDTH;
  QByteArray indicator(indicator_bytes, static_cast<char>(0));

  for (const int index : indexs) {
    const quint64 _shift = indicator_bytes * CHAR_WIDTH - index;
    const quint64 pos = getBitfieldPos(_shift);

    set_bitfield(1, pos, 1, reinterpret_cast<uint8_t *>(indicator.data()),
                 indicator.size());
  }

  for (int i = 1; i < indicator_bytes; i++) {
    const quint64 _shift = i * CHAR_WIDTH;
    const quint64 pos = getBitfieldPos(_shift);
    // indicator = indicator | ((quint64)1 << _shift);

    set_bitfield(1, pos, 1, reinterpret_cast<uint8_t *>(indicator.data()),
                 indicator.size());
  }

  std::reverse(indicator.begin(), indicator.end());

  QByteArray res = std::move(indicator);
  res.append(result);

  return std::make_tuple(encoded_num, res);
}

QVariantMap AsterixEncoder::parseJson(const QString &str) const {
  {
    QJsonParseError error;

    QJsonObject obj = QJsonDocument::fromJson(str.toLatin1(), &error).object();
    if (error.error != QJsonParseError::NoError) {
      qDebug() << "parse error: " << error.errorString();

      return {};
    }

    return obj.toVariantMap();
  }
}

} // namespace qasterix
