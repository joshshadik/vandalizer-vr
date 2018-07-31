#include "app.h"

#include "resourceManager.h"
#include "primitives.h"

#include "util.h"

#include <vector>

#include <json.hpp>

#include <glm/gtc/constants.hpp>


void App::init()
{
    Primitives::init();
    
    _quadGeo = Primitives::quad();

    _gUniforms.view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, -2.15f));
    _gUniforms.projection = glm::perspective(1.2f, 1.0f, 1.f, 30.0f);
    _gUniforms.invVP = glm::inverse(_gUniforms.projection * _gUniforms.view);

    printf("size of gUniforms: %d\n", sizeof(_gUniforms));

    MaterialUniformBlock::Descriptor globalDescriptor{
        { { "view", 0 },{ "projection", sizeof(glm::mat4) },{ "invVP", sizeof(glm::mat4) * 2 } },
        sizeof(glm::mat4) * 3
    };

    printf("size of descriptor: %d\n", globalDescriptor.second);

    _globalUniformBlock = ResourceManager::global()->getNextUniformBlock();

    *_globalUniformBlock = MaterialUniformBlock(globalDescriptor);
    _globalUniformBlock->setValue("view", &_gUniforms.view, sizeof(glm::mat4));
    _globalUniformBlock->setValue("projection", &_gUniforms.projection, sizeof(glm::mat4));
    _globalUniformBlock->setValue("invVP", &_gUniforms.invVP, sizeof(glm::mat4));

    _globalUniformBlock->uploadData();

    _globalUniformBlock->bind(0);

    vs = ResourceManager::global()->getNextShader();
    fs = ResourceManager::global()->getNextShader();

    Shader::loadFromFile(vs, "resources/shaders/standard.vs", Shader::Vertex);
    Shader::loadFromFile(fs, "resources/shaders/standard.fs", Shader::Fragment);

    program = ResourceManager::global()->getNextShaderProgram();
    *program = ShaderProgram(vs, fs);

    material = ResourceManager::global()->getNextMaterial();
    *material = Material(program);

    boxMesh = ResourceManager::global()->getNextMesh();
    *boxMesh = Mesh(_quadGeo, material);

    _modelPos = glm::vec3(0.0f, 0.0f, 0.0f);
    _modelRot = glm::quat_cast(glm::rotate(glm::mat4(1.0f), -glm::half_pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f)));
    _modelScale = glm::vec3(0.001f);

    _modelMtx = glm::translate(glm::mat4_cast(_modelRot) * glm::scale(glm::mat4(),_modelScale) , _modelPos);

    _vrOffset = glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, -0.75f));

    GLTFModel::loadFromFile(&_gltfModel, "resources/models/venus_de_milo/scene.gltf", program);
    //_modelMtx = glm::scale(glm::translate(glm::mat4_cast(_modelRot), glm::vec3(0.0f, 7.5f, 0.0f)), glm::vec3(0.01f));
    _gltfModel.setMatrix(_modelMtx);
    _mainModel = _gltfModel.model();


#ifdef PROPRIETARY_ASSETS
    roomVS = ResourceManager::global()->getNextShader();
    roomFS = ResourceManager::global()->getNextShader();

    Shader::loadFromFile(roomVS, "resources/shaders/room.vs", Shader::Vertex);
    Shader::loadFromFile(roomFS, "resources/shaders/room.fs", Shader::Fragment);

    roomProgram = ResourceManager::global()->getNextShaderProgram();
    *roomProgram = ShaderProgram(roomVS, roomFS);

    //_roomMtx = glm::scale(glm::translate(glm::mat4_cast(_modelRot), glm::vec3(0.0f, 7.5f, 0.0f)), glm::vec3(0.01f));
    GLTFModel::loadFromFile(&_roomGLTFModel, "resources/models/gallery/scene.gltf", roomProgram);
    _roomModel = _roomGLTFModel.model();
    //_roomModel.setMatrix(_roomMtx);
    _roomModel->root()->setLocal(glm::scale(glm::translate(glm::mat4(), glm::vec3(-2.0f, 0.0f, -0.25f)) 
            * glm::mat4_cast(glm::angleAxis(glm::pi<float>(), glm::vec3(0.0f, 1.0f, 0.0f)) * _modelRot), glm::vec3(0.02f)));
    //_roomModel->root()->setLocal(glm::scale(glm::translate(glm::mat4_cast(_modelRot), glm::vec3(0.0f, 7.5f, 0.0f)), glm::vec3(0.01f)));
    _roomModel->update(0.0f);
    //_gltfModel.setMatrix(glm::rotate(glm::mat4(1.0f), -1.57f, glm::vec3(1.0f, 0.0f, 0.0f)));

