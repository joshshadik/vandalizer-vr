#pragma once

#include "material.h"
#include "shader.h"
#include "mesh.h"
#include "framebuffer.h"
#include "gltfModel.h"
#include "particleSystem.h"
#include "swapchain.h"

#include "controls.h"


#ifdef USE_WASM
#include <GLES3/gl3.h>
#include <GLES3/gl3platform.h>

#include <emscripten.h>
#else
#include <GL/glew.h>
#include <GL/GL.h>
#endif


#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <memory>

#define PROPRIETARY_ASSETS

using namespace jkps::gl;
using namespace jkps::engine;
using namespace jkps::util;

struct GlobalUniforms
{
    glm::mat4 view;
    glm::mat4 projection;
	glm::mat4 invVP;
};


class App
{
public:

    App() {}

    void init();

    void render(const glm::ivec4& viewport);

    void update(double dt);

    void setControls(Controls* controls);

    void resize(const glm::ivec2& size);

    void overrideViewProjection(const glm::mat4& view, const glm::mat4& projection);

private:

	struct PaintMesh
	{
		PaintMesh(Swapchain&& sc) : swapchain(std::move(sc)) {}

		Mesh* mesh;
		Transform* transform;
		Swapchain swapchain;
		GLint texLocation;
		GLint paintTexLocation;
        Texture* originalTexture;
	};

    Shader* vs;
    Shader* fs;

    Shader* composeVS;
    Shader* composeFS;

    ShaderProgram* program;
    ShaderProgram* composeProgram;

    Material* material;
    Geometry* _quadGeo;

    Mesh* boxMesh;

    GLTFModel _gltfModel;
    GLTFModel _sprayCanGLTFModel;

	Model* _mainModel;
    Model* _sprayCanModel;

    glm::mat4 _modelMtx;
    glm::quat _modelRot;
    glm::vec3 _modelScale;
    glm::vec3 _modelPos;

    glm::mat4 _vrOffset;

#ifdef PROPRIETARY_ASSETS
    GLTFModel _roomGLTFModel;
    Model* _roomModel;

    Shader* roomVS;
    Shader* roomFS;

    ShaderProgram* roomProgram;

#endif

    Framebuffer* _screenBuffer;

    Material* _composeMaterial;
    Mesh* _composeMesh;

	Material* _sprayPaintMaterial;
	ShaderProgram* _sprayPaintProgram;
	GLint _toolVPLoc;
    GLint _paintColorLoc;

    glm::ivec2 _screenSize;

    GlobalUniforms _gUniforms;
    MaterialUniformBlock* _globalUniformBlock;
    std::array<Texture*, 4> _colorScreenTextures;



	std::vector<PaintMesh> _paintSwapchains;

    glm::vec4 _paintColor;

    Texture* _depthTexture;

    Controls* _controls;

    jkps::engine::ParticleSystem _sprayParticles;
    Transform* _particleParent;
    Material* _sprayMaterial;
    GLint _sprayColorLoc;

    const glm::mat4 _sprayProjection = glm::perspective(2.55f, 1.0f, 0.005f, 0.2f);

    bool _painting = false;

};