#include "qasterix/internal/asterixdecoder.h"

#include <QBuffer>
#include <QDataStream>
#include <QFile>
#include <QtDebug>

#include "3rdparty/bitfield-c/src/bitfield/bitfield.h"

#include "asterixconfigkeeper.h"
#include "asterixnames.h"

namespace qasterix {

AsterixDecoder::AsterixDecoder(QObject *parent)
    : QObject{parent}, p{0}, length{0} {}

QVariantMap AsterixDecoder::decode(const QByteArray &data) {
  clear();

  bytes = data;
  length = 0;
  p = 0;

  if (bytes.size() < 1) {
    clear();

    return {};
  }

  const quint8 catNum =
      *reinterpret_cast<const quint8 *>(&bytes.constData()[p]);
  if (!AsterixConfigKeeper::hasCategory(catNum)) {
    clear();

    return {};
  }

  p += 1;

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

  if (bytes.size() < (p + 2)) {
    clear();

    return {};
  }

  QByteArray lenData = bytes.mid(p, 2);
  QDataStream lds(lenData);
  lds.setByteOrder(QDataStream::BigEndian);
  lds >> length;
  p += 2;

  decode();

  QVariantMap res = std::move(decoded);
  clear();

  return res;
}

void AsterixDecoder::clear() {
  p = 0;
  bytes = QByteArray();
  length = 0;

  cat = QDomElement();
  dataitems = QDomNodeList();
  uapitems = QDomNodeList();
  decoded = QVariantMap();
}

void AsterixDecoder::decode() {
  quint64 fspec_octets = 0;
  qint16 fspec_octets_len = 0;

  while (true) {
    char _b = bytes.at(p);
    p += 1;
    fspec_octets = (fspec_octets << CHAR_WIDTH) + _b;
    fspec_octets_len += 1;

    if ((_b & 1) == 0) {
      break;
    }
  }

  QStringList itemids;

  quint64 mask = (quint64)1
                 << ((quint64)CHAR_WIDTH * fspec_octets_len - (quint64)1);
  for (int i = 0; i < CHAR_WIDTH * fspec_octets_len; i++) {
    if ((fspec_octets & mask) > 0) {
      const QDomElement uapitem = uapitems.at(i).toElement();
      QString itemid = uapitem.text();
      if (itemid != '-') {
        itemids.push_back(itemid);
      }
    }

    mask = mask >> 1;
  }

  for (const auto &itemid : itemids) {
    for (int i = 0; i < dataitems.count(); i++) {
      const QDomElement dataitem = dataitems.at(i).toElement();
      if (dataitem.attribute(AN::idName) == itemid) {
        QDomNodeList dataitem_elemets =
            dataitem.elementsByTagName(AN::dataItemFromatName);
        if (dataitem_elemets.isEmpty()) {
          continue;
        }

        QDomNode dataitemformat = dataitem_elemets.at(0);
        QDomNodeList dataitemformat_cn = dataitemformat.childNodes();
        for (int j = 0; j < dataitemformat_cn.count(); j++) {
          const QDomElement cn = dataitemformat_cn.at(j).toElement();

          QVariant r;
          if (cn.nodeName() == AN::fixedName) {
            r = decode_fixed(cn);
          } else if (cn.nodeName() == AN::repetitiveName) {
            r = decode_repetitive(cn);
          } else if (cn.nodeName() == AN::variableName) {
            r = decode_variable(cn);
          } else if (cn.nodeName() == AN::compoundName) {
            r = decode_compound(cn);
          }

          if (!r.isNull()) {
            decoded[itemid] = r;
          }
        }
      }
    }
  }
}

quint16 AsterixDecoder::getBitfieldPos(const quint16 pos) const {
  const quint16 posF = (pos / CHAR_WIDTH) * CHAR_WIDTH;
  const quint16 posM = pos % CHAR_WIDTH;
  const quint16 newPos = posF + CHAR_WIDTH - 1 - posM;

  return newPos;
}

QVariantMap AsterixDecoder::decode_fixed(const QDomElement &datafield) {
  QVariantMap results;
  int length = datafield.attribute(AN::lengthName).toInt();
  int length_b = length * CHAR_WIDTH;
  QDomNodeList bitslist = datafield.elementsByTagName(AN::bitsName);

  QByteArray _bytes = bytes.mid(p, length);

  p += length;

  for (int i = 0; i < bitslist.size(); i++) {
    const QDomElement bits = bitslist.at(i).toElement();
    const QDomNodeList bits_elements =
        bits.elementsByTagName(AN::bitsShortName);
    if (bits_elements.isEmpty()) {
      continue;
    }

    const QString bit_name = bits_elements.at(0).toElement().text();
    static const QStringList bit_names = {AN::fxName, AN::spareName};
    if (bit_names.contains(bit_name)) {
      continue;
    }

    QString bit = bits.attribute(AN::bitName);
    if (bit != "") {
      const quint16 bitI = bit.toInt();
      const quint16 bitI_r = length_b - bitI;
      const quint64 resultU =
          get_bitfield(reinterpret_cast<const uint8_t *>(_bytes.constData()),
                       _bytes.size(), bitI_r, 1);
      if (resultU == 0) {
        continue;
      }

      results[bit_name] = resultU;
    } else {
      quint16 from_ = bits.attribute(AN::fromName).toInt();
      quint16 to_ = bits.attribute(AN::toName).toInt();
      if (from_ < to_) {
        std::swap(from_, to_);
      }
      const quint16 size_ = from_ - to_ + 1;
      const quint16 from_r_ = length_b - from_;

      quint64 resultU =
          get_bitfield(reinterpret_cast<const uint8_t *>(_bytes.constData()),
                       _bytes.size(), from_r_, size_) &
          bitmask(size_);

      if (resultU == 0) {
        continue;
      }

      bool isSigned = false;
      if (bits.attribute(AN::encodeName) == AN::signedName) {
        if ((resultU & ((quint64)1 << (size_ - 1))) > 0) {
          quint64 resultS = ((qint64)(-1) & ~bitmask(size_)) + (qint64)resultU;

          results[bit_name] = resultS;
        }
        isSigned = true;
      }

      if (!results.contains(bit_name)) {
        results[bit_name] = resultU;
      }

      const QDomNodeList BitsUnit = bits.elementsByTagName(AN::bitsUnitName);
      if (!BitsUnit.isEmpty()) {
        QString scaleStr = BitsUnit.at(0).toElement().attribute(AN::scaleName);
        if (scaleStr == "") {
          scaleStr = "1.0";
        }

        const double scale = scaleStr.toDouble();
        if (isSigned) {
          const qint64 resultS = results[bit_name].toLongLong();
          const double resultReal = resultS * scale;
          results[bit_name] = resultReal;
        } else {
          const double resultReal = resultU * scale;
          results[bit_name] = resultReal;
        }
      }
    }
  }

  return results;
}

QVariantMap AsterixDecoder::decode_variable(const QDomElement &datafield) {
  QVariantMap results;

  QDomNodeList fixed_elements = datafield.elementsByTagName(AN::fixedName);
  for (int i = 0; i < fixed_elements.count(); i++) {
    const QDomElement fixed = fixed_elements.at(i).toElement();
    QVariantMap r = decode_fixed(fixed);
    if (r.contains(AN::fxName)) {
      continue;
    }

    results.insert(r);
  }

  return results;
}

QVariantList AsterixDecoder::decode_repetitive(const QDomElement &datafield) {
  quint8 rep = bytes.at(p);
  p += 1;

  QVariantList results;

  QDomNodeList datafield_elements = datafield.elementsByTagName(AN::fixedName);
  if (datafield_elements.isEmpty()) {
    return {};
  }

  QDomElement fixed = datafield_elements.at(0).toElement();
  for (int i = 0; i < rep; i++) {
    QVariantMap r = decode_fixed(fixed);
    if (r.isEmpty()) {
      continue;
    }

    results.push_back(r);
  }

  return results;
}

QVariantMap AsterixDecoder::decode_compound(const QDomElement &datafield) {
  quint64 indicator_octets = 0;
  qint32 indicator_octetslen = 0;

  while (true) {
    char _b = bytes[p];
    p += 1;

    indicator_octets = (indicator_octets << CHAR_WIDTH) + _b;
    indicator_octetslen += 1;

    if ((_b & 1) == 0) {
      break;
    }
  }

  QVector<quint16> indicators;
  const quint64 mask = (quint64)1 << (CHAR_WIDTH * indicator_octetslen - 1);
  quint32 indicator = 0;
  for (int i = 0; i < CHAR_WIDTH * indicator_octetslen; i++) {
    if (i % CHAR_WIDTH == 7) {
      continue;
    }

    if ((indicator_octets & (mask >> i)) > 0) {
      indicators.push_back(indicator);
    }

    indicator += 1;
  }

  QVariantMap results;
  quint16 index = 0;
  QDomNodeList datafield_cn = datafield.childNodes();
  for (int i = 0; i < datafield_cn.count(); i++) {
    QDomElement cn = datafield_cn.at(i).toElement();

    const QStringList cnList = {AN::fixedName, AN::repetitiveName,
                                AN::variableName, AN::compoundName};
    const QString nodeName = cn.nodeName();
    if (!cnList.contains(nodeName)) {
      continue;
    }

    if (!indicators.contains(index)) {
      index += 1;
      continue;
    }

    int rep_counter = 0;

    QVariant r;
    if (nodeName == AN::fixedName) {
      r = decode_fixed(cn);
    } else if (nodeName == AN::repetitiveName) {
      r = decode_repetitive(cn);
      rep_counter++;
    } else if (nodeName == AN::variableName) {
      r = decode_variable(cn);
    } else if (nodeName == AN::compoundName) {
      r = decode_compound(cn);
    }

    index += 1;

    if (r.isNull()) {
      continue;
    }

    if (r.type() == QVariant::Map) {
      results.insert(r.toMap());
    } else if (r.type() == QVariant::List) {
      const QString repStr = QString("R_%1").arg(rep_counter);
      results[repStr] = r.toList();
    }
  }

  return results;
}

} // namespace qasterix
