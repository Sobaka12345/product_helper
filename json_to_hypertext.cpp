#include "json_to_hypertext.h"
#include <QImage>
#include <QBuffer>
#include <QJsonArray>

void addHeader(char headerLevel, const QString& header, QString & processed)
{
    if(header.isEmpty())
        return;
    QString hdOpen = "<h1>";
    QString hdClose = "</h1>";
    hdOpen[2] = headerLevel;
    hdClose[3] = headerLevel;
    processed += hdOpen + header + hdClose;
}

void addArrayAsList(const QString& name, const QJsonArray& list, QString & processed, QChar delim = QChar())
{
    if(list.isEmpty())
        return;
    processed += "<p><b>" + name + "</b></p>";
    processed += "<ul>";
    for(const auto& x: list)
        if(!x.toString().isEmpty()) {
            if(!delim.isNull())
                processed += "<li>" + x.toString().split(delim).back().split('-').join(' ') + "</li>";
            else
                processed += "<li>" + x.toString() + "</li>";
        }
    processed += "</ul>";
}

void addArrayAsLine(const QString& name, const QJsonArray& list, QString & processed, QChar delim = QChar())
{
    if(list.isEmpty())
        return;
    processed += "<p><b>" + name + ": </b>";
    for(auto it = list.begin(); it != list.end() - 1; it++)
        if(!it->toString().isEmpty()) {
            if(!delim.isNull())
                processed += it->toString().split(delim).back().split('-').join(' ') + ", ";
            else
                processed += it->toString() + ", ";
        }
    auto last = list.last().toString();
    if(!last.isEmpty()) {
        if(!delim.isNull())
            processed += last.split(delim).back().split('-').join(' ');
        else
            processed += last;
    }
    processed += "</p>";
}

void addKeyVal(const QString& key, const QString& val, QString & processed)
{
    if(val.isEmpty())
        return;
    processed += "<p><b>" + key + ": </b>" + val + "</p>";
}
#include <QDebug>
void addImage(const QString& path, QString& processed)
{
    QImage img(path);
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    img.save(&buffer, "PNG");
    processed += QString("<br><img src=\"data:image/png;base64,") + byteArray.toBase64() + "\"/><br>";
}

QString jsonFacts(const QJsonValue& doc)
{
    auto info = doc.toVariant().toMap();
    QString str;
    addHeader('1', "Информация о продукте", str);
    addKeyVal("Код", doc["barcode"].toString(), str);
    addHeader('2', "Характеристики", str);
    addKeyVal("Наименование", doc["Name"].toString(), str);
    addArrayAsList("Категории", doc["CategoryTags"].toArray(), str, QChar('/'));
    addKeyVal("Брэнды", doc["BrandName"].toString().isEmpty()
            ? doc["OFF"]["brands"].toString(): doc["BrandName"].toString(), str);
    addKeyVal("Упаковка", doc["OFF"]["packaging"].toString(), str);
    addKeyVal("Масса нетто", doc["OFF"]["quantity"].toString(), str);
    addKeyVal("Страны", doc["OFF"]["countries"].toString(), str);
    addArrayAsLine("Метки, награды, сертификаты", doc["OFF"]["labels_tags"].toArray(), str, QChar(':'));

    addHeader('2', "Ингредиенты", str);
    addKeyVal("Состав", doc["OFF"]["ingredients_text"].toString(), str);
    addArrayAsLine("Аллергены", doc["OFF"]["allergens_tags"].toArray(), str, QChar(':'));
    addArrayAsList("Добавки", doc["OFF"]["additives_tags"].toArray(), str, QChar(':'));
    addArrayAsLine("Ингредиенты из пальмового масла", doc["OFF"]["ingredients_from_palm_oil_tags"].toArray(), str, QChar(':'));
    addArrayAsLine("группа NOVA", doc["OFF"]["nova_groups_tags"].toArray(), str, QChar(':'));
    addImage(":/pics/"+ QString::number(doc["OFF"]["nova_group"].toInt()) +".png", str);

    addHeader('2', "Питательные свойства", str);
    addImage(":/pics/"+ doc["OFF"]["nutriscore_data"]["grade"].toString() +".png", str);

    //Пищевая ценность
    str += "<p><b>Пищевая ценность на 100г:</b></p>";
    auto func = [&str] (const QString& key, const QString& val) {
        if(val.isEmpty())
            return;
        if(val == "high")
            str+= "<p><span style='color:red;font-size:large;'><b>◯ </b></span><span>" + key + " в большом количестве</span></p>";
        else if(val == "moderate")
            str+= "<p><span style='color:yellow;font-size:large;'><b>◯ </b></span><span>" + key + " в умеренном количестве</span></p>";
        else if(val == "low")
            str+= "<p><span style='color:green;font-size:large;'><b>◯ </b></span><span>" + key + " в малом количестве</span></p>";
    };

    func("Жиры",doc["OFF"]["nutrient_levels"]["fat"].toString());
    func("Насыщенные жиры",doc["OFF"]["nutrient_levels"]["saturated-fat"].toString());
    func("Сахара",doc["OFF"]["nutrient_levels"]["sugars"].toString());
    func("Соль",doc["OFF"]["nutrient_levels"]["salt"].toString());

    return str;
}
/*
QString jsonFacts(const QJsonDocument& doc)
{
    auto info = doc.toVariant().toMap();
    QString str;
    addHeader('1', "Информация о продукте", str);
    addHeader('2', "Характеристики", str);
    addKeyVal("Наименование", doc["Name"].toString(), str);
    addArrayAsList("Категории", doc["CategoryTags"].toArray(), str, QChar('/'));
    addKeyVal("Брэнды", doc["BrandName"].toString().isEmpty()
            ? doc["OFF"]["brands"].toString(): doc["BrandName"].toString(), str);
    addKeyVal("Упаковка", doc["OFF"]["packaging"].toString(), str);
    addKeyVal("Масса нетто", doc["OFF"]["quantity"].toString(), str);
    addKeyVal("Страны", doc["OFF"]["countries"].toString(), str);
    addArrayAsLine("Метки, награды, сертификаты", doc["OFF"]["labels_tags"].toArray(), str, QChar(':'));

    addHeader('2', "Ингредиенты", str);
    addKeyVal("Состав", doc["OFF"]["ingredients_text"].toString(), str);
    addArrayAsLine("Аллергены", doc["OFF"]["allergens_tags"].toArray(), str, QChar(':'));
    addArrayAsList("Добавки", doc["OFF"]["additives_tags"].toArray(), str, QChar(':'));
    addArrayAsLine("Ингредиенты из пальмового масла", doc["OFF"]["ingredients_from_palm_oil_tags"].toArray(), str, QChar(':'));
    addArrayAsLine("группа NOVA", doc["OFF"]["nova_groups_tags"].toArray(), str, QChar(':'));
    addImage(":/pics/"+ QString::number(doc["OFF"]["nova_group"].toInt()) +".png", str);

    addHeader('2', "Питательные свойства", str);
    addImage(":/pics/"+ doc["OFF"]["nutriscore_data"]["grade"].toString() +".png", str);

    return str;
}
*/

QString jsonRKResearches(const QJsonValue& doc)
{
    auto info = doc.toVariant().toMap();

    if(info.empty())
        return QString("Нету данных об исследованиях этого продукта. :C");
    else
    {
        QString prodRes = "<H1> РОСКАЧЕСТВО </H1>"
                    "<table><tr><td width='50%'>Оценка</td><td width='50%'>" +
                    info["points"].toString() + "</td></tr>";
        auto indicators = info["indicators"].toList();
        for(auto it = indicators.begin(); it != indicators.end(); it++)
        {
            prodRes += "<tr>";
            auto map = it->toMap();
            prodRes += "<td width='50%'>" + map["name"].toString() + "</td><td width='50%'>"
                    + map["value"].toString() + "</td>";
            prodRes += "</tr>";
        }
        prodRes += "</table><H2>Результаты исследования</H2>" + info["prod_info"].toMap()["research_results"].toString() + "";

        prodRes += "<table border='1' cellpadding='3'>"
                     "<tr><td width='50%' align='center'>+</td><td width='50%' align='center'>-</td></tr>";

        auto pros = info["prod_info"].toMap()["+"].toList();
        auto cons = info["prod_info"].toMap()["-"].toList();
        for(int i = 0; i < qMax(pros.size(), cons.size()); i++)
        {
            QString first = "", second = "";
            if(i < pros.size())
                first = pros[i].toString();
            if(i < cons.size())
                second = pros[i].toString();
            prodRes += "<tr>";
            prodRes += "<td width='50%' align='center'>" + first + "</td><td width='50%' align='center'>"
                    + second + "</td>";
            prodRes += "</tr>";
        }
        prodRes += "</table>";
        return prodRes;
    }

}
/*
QString jsonRKResearches(const QJsonDocument& doc)
{
    auto info = doc.toVariant().toMap();

    if(info.empty())
        return QString("Нету данных об исследованиях этого продукта. :C");
    else
    {
        QString prodRes = "<H1> РОСКАЧЕСТВО </H1>"
                    "<table><tr><td width='50%'>Оценка</td><td width='50%'>" +
                    info["points"].toString() + "</td></tr>";
        auto indicators = info["indicators"].toList();
        for(auto it = indicators.begin(); it != indicators.end(); it++)
        {
            prodRes += "<tr>";
            auto map = it->toMap();
            prodRes += "<td width='50%'>" + map["name"].toString() + "</td><td width='50%'>"
                    + map["value"].toString() + "</td>";
            prodRes += "</tr>";
        }
        prodRes += "</table><H2>Результаты исследования</H2>" + info["prod_info"].toMap()["research_results"].toString() + "";

        prodRes += "<table border='1'>"
                     "<tr><td width='50%'>+</td><td width='50%'>-</td></tr>";

        auto pros = info["prod_info"].toMap()["+"].toList();
        auto cons = info["prod_info"].toMap()["-"].toList();
        for(int i = 0; i < qMax(pros.size(), cons.size()); i++)
        {
            QString first = "", second = "";
            if(i < pros.size())
                first = pros[i].toString();
            if(i < cons.size())
                second = pros[i].toString();
            prodRes += "<tr>";
            prodRes += "<td width='50%'>" + first + "</td><td width='50%'>"
                    + second + "</td>";
            prodRes += "</tr>";
        }
        prodRes += "</table>";
        return prodRes;
    }

}
*/

QString jsonComments(const QJsonArray& doc)
{
    QString str;
    if(doc.isEmpty())
        str = "<p align='center'>Комментарии отсутствуют</p>";
    for(const auto& x: doc) {
        str += "<table width='100%' cellspacing=3 cellpadding=3 bgcolor='#bbd5db'>"
               "<tr><td>" + x["email"].toString() + "</td>" +
               "<td align='right'>" + x["strTime"].toString().split(' ')[0] + "</td><tr><td colspan=2>" +
                x["text"].toString() + "</td></tr>" +
                "<tr><td style='vertical-align:middle;'>___________________</td>"
                "<td align='right' style='color:gold;font-size:xx-large;'>";
                for(int i = 0; i < x["note"].toInt(); i++)
                    str+= "⭐";
                str+= "</td></tr></table>";
    }
    return str;
}
