//
// Created by Yohanes Turnip on 2026-08-07.
//

#include "ManifoldRenderer.h"
#include <algorithm>

namespace ags::app
{
    namespace
    {
        struct ViridisStop { float t, r, g, b; };

        constexpr std::array<ViridisStop, 8> viridisLut {{
            {0.0f, 0.267f, 0.005f, 0.329f},
            {0.14f, 0.283f, 0.141f, 0.458f},
            {0.29f, 0.254f, 0.265f, 0.530f},
            {0.43f, 0.207f, 0.372f, 0.553f},
            {0.57f, 0.164f, 0.471f, 0.558f},
            {0.71f, 0.128f, 0.567f, 0.551f},
            {0.86f, 0.369f, 0.789f, 0.383f},
            {1.0f,  0.993f, 0.906f, 0.144f}
        }};

        void viridisColor(float t, float& r, float& g, float& b)
        {
            t = std::clamp(t, 0.0f, 1.0f);
            for (size_t i = 0; i + 1 < viridisLut.size(); ++i)
            {
                const auto& a = viridisLut[i];
                const auto& b2 = viridisLut[i + 1];
                if (t >= a.t && t <= b2.t)
                {
                    const float localT = (t - a.t) / (b2.t - a.t);
                    r = a.r + localT * (b2.r - a.r);
                    g = a.g + localT * (b2.g - a.g);
                    b = a.b + localT * (b2.b - a.b);
                    return;
                }
            }
            r = viridisLut.back().r; g = viridisLut.back().g; b = viridisLut.back().b;
        }

        constexpr auto* vertexShaderSource = R"(
            #version 150 core
            in vec3 position;
            in float pointSize;
            in vec3 color;
            uniform mat4 projectionMatrix;
            uniform mat4 viewMatrix;
            uniform float perspectiveScale;
            uniform float minPointPixels;
            uniform float maxPointPixels;
            uniform float nearViewDepth;
            uniform float farViewDepth;
            out vec3 fragColor;
            out float depthFactor;
            void main()
            {
                vec4 viewPos = viewMatrix * vec4(position, 1.0);
                gl_Position = projectionMatrix * viewPos;

                float viewDepth = -viewPos.z;
                gl_PointSize = clamp(pointSize * (perspectiveScale / viewDepth), minPointPixels, maxPointPixels);
                fragColor = color;

                float depthRange = farViewDepth - nearViewDepth;
                depthFactor = clamp((farViewDepth - viewDepth) / depthRange, 0.0, 1.0); // 1 = near, 0 = far
            }
        )";

