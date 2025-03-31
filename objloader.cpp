#include "objloader.h"

// 벡터 연산용 구조체
struct Vec3 {
    float x, y, z;

    Vec3() : x(0), y(0), z(0) {}  // 기본 생성자
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}  // 명시적 생성자
    Vec3(const Vertex& v) : x(v.x), y(v.y), z(v.z) {} // Vertex 변환 생성자

    Vec3 operator-(const Vec3& v) const {
        return {x - v.x, y - v.y, z - v.z};
    }

    Vec3 cross(const Vec3& v) const {
        return {
            y * v.z - z * v.y,
            z * v.x - x * v.z,
            x * v.y - y * v.x
        };
    }

    float dot(const Vec3& v) const {
        return x * v.x + y * v.y + z * v.z;
    }

    Vec3 normalize() const {
        float length = sqrt(x * x + y * y + z * z);
        return {x / length, y / length, z / length};
    }

};

bool ObjLoader::load(const std::string& filename) {
    std::ifstream fin(filename); // include <fstream>
    if (!fin.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl; // include <iostream>
        return false;
    }

    std::string line;
    while (std::getline(fin, line)) {

        //std::istringstream : 공백(띄어쓰기, 탭, 개행)을 자동으로 구분자(separator)로 사용해서 데이터를 읽음
        std::istringstream ss(line); // include <sstream>
        std::string type;
        ss >> type;

        if (type == "v") {
            Vertex v;
            ss >> v.x >> v.y >> v.z;
            vertices.push_back(v);
        } else if (type == "f") {
            Face f;
            ss >> f.v1 >> f.v2 >> f.v3;

            // index 맞추기
            f.v1--;
            f.v2--;
            f.v3--;

            faces.push_back(f);
        }
    }

    fin.close();

    // 카메라 기준 자동 보정 추가
    Vec3 cameraPos = {0.0f, 0.0f, 5.0f}; // 카메라를 (0,0,5) 위치로 가정

    for (auto& face : faces) {
        Vec3 v1 = vertices[face.v1];
        Vec3 v2 = vertices[face.v2];
        Vec3 v3 = vertices[face.v3];

        // 1. 삼각형의 법선 벡터 계산
        Vec3 edge1 = v2 - v1;
        Vec3 edge2 = v3 - v1;
        Vec3 normal = edge1.cross(edge2).normalize();

        // 2. 삼각형 중심 좌표 계산
        Vec3 center = {(v1.x + v2.x + v3.x) / 3.0f, (v1.y + v2.y + v3.y) / 3.0f, (v1.z + v2.z + v3.z) / 3.0f};

        // 3. 삼각형 중심에서 카메라를 향하는 벡터 계산
        Vec3 viewVector = (cameraPos - center).normalize();

        // 4. 뷰 벡터와 법선 벡터의 내적(dot product) 계산
        float dotProduct = viewVector.dot(normal);

        // 5. 삼각형이 카메라를 향하지 않으면 뒤집기 (시계방향 ↔ 반시계방향)
        if (dotProduct < 0) {
            std::swap(face.v1, face.v3);
            std::cout << "Flipped Face: " << face.v1 << ", " << face.v2 << ", " << face.v3 << std::endl;
        } else {
            std::cout << "Face Normal: " << face.v1 << ", " << face.v2 << ", " << face.v3 << std::endl;
        }
    }

    std::cout << "Loaded Vertices:" << std::endl;
    for (size_t i = 0; i < vertices.size(); i++) {
        std::cout << i+1 << ": (" << vertices[i].x << ", " << vertices[i].y << ", " << vertices[i].z << ")" << std::endl;
    }

    std::cout << "Loaded Faces:" << std::endl;
    for (const auto& face : faces) {
        std::cout << "Face: " << face.v1 << ", " << face.v2 << ", " << face.v3 << std::endl;
    }

    return true;
}
