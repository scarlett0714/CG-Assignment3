#include "openglwindow.h"
#include <OpenGL/glu.h>
#include <iostream>

#include <QVBoxLayout>
#include <QHBoxLayout>

OpenGLWindow::OpenGLWindow(QWidget *parent)
    : QOpenGLWidget(parent), cowTexture(nullptr)
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
    glEnable(GL_TEXTURE_2D); // 텍스처 사용 활성화
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glShadeModel(shadingModel);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

    // LIGHT0 설정
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, light0Pos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, specularColor);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularColor);

    // LIGHT1 설정
    glEnable(GL_LIGHT1);
    glLightfv(GL_LIGHT1, GL_POSITION, light1Pos);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1Diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light1Diffuse); // specular도 동일하게


    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);

    // 텍스처 로드
    QImage img("/Users/hwang-yoonseon/Desktop/konkuk/wsu/cg/assignment_3/cow-tex-fin.jpg");
    if (!img.isNull()) {
        cowTexture = new QOpenGLTexture(img.mirrored());
        cowTexture->setMinificationFilter(QOpenGLTexture::Linear);
        cowTexture->setMagnificationFilter(QOpenGLTexture::Linear);
        cowTexture->setWrapMode(QOpenGLTexture::Repeat);
        std::cout << "Texture loaded successfully." << std::endl;
    } else {
        std::cerr << "Failed to load cow_texture.jpg" << std::endl;
    }
}

void OpenGLWindow::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
}

void OpenGLWindow::paintGL() {

    // (1) Ray Tracing 텍스처 생성
    if (useRayTracing) {
        renderRayTracing();
        return;
    }

    // (2) OpenGL 씬 클리어 및 설정
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

    // Light 1
    if (light1On) {
        glEnable(GL_LIGHT1);
        glLightfv(GL_LIGHT1, GL_POSITION, light1Pos);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, light1Diffuse);
        glLightfv(GL_LIGHT1, GL_SPECULAR, light1Diffuse);
    } else {
        glDisable(GL_LIGHT1);
    }

    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);
    glShadeModel(shadingModel);

    // (3) 씬 렌더링
    drawFloorAndWalls();

    // Draw first cow (left)
    glPushMatrix();
    glTranslatef(-2.5f, autoOffsetY, 0.0f);
    glRotatef(cow1RotationX, 1.0f, 0.0f, 0.0f);
    glRotatef(cow1RotationY, 0.0f, 1.0f, 0.0f);
    glRotatef(cow1RotationZ, 0.0f, 0.0f, 1.0f);
    glScalef(0.3f, 0.3f, 0.3f);
    glColor3f(1.0, 1.0, 1.0);
    drawCow();
    glPopMatrix();

    // Draw second cow (right)
    glPushMatrix();
    glTranslatef(2.5f, autoOffsetY, 0.0f);
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

        // === autoOffsetY 계산 ===
        float minY = std::numeric_limits<float>::max();
        for (const auto& v : objLoader.vertices) {
            if (v.y < minY) minY = v.y;
        }
        autoOffsetY = -minY;  // 바닥에 닿도록 offset 설정

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

    if (cowTexture) cowTexture->bind(); // 텍스처 바인딩

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

            // 임의 텍스처 좌표 생성 (x, z 기반)
            glTexCoord2f((v1.x + 1.0f) * 0.5f, (v1.z + 1.0f) * 0.5f); glVertex3f(v1.x, v1.y, v1.z);
            glTexCoord2f((v2.x + 1.0f) * 0.5f, (v2.z + 1.0f) * 0.5f); glVertex3f(v2.x, v2.y, v2.z);
            glTexCoord2f((v3.x + 1.0f) * 0.5f, (v3.z + 1.0f) * 0.5f); glVertex3f(v3.x, v3.y, v3.z);
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
                glTexCoord2f((v.x + 1.0f) * 0.5f, (v.z + 1.0f) * 0.5f); // 임의 텍스처 좌표 (v.x, v.z 기준)
                glVertex3f(v.x, v.y, v.z);
            }
        }
        glEnd();
    }

    if (cowTexture) cowTexture->release(); // 텍스처 해제

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

