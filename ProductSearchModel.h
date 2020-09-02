#ifndef PRODUCTSEARCHMODEL_H
#define PRODUCTSEARCHMODEL_H

#include <QAbstractItemModel>
#include <QSqlDatabase>
#include <QSqlRecord>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QSqlQuery>
#include <QFuture>
#include <QStringList>
#include <QStringListModel>
#include <QQueue>

#include <QDebug>

#include "json_to_hypertext.h"
#include "QueryManager.h"

class ProductSearchModel: public QAbstractItemModel
{
    Q_OBJECT
    Q_PROPERTY(const QString& comments READ getComments NOTIFY commentsChanged)

    const int PAGE_LENGTH = 15;

public:
    ProductSearchModel(QSqlDatabase& db,QueryManager &mgr, QObject * parent = nullptr):
        QAbstractItemModel(parent),
        m_db(db),
        m_mgr(mgr),
        m_labels(m_labelsList)
    {
        connect(&m_mgr, &QueryManager::dataLoaded, this, &ProductSearchModel::processDocument);
    }

    Q_INVOKABLE void sendQuery(QVariant args, quint32 catID)
    {
        auto res = m_db.exec("SELECT name FROM category WHERE id = " + QString::number(catID) + ";");
        res.next();
        m_mgr.customGetRequest(QString("http://192.168.1.101:6969/api/productsList/")
                             + "CategoryTags=" + res.record().value(0).toString().split("/").join("|")
                             + "&" + args.toString());
    }

    Q_INVOKABLE QStringList getLabels(quint32 catID)
    {
        QSqlQuery res = m_db.exec("SELECT labels FROM category WHERE id = " + QString::number(catID) + ";");
        m_labelsList.clear();
        res.next();
        QStringList tmp = res.record()
                      .value(0)
                      .toString()
                      .split(";");
        tmp.sort();
        m_labelsList.append(qMove(tmp));

        return m_labelsList;
    }

    Q_INVOKABLE bool canFetch() const
    {
        return m_canFetch;
    }

    Q_INVOKABLE void fetchMore(const QModelIndex &parent = QModelIndex()) override
    {
        Q_UNUSED(parent);

        QVector<QJsonObject> temp;
        if(m_unprocessed.isEmpty()) {
            for(quint8 i = 0; i < PAGE_LENGTH; i++)
            {
                if(!m_queryData.next()) {
                    m_canFetch = false;
                    break;
                }
                temp.append(QJsonDocument::fromJson(
                                m_queryData.record()
                                           .value(1)
                                           .toString()
                                           .toUtf8()
                                ).object()
                            );
            }
        } else {
            for(quint8 i = 0; i < PAGE_LENGTH; i++)
            {
                if(m_unprocessed.isEmpty()) {
                    m_canFetch = false;
                    break;
                }
                temp.append(m_unprocessed.dequeue());
            }
        }

        beginInsertRows(QModelIndex(), m_docs.size(), m_docs.size() + temp.size() - 1);

        for(int i =0; i < temp.size(); i++)
            m_docs.append(qMove(temp[i]));

        endInsertRows();
    }

    Q_INVOKABLE void update(quint32 catID)
    {
        m_unprocessed.clear();
        m_queryData = m_db.exec("SELECT product.id, product_info FROM product, product_category_link, "
                                "category where category.id = " + QString::number(catID) + " AND "
                                "product.id = product_category_link.product_id"
                                " AND category.id = product_category_link.category_id;");
        m_canFetch = true;
        m_docs.clear();

        for(quint8 i = 0; i < PAGE_LENGTH; i++)
        {
            if(!m_queryData.next()) {
                m_canFetch = false;
                break;
            }
            m_docs.append(QJsonDocument::fromJson(m_queryData.record().value(1).toString().toUtf8()).object());
        }
    }

    Q_INVOKABLE void requestComments(const QString& barcode)
    {
        m_comments.clear();
        m_mgr.customGetRequest(QString("http://192.168.1.101:6969/api/getComments/" + barcode));
    }

