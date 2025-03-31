#include "openglwindow.h"
#include <OpenGL/glu.h>
#include <iostream>

OpenGLWindow::OpenGLWindow(QWidget *parent)
    : QOpenGLWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus); // 입력을 받을 수 있도록 설정
    installEventFilter(this); // QT 이벤트 필터 감지
}

OpenGLWindow::~OpenGLWindow() {}

void OpenGLWindow::initializeGL() {
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
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
    gluLookAt(0.0, 0.0, 5.0,   // camera position
              0.0, 0.0, 0.0,   // look at
              0.0, 1.0, 0.0);  // up vector

    glPushMatrix();

    glRotatef(modelRotationX, 1.0f, 0.0f, 0.0f);
    glRotatef(modelRotationY, 0.0f, 1.0f, 0.0f);
    glRotatef(modelRotationZ, 0.0f, 0.0f, 1.0f);

    glScalef(0.3f, 0.3f, 0.3f);

    glColor3f(1.0, 1.0, 1.0);

    glBegin(GL_TRIANGLES);
    for (const auto& face : objLoader.faces) {
        glVertex3f(objLoader.vertices[face.v1].x, objLoader.vertices[face.v1].y, objLoader.vertices[face.v1].z);
        glVertex3f(objLoader.vertices[face.v2].x, objLoader.vertices[face.v2].y, objLoader.vertices[face.v2].z);
        glVertex3f(objLoader.vertices[face.v3].x, objLoader.vertices[face.v3].y, objLoader.vertices[face.v3].z);
    }
    glEnd();

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

// 마우스를 클릭하면 드래그 시작
void OpenGLWindow::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        isModelRotating = true;
        lastMousePosition = event->pos();
    }
}

// 마우스를 이동하면 카메라 회전
void OpenGLWindow::mouseMoveEvent(QMouseEvent *event) {
    float dx = event->pos().x() - lastMousePosition.x();
    float dy = event->pos().y() - lastMousePosition.y();

    if (isModelRotating) {
        modelRotationY += dx * 0.5f;
        modelRotationX += dy * 0.5f;
        update();
    }

    lastMousePosition = event->pos(); // 현재 마우스 위치 저장
}

// 마우스를 놓으면 드래그 종료
void OpenGLWindow::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        isDragging = false;
    }else if (event->button() == Qt::RightButton) {
        isModelRotating = false;
    }
}
