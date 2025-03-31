#include <QApplication>
#include <QCoreApplication>
#include "openglwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    OpenGLWindow window;
    window.resize(800, 600);
    window.show();

    // QString objFilePath = QCoreApplication::applicationDirPath() + "/cow.obj";
    window.loadModel("/Users/hwang-yoonseon/Desktop/konkuk/wsu/cg/assignment_3/cow.obj");

    return app.exec();
}
