#ifndef JSON_TO_HYPERTEXT_H
#define JSON_TO_HYPERTEXT_H

#include <QString>
#include <QVariant>
#include <QJsonDocument>


//QString jsonRKResearches(const QJsonDocument& doc);
QString jsonRKResearches(const QJsonValue& doc);

//QString jsonFacts(const QJsonDocument& doc);
QString jsonFacts(const QJsonValue& doc);

QString jsonComments(const QJsonArray& doc);

#endif // JSON_TO_HYPERTEXT_H
