#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtDebug>

#include <qasterix/AsterixDecoder>
#include <qasterix/AsterixEncoder>
#include <qasterix/AsterixLibrary>

int main(int argc, char *argv[]) {
  QCoreApplication app(argc, argv);

  QAsterixLibrary::init();

  // Prepare data
  const int catNum = 240;
  const QString encObjStr =
      "{\"030\": [{\"CHAR\": 1 }, {\"CHAR\": 1}]}"; //"3E0009010241400404"

  const QVariantMap encObj =
      QJsonDocument::fromJson(encObjStr.toLatin1()).object().toVariantMap();
  qDebug() << "src obj: " << encObj;

  // Encode
  qasterix::AsterixEncoder encoder;
  const QByteArray data = encoder.encode(catNum, encObj);
  qDebug() << "encoded data: " << data.toHex();

  // Decode
  qasterix::AsterixDecoder decoder;
  const QVariantMap decObj = decoder.decode(data);
  qDebug() << "dst obj: " << decObj;

  return 0;
}
