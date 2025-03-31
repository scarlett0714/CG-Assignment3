#ifndef OBJLOADER_H
#define OBJLOADER_H

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>

// 데이터 구조 정의
struct Vertex{
    float x, y, z;
};

struct Face{ // a triangle face
    float v1, v2, v3;
};

class ObjLoader{
public:
    //변수 정의
    std::vector<Vertex> vertices;
    std::vector<Face> faces;

    //메소드 정의
    bool load(const std::string& filename);
};

#endif // OBJLOADER_H