void OpenGLWindow::toggleLight1(bool enabled) {
    light1On = enabled;
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
    QWidget* wrapper = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(wrapper);

    // ----------- 상단 UI 영역 ----------
    QWidget* topUI = new QWidget(this);
    QVBoxLayout* controlLayout = new QVBoxLayout(topUI);

    // Light 체크박스
    light0Checkbox = new QCheckBox("Light 0", this);
    light0Checkbox->setChecked(true);
    connect(light0Checkbox, &QCheckBox::toggled, this, &OpenGLWindow::toggleLight0);

    // Light 1 토글
    light1Checkbox = new QCheckBox("Light 1", this);
    light1Checkbox->setChecked(true);
    connect(light1Checkbox, &QCheckBox::toggled, this, &OpenGLWindow::toggleLight1);

    controlLayout->addWidget(light0Checkbox);
    controlLayout->addWidget(light1Checkbox);

    // 셰이딩 버튼 가로 정렬
    flatButton = new QPushButton("Flat Shading", this);
    gouraudButton = new QPushButton("Gouraud Shading", this);
    connect(flatButton, &QPushButton::clicked, this, &OpenGLWindow::setFlatShading);
    connect(gouraudButton, &QPushButton::clicked, this, &OpenGLWindow::setGouraudShading);

    QHBoxLayout* shadingButtonsLayout = new QHBoxLayout();
    shadingButtonsLayout->addWidget(flatButton);
    shadingButtonsLayout->addWidget(gouraudButton);

    // Ambient 슬라이더
    ambientRSlider = new QSlider(Qt::Horizontal, this);
    ambientGSlider = new QSlider(Qt::Horizontal, this);
    ambientBSlider = new QSlider(Qt::Horizontal, this);
    ambientASlider = new QSlider(Qt::Horizontal, this);

    // Specular 슬라이더
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

    // 슬라이더 연결
    connect(ambientRSlider, &QSlider::valueChanged, this, &OpenGLWindow::updateAmbientR);
    connect(ambientGSlider, &QSlider::valueChanged, this, &OpenGLWindow::updateAmbientG);
    connect(ambientBSlider, &QSlider::valueChanged, this, &OpenGLWindow::updateAmbientB);
    connect(ambientASlider, &QSlider::valueChanged, this, &OpenGLWindow::updateAmbientA);

    connect(specularRSlider, &QSlider::valueChanged, this, &OpenGLWindow::updateSpecularR);
    connect(specularGSlider, &QSlider::valueChanged, this, &OpenGLWindow::updateSpecularG);
    connect(specularBSlider, &QSlider::valueChanged, this, &OpenGLWindow::updateSpecularB);
    connect(specularASlider, &QSlider::valueChanged, this, &OpenGLWindow::updateSpecularA);

    // Ambient 슬라이더 묶음
    QVBoxLayout* ambientLayout = new QVBoxLayout();
    QLabel* ambientLabel = new QLabel("Ambient Light RGBA", this);
    ambientLabel->setAlignment(Qt::AlignCenter);
    ambientLayout->addWidget(ambientLabel);
    ambientLayout->addWidget(ambientRSlider);
    ambientLayout->addWidget(ambientGSlider);
    ambientLayout->addWidget(ambientBSlider);
    ambientLayout->addWidget(ambientASlider);

    // Specular 슬라이더 묶음
    QVBoxLayout* specularLayout = new QVBoxLayout();
    QLabel* specularLabel = new QLabel("Specular Color RGBA", this);
    specularLabel->setAlignment(Qt::AlignCenter);
    specularLayout->addWidget(specularLabel);
    specularLayout->addWidget(specularRSlider);
    specularLayout->addWidget(specularGSlider);
    specularLayout->addWidget(specularBSlider);
    specularLayout->addWidget(specularASlider);

    // 양쪽으로 배치
    QHBoxLayout* slidersLayout = new QHBoxLayout();
    slidersLayout->addLayout(ambientLayout);
    slidersLayout->addLayout(specularLayout);

    // UI 정리
    controlLayout->addWidget(light0Checkbox);
    controlLayout->addLayout(shadingButtonsLayout);
    controlLayout->addLayout(slidersLayout);

    // ------------ 전체 구성 ------------
    mainLayout->addWidget(topUI);
    mainLayout->addStretch(); // 나머지 공간은 OpenGL 렌더링용으로 비워둠

    wrapper->setLayout(mainLayout);
    wrapper->setMinimumHeight(250); // 상단 UI 공간 확보 (원하는 높이로 조절)

    QVBoxLayout* outerLayout = new QVBoxLayout(this);
    outerLayout->addWidget(wrapper);
    setLayout(outerLayout);
}

// 레이와 삼각형 교차 체크
bool OpenGLWindow::intersectRayTriangle(const Ray& ray, const QVector3D& v0, const QVector3D& v1, const QVector3D& v2, float& t, QVector3D& normal) {
    const float EPSILON = 1e-6;
    QVector3D edge1 = v1 - v0;
    QVector3D edge2 = v2 - v0;
    QVector3D h = QVector3D::crossProduct(ray.direction, edge2);
    float a = QVector3D::dotProduct(edge1, h);
    if (fabs(a) < EPSILON) return false;

    float f = 1.0 / a;
    QVector3D s = ray.origin - v0;
    float u = f * QVector3D::dotProduct(s, h);
    if (u < 0.0 || u > 1.0) return false;

    QVector3D q = QVector3D::crossProduct(s, edge1);
    float v = f * QVector3D::dotProduct(ray.direction, q);
    if (v < 0.0 || u + v > 1.0) return false;

    t = f * QVector3D::dotProduct(edge2, q);
    if (t > EPSILON) {
        normal = QVector3D::crossProduct(edge1, edge2).normalized();
        return true;
    }
    return false;
}

