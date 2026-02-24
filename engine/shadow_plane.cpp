#include "shadow_plane.h"
#include "scene.h"
#include "mesh.h"

#include <GL/freeglut.h>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <iostream>

# define M_PI 3.14159265358979323846

ShadowPlane::ShadowPlane()
    : _radius(10.0f),
    _shadowOffset(0.01f),
    _shadowColor(0.0f, 0.0f, 0.0f, 0.6f),
    _type(Eng::ShadowPlaneType::ENDLESS) {
}

ShadowPlane::~ShadowPlane() {
}

void ShadowPlane::setRadius(float radius) {
    _radius = radius;
}

float ShadowPlane::getRadius() const {
    return _radius;
}

void ShadowPlane::setType(Eng::ShadowPlaneType type) {
    _type = type;
}

Eng::ShadowPlaneType ShadowPlane::getType() const {
    return _type;
}

void ShadowPlane::setShadowColor(const glm::vec4& color) {
    _shadowColor = color;
}

glm::vec4 ShadowPlane::getShadowColor() const {
    return _shadowColor;
}

void ShadowPlane::setShadowOffset(float offset) {
    _shadowOffset = offset;
}

float ShadowPlane::getShadowOffset() const {
    return _shadowOffset;
}

glm::mat4 ShadowPlane::computeInfiniteShadowMatrix(glm::vec4 lightPos) const {
    float planeHeight = getWC()[3].y;

    glm::vec4 groundPlane(0.0f, 1.0f, 0.0f, -planeHeight - _shadowOffset);

    float dot = glm::dot(groundPlane, lightPos);
    glm::mat4 shadowMat(1.0f);

    shadowMat[0][0] = dot - lightPos.x * groundPlane.x;
    shadowMat[1][0] = 0.f - lightPos.x * groundPlane.y;
    shadowMat[2][0] = 0.f - lightPos.x * groundPlane.z;
    shadowMat[3][0] = 0.f - lightPos.x * groundPlane.w;

    shadowMat[0][1] = 0.f - lightPos.y * groundPlane.x;
    shadowMat[1][1] = dot - lightPos.y * groundPlane.y;
    shadowMat[2][1] = 0.f - lightPos.y * groundPlane.z;
    shadowMat[3][1] = 0.f - lightPos.y * groundPlane.w;

    shadowMat[0][2] = 0.f - lightPos.z * groundPlane.x;
    shadowMat[1][2] = 0.f - lightPos.z * groundPlane.y;
    shadowMat[2][2] = dot - lightPos.z * groundPlane.z;
    shadowMat[3][2] = 0.f - lightPos.z * groundPlane.w;

    shadowMat[0][3] = 0.f - lightPos.w * groundPlane.x;
    shadowMat[1][3] = 0.f - lightPos.w * groundPlane.y;
    shadowMat[2][3] = 0.f - lightPos.w * groundPlane.z;
    shadowMat[3][3] = dot - lightPos.w * groundPlane.w;

    return shadowMat;
}

glm::mat4 ShadowPlane::computeShadowMatrixForLight(Light* light) const {
    if (!light) return glm::mat4(1.0f);

    glm::vec4 realLightPos = light->getWC()[3];
    glm::vec4 shadowCalcPos = realLightPos;
    shadowCalcPos.w = 1.0f;

    float minHeight = 5.0f;
    if (shadowCalcPos.y < minHeight) {
        shadowCalcPos.y = minHeight;
    }

    return computeInfiniteShadowMatrix(shadowCalcPos);
}

bool ShadowPlane::containsPoint(const glm::vec3& point) const {
    float planeHeight = getWC()[3].y;
    return point.y >= planeHeight;
}

void ShadowPlane::render(glm::mat4 cameraInverse)
{
    Scene* scene = dynamic_cast<Scene*>(getParent());
    if (!scene) return;

    std::vector<Light*> lights;
    for (Node* n : scene->getRenderList())
        if (Light* l = dynamic_cast<Light*>(n))
            lights.push_back(l);

    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT |
        GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT |
        GL_CURRENT_BIT);

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_STENCIL_TEST);
    glClearStencil(0);
    glClear(GL_STENCIL_BUFFER_BIT);
    glStencilMask(0xFF);

    if (_type == Eng::ShadowPlaneType::CIRCLE)
    {
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);

        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

        glDisable(GL_CULL_FACE);

        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(glm::value_ptr(cameraInverse * getWC()));

        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(0.0f, 0.0f, 0.0f);
        for (int i = 0; i <= 32; ++i) {
            float a = 2.0f * M_PI * i / 32.0f;
            glVertex3f(cos(a) * _radius, 0.0f, sin(a) * _radius);
        }
        glEnd();

        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        glStencilFunc(GL_EQUAL, 1, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);

        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.0f, 1.0f);

        glColor4f(0.0f, 0.0f, 0.0f, _shadowColor[3]);
    }
    else
    {
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.0f, 1.0f);

        glStencilFunc(GL_EQUAL, 0, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);

        glColor4f(0.0f, 0.0f, 0.0f, _shadowColor[3]);
    }

    for (Node* n : scene->getRenderList())
    {
        Mesh* mesh = dynamic_cast<Mesh*>(n);
        if (!mesh || !mesh->castsShadow())
            continue;

        glm::vec3 p = glm::vec3(mesh->getWC()[3]);
        if (!containsPoint(p))
            continue;

        for (Light* l : lights)
        {
            if (l->getCastShadow())
            {
                glm::mat4 shadowMat = computeShadowMatrixForLight(l);
                glm::mat4 finalMatrix = cameraInverse * shadowMat * mesh->getWC();
                mesh->renderShadow(finalMatrix);
            }
        }
    }

    glDisable(GL_STENCIL_TEST);
    glDisable(GL_POLYGON_OFFSET_FILL);
    glDepthMask(GL_TRUE);
    glPopAttrib();

    Node::render(cameraInverse);
}