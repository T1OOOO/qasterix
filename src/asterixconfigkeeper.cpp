#include "asterixconfigkeeper.h"

#include <QDir>
#include <QDomDocument>
#include <QFile>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

namespace {
constexpr int catNumSpace = 3;
constexpr int catNumDecimal = 10;
constexpr int majorMultiplicator = 1000;
} // namespace

AsterixConfigKeeper::AsterixConfigKeeper() {}

const bool AsterixConfigKeeper::hasCategory(const quint8 category) {
  static const QDir dir(":/config");

  const QString fileBegin =
      QString("asterix_cat%1")
          .arg(category, catNumSpace, catNumDecimal, QLatin1Char('0'));
  const QString filePattern = QString("%1_*_*.xml").arg(fileBegin);
  const QStringList fileNames = dir.entryList({filePattern});

  return !fileNames.isEmpty();
}

const QDomElement AsterixConfigKeeper::getCategory(const int category) {
  static const QDir dir(":/config");

  const QString fileBegin =
      QString("asterix_cat%1").arg(category, 3, 10, QLatin1Char('0'));
  const QString filePattern = QString("%1_*_*.xml").arg(fileBegin);
  QStringList fileNames = dir.entryList({filePattern});
  if (fileNames.isEmpty()) {
    return {};
  }

  const QString patternStr = QString("^%1_(\\d+)_(\\d+)\\.xml$").arg(fileBegin);
  const QRegularExpression pattern(patternStr);
  std::sort(fileNames.begin(), fileNames.end(),
            [&pattern](const auto &fileName1, const auto &fileName2) -> bool {
              const QRegularExpressionMatch match1 = pattern.match(fileName1);
              const QRegularExpressionMatch match2 = pattern.match(fileName2);
              if (!match1.hasMatch() || !match2.hasMatch()) {
                return false;
              }

              const int major1 = match1.captured(1).toInt();
              const int minor1 = match1.captured(2).toInt();
              const int number1 = major1 * majorMultiplicator + minor1;

              const int major2 = match2.captured(1).toInt();
              const int minor2 = match2.captured(2).toInt();
              const int number2 = major2 * majorMultiplicator + minor2;

              return (number1 < number2);
            });

  const QString &fileName = fileNames.last();
  const QString fileNameFull = dir.absoluteFilePath(fileName);
  QFile file(fileNameFull);
  if (!file.open(QIODevice::ReadOnly)) {
    return {};
  }

  QDomDocument dom;
  if (!dom.setContent(&file)) {
    file.close();
    return {};
  }
  file.close();

  return dom.documentElement();
}