    Q_INVOKABLE void setUserEmail(const QString& email)
    {
        m_userEmail = email;
    }

    Q_INVOKABLE void sendFeedback(int mark, const QString& barcode, const QString& feedback = QString())
    {
        if(!mark || barcode.isEmpty())
            return;
        QByteArray data = QString("{\"email\":\"" + m_userEmail + "\",\"code\":\"" + barcode + "\",\"mark\":" + QString::number(mark)).toUtf8();
        if(!feedback.isEmpty())
            data.append(",\"feedback\":\"" + feedback + "\"");
        data.append("}");
        m_mgr.customPostRequest(QUrl("http://192.168.1.101:6969/api/postFeedback"), data);
    }


    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    {
        if(role < Qt::UserRole + 5)
            return m_docs.at(index.row())[QString(roleNames()[role])];
        else if(role < Qt::UserRole + 12)
            return m_docs.at(index.row())["OFF"][QString(roleNames()[role])];
        else if(role < Qt::UserRole + 13)
            return m_docs.at(index.row())["OFF"]["nutriscore_data"][QString(roleNames()[role])];
        else if(role == Qt::UserRole + 13)
            return jsonFacts(m_docs.at(index.row()));
        else if(role == Qt::UserRole + 14)
            return jsonRKResearches(m_docs.at(index.row())[QString(roleNames()[role])]);

        return QVariant();
    }

    QHash<int, QByteArray> roleNames() const override
    {
        QHash<int, QByteArray> roles;

        roles[Qt::UserRole + 1] = "barcode";
        roles[Qt::UserRole + 2] = "Name";
        roles[Qt::UserRole + 3] = "BrandName";
        roles[Qt::UserRole + 4] = "CategoryTags";
        roles[Qt::UserRole + 5] = "allergens";
        roles[Qt::UserRole + 6] = "labels";
        roles[Qt::UserRole + 7] = "nova_group";
        roles[Qt::UserRole + 8] = "additives_tags";
        roles[Qt::UserRole + 9] = "labels";
        roles[Qt::UserRole + 10] = "countries";
        roles[Qt::UserRole + 11] = "ingredients_text";
        roles[Qt::UserRole + 12] = "grade";
        roles[Qt::UserRole + 13] = "FACTS";
        roles[Qt::UserRole + 14] = "RK";

        return roles;
    }

    const QString& getComments()
    {
        return m_comments;
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
        Q_UNUSED(parent)
        return m_docs.length();
    }

    int columnCount(const QModelIndex &parent = QModelIndex()) const override
    {
        Q_UNUSED(parent)
        return 1;
    }

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override
    {
        Q_UNUSED(parent)
        return createIndex(row, column, nullptr);
    }

    QModelIndex parent(const QModelIndex &child) const override
    {
        return child;
    }


private:
    QSqlDatabase& m_db;
    QSqlQuery m_queryData;
    QueryManager &m_mgr;
    QStringList m_labelsList;
    QStringListModel m_labels;
    QString m_userEmail;
    bool m_canFetch;

    QVector<QJsonObject> m_docs;
    QQueue<QJsonObject> m_unprocessed;
    QString m_comments;

private slots:
    void processDocument()
    {
        QJsonDocument res = m_mgr.resultCustom();
        if(!res.isArray())
        {
            m_comments = jsonComments(res["feedbacks"].toArray());
            emit commentsChanged();
            return;
        }
        auto array = res.array();

        m_docs.clear();
        m_queryData.clear();
        m_unprocessed.clear();

        emit layoutAboutToBeChanged();
        int i = 0;
        for(; i < qMin(PAGE_LENGTH, array.size()); i++)
            m_docs.append(array[i].toObject());

        emit layoutChanged();

        if(i == array.size() - 1)
            m_canFetch = false;
        else
            m_canFetch = true;
        for(; i < array.size(); i++)
            m_unprocessed.enqueue(array[i].toObject());
    }

signals:
    void commentsChanged();
};

#endif // PRODUCTSEARCHMODEL_H
