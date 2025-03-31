#ifndef OPENGLWINDOW_H
#define OPENGLWINDOW_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QMouseEvent>
#include "objloader.h"

class OpenGLWindow : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit OpenGLWindow(QWidget *parent = nullptr);
    ~OpenGLWindow();

    void loadModel(const std::string& filename);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    ObjLoader objLoader;

    // 마우스 이벤트
    bool isDragging = false;  // 드래그 상태 확인
    bool isModelRotating = false; // 모델 회전 상태 확인
    QPoint lastMousePosition; // 이전 마우스 위치 저장

    float yaw = -90.0f;   // Y축 회전 (초기값: -90도)
    float pitch = 0.0f;   // X축 회전 (초기값: 0도)
    float sensitivity = 0.2f; // 마우스 감도 조절

    // 모델 회전 변수
    float modelRotationX = 0.0f;    // 모델의 X축 회전 각도
    float modelRotationY = 0.0f;    // 모델의 Y축 회전 각도
    float modelRotationZ = 0.0f;    // 모델의 Z축 회전 각도 (선택 사항)
    float modelRotationSensitivity = 0.5f; // 모델 회전 감도
};

#endif // OPENGLWINDOW_H
