/*
* Copyright (C) 2016
* Computer Graphics Group, The Institute of Computer Graphics and Algorithms, TU Wien
* Written by Tobias Klein <tklein@cg.tuwien.ac.at>
* All rights reserved.
*/

#pragma once

#include <glm/glm.hpp>

#define M_PI 3.14159265358979323846

class Camera
{
public:

    Camera();
    ~Camera(void);

    void reset();

    void buildViewMatrix(void);
    void buildProjectionMatrix(void);

    const glm::mat4 & getViewMatrix() { return mViewMatrix; };
    const glm::mat4 & getProjectionMatrix() { return mProjectionMatrix; };

    void moveLeft(float t);
    void moveRight(float t);
    void moveForwards(float t);
    void moveBackwards(float t);
    void moveUp(float t);
    void moveDown(float t);

    void rotateAzimuth(float t);
    void rotatePolar(float t);
    void rotateUp(float t);
    void rotateDown(float t);
    void rotateLeft(float t);
    void rotateRight(float t);

    void zoom(float t);

    glm::vec3 getUp() { return mUp; };
    void setUp(glm::vec3 &up) { mUp = up; };
    glm::vec3 getTarget() { return mTarget; };
    void setTarget(glm::vec3 &target) { mTarget = target; };

    glm::vec3 getPosition() { return mPos; };

    bool isOrthogonal() { return mOrthogonal; };
    void setOrthogonal(bool orthogonal) { mOrthogonal = orthogonal; };
    void setOrthogonalBorders(float left, float right, float top, float bottom);

    float getFieldOfView() { return mFieldOfView; };
    void setFieldOfView(float fieldOfView) { mFieldOfView = fieldOfView; };

    float getNearPlane() { return mNear; };
    void setNearPlane(float nearPlane) { mNear = nearPlane; };

    float getFarPlane() { return mFar; };
    void setFarPlane(float farPlane) { mFar = farPlane; buildProjectionMatrix(); };

    void setAspect(float aspect) { mAspect = aspect;  buildProjectionMatrix();};

private:
    glm::mat4 mViewMatrix;
    glm::mat4 mProjectionMatrix;

    float mRadius;
    float mAzimuth;
    float mPolar;

    glm::vec3 mUp;
    glm::vec3 mTarget;
    glm::vec3 mPos;
    float mFieldOfView;
    float mAspect;
    float mNear;
    float mFar;
    float mLeft;
    float mRight;
    float mTop;
    float mBottom;
    bool mOrthogonal;
};
