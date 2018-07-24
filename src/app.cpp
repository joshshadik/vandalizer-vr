#include "app.h"

#include "resourceManager.h"
#include "primitives.h"

#include <vector>

#include <json.hpp>

#include <glm/gtc/matrix_transform.hpp>


void App::init()
{
    Primitives::init();
    _quadGeo = Primitives::quad();

    _gUniforms.view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, -2.15f));
    _gUniforms.projection = glm::perspective(1.2f, 1.0f, 1.f, 30.0f);
    _gUniforms.invVP = glm::inverse(_gUniforms.projection * _gUniforms.view);

    printf("size of gUniforms: %d\n", sizeof(_gUniforms));

    MaterialUniformBlock::Descriptor globalDescriptor{
		{{"view", 0}, {"projection", sizeof(glm::mat4)}, {"invVP", sizeof(glm::mat4) * 2} },
        sizeof(glm::mat4) * 3
    };

    printf("size of descriptor: %d\n", globalDescriptor.second);

    _globalUniformBlock = ResourceManager::default()->getNextUniformBlock();

    *_globalUniformBlock = MaterialUniformBlock(globalDescriptor);
    _globalUniformBlock->setValue("view", &_gUniforms.view, sizeof(glm::mat4));
    _globalUniformBlock->setValue("projection", &_gUniforms.projection, sizeof(glm::mat4));
    _globalUniformBlock->setValue("invVP", &_gUniforms.invVP, sizeof(glm::mat4));

    _globalUniformBlock->uploadData();

    _globalUniformBlock->bind(0);

    vs = ResourceManager::default()->getNextShader();
    fs = ResourceManager::default()->getNextShader();

    Shader::loadFromFile(vs, "resources/shaders/standard.vs", Shader::Vertex); 
    Shader::loadFromFile(fs, "resources/shaders/standard.fs", Shader::Fragment);

    program = ResourceManager::default()->getNextShaderProgram();
    *program = ShaderProgram(vs, fs);

    material = ResourceManager::default()->getNextMaterial();
    *material = Material(program);

    boxMesh = ResourceManager::default()->getNextMesh();
    *boxMesh = Mesh(_quadGeo, material);

    _modelPos = glm::vec3(0.0f, 0.5f, 0.0f);
    _modelRot = glm::quat_cast(glm::rotate(glm::mat4(1.0f), -1.57f, glm::vec3(1.0f, 0.0f, 0.0f)));
    _modelScale = glm::vec3(0.001f);

    _modelMtx =  glm::scale(glm::translate(glm::mat4_cast(_modelRot), _modelPos), _modelScale);

    GLTFModel::loadFromFile(&_gltfModel, "resources/models/venus_de_milo/scene.gltf", program);
    _gltfModel.setMatrix( _modelMtx );
	_mainModel = _gltfModel.model();


    //_roomMtx = glm::scale(glm::translate(glm::mat4_cast(_modelRot), glm::vec3(0.0f, 7.5f, 0.0f)), glm::vec3(0.01f));
    //GLTFModel::loadFromFile(&_roomModel, "resources/models/room/scene.gltf", program);
    //_roomModel.setMatrix(_roomMtx);

    //_gltfModel.setMatrix(glm::rotate(glm::mat4(1.0f), -1.57f, glm::vec3(1.0f, 0.0f, 0.0f)));

    Shader* sprayUpdateVS = ResourceManager::default()->getNextShader();
    Shader* sprayUpdateFS = ResourceManager::default()->getNextShader();
    Shader* sprayRenderVS = ResourceManager::default()->getNextShader();
    Shader* sprayRenderFS = ResourceManager::default()->getNextShader();

    Shader::loadFromFile(sprayUpdateVS, "resources/shaders/screen.vs", Shader::Vertex);
    Shader::loadFromFile(sprayUpdateFS, "resources/shaders/spray/pos-vel.fs", Shader::Fragment);
    Shader::loadFromFile(sprayRenderVS, "resources/shaders/spray/particle.vs", Shader::Vertex);
    Shader::loadFromFile(sprayRenderFS, "resources/shaders/spray/particle.fs", Shader::Fragment);

    ShaderProgram* updateProgram = ResourceManager::default()->getNextShaderProgram();
    ShaderProgram* sprayProgram = ResourceManager::default()->getNextShaderProgram();
    Material* updateMaterial = ResourceManager::default()->getNextMaterial();
    _sprayMaterial = ResourceManager::default()->getNextMaterial();

    *updateProgram = ShaderProgram(sprayUpdateVS, sprayUpdateFS);
    *sprayProgram = ShaderProgram(sprayRenderVS, sprayRenderFS);
    *updateMaterial = Material(updateProgram);
    *_sprayMaterial = Material(sprayProgram);

    _sprayParticles.init(Primitives::quad(), _sprayMaterial, updateMaterial, 128);
    int diffLoc = _sprayMaterial->getUniformLocation("diffuseColor");
    _sprayMaterial->setUniform(diffLoc, glm::vec4(1.0f, 0.0f, 0.0f, 0.25f));
	_sprayMaterial->setBlended(true);
	_sprayMaterial->setBlendFunction(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	_sprayParticles.lifetime(4.0f);
	_sprayParticles.direction(glm::vec3(1.0f, 1.0f, -1.0f));
	_sprayParticles.magnitude(20.0f);
	_sprayParticles.randomness(0.15f);
	_sprayParticles.origin(glm::vec3(-0.5f, 0.5f, 0.0f));


	Shader* sprayPaintVS = ResourceManager::default()->getNextShader();
	Shader* sprayPaintFS = ResourceManager::default()->getNextShader();

	Shader::loadFromFile(sprayPaintVS, "resources/shaders/paint/spray.vs", Shader::Vertex);
	Shader::loadFromFile(sprayPaintFS, "resources/shaders/paint/spray.fs", Shader::Fragment);

	_sprayPaintProgram = ResourceManager::default()->getNextShaderProgram();
	*_sprayPaintProgram = ShaderProgram(sprayPaintVS, sprayPaintFS);
	_sprayPaintMaterial = ResourceManager::default()->getNextMaterial();
	*_sprayPaintMaterial = Material(_sprayPaintProgram);
	_toolVPLoc = _sprayPaintMaterial->getUniformLocation("toolVP");

    

    composeVS = ResourceManager::default()->getNextShader();
    composeFS = ResourceManager::default()->getNextShader();

    Shader::loadFromFile(composeVS, "resources/shaders/compose.vs", Shader::Vertex);
    Shader::loadFromFile(composeFS, "resources/shaders/compose.fs", Shader::Fragment);

    composeProgram = ResourceManager::default()->getNextShaderProgram();
    _composeMaterial = ResourceManager::default()->getNextMaterial();
    _composeMesh = ResourceManager::default()->getNextMesh();

    *composeProgram = ShaderProgram(composeVS, composeFS);
    *_composeMaterial = Material(composeProgram);
    *_composeMesh = Mesh(_quadGeo, _composeMaterial);

    for (int i = 0; i < 4; ++i)
    {
        _colorScreenTextures[i] = (ResourceManager::default()->getNextTexture());
    }

    _depthTexture = ResourceManager::default()->getNextTexture();

    _screenBuffer = ResourceManager::default()->getNextFramebuffer();

    int depthLoc = _composeMaterial->getUniformLocation("uDepthTex");

    printf("depth loc: %d\n", depthLoc);

    _composeMaterial->setUniform(_composeMaterial->getUniformLocation("uColorTex"), _colorScreenTextures[0]);
    _composeMaterial->setUniform(_composeMaterial->getUniformLocation("uPositionTex"), _colorScreenTextures[1]);
    _composeMaterial->setUniform(_composeMaterial->getUniformLocation("uNormalTex"), _colorScreenTextures[2]);
    _composeMaterial->setUniform(_composeMaterial->getUniformLocation("uMetalRoughOccTex"), _colorScreenTextures[3]);
	_composeMaterial->setUniform(_composeMaterial->getUniformLocation("uDepthTex"), _depthTexture);


    Texture* sky = ResourceManager::default()->getNextTexture();
	Texture::loadFromFile(sky, "resources/textures/241-sky.png", 3);

	_composeMaterial->setUniform(_composeMaterial->getUniformLocation("uSkyTex"), sky);

    _composeMaterial->setUniform(_composeMaterial->getUniformLocation("uLightDirection"), glm::normalize(glm::vec3(0.0, -1.0, -1.0)));

	{
		auto meshList = _mainModel->getMeshes(Model::Layer::Opaque);
		if (meshList)
		{
			auto iter = meshList->iter();

			if (iter->begin())
			{
				do
				{
					auto meshes = iter->current().meshes();
					auto mIt = meshes->iter();
					if (mIt->begin())
					{
						do
						{
							auto mesh = mIt->current();

							Texture* texOrig = mesh->material()->getTexture(mesh->material()->getUniformLocation("uDiffuse"));
							Texture* tex0 = ResourceManager::default()->getNextTexture();
							Texture* tex1 = ResourceManager::default()->getNextTexture();

							*tex0 = Texture(texOrig->size(), GL_RGBA8, GL_RGBA);
							*tex1 = Texture(texOrig->size(), GL_RGBA8, GL_RGBA);

							std::array<Framebuffer*, 2> fbos;
							{
								NodeList<Texture*> fboColors(ResourceManager::default());
								fboColors.insertBack(tex0);

								fbos[0] = ResourceManager::default()->getNextFramebuffer();
								*fbos[0] = Framebuffer(fboColors, nullptr, tex0->size());
							}

							{
								NodeList<Texture*> fboColors(ResourceManager::default());
								fboColors.insertBack(tex1);

								fbos[1] = ResourceManager::default()->getNextFramebuffer();
								*fbos[1] = Framebuffer(fboColors, nullptr, tex1->size());
							}

							PaintMesh pm{ Swapchain(fbos) };
							pm.mesh = mesh;
							pm.transform = iter->current().transform();
							pm.texLocation = mesh->material()->getUniformLocation("uDiffuse");
							pm.paintTexLocation = _sprayPaintMaterial->getUniformLocation("uDiffuse");

							mesh->material()->setUniform(mesh->material()->getUniformLocation("uDiffuse_TexSize"), glm::vec2(1.0f / texOrig->size().x, 1.0f / texOrig->size().y));


							_paintSwapchains.push_back(pm);

						} while (mIt->next());
					}
				} while (iter->next());
			}
		}

	}

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

#ifdef _DEBUG
	{
		GLenum er = glGetError();
		if (er != 0)
		{
			printf("error: %d \n", er);
		}
	}
#endif
}

