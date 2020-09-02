#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QMetaType>
#include <QFile>

#include <QtAndroidTools.h>

#include <glm/mat4x4.hpp>
Q_DECLARE_METATYPE(glm::mat4x4)

#include "ARVideoFilterRunnable.h"
#include "HistorySQLQueryModel.h"
#include "CategorySQLQueryModel.h"
#include "ProductSearchModel.h"

int main(int argc, char *argv[])
{
    QtAndroidTools::InitializeQmlTools();
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);

    //create database if not exists
    if(!QFile("./localDB.db").exists())
    {
        QFile DbFile;
        DbFile.setFileName("assets:/databases/localDB.db");
        DbFile.copy("./localDB.db");
        QFile::setPermissions("./localDB.db", QFile::WriteOwner | QFile::ReadOwner);
    }

    qRegisterMetaType<glm::mat4x4>();

    //SQLite database connection
    QSqlDatabase database(QSqlDatabase::addDatabase("QSQLITE"));
    database.setDatabaseName("localDB.db");
    database.open();

    //QueryManager
    QueryManager mgr("http://192.168.1.101:6969/api/products/");

    ARVideoFilter filter(mgr);
    HistorySQLQueryModel historyModel(database);
    QObject::connect(&filter, &ARVideoFilter::insertIntoHistory, &historyModel, &HistorySQLQueryModel::insertProduct);

    CategorySQLQueryModel categoryModel(database);
    QSortFilterProxyModel categoryFilter;
    categoryFilter.setSourceModel(&categoryModel);
    categoryFilter.setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
    categoryFilter.setFilterRole(258);

    ProductSearchModel productSearchModel(database, mgr);

    QQmlApplicationEngine engine;

    //Pass objects to QML
    engine.rootContext()->setContextProperty("category", &categoryFilter);
    engine.rootContext()->setContextProperty("filter", &filter);
    engine.rootContext()->setContextProperty("history", &historyModel);
    engine.rootContext()->setContextProperty("product", &productSearchModel);

    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);


    return app.exec();
}