#endif


    GLTFModel::loadFromFile(&_sprayCanGLTFModel, "resources/models/spray_can/scene.gltf", program);
    _sprayCanModel = _sprayCanGLTFModel.model();

    Shader* sprayUpdateVS = ResourceManager::global()->getNextShader();
    Shader* sprayUpdateFS = ResourceManager::global()->getNextShader();
    Shader* sprayRenderVS = ResourceManager::global()->getNextShader();
    Shader* sprayRenderFS = ResourceManager::global()->getNextShader();

    Shader::loadFromFile(sprayUpdateVS, "resources/shaders/screen.vs", Shader::Vertex);
    Shader::loadFromFile(sprayUpdateFS, "resources/shaders/spray/pos-vel.fs", Shader::Fragment);
    Shader::loadFromFile(sprayRenderVS, "resources/shaders/spray/particle.vs", Shader::Vertex);
    Shader::loadFromFile(sprayRenderFS, "resources/shaders/spray/particle.fs", Shader::Fragment);

    ShaderProgram* updateProgram = ResourceManager::global()->getNextShaderProgram();
    ShaderProgram* sprayProgram = ResourceManager::global()->getNextShaderProgram();
    Material* updateMaterial = ResourceManager::global()->getNextMaterial();
    _sprayMaterial = ResourceManager::global()->getNextMaterial();

    *updateProgram = ShaderProgram(sprayUpdateVS, sprayUpdateFS);
    *sprayProgram = ShaderProgram(sprayRenderVS, sprayRenderFS);
    *updateMaterial = Material(updateProgram);
    *_sprayMaterial = Material(sprayProgram);

    _particleParent = ResourceManager::global()->getNextTransform();

    _sprayParticles.init(Primitives::quad(), _sprayMaterial, updateMaterial, 64);
    _sprayParticles.setParent(_particleParent);
    _sprayColorLoc = _sprayMaterial->getUniformLocation("diffuseColor");

    _paintColor = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
    _sprayMaterial->setUniform(_sprayColorLoc, glm::vec4(_paintColor.r * 0.7f, _paintColor.g * 0.7f, _paintColor.b * 0.7f, 0.1f));
    _sprayMaterial->setBlended(true);
    _sprayMaterial->setBlendFunction(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    _sprayParticles.lifetime(0.25f);
    _sprayParticles.direction(glm::vec3(0.0f, 0.0f, -1.0f));
    _sprayParticles.magnitude(20.0f);
    _sprayParticles.randomness(0.1f);
    _sprayParticles.origin(glm::vec3(0.0f, 0.0f, 0.0f));
    _sprayParticles.size(0.0005f);


    Shader* sprayPaintVS = ResourceManager::global()->getNextShader();
    Shader* sprayPaintFS = ResourceManager::global()->getNextShader();

    Shader::loadFromFile(sprayPaintVS, "resources/shaders/paint/spray.vs", Shader::Vertex);
    Shader::loadFromFile(sprayPaintFS, "resources/shaders/paint/spray.fs", Shader::Fragment);

    _sprayPaintProgram = ResourceManager::global()->getNextShaderProgram();
    *_sprayPaintProgram = ShaderProgram(sprayPaintVS, sprayPaintFS);
    _sprayPaintMaterial = ResourceManager::global()->getNextMaterial();
    *_sprayPaintMaterial = Material(_sprayPaintProgram);
    _toolVPLoc = _sprayPaintMaterial->getUniformLocation("toolVP");
    _paintColorLoc = _sprayPaintMaterial->getUniformLocation("paintColor");

    _sprayPaintMaterial->setUniform(_paintColorLoc, _paintColor);

    composeVS = ResourceManager::global()->getNextShader();
    composeFS = ResourceManager::global()->getNextShader();

    Shader::loadFromFile(composeVS, "resources/shaders/compose.vs", Shader::Vertex);
    Shader::loadFromFile(composeFS, "resources/shaders/compose.fs", Shader::Fragment);

    composeProgram = ResourceManager::global()->getNextShaderProgram();
    _composeMaterial = ResourceManager::global()->getNextMaterial();
    _composeMesh = ResourceManager::global()->getNextMesh();

    *composeProgram = ShaderProgram(composeVS, composeFS);
    *_composeMaterial = Material(composeProgram);
    *_composeMesh = Mesh(_quadGeo, _composeMaterial);

    for (int i = 0; i < 4; ++i)
    {
        _colorScreenTextures[i] = (ResourceManager::global()->getNextTexture());
    }

    _depthTexture = ResourceManager::global()->getNextTexture();

    _screenBuffer = ResourceManager::global()->getNextFramebuffer();


    int depthLoc = _composeMaterial->getUniformLocation("uDepthTex");

    printf("depth loc: %d\n", depthLoc);

    _composeMaterial->setUniform(_composeMaterial->getUniformLocation("uColorTex"), _colorScreenTextures[0]);
    _composeMaterial->setUniform(_composeMaterial->getUniformLocation("uPositionTex"), _colorScreenTextures[1]);
    _composeMaterial->setUniform(_composeMaterial->getUniformLocation("uNormalTex"), _colorScreenTextures[2]);
    _composeMaterial->setUniform(_composeMaterial->getUniformLocation("uMetalRoughOccTex"), _colorScreenTextures[3]);
    _composeMaterial->setUniform(_composeMaterial->getUniformLocation("uDepthTex"), _depthTexture);


    Texture* sky = ResourceManager::global()->getNextTexture();
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
                            Texture* tex0 = ResourceManager::global()->getNextTexture();
                            Texture* tex1 = ResourceManager::global()->getNextTexture();

                            *tex0 = Texture(texOrig->size(), GL_RGBA8, GL_RGBA);
                            *tex1 = Texture(texOrig->size(), GL_RGBA8, GL_RGBA);

                            std::array<Framebuffer*, 2> fbos;
                            {
                                NodeList<Texture*> fboColors(ResourceManager::global());
                                fboColors.insertBack(tex0);

                                fbos[0] = ResourceManager::global()->getNextFramebuffer();
                                *fbos[0] = Framebuffer(fboColors, nullptr, tex0->size());
                            }

                            {
                                NodeList<Texture*> fboColors(ResourceManager::global());
                                fboColors.insertBack(tex1);

                                fbos[1] = ResourceManager::global()->getNextFramebuffer();
                                *fbos[1] = Framebuffer(fboColors, nullptr, tex1->size());
                            }

                            PaintMesh pm{ Swapchain(fbos) };
                            pm.mesh = mesh;
                            pm.transform = iter->current().transform();
                            pm.texLocation = mesh->material()->getUniformLocation("uDiffuse");
                            pm.paintTexLocation = _sprayPaintMaterial->getUniformLocation("uDiffuse");
                            pm.originalTexture = texOrig;

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

#ifdef PROPRIETARY_ASSETS
    _roomModel->render(Model::Layer::Opaque);
#endif
    _mainModel->render(Model::Layer::Opaque);
    _sprayCanModel->render(Model::Layer::Opaque);
    

    //boxMesh->render(glm::translate(glm::mat4(), glm::vec3(0.0, 0.0, -1.0)));

    Framebuffer::bindDefaultVP(viewport);

    glClear(GL_DEPTH_BUFFER_BIT);

    glDepthFunc(GL_ALWAYS);
    _composeMesh->render();
    glDepthFunc(GL_LEQUAL);


    glDepthMask(GL_FALSE);

#ifdef PROPRIETARY_ASSETS
    _roomModel->render(Model::Layer::Transparent);
#endif

    if (_painting)
    {
        _sprayParticles.render();
    }
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
    _mainModel->update(dt);

    Controls::VRController controller0 = _controls->getVRController(0);

    glm::quat sprayRot = glm::quat_cast(glm::inverse(_vrOffset)) * controller0.orientation * glm::angleAxis(-1.57f, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::vec3 sprayPos = glm::inverse(_vrOffset) * glm::vec4(controller0.position, 1.0);
    //_sprayParticles.origin(sprayPos);
    //_sprayParticles.direction(glm::vec3(0.0f, 0.0f, -1.0f) * glm::conjugate(sprayRot));
    _painting = (controller0.pressedFlags & 2) == 2;

    glm::mat4 sprayMtx = glm::translate(glm::mat4(), sprayPos);
    

    _particleParent->setLocal(sprayMtx * glm::mat4_cast(sprayRot));
    _particleParent->update();
    _sprayCanModel->root()->setLocal(sprayMtx  *  glm::mat4_cast(sprayRot * glm::angleAxis(-glm::pi<float>(), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::angleAxis(-glm::half_pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f)))
            * glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, -0.09f)) * glm::scale(glm::mat4(), glm::vec3(0.01f)));
    _sprayCanModel->update(dt);

    sprayMtx = sprayMtx * glm::mat4_cast(sprayRot);

    if( (controller0.pressedFlags & 16) == 16 || (controller0.pressedFlags & 1) == 1  || (controller0.pressedFlags & 4) == 4)
    {
        float hValue = controller0.axis[2];
        float vValue = controller0.axis[3];

        if (glm::abs(controller0.axis[0]) > glm::abs(hValue))
        {
            hValue = controller0.axis[0];
        }
        if (glm::abs(controller0.axis[1]) > glm::abs(vValue))
        {
            vValue = controller0.axis[1];
        }

        if( glm::abs(hValue) > 0.01 && glm::abs(vValue) > 0.01 )
        {
            float hue = atan2(vValue, hValue);
            hue = (hue + glm::pi<float>()) / glm::two_pi<float>();

            float saturation = glm::sqrt(hValue * hValue + vValue * vValue);

            glm::vec3 rgb = HSVtoRGB(glm::vec3(hue, 1.0f, saturation));

            //_paintColor = glm::vec4((hValue + 1.0f) * 0.5f, (vValue + 1.0f) * 0.5f, 1.0f, 1.0f);

            _paintColor = glm::vec4(rgb, 1.0f);

            _sprayPaintMaterial->setUniform(_paintColorLoc, _paintColor);
            _sprayMaterial->setUniform(_sprayColorLoc, glm::vec4(_paintColor.r * 0.7f, _paintColor.g * 0.7f, _paintColor.b * 0.7f, 0.1f));
        }
    }

    _sprayParticles.update(dt);

    glm::mat4 toolVP;

    if (_painting)
    {
        
        //printf("controller position: %f, %f, %f \n", controller0.position.x, controller0.position.y, controller0.position.z);
        toolVP = _sprayProjection * glm::inverse(sprayMtx); // *glm::mat4_cast(sprayRot));
    }

    if (_controls->buttonHeld(Controls::MOUSE_RIGHT))
    {
        float rotAmount = _controls->deltaMousePos().x * 90.0f * (float)dt;

        _gUniforms.view = glm::rotate(_gUniforms.view, rotAmount, glm::vec3(0.0, 1.0, 0.0));
        _gUniforms.invVP = glm::inverse(_gUniforms.projection * _gUniforms.view);

        _globalUniformBlock->setValue("view", &_gUniforms.view, sizeof(glm::mat4));
        _globalUniformBlock->setValue("invVP", &_gUniforms.invVP, sizeof(glm::mat4));

        _globalUniformBlock->uploadData();
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
        scaledOffset.z = 0.5f;

        toolVP = (glm::translate(glm::mat4(), glm::vec3(scaledOffset)) * (glm::perspective(1.2f, 1.0f, 1.f, 10.0f) * _gUniforms.view));

        _painting = true;
    }

    if (_painting)
    {
        static bool firstPaint = true;
        _sprayPaintMaterial->setUniform(_toolVPLoc, toolVP);

        glDisable(GL_CULL_FACE);
        for (int i = 0; i < _paintSwapchains.size(); ++i)
        {
            PaintMesh* pm = &_paintSwapchains[i];

            pm->swapchain.back()->bind();

            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            _sprayPaintMaterial->setUniform(pm->paintTexLocation, firstPaint ? pm->originalTexture : pm->swapchain.front()->color());
            pm->mesh->render(pm->transform->worldMatrix(), _sprayPaintMaterial);

            pm->swapchain.swap();

            Framebuffer::bindDefault();

            pm->mesh->material()->setUniform(pm->texLocation, pm->swapchain.front()->color());
        }

        if( firstPaint == true )
        {
            firstPaint = false;
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
        NodeList<Texture*> fboColors(ResourceManager::global());
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

    if (firstResize)
    {
        firstResize = false;
    }

}

void App::overrideViewProjection(const glm::mat4 & view, const glm::mat4 & projection)
{
    glm::mat4 v = view * _vrOffset;
    _globalUniformBlock->setValue("view", &v, sizeof(glm::mat4));
    _globalUniformBlock->setValue("projection", &projection, sizeof(glm::mat4));
    glm::mat4 invVP = glm::inverse(projection * v);
    _globalUniformBlock->setValue("invVP", &invVP, sizeof(glm::mat4));

    _globalUniformBlock->uploadData();
}
