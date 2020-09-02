#ifndef ARVIDEOFILTER_H
#define ARVIDEOFILTER_H

#include <QAbstractVideoFilter>
#include <QVideoFilterRunnable>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>
#include <QOpenGLContext>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QThread>
#include <QUrl>

#include "QueryManager.h"
#include "OBJModelLoader.h"
#include "BarcodeFinder.h"
#include "HistorySQLQueryModel.h"
#include "json_to_hypertext.h"

class ARVideoFilter;

class ARVideoFilterRunnable : public QVideoFilterRunnable
{

public:
    QVideoFrame run(QVideoFrame *input, const QVideoSurfaceFormat &surfaceFormat, RunFlags flags);
    ARVideoFilterRunnable(ARVideoFilter * filter = nullptr);
    virtual ~ARVideoFilterRunnable();

private:
    ARVideoFilter * m_filter;
    QString m_lastCode;
    ThreadLauncher m_launcher, m_launcherContour;
    QThread m_recognitionThread;
    OBJModelLoader m_board, m_titles, m_circle;
    QMap<QChar, OBJModelLoader> m_symbols;
    QOpenGLShaderProgram m_modelProgram;
    GLuint m_ARFBO, m_ARTex, m_ARRBO;
    QJsonDocument m_product;
    glm::mat4x4 m_MVP;
    QImage readImage(QVideoFrame * input);
    inline void drawObject(const OBJModelLoader& obj);
    #ifdef FAST_ARM
    inline void findContour(QImage &image);
    #endif
};

class ARVideoFilter : public QAbstractVideoFilter
{
    Q_OBJECT;
    Q_PROPERTY(QString productAtts READ getProductAtts NOTIFY productAttsChanged);
    Q_PROPERTY(QString productRes READ getProductRes NOTIFY productResChanged);
    Q_PROPERTY(QString barcode READ getBarcode NOTIFY barcodeChanged);

public:
    ARVideoFilter(QueryManager& mgr, QObject * parent = nullptr) :
        QAbstractVideoFilter(parent),
        m_mgr(mgr)
    {
        connect(this, SIGNAL(request(QString)), this, SLOT(requestProduct(QString)));
    }

    void setProduct(const QJsonDocument & doc)
    {
        //штрих-код
        m_barcode = doc["product_info"]["barcode"].toString();
        //Данные о продукте
        m_prodAtts = jsonFacts(doc["product_info"]);
        //Данные об исследованиях
        m_prodRes = jsonRKResearches(doc["product_info"]["RK"]);

        emit productAttsChanged();
        emit productResChanged();
        emit barcodeChanged();
    }

    QVideoFilterRunnable * createFilterRunnable() override { return new ARVideoFilterRunnable(this); }

    const QString& getProductAtts() const { return m_prodAtts; }

    const QString& getProductRes() const { return m_prodRes; }

    bool isRequesting() const { return m_mgr.is_requesting(); }
    QJsonDocument result() const { return m_mgr.resultAsync(); }

    QString getBarcode() { return m_barcode; }

    ~ARVideoFilter() {}

private:
    QString m_prodAtts, m_prodRes, m_barcode;
    QueryManager& m_mgr;

private slots:
    void requestProduct(QString code) {  m_mgr.requestProductAsync(code); }

signals:
    void finished(QObject * result);
    void productDataModelChanged();
    void insertIntoHistory(const QJsonDocument& doc);
    void productAttsChanged();
    void productResChanged();
    void barcodeChanged();
    void request(QString code);
};


#endif // ARVIDEOFILTER_H
