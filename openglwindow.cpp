#include "openglwindow.h"
#include <OpenGL/glu.h>
#include <iostream>

#include <QVBoxLayout>
#include <QHBoxLayout>

OpenGLWindow::OpenGLWindow(QWidget *parent)
    : QOpenGLWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus); // 입력을 받을 수 있도록 설정
    installEventFilter(this); // QT 이벤트 필터 감지
    setupUI(); // UI 초기화 호출
}

OpenGLWindow::~OpenGLWindow() {}

void OpenGLWindow::initializeGL() {
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glShadeModel(shadingModel);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, light0Pos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, specularColor);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularColor);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);
}

void OpenGLWindow::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
}

void OpenGLWindow::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, float(width()) / height(), 0.1, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.0, 3.0, 10.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    if (light0On) {
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glLightfv(GL_LIGHT0, GL_POSITION, light0Pos);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, specularColor);
        glLightfv(GL_LIGHT0, GL_SPECULAR, specularColor);
    } else {
        glEnable(GL_LIGHTING);
        GLfloat noDiffuse[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        glLightfv(GL_LIGHT0, GL_DIFFUSE, noDiffuse);
        glLightfv(GL_LIGHT0, GL_SPECULAR, noDiffuse);
    }

    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);
    glShadeModel(shadingModel);

    drawFloorAndWalls();

    // Draw first cow (left)
    glPushMatrix();
    glTranslatef(-2.5f, 0.0f, 0.0f);
    glRotatef(cow1RotationX, 1.0f, 0.0f, 0.0f);
    glRotatef(cow1RotationY, 0.0f, 1.0f, 0.0f);
    glRotatef(cow1RotationZ, 0.0f, 0.0f, 1.0f);
    glScalef(0.3f, 0.3f, 0.3f);
    glColor3f(1.0, 1.0, 1.0);
    drawCow();
    glPopMatrix();

    // Draw second cow (right)
    glPushMatrix();
    glTranslatef(2.5f, 0.0f, 0.0f);
    glRotatef(cow2RotationX, 1.0f, 0.0f, 0.0f);
    glRotatef(cow2RotationY, 0.0f, 1.0f, 0.0f);
    glRotatef(cow2RotationZ, 0.0f, 0.0f, 1.0f);
    glScalef(0.3f, 0.3f, 0.3f);
    glColor3f(1.0, 1.0, 1.0);
    drawCow();
    glPopMatrix();
}

void OpenGLWindow::loadModel(const std::string& filename) {
    if (objLoader.load(filename)) {
        std::cout << "Model loaded: " << filename << std::endl;
        update();
    } else {
        std::cerr << "Failed to load model." << std::endl;
    }
}

void OpenGLWindow::mouseMoveEvent(QMouseEvent *event) {
    float dx = event->pos().x() - lastMousePosition.x();
    float dy = event->pos().y() - lastMousePosition.y();

    if (isModelRotating) {
        if (rotateFirstCow) {
            cow1RotationY += dx * 0.5f;
            cow1RotationX += dy * 0.5f;
        } else {
            cow2RotationY += dx * 0.5f;
            cow2RotationX += dy * 0.5f;
        }
        update();
    }

    lastMousePosition = event->pos();
}

void OpenGLWindow::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        isModelRotating = true;
        lastMousePosition = event->pos();

        // Determine which cow to rotate based on click x position
        rotateFirstCow = (event->x() < width() / 2);
    }
}

void OpenGLWindow::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        isModelRotating = false;
    }
}

