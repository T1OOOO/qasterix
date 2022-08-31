#include "asterixtest.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QtDebug>

struct CheckStruct {
  int cat;
  QString jsonStr;
  QString dataStr;

  CheckStruct(int cat, QString jsonStr, QString dataStr)
      : cat(cat), jsonStr(jsonStr), dataStr(dataStr) {}
};

namespace {
const QVector<CheckStruct> checkList = {
    {62, "{\"100\": {\"X\": 4194303, \"Y\": 1}}", "3E000A047FFFFE000002"},
    {62, "{\"010\": {\"SIC\": 255, \"SAC\": 255}}", "3E000680FFFF"},
    {62, "{\"060\": {\"CH\": 1, \"Mode3A\": 1}}", "3E000701402001"},
    {62, "{\"290\": {\"TRK\": 1, \"UAT\": 1}}", "3E0009010241400404"},
    {62,
     "{\"295\": {\"MFL\": 1, \"SAL\": 1, \"RAN\": 1, \"PUN\": 1, \"BPS\": 1}}",
     "3E001001014041110903100404040404"},
    {240, "{\"030\": [{\"CHAR\": 1 }, {\"CHAR\": 1}]}", "F0000710020101"}};
}

QVariantMap getObj(const QString &str) {
  QVariantMap obj =
      QJsonDocument::fromJson(str.toLatin1()).object().toVariantMap();

  return obj;
}

TEST_F(AsterixTest, EncodeTest) {
  for (const auto &check : checkList) {
    const QVariantMap testObj = getObj(check.jsonStr);
    const QByteArray testData = QByteArray::fromHex(check.dataStr.toLatin1());
    const QByteArray encData = encoder.encode(check.cat, testObj);

    if (testData != encData) {
      qDebug() << testData << "!= " << encData;
    }

    EXPECT_EQ(testData, encData)
        << "Failed to encode message: " << testData.toHex().toStdString()
        << "; " << encData.toHex().toStdString();
  }
}

TEST_F(AsterixTest, DecodeTest) {
  for (const auto &check : checkList) {
    const QVariantMap testObj = getObj(check.jsonStr);
    const QByteArray testData = QByteArray::fromHex(check.dataStr.toLatin1());
    const QVariantMap decObj = decoder.decode(testData);

    if (testObj != decObj) {
      qDebug() << testObj << "!= " << decObj;
    }

    EXPECT_EQ(testObj, decObj) << "Failed to decode message: ";
  }
}

TEST_F(AsterixTest, EncodeDecodeTest) {
  for (const auto &check : checkList) {
    const QVariantMap testObj = getObj(check.jsonStr);
    const QByteArray encData = encoder.encode(check.cat, testObj);
    const QVariantMap decObj = decoder.decode(encData);

    if (testObj != decObj) {
      qDebug() << testObj << "!= " << decObj;
    }

    EXPECT_EQ(testObj, decObj)
        << "Failed to encode decode message: " << check.jsonStr.toStdString();
  }
}

TEST_F(AsterixTest, DecodeEncodeTest) {
  for (const auto &check : checkList) {
    const QVariantMap testObj = getObj(check.jsonStr);
    const QByteArray testData = QByteArray::fromHex(check.dataStr.toLatin1());
    const QVariantMap decObj = decoder.decode(testData);
    const QByteArray encData = encoder.encode(check.cat, decObj);

    if (testData != encData) {
      qDebug() << testData << "!= " << encData;
    }

    EXPECT_EQ(testData, encData)
        << "Failed to decode encode message: " << check.jsonStr.toStdString();
  }
}
