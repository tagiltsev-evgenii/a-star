#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
//#include "unit.h"
#include "objectlist.h"

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QGuiApplication app(argc, argv);

    ObjectList objectList;

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

//    qmlRegisterType<Unit>("shortcut.example", 1, 0, "Unit");
    engine.rootContext()->setContextProperty("objectList", &objectList);
    engine.load(url);

    return app.exec();
}