void OpenGLWindow::drawCow() {
    if (shadingModel == GL_FLAT) {
        glShadeModel(GL_FLAT);
        glBegin(GL_TRIANGLES);
        for (const auto& face : objLoader.faces) {
            const auto& v1 = objLoader.vertices[face.v1];
            const auto& v2 = objLoader.vertices[face.v2];
            const auto& v3 = objLoader.vertices[face.v3];

            float ux = v2.x - v1.x, uy = v2.y - v1.y, uz = v2.z - v1.z;
            float vx = v3.x - v1.x, vy = v3.y - v1.y, vz = v3.z - v1.z;

            float nx = uy * vz - uz * vy;
            float ny = uz * vx - ux * vz;
            float nz = ux * vy - uy * vx;

            float length = sqrt(nx * nx + ny * ny + nz * nz);
            if (length != 0.0f) { nx /= length; ny /= length; nz /= length; }

            glNormal3f(nx, ny, nz);
            glVertex3f(v1.x, v1.y, v1.z);
            glVertex3f(v2.x, v2.y, v2.z);
            glVertex3f(v3.x, v3.y, v3.z);
        }
        glEnd();
    } else {
        glShadeModel(GL_SMOOTH);

        // 1. 정점별 normal을 계산하기 위한 배열 초기화
        std::vector<QVector3D> vertexNormals(objLoader.vertices.size(), QVector3D(0, 0, 0));
        std::vector<int> count(objLoader.vertices.size(), 0);

        // 2. 각 face의 normal을 각 vertex에 더해줌
        for (const auto& face : objLoader.faces) {
            const auto& v1 = objLoader.vertices[face.v1];
            const auto& v2 = objLoader.vertices[face.v2];
            const auto& v3 = objLoader.vertices[face.v3];

            QVector3D vec1(v1.x, v1.y, v1.z);
            QVector3D vec2(v2.x, v2.y, v2.z);
            QVector3D vec3(v3.x, v3.y, v3.z);

            QVector3D normal = QVector3D::normal(vec2 - vec1, vec3 - vec1); // cross product
            normal.normalize();

            vertexNormals[face.v1] += normal;
            vertexNormals[face.v2] += normal;
            vertexNormals[face.v3] += normal;

            count[face.v1]++;
            count[face.v2]++;
            count[face.v3]++;
        }

        // 3. 평균화
        for (int i = 0; i < vertexNormals.size(); ++i) {
            if (count[i] > 0)
                vertexNormals[i] /= count[i];
        }

        // 4. 그리기
        glBegin(GL_TRIANGLES);
        for (const auto& face : objLoader.faces) {
            for (int idx : {face.v1, face.v2, face.v3}) {
                const auto& v = objLoader.vertices[idx];
                const QVector3D& n = vertexNormals[idx];
                glNormal3f(n.x(), n.y(), n.z());
                glVertex3f(v.x, v.y, v.z);
            }
        }
        glEnd();
    }

}

void OpenGLWindow::drawFloorAndWalls() {
    glDisable(GL_LIGHTING);
    glColor3f(0.5f, 0.5f, 0.5f);

    // 바닥
    glBegin(GL_QUADS);
    glVertex3f(-10.0f, -1.0f, -10.0f);
    glVertex3f(-10.0f, -1.0f, 10.0f);
    glVertex3f(10.0f, -1.0f, 10.0f);
    glVertex3f(10.0f, -1.0f, -10.0f);
    glEnd();

    // 벽 1
    glColor3f(0.4f, 0.4f, 0.6f);
    glBegin(GL_QUADS);
    glVertex3f(-10.0f, -1.0f, -10.0f);
    glVertex3f(-10.0f, 5.0f, -10.0f);
    glVertex3f(10.0f, 5.0f, -10.0f);
    glVertex3f(10.0f, -1.0f, -10.0f);
    glEnd();

    // 벽 2
    glColor3f(0.6f, 0.4f, 0.4f);
    glBegin(GL_QUADS);
    glVertex3f(-10.0f, -1.0f, 10.0f);
    glVertex3f(-10.0f, 5.0f, 10.0f);
    glVertex3f(10.0f, 5.0f, 10.0f);
    glVertex3f(10.0f, -1.0f, 10.0f);
    glEnd();

    // 벽 3 (왼쪽)
    glColor3f(0.4f, 0.6f, 0.4f);
    glBegin(GL_QUADS);
    glVertex3f(-10.0f, -1.0f, -10.0f);
    glVertex3f(-10.0f, 5.0f, -10.0f);
    glVertex3f(-10.0f, 5.0f, 10.0f);
    glVertex3f(-10.0f, -1.0f, 10.0f);
    glEnd();

    glEnable(GL_LIGHTING);
}

void OpenGLWindow::toggleLight0(bool enabled) {
    light0On = enabled;
    makeCurrent();
    update();
}

void OpenGLWindow::setFlatShading() {
    shadingModel = GL_FLAT;
    makeCurrent();
    glShadeModel(shadingModel);
    update();
}

void OpenGLWindow::setGouraudShading() {
    shadingModel = GL_SMOOTH;
    makeCurrent();
    glShadeModel(shadingModel);
    update();
}

