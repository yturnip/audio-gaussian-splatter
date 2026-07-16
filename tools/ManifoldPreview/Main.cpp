//
// Created by Yohanes Turnip on 2026-07-13.
//
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_opengl/juce_opengl.h>

#include <glm/gtc/matrix_transform.hpp>

#include <ags/manifold/GaussianManifold.h>
#include <ags/manifold/Generators/SphereBranchingGenerator.h>
#include <ags/manifold/Generators/DomeBranchingGenerator.h>

using namespace juce::gl;

class ManifoldPreviewComponent : public juce::OpenGLAppComponent
{
public:
    ManifoldPreviewComponent()
    {
        setSize(800, 600);
        regenerate(true);
    }

    ~ManifoldPreviewComponent() override
    {
        shutdownOpenGL();
    }

    void regenerate(bool useDome) {
        ags::manifold::GeneratorConfig config;
        config.numHubs = 64;
        config.pointsPerHub = 20;
        config.spread = 0.1f;
        config.seed = 1234;

        if (useDome)
        {
            ags::manifold::DomeBranchingGenerator gen;
            manifold = gen.generate(config);
        }
        else
        {
            ags::manifold::SphereBranchingGenerator gen;
            manifold = gen.generate(config);
        }
    }

    void initialise() override {}
    void shutdown() override {}

    void drawBillboardCircle(const glm::vec3& center, float radius, int segments = 10)
    {
        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(center.x, center.y, center.z);

        for (int i = 0; i <= segments; ++i)
        {
            const float angle = (float) i / (float) segments * juce::MathConstants<float>::twoPi;
            glVertex3f(center.x + radius * std::cos(angle),
                       center.y + radius * std::sin(angle),
                       center.z);
        }

        glEnd();
    }

    void render() override
    {
        juce::OpenGLHelpers::clear(juce::Colour::fromFloatRGBA(0.15f, 0.15f, 0.18f, 1.0f));

        auto desktopScale = (float) openGLContext.getRenderingScale();
        glViewport(0, 0,
                   juce::roundToInt(desktopScale * (float) getWidth()),
                   juce::roundToInt(desktopScale * (float) getHeight()));

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-1.2, 1.2, -1.2, 1.2, -10, 10);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        const glm::mat3 spin = glm::mat3(glm::rotate(glm::mat4(1.0f),
                                                  glm::radians(rotationDegrees),
                                                  glm::vec3(0.0f, 1.0f, 0.0f)));

        glBegin(GL_QUADS);

        for (const auto& splat : manifold.splats())
        {
            glColor4f(splat.shColor, 0.5f, 1.0f - splat.shColor, 0.75f);

            const glm::vec3 rotatedPos = spin * splat.position;
            const float halfSize = juce::jlimit(0.008f, 0.05f, splat.eccentricity * 0.012f);
            //const auto& p = splat.position;

            /*glVertex3f(rotatedPos.x - halfSize, rotatedPos.y - halfSize, rotatedPos.z);
            glVertex3f(rotatedPos.x + halfSize, rotatedPos.y - halfSize, rotatedPos.z);
            glVertex3f(rotatedPos.x + halfSize, rotatedPos.y + halfSize, rotatedPos.z);
            glVertex3f(rotatedPos.x - halfSize, rotatedPos.y + halfSize, rotatedPos.z);*/
            drawBillboardCircle(rotatedPos, halfSize);
        }

        glEnd();

        rotationDegrees += 0.3f;
        if (rotationDegrees > 360.0f)
            rotationDegrees -= 360.0f;
    }

    void paint(juce::Graphics&) override {}

    void resized() override {}

private:
    ags::manifold::GaussianManifold manifold;
    float rotationDegrees { 0.0f };
};

class ManifoldPreviewWindow : public juce::DocumentWindow
{
public:
    ManifoldPreviewWindow()
        : DocumentWindow("Manifold Preview",
                          juce::Colours::darkgrey,
                          DocumentWindow::allButtons)
    {
        setUsingNativeTitleBar(true);
        setContentOwned(new ManifoldPreviewComponent(), true);
        centreWithSize(800, 600);
        setVisible(true);
    }

    void closeButtonPressed() override
    {
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }
};

class ManifoldPreviewApp : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override { return "ManifoldPreview"; }
    const juce::String getApplicationVersion() override { return "0.1"; }

    void initialise(const juce::String&) override
    {
        mainWindow = std::make_unique<ManifoldPreviewWindow>();
    }

    void shutdown() override
    {
        mainWindow = nullptr;
    }

private:
    std::unique_ptr<ManifoldPreviewWindow> mainWindow;
};

START_JUCE_APPLICATION(ManifoldPreviewApp)