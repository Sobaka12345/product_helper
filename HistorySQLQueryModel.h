#ifndef HISTORYSQLQUERYMODEL_H
#define HISTORYSQLQUERYMODEL_H

#include <QSqlQueryModel>
#include <QJsonDocument>
#include <QSqlQuery>
#include <QSqlError>
#include <QHash>
#include <QDebug>

#include "json_to_hypertext.h"

class HistorySQLQueryModel : public QSqlQueryModel
{
    Q_OBJECT

private:
    const char* SQL_SELECT = "SELECT barcode, product_info FROM history ORDER BY serial DESC;";
    QSqlDatabase& m_db;

public:
    HistorySQLQueryModel(QSqlDatabase& db, QObject *parent = nullptr) :
        QSqlQueryModel(parent),
        m_db(db)
    {
        refresh();
    }

    void refresh(){
        setQuery(SQL_SELECT, m_db);
    }

    QVariant data(const QModelIndex &index, int role) const override
    {
        QVariant value;
        switch (role) {
            case Qt::UserRole + 1: //barcode
            {
                int columnIdx = role - Qt::UserRole - 1;
                QModelIndex modelIndex = this->index(index.row(), columnIdx);
                value = QSqlQueryModel::data(modelIndex, Qt::DisplayRole);
                break;
            }
            case Qt::UserRole + 2: //RK
            {
                QModelIndex modelIndex = this->index(index.row(), 1);
                value = jsonRKResearches(QJsonDocument::fromJson(
                                             QSqlQueryModel::data(modelIndex, Qt::DisplayRole)
                                             .toString()
                                             .toUtf8()
                                        )["product_info"]["RK"]
                        );
                break;
            }
            case Qt::UserRole + 3: //FACTS
            {
                QModelIndex modelIndex = this->index(index.row(), 1);
                value = jsonFacts(QJsonDocument::fromJson(
                                      QSqlQueryModel::data(modelIndex, Qt::DisplayRole)
                                      .toString()
                                      .toUtf8()
                                 )["product_info"]
                        );
                break;
            }
            case Qt::UserRole + 4: //Name
            {
                QModelIndex modelIndex = this->index(index.row(), 1);
                value = QJsonDocument::fromJson(
                            QSqlQueryModel::data(modelIndex, Qt::DisplayRole)
                            .toString()
                            .toUtf8()
                            )["product_info"]["Name"];
                break;
            }
            case Qt::UserRole + 5: //BrandName
            {
                QModelIndex modelIndex = this->index(index.row(), 1);
                auto doc = QJsonDocument::fromJson(QSqlQueryModel::data(modelIndex, Qt::DisplayRole)
                                                   .toString()
                                                   .toUtf8()
                                                   )["product_info"];
                if(doc["BrandName"].isUndefined() || doc["BrandName"].isNull())
                    value = doc["OFF"]["brands"];
                else
                    value = doc["BrandName"];
                break;
            }
            case Qt::UserRole + 6: //grade
            {
                QModelIndex modelIndex = this->index(index.row(), 1);
                value = QJsonDocument::fromJson(QSqlQueryModel::data(modelIndex, Qt::DisplayRole)
                                                .toString()
                                                .toUtf8()
                                                )["product_info"]["OFF"]["nutriscore_data"]["grade"];
                break;
            }
            case Qt::UserRole + 7: //nova_group
            {
                QModelIndex modelIndex = this->index(index.row(), 1);
                value = QJsonDocument::fromJson(QSqlQueryModel::data(modelIndex, Qt::DisplayRole)
                                                .toString()
                                                .toUtf8()
                                                )["product_info"]["OFF"]["nova_group"];
                break;
            }
            default:
            {
                value = QSqlQueryModel::data(index, role);
            }
        }
        return value;
    }

    QHash<int, QByteArray> roleNames() const override
    {
        QHash<int, QByteArray> roleNames = QSqlQueryModel::roleNames();;

        roleNames[Qt::UserRole + 1] = "barcode";
        roleNames[Qt::UserRole + 2] = "RK";
        roleNames[Qt::UserRole + 3] = "FACTS";
        roleNames[Qt::UserRole + 4] = "Name";
        roleNames[Qt::UserRole + 5] = "BrandName";
        roleNames[Qt::UserRole + 6] = "grade";
        roleNames[Qt::UserRole + 7] = "nova_group";

        return roleNames;
    }

signals:

public slots:
    void insertProduct(const QJsonDocument& doc)
    {
        QString query = "INSERT INTO history (product_info, barcode) VALUES ('"+
                doc.toJson(QJsonDocument::Compact) +
                "', '" + doc["product_info"]["barcode"].toString() + "');";
        QSqlQuery sl = m_db.exec(query.toUtf8());

        refresh();
    }

};

#endif // HISTORYSQLQUERYMODEL_H