        constexpr auto* fragmentShaderSource = R"(
            #version 150 core
            in vec3 fragColor;
            in float depthFactor;
            out vec4 outColor;
            void main()
            {
                vec2 centered = gl_PointCoord - vec2(0.5);
                if (length(centered) > 0.5)
                    discard;

                float shade = mix(0.35, 1.0, depthFactor);
                float alpha = mix(0.2, 0.55, depthFactor);
                outColor = vec4(fragColor * shade, alpha);
            }
        )";

        constexpr auto* gridVertexShaderSource = R"(
            #version 150 core
            in vec3 position;
            uniform mat4 projectionMatrix;
            uniform mat4 viewMatrix;
            void main()
            {
                gl_Position = projectionMatrix * viewMatrix * vec4(position, 1.0);
            }
        )";

        constexpr auto* gridFragmentShaderSource = R"(
            #version 150 core
            uniform vec4 lineColor;
            out vec4 outColor;
            void main()
            {
                outColor = lineColor;
            }
        )";

        GLuint makeStaticLineBuffer(GLuint& vaoOut, const float* data, size_t floatCount)
        {
            using namespace juce::gl;

            GLuint vbo = 0;
            glGenVertexArrays(1, &vaoOut);
            glBindVertexArray(vaoOut);

            glGenBuffers(1, &vbo);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(floatCount * sizeof(float)), data, GL_STATIC_DRAW);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

            glBindVertexArray(0);
            return vbo;
        }
    }

    std::vector<ManifoldRenderer::SplatVertex>
    ManifoldRenderer::buildVertices(const ags::manifold::GaussianManifold& manifold) const
    {
        std::vector<SplatVertex> vertices;
        vertices.reserve(manifold.size());

        for (const auto& splat : manifold.splats())
        {
            float r, g, b;
            viridisColor(splat.shColor, r, g, b);

            vertices.push_back(SplatVertex {
                splat.position.x, splat.position.y, splat.position.z,
                splat.eccentricity * pointSizeScale,
                r, g, b
            });
        }
        return vertices;
    }

    void ManifoldRenderer::setManifold(const ags::manifold::GaussianManifold& manifold)
    {
        auto vertices = buildVertices(manifold);

        const juce::ScopedLock lock(dataLock);
        pendingVertices = std::move(vertices);
        hasNewData = true;
    }

    void ManifoldRenderer::buildGridGeometry()
    {
        std::vector<float> gridVertices;
        auto addLine = [&](float x1, float y1, float z1, float x2, float y2, float z2)
        {
            gridVertices.insert(gridVertices.end(), {x1, y1, z1, x2, y2, z2});
        };

        for (float a : {-1.0f, 1.0f})
        {
            for (float b : {-1.0f, 1.0f})
            {
                addLine(a, b, -1.0f, a, b, 1.0f);
                addLine(a, -1.0f, b, a, 1.0f, b);
                addLine(-1.0f, a, b, 1.0f, a, b);
            }
        }

        gridVertexCount = static_cast<int>(gridVertices.size() / 3);
        gridVbo = makeStaticLineBuffer(gridVao, gridVertices.data(), gridVertices.size());
    }

    void ManifoldRenderer::buildAxisGeometry()
    {
        static constexpr std::array<float, 18> axisVertices = {
            -1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f, // X axis
             0.0f,-1.0f, 0.0f,  0.0f, 1.0f, 0.0f, // Y axis
             0.0f, 0.0f,-1.0f,  0.0f, 0.0f, 1.0f  // Z axis
        };

        axisVbo = makeStaticLineBuffer(axisVao, axisVertices.data(), axisVertices.size());
    }

    void ManifoldRenderer::newOpenGLContextCreated()
    {
        using namespace juce::gl;

        shaderProgram = std::make_unique<juce::OpenGLShaderProgram>(*juce::OpenGLContext::getCurrentContext());
        shaderProgram->addVertexShader(vertexShaderSource);
        shaderProgram->addFragmentShader(fragmentShaderSource);
        glBindAttribLocation(shaderProgram->getProgramID(), 0, "position");
        glBindAttribLocation(shaderProgram->getProgramID(), 1, "pointSize");
        glBindAttribLocation(shaderProgram->getProgramID(), 2, "color");
        shaderProgram->link();

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        gridShaderProgram = std::make_unique<juce::OpenGLShaderProgram>(*juce::OpenGLContext::getCurrentContext());
        gridShaderProgram->addVertexShader(gridVertexShaderSource);
        gridShaderProgram->addFragmentShader(gridFragmentShaderSource);
        glBindAttribLocation(gridShaderProgram->getProgramID(), 0, "position");
        gridShaderProgram->link();

        buildGridGeometry();
        buildAxisGeometry();
    }

    void ManifoldRenderer::drawGrid(const juce::Matrix3D<float>& projection, const juce::Matrix3D<float>& view)
    {
        using namespace juce::gl;

        if (gridShaderProgram == nullptr || gridVertexCount == 0)
            return;

        gridShaderProgram->use();
        gridShaderProgram->setUniformMat4("projectionMatrix", projection.mat, 1, false);
        gridShaderProgram->setUniformMat4("viewMatrix", view.mat, 1, false);
        glUniform4f(glGetUniformLocation(gridShaderProgram->getProgramID(), "lineColor"), 0.6f, 0.6f, 0.6f, 1.0f);

        glBindVertexArray(gridVao);
        glDrawArrays(GL_LINES, 0, gridVertexCount);
        glBindVertexArray(0);
    }

    void ManifoldRenderer::drawAxes(const juce::Matrix3D<float>& projection, const juce::Matrix3D<float>& view)
    {
        using namespace juce::gl;

        if (gridShaderProgram == nullptr)
            return;

        gridShaderProgram->use();
        gridShaderProgram->setUniformMat4("projectionMatrix", projection.mat, 1, false);
        gridShaderProgram->setUniformMat4("viewMatrix", view.mat, 1, false);

        const GLint colorLoc = glGetUniformLocation(gridShaderProgram->getProgramID(), "lineColor");

        glBindVertexArray(axisVao);
        glUniform4f(colorLoc, 0.85f, 0.2f, 0.2f, 1.0f);  // X = red
        glDrawArrays(GL_LINES, 0, 2);
        glUniform4f(colorLoc, 0.2f, 0.7f, 0.2f, 1.0f);   // Y = green
        glDrawArrays(GL_LINES, 2, 2);
        glUniform4f(colorLoc, 0.2f, 0.4f, 0.85f, 1.0f);  // Z = blue
        glDrawArrays(GL_LINES, 4, 2);
        glBindVertexArray(0);
    }

    void ManifoldRenderer::drawSplats(const juce::Matrix3D<float>& projection, const juce::Matrix3D<float>& view)
    {
        using namespace juce::gl;

        if (gpuVertices.empty() || shaderProgram == nullptr)
            return;

        glEnable(GL_PROGRAM_POINT_SIZE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);

        shaderProgram->use();
        shaderProgram->setUniformMat4("projectionMatrix", projection.mat, 1, false);
        shaderProgram->setUniformMat4("viewMatrix", view.mat, 1, false);
        shaderProgram->setUniform("perspectiveScale", perspectiveScale);
        shaderProgram->setUniform("minPointPixels", minPointPixels);
        shaderProgram->setUniform("maxPointPixels", maxPointPixels);
        shaderProgram->setUniform("nearViewDepth", nearViewDepth);
        shaderProgram->setUniform("farViewDepth", farViewDepth);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(gpuVertices.size() * sizeof(SplatVertex)),
                     gpuVertices.data(), GL_DYNAMIC_DRAW);

        const auto stride = static_cast<GLsizei>(sizeof(SplatVertex));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(SplatVertex, x));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(SplatVertex, pointSize));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(SplatVertex, r));

        glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(gpuVertices.size()));

        glDepthMask(GL_TRUE);
        glBindVertexArray(0);
    }

    void ManifoldRenderer::renderOpenGL()
    {
        using namespace juce::gl;

        {
            const juce::ScopedLock lock(dataLock);
            if (hasNewData)
            {
                gpuVertices = std::move(pendingVertices);
                hasNewData = false;
            }
        }

        glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        const auto projection = juce::Matrix3D<float>::fromFrustum(-0.5f, 0.5f, -0.5f, 0.5f, 1.0f, 10.0f);
        const auto view = juce::Matrix3D<float>::fromTranslation({0.0f, 0.0f, -cameraDistance});

        drawGrid(projection, view);
        drawAxes(projection, view);
        drawSplats(projection, view);
    }

    void ManifoldRenderer::openGLContextClosing()
    {
        using namespace juce::gl;
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &gridVao);
        glDeleteBuffers(1, &gridVbo);
        glDeleteVertexArrays(1, &axisVao);
        glDeleteBuffers(1, &axisVbo);
        shaderProgram.reset();
        gridShaderProgram.reset();
    }
}