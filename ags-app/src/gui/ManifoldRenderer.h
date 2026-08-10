//
// Created by Yohanes Turnip on 2026-08-07.
//

#ifndef AUDIOGAUSSIANSPLATTER_MANIFOLDRENDERER_H
#define AUDIOGAUSSIANSPLATTER_MANIFOLDRENDERER_H
#pragma once

#include <juce_opengl/juce_opengl.h>
#include <array>
#include <vector>
#include "ags/manifold/GaussianManifold.h"

namespace ags::app
{
    class ManifoldRenderer final : public juce::OpenGLRenderer
    {
    public:
        void setManifold(const ags::manifold::GaussianManifold& manifold);

        void newOpenGLContextCreated() override;
        void renderOpenGL() override;
        void openGLContextClosing() override;

    private:
        struct SplatVertex
        {
            float x, y, z;
            float pointSize;
            float r, g, b;
        };

        std::vector<SplatVertex> buildVertices(const ags::manifold::GaussianManifold& manifold) const;
        void buildGridGeometry();
        void buildAxisGeometry();
        void drawGrid(const juce::Matrix3D<float>& projection, const juce::Matrix3D<float>& view);
        void drawAxes(const juce::Matrix3D<float>& projection, const juce::Matrix3D<float>& view);
        void drawSplats(const juce::Matrix3D<float>& projection, const juce::Matrix3D<float>& view);

        juce::CriticalSection dataLock;
        std::vector<SplatVertex> pendingVertices;
        bool hasNewData { false };

        std::vector<SplatVertex> gpuVertices;
        GLuint vao { 0 };
        GLuint vbo { 0 };
        std::unique_ptr<juce::OpenGLShaderProgram> shaderProgram;

        GLuint gridVao { 0 };
        GLuint gridVbo { 0 };
        int gridVertexCount { 0 };
        std::unique_ptr<juce::OpenGLShaderProgram> gridShaderProgram;

        GLuint axisVao { 0 };
        GLuint axisVbo { 0 };

        // Matches Python reference's s=eccentricity*10 marker-area convention,
        // rescaled for OpenGL point-diameter units.
        static constexpr float pointSizeScale = 2.0f;

        // Camera and projection are fixed for this rendering pass; rotation
        // is applied to the manifold data itself upstream in PluginEditor.
        static constexpr float cameraDistance = 3.0f;
        static constexpr float nearViewDepth = 2.0f; // sphere front, camera-relative
        static constexpr float farViewDepth = 4.0f;  // sphere back, camera-relative
        static constexpr float minPointPixels = 2.0f;
        static constexpr float maxPointPixels = 14.0f;
        static constexpr float perspectiveScale = 8.0f;
    };
}
#endif //AUDIOGAUSSIANSPLATTER_MANIFOLDRENDERER_H