// 장면 내 레이 교차 확인
OpenGLWindow::HitInfo OpenGLWindow::traceRay(const Ray& ray) {
    float closestT = 1e6;
    HitInfo result;

    // (1) 모델 교차 검사
    for (const auto& face : objLoader.faces) {
        const auto& v0 = objLoader.vertices[face.v1];
        const auto& v1 = objLoader.vertices[face.v2];
        const auto& v2 = objLoader.vertices[face.v3];

        QVector3D vert0(v0.x, v0.y, v0.z);
        QVector3D vert1(v1.x, v1.y, v1.z);
        QVector3D vert2(v2.x, v2.y, v2.z);

        float t;
        QVector3D normal;
        if (intersectRayTriangle(ray, vert0, vert1, vert2, t, normal)) {
            if (t < closestT && !std::isnan(t)) {
                closestT = t;
                result.hit = true;
                result.distance = t;
                result.position = ray.origin + ray.direction * t;
                result.normal = normal;
                result.objectId = 1; // 소
            }
        }
    }

    // (2) 바닥 y = -1 평면 검사
    if (fabs(ray.direction.y()) > 1e-6f) {
        float t = (-1.0f - ray.origin.y()) / ray.direction.y();
        if (t > 0.0f && t < closestT) {
            QVector3D hitPoint = ray.origin + t * ray.direction;
            if (hitPoint.x() >= -10.0f && hitPoint.x() <= 10.0f &&
                hitPoint.z() >= -10.0f && hitPoint.z() <= 10.0f) {
                closestT = t;
                result.hit = true;
                result.distance = t;
                result.position = hitPoint;
                result.normal = QVector3D(0, 1, 0); // 바닥 normal
                result.objectId = 0; // 바닥
            }
        }
    }

    return result;
}


// 섀도우 레이
bool OpenGLWindow::isInShadow(const QVector3D& point, const QVector3D& lightPos) {
    QVector3D dir = (lightPos - point).normalized();
    Ray shadowRay{ point + dir * 0.01f, dir };
    HitInfo hit = traceRay(shadowRay);
    float distToLight = (lightPos - point).length();
    return hit.hit && hit.distance < distToLight;
}

QVector3D OpenGLWindow::traceRecursive(const Ray& ray, int depth) {
    if (depth > maxDepth) return QVector3D(0.1f, 0.1f, 0.1f); // 배경색

    HitInfo hit = traceRay(ray);
    if (!hit.hit || std::isnan(hit.normal.x())) {
        return QVector3D(0.2f, 0.2f, 0.2f);
    }

    QVector3D color(0.0f, 0.0f, 0.0f);
    QVector3D lightDir = (lightPos - hit.position).normalized();

    // 바닥에 그림자만
    if (hit.objectId == 0) {
        color = QVector3D(0.3f, 0.3f, 0.3f); // 기본 바닥색
        if (isInShadow(hit.position, lightPos)) {
            color *= 0.2f; // 그림자 영역은 어둡게
        }
        return color;
    }

    // 소일 경우
    if (!isInShadow(hit.position, lightPos)) {
        float diffuse = std::max(QVector3D::dotProduct(hit.normal.normalized(), lightDir), 0.0f);
        color += diffuse * QVector3D(1.0f, 1.0f, 1.0f); // 흰색광
    }

    // 반사
    QVector3D reflectDir = ray.direction - 2.0f * QVector3D::dotProduct(ray.direction, hit.normal) * hit.normal;
    reflectDir.normalize();

    if (!std::isnan(reflectDir.x())) {
        Ray reflectRay;
        reflectRay.origin = hit.position + hit.normal * 0.01f;
        reflectRay.direction = reflectDir;
        QVector3D reflectColor = traceRecursive(reflectRay, depth + 1);
        color += 0.5f * reflectColor;
    }

    return color;
}



// 전체 이미지 렌더링
void OpenGLWindow::renderRayTracing() {
    QImage image(width(), height(), QImage::Format_RGB32);
    QVector3D modelOffset(0.0f, 0.0f, 0.0f);

    for (int y = 0; y < height(); ++y) {
        for (int x = 0; x < width(); ++x) {
            float ndcX = (2.0f * x / width()) - 1.0f;
            float ndcY = 1.0f - (2.0f * y / height());
            QVector3D rayOrigin(0.0f, 3.0f, 10.0f);
            QVector3D rayDir(ndcX, ndcY, -1.0f);
            rayDir.normalize();

            QVector3D color = traceRecursive(Ray{rayOrigin, rayDir}, 0);
            int r = std::min(255, int(color.x() * 255));
            int g = std::min(255, int(color.y() * 255));
            int b = std::min(255, int(color.z() * 255));
            image.setPixel(x, y, qRgb(r, g, b));
        }
    }

    QPainter painter(this);
    painter.drawImage(0, 0, image);
}