void App::render(const glm::ivec4& viewport)
{
    _screenBuffer->bind();

    glClearColor(1.0f, 0.0f, 1.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_mainModel->render(Model::Layer::Opaque);
    //_roomModel.render(GLTFModel::Layer::Opaque);

    //boxMesh->render(glm::translate(glm::mat4(), glm::vec3(0.0, 0.0, -1.0)));

    Framebuffer::bindDefaultVP(viewport);

    glClear(GL_DEPTH_BUFFER_BIT);

	glDepthFunc(GL_ALWAYS);
    _composeMesh->render();
    glDepthFunc(GL_LEQUAL);


	glDepthMask(GL_FALSE);
	_mainModel->render(Model::Layer::Transparent);
    //_roomModel.render(GLTFModel::Layer::Transparent);

	_sprayParticles.render();
	glDepthMask(GL_TRUE);
    

#ifdef _DEBUG
    GLenum er = glGetError();
    if (er != 0)
    {
        printf("error: %d \n", er);
    }
#endif
}

void App::update(double dt)
{
    _sprayParticles.update(dt);
    _mainModel->update(dt);

    if (_controls->buttonHeld(Controls::MOUSE_RIGHT))
    {
        float rotAmount = _controls->deltaMousePos().x * 90.0f * (float)dt;

		_gUniforms.view = glm::rotate(_gUniforms.view, rotAmount, glm::vec3(0.0, 1.0, 0.0));
		_gUniforms.invVP = glm::inverse(_gUniforms.projection * _gUniforms.view);

        _globalUniformBlock->setValue("view", &_gUniforms.view, sizeof(glm::mat4));
        _globalUniformBlock->setValue("invVP", &_gUniforms.invVP, sizeof(glm::mat4));

		_globalUniformBlock->uploadData();

        //_modelRot = glm::angleAxis(rotAmount, glm::vec3(0.0, 1.0, 0.0)) * _modelRot;
        //_modelMtx = glm::scale(glm::translate(glm::mat4_cast(_modelRot), _modelPos), _modelScale);
        //_gltfModel.setMatrix(_modelMtx);

    }
    else if (_controls->buttonHeld(Controls::MOUSE_LEFT))
    {
        glm::vec4 scaledOffset = glm::vec4(_controls->mousePos(), 0.0f, 1.0f);
        scaledOffset.x = 1.0f - scaledOffset.x;
        scaledOffset = glm::vec4(scaledOffset.x - 0.5f, scaledOffset.y - 0.5f, 0.0, 1.0f);
        scaledOffset *= 1.5f;
        scaledOffset.w = 1.0f;

        scaledOffset = glm::inverse(_gUniforms.projection) * scaledOffset;

        scaledOffset.x = scaledOffset.x / scaledOffset.w;
        scaledOffset.y = scaledOffset.y / scaledOffset.w;

        glm::mat4 toolVP = (glm::translate(glm::mat4(), glm::vec3(scaledOffset)) * (glm::perspective(1.2f, 1.0f, 1.f, 30.0f) * _gUniforms.view));

        _sprayPaintMaterial->setUniform(_toolVPLoc, toolVP);
        _sprayParticles.origin(_controls->getVRController(0).position);


        glDisable(GL_CULL_FACE);
        for (int i = 0; i < _paintSwapchains.size(); ++i)
        {
            PaintMesh* pm = &_paintSwapchains[i];

            pm->swapchain.back()->bind();

            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            _sprayPaintMaterial->setUniform(pm->paintTexLocation, pm->swapchain.front()->color());
            pm->mesh->render(pm->transform->worldMatrix(), _sprayPaintMaterial);

            pm->swapchain.swap();

            Framebuffer::bindDefault();

            pm->mesh->material()->setUniform(pm->texLocation, pm->swapchain.front()->color());
        }

        glEnable(GL_CULL_FACE);
    }



#ifdef _DEBUG
    GLenum er = glGetError();
    if (er != 0)
    {
        printf("error: %d \n", er);
    }
#endif
}

void App::setControls(Controls * controls)
{
    _controls = controls;
}

void App::resize(const glm::ivec2 & size)
{
	static bool firstResize = true;

    _screenSize = size;

    _gUniforms.projection = glm::perspective(1.2f, size.x / (float)size.y, 1.f, 30.0f);
    _gUniforms.invVP = glm::inverse(_gUniforms.projection * _gUniforms.view);

    _globalUniformBlock->setValue("view", &_gUniforms.view, sizeof(glm::mat4));
    _globalUniformBlock->setValue("projection", &_gUniforms.projection, sizeof(glm::mat4));
    _globalUniformBlock->setValue("invVP", &_gUniforms.invVP, sizeof(glm::mat4));

    _globalUniformBlock->uploadData();

    *_colorScreenTextures[0] = Texture(size, GL_RGBA8, GL_RGBA);
    *_colorScreenTextures[1] = Texture(size, GL_RGBA32F, GL_RGBA, GL_FLOAT);
    *_colorScreenTextures[2] = Texture(size, GL_RGBA8, GL_RGBA);
    *_colorScreenTextures[3] = Texture(size, GL_RGBA8, GL_RGBA);

	*_depthTexture = Texture(size, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT);

	if (firstResize)
	{
		NodeList<Texture*> fboColors(ResourceManager::default());
		for (int i = 0; i < 4; ++i)
		{
			fboColors.insertBack(_colorScreenTextures[i]);
		}

		*_screenBuffer = Framebuffer(fboColors, _depthTexture, size);
	}
	else
	{
		NodeList<Texture*> fboColors = *_screenBuffer->colors();
		*_screenBuffer = Framebuffer(fboColors, _depthTexture, size);
	}

    Framebuffer::bindDefault();

     int loc = _sprayMaterial->getUniformLocation("screenSize");
    _sprayMaterial->setUniform(loc, glm::vec2(_screenSize));

	if (firstResize)
	{
		firstResize = false;
	}
}

void App::overrideViewProjection(const glm::mat4 & view, const glm::mat4 & projection)
{
    _globalUniformBlock->setValue("view", &view, sizeof(glm::mat4));
    _globalUniformBlock->setValue("projection", &projection, sizeof(glm::mat4));
    glm::mat4 invVP = glm::inverse(projection * view);
    _globalUniformBlock->setValue("invVP", &invVP, sizeof(glm::mat4));

    _globalUniformBlock->uploadData();
}
