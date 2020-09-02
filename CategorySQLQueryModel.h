#ifndef CATEGORYSQLQUERYMODEL_H
#define CATEGORYSQLQUERYMODEL_H

#include <QSortFilterProxyModel>
#include <QSqlQueryModel>

class CategorySQLQueryModel : public QSqlQueryModel
{
    Q_OBJECT
    const char* SQL_SELECT = "SELECT id, name FROM category ORDER BY id;";
    QSqlDatabase& m_db;

public:
    CategorySQLQueryModel(QSqlDatabase& db, QObject *parent = nullptr) :
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
        if(role < Qt::UserRole)
        {
            value = QSqlQueryModel::data(index, role);
        }
        else
        {
            int columnIdx = role - Qt::UserRole - 1;
            QModelIndex modelIndex = this->index(index.row(), columnIdx);
            value = QSqlQueryModel::data(modelIndex, Qt::DisplayRole);
        }
        return value;
    }

    QHash<int, QByteArray> roleNames() const override
    {
         QHash<int, QByteArray> roleNames = QSqlQueryModel::roleNames();

        roleNames[Qt::UserRole + 1] = "id";
        roleNames[Qt::UserRole + 2] = "name";

        return roleNames;
    }
};

#endif // CATEGORYSQLQUERYMODEL_H