void OpenGLWindow::updateAmbientR(int value) {
    ambientLight[0] = value / 100.0f;
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);
    update();
}

void OpenGLWindow::updateAmbientG(int value) {
    ambientLight[1] = value / 100.0f;
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);
    update();
}

void OpenGLWindow::updateAmbientB(int value) {
    ambientLight[2] = value / 100.0f;
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);
    update();
}

void OpenGLWindow::updateAmbientA(int value) {
    ambientLight[3] = value / 100.0f;
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);
    update();
}

void OpenGLWindow::updateSpecularR(int value) {
    specularColor[0] = value / 100.0f;
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularColor);
    update();
}

void OpenGLWindow::updateSpecularG(int value) {
    specularColor[1] = value / 100.0f;
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularColor);
    update();
}

void OpenGLWindow::updateSpecularB(int value) {
    specularColor[2] = value / 100.0f;
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularColor);
    update();
}

void OpenGLWindow::updateSpecularA(int value) {
    specularColor[3] = value / 100.0f;
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularColor);
    update();
}

void OpenGLWindow::setupUI() {
    QVBoxLayout* layout = new QVBoxLayout(this);

    light0Checkbox = new QCheckBox("Light 0", this);
    light0Checkbox->setChecked(true);
    connect(light0Checkbox, &QCheckBox::toggled, this, &OpenGLWindow::toggleLight0);

    flatButton = new QPushButton("Flat Shading", this);
    gouraudButton = new QPushButton("Gouraud Shading", this);
    connect(flatButton, &QPushButton::clicked, this, &OpenGLWindow::setFlatShading);
    connect(gouraudButton, &QPushButton::clicked, this, &OpenGLWindow::setGouraudShading);

    QLabel* ambientLabel = new QLabel("Ambient Light RGBA", this);
    QLabel* specularLabel = new QLabel("Specular Color RGBA", this);

    ambientRSlider = new QSlider(Qt::Horizontal, this);
    ambientGSlider = new QSlider(Qt::Horizontal, this);
    ambientBSlider = new QSlider(Qt::Horizontal, this);
    ambientASlider = new QSlider(Qt::Horizontal, this);

    specularRSlider = new QSlider(Qt::Horizontal, this);
    specularGSlider = new QSlider(Qt::Horizontal, this);
    specularBSlider = new QSlider(Qt::Horizontal, this);
    specularASlider = new QSlider(Qt::Horizontal, this);

    QList<QSlider*> sliders = {
        ambientRSlider, ambientGSlider, ambientBSlider, ambientASlider,
        specularRSlider, specularGSlider, specularBSlider, specularASlider
    };
    for (QSlider* slider : sliders) {
        slider->setRange(0, 100);
        slider->setValue(100);
    }

    connect(ambientRSlider, &QSlider::valueChanged, this, &OpenGLWindow::updateAmbientR);
    connect(ambientGSlider, &QSlider::valueChanged, this, &OpenGLWindow::updateAmbientG);
    connect(ambientBSlider, &QSlider::valueChanged, this, &OpenGLWindow::updateAmbientB);
    connect(ambientASlider, &QSlider::valueChanged, this, &OpenGLWindow::updateAmbientA);

    connect(specularRSlider, &QSlider::valueChanged, this, &OpenGLWindow::updateSpecularR);
    connect(specularGSlider, &QSlider::valueChanged, this, &OpenGLWindow::updateSpecularG);
    connect(specularBSlider, &QSlider::valueChanged, this, &OpenGLWindow::updateSpecularB);
    connect(specularASlider, &QSlider::valueChanged, this, &OpenGLWindow::updateSpecularA);

    layout->addWidget(light0Checkbox);
    layout->addWidget(flatButton);
    layout->addWidget(gouraudButton);
    layout->addWidget(ambientLabel);
    layout->addWidget(ambientRSlider);
    layout->addWidget(ambientGSlider);
    layout->addWidget(ambientBSlider);
    layout->addWidget(ambientASlider);
    layout->addWidget(specularLabel);
    layout->addWidget(specularRSlider);
    layout->addWidget(specularGSlider);
    layout->addWidget(specularBSlider);
    layout->addWidget(specularASlider);

    setLayout(layout);
}


