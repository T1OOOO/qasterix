#include "asterixtest.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QtDebug>
#include <QtMath>

struct CheckStruct {
  int cat;
  QString jsonStr;
  QString dataStr;

  CheckStruct(int cat, QString jsonStr, QString dataStr)
      : cat(cat), jsonStr(jsonStr), dataStr(dataStr) {}
};

bool IsEqualDouble(double dX, double dY) {
  static const double dEpsilon = 0.0001; // or some other small number
  return fabs(dX - dY) <= dEpsilon * fabs(dX);
}

bool compareVariants(const QVariant &v1, const QVariant &v2) {
  if (v1.type() != v1.type()) {
    return false;
  }

  if (v1.type() == QVariant::List) {
    const QVariantList v1List = v1.toList();
    const QVariantList v2List = v2.toList();

    if (v1List.size() != v2List.size()) {
      return false;
    }

    for (int i = 0; i < v1List.size(); i++) {
      const QVariant &v1Value = v1List.at(i);
      const QVariant &v2Value = v2List.at(i);
      if (!compareVariants(v1Value, v2Value)) {
        return false;
      }
    }
  } else if (v1.type() == QVariant::Map) {
    const QVariantMap v1Map = v1.toMap();
    const QVariantMap v2Map = v2.toMap();

    const QStringList v1Keys = v1Map.keys();
    const QStringList v2Keys = v2Map.keys();
    if (v1Keys.size() != v2Keys.size()) {
      return false;
    }

    for (const QString &v1Key : v1Keys) {
      if (!v2Map.contains(v1Key)) {
        return false;
      }

      const QVariant &v1Value = v1Map[v1Key];
      const QVariant &v2Value = v2Map[v1Key];
      if (!compareVariants(v1Value, v2Value)) {
        return false;
      }
    }
  } else if (v1.type() == QVariant::Double) {
    const double v1Value = v1.toDouble();
    const double v2Value = v2.toDouble();

    bool res = IsEqualDouble(v1Value, v2Value);

    return res;
  }

  return v1 == v2;
}

namespace {
const QVector<CheckStruct> checkList = {
    //    {62, R"({"100": {"X": 4194303, "Y": 1}})",
    //    "3E000A047FFFFE000002"},
    //{62, R"({"100": {"X": -1, "Y": 1}})", "3E000A04FFFFFE000002"},
    //    {62, R"({"105": {"Lat": -0.9999972581863403, "Lon":
    //    0.9999972581863403}})",
    //     "3E000C08FFFD27D30002D82D"},
    //    {62, R"({"010": {"SIC": 255, "SAC": 255}})", "3E000680FFFF"},
    //    {62, R"({"060": {"CH": 1, "Mode3A": 1}})", "3E000701402001"},
    //    {62, R"({"290": {"TRK": 1, "UAT": 1}})", "3E0009010241400404"},
    //    {62, R"({"295": {"MFL": 1, "SAL": 1, "RAN": 1, "PUN": 1, "BPS":
    //    1}})",
    //     "3E001001014041110903100404040404"},
    //    {240, R"({"030": [{"CHAR": 1 }, {"CHAR": 1}]})",
    //    "F0000710020101"}};
    {62, R"({"290": {"TRK": 1, "UAT": 1, "LOP": 1}})", "3E000801028081C0"},
};
}

QVariantMap getObj(const QString &str) {
  QVariantMap obj =
      QJsonDocument::fromJson(str.toLatin1()).object().toVariantMap();

  return obj;
}

TEST_F(AsterixTest, EncodeDecodeTest) {
  for (const auto &check : checkList) {
    const QVariantMap testObj = getObj(check.jsonStr);
    const QByteArray encData = encoder.encode(check.cat, testObj);
    const QVariantMap decObj = decoder.decode(encData);
    const QByteArray testData = QByteArray::fromHex(check.dataStr.toLatin1());

    if (!compareVariants(testObj, decObj)) {
      qDebug() << testObj << "!= " << decObj;
      qDebug() << testData.toHex() << "!= " << encData.toHex();
    }

    EXPECT_TRUE(compareVariants(testObj, decObj))
        << "Failed to encode decode message: " << check.jsonStr.toStdString();
  }
}

// TEST_F(AsterixTest, EncodeTest) {
//   for (const auto &check : checkList) {
//     const QVariantMap testObj = getObj(check.jsonStr);
//     const QByteArray testData =
//     QByteArray::fromHex(check.dataStr.toLatin1()); const QByteArray encData =
//     encoder.encode(check.cat, testObj);

//    if (testData != encData) {
//      qDebug() << testData << "!= " << encData;
//    }

//    EXPECT_EQ(testData, encData)
//        << "Failed to encode message: " << testData.toHex().toStdString()
//        << "; " << encData.toHex().toStdString();
//  }
//}

// TEST_F(AsterixTest, DecodeTest) {
//   for (const auto &check : checkList) {
//     const QVariantMap testObj = getObj(check.jsonStr);
//     const QByteArray testData =
//     QByteArray::fromHex(check.dataStr.toLatin1()); const QVariantMap decObj =
//     decoder.decode(testData);

//    if (testObj != decObj) {
//      qDebug() << testObj << "!= " << decObj;
//    }

//    EXPECT_EQ(testObj, decObj) << "Failed to decode message: ";
//  }
//}

// TEST_F(AsterixTest, DecodeEncodeTest) {
//   for (const auto &check : checkList) {
//     const QVariantMap testObj = getObj(check.jsonStr);
//     const QByteArray testData =
//     QByteArray::fromHex(check.dataStr.toLatin1()); const QVariantMap decObj =
//     decoder.decode(testData); const QByteArray encData =
//     encoder.encode(check.cat, decObj);

//    if (testData != encData) {
//      qDebug() << testData << "!= " << encData;
//    }

//    EXPECT_EQ(testData, encData)
//        << "Failed to decode encode message: " << check.jsonStr.toStdString();
//  }
//}
