#ifndef QUERYMANAGER_H
#define QUERYMANAGER_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonParseError>

class QueryManager : public QObject
{
    Q_OBJECT

    QNetworkAccessManager m_mgr;
    QString m_url;
    bool m_requestState;
    QJsonDocument m_docAsync, m_doc;
    QNetworkReply * m_asyncGet, * m_customGet, * m_customPost;

public:
    QueryManager(const QString& url, QObject * parent = nullptr):
        QObject(parent),
        m_mgr(this),
        m_url(url),
        m_requestState(false)
    {}

    void customGetRequest(const QUrl& url)
    {
        m_customGet = m_mgr.get(QNetworkRequest(QUrl(url)));
        connect(m_customGet, SIGNAL(finished()), this, SLOT(performGetAnswer()));
    }

    void requestProductAsync(const QString& barcode)
    {
        m_requestState = true;
        m_asyncGet = m_mgr.get(QNetworkRequest(QUrl(m_url + barcode)));
        connect(m_asyncGet, SIGNAL(finished()), this, SLOT(performAsyncGetAnswer()));
    }

    void customPostRequest(const QUrl& url, const QByteArray& data)
    {
        QNetworkRequest req(url);
        req.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "application/json");
        req.setHeader(QNetworkRequest::KnownHeaders::ContentLengthHeader, data.length());
        m_customPost = m_mgr.post(req, data);
        connect(m_customPost, SIGNAL(finished()), this, SLOT(performPostAnswer()));
    }

    QJsonDocument resultCustom() { return qMove(m_doc); }

    QJsonDocument resultAsync() { return qMove(m_docAsync); }

    bool is_requesting() const { return m_requestState; }

signals:
    void dataLoaded();

private slots:
    void performAsyncGetAnswer()
    {
        if(m_asyncGet->error() != QNetworkReply::NoError)
        {
            m_requestState = false;
            m_asyncGet->deleteLater();
            return;
        }

        QJsonParseError jsonError;
        m_docAsync = QJsonDocument::fromJson(m_asyncGet->readAll(), &jsonError);

        if (jsonError.error != QJsonParseError::NoError)
            qDebug() << jsonError.errorString();

        m_asyncGet->deleteLater();
        m_requestState = false;
    }

    void performPostAnswer()
    {
        m_customPost->deleteLater();
    }

    void performGetAnswer()
    {
        if(m_customGet->error() != QNetworkReply::NoError)
        {
            m_customGet->deleteLater();
            return;
        }

        QJsonParseError jsonError;
        m_doc = QJsonDocument::fromJson(m_customGet->readAll(), &jsonError);

        if (jsonError.error != QJsonParseError::NoError)
            qDebug() << jsonError.errorString();

        emit dataLoaded();

        m_customGet->deleteLater();
    }
};

#endif // QUERYMANAGER_H
