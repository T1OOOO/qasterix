# qasterix
Library for encode and decode asterix messages

This project was inspired by project https://github.com/wontor/pyasterix
I have not found any free project for encode and decode asterix messages for c++.
I implemented it really close to python project for simpliciy. Also i wrote several tests for it.

Sample code:
```C++
  QAsterixLibrary::init(); //Important to read xml config files

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
```