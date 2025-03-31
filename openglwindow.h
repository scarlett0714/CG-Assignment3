#ifndef OPENGLWINDOW_H
#define OPENGLWINDOW_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QMouseEvent>

//UI Widget
#include <QCheckBox>
#include <QPushButton>
#include <QSlider>
#include <QLabel>


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

private slots: //assignment3 - 1
    // 조명 On/Off 토글
    void toggleLight0(bool enabled);

    // 셰이딩 모드 전환
    void setFlatShading();
    void setGouraudShading();

    // Ambient RGBA 조절
    void updateAmbientR(int value);
    void updateAmbientG(int value);
    void updateAmbientB(int value);
    void updateAmbientA(int value);

    // Specular RGBA 조절
    void updateSpecularR(int value);
    void updateSpecularG(int value);
    void updateSpecularB(int value);
    void updateSpecularA(int value);

private:
    ObjLoader objLoader;

    // 마우스 이벤트
    bool isDragging = false;
    bool isModelRotating = false;
    bool rotateFirstCow = true; // 클릭 위치로 회전 대상 결정
    QPoint lastMousePosition;

    // 각 소의 회전 값
    float cow1RotationX = 0.0f;
    float cow1RotationY = 0.0f;
    float cow1RotationZ = 0.0f;

    float cow2RotationX = 0.0f;
    float cow2RotationY = 0.0f;
    float cow2RotationZ = 0.0f;

    // 소 + 환경 그리기
    void drawCow();
    void drawFloorAndWalls();

    // 조명 상태
    bool light0On = true;
    GLfloat light0Pos[4] = { 5.0f, 5.0f, 5.0f, 1.0f };

    // 조명 특성
    GLfloat ambientLight[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat specularColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

    // 셰이딩 모드
    GLenum shadingModel = GL_SMOOTH;

    // UI 구성 요소
    QCheckBox* light0Checkbox;
    QPushButton* flatButton;
    QPushButton* gouraudButton;

    QSlider* ambientRSlider;
    QSlider* ambientGSlider;
    QSlider* ambientBSlider;
    QSlider* ambientASlider;

    QSlider* specularRSlider;
    QSlider* specularGSlider;
    QSlider* specularBSlider;
    QSlider* specularASlider;

    void setupUI(); // UI 초기화 함수
};

#endif // OPENGLWINDOW_H
