#include "utils/SmartParser.h"
#include <QRegularExpression>
#include <QDateTime>

ParsedTask SmartParser::parse(const QString& rawText) {
    ParsedTask result;
    result.cleanText = rawText;
    result.alarmTime = 0;

    // Detect patterns like "in 15m", "in 15 mins", "in 1h"
    QRegularExpression re(R"(\b(?:in(?: exactly)?)\s+(\d+)\s*(m|min|mins|minutes|h|hr|hrs|hours)\b)", QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match = re.match(rawText);
    
    if (match.hasMatch()) {
        int amount = match.captured(1).toInt();
        QString unit = match.captured(2).toLower();
        
        qint64 msToAdd = 0;
        if (unit.startsWith("m")) {
            msToAdd = amount * 60 * 1000LL;
        } else if (unit.startsWith("h")) {
            msToAdd = amount * 60 * 60 * 1000LL;
        }

        if (msToAdd > 0) {
            result.alarmTime = QDateTime::currentMSecsSinceEpoch() + msToAdd;
            // Optionally, we could strip the "in 15m" from the text here, but keeping it is good for visibility.
        }
    }

    return result;
}
