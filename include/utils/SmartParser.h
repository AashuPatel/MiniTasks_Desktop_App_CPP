#ifndef SMARTPARSER_H
#define SMARTPARSER_H

#include <QString>

struct ParsedTask {
    QString cleanText;
    qint64 alarmTime; // Epoch ms, 0 if no alarm
};

class SmartParser {
public:
    static ParsedTask parse(const QString& rawText);
};

#endif // SMARTPARSER_H
