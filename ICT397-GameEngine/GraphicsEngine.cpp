#include "GraphicsEngine.h"
#include "GameObjectFactory.h"
#include <iostream>
#include "Color.h"
#include "CCamera.h"
#include "InputManager.h"
#include "./ThirdParty/imgui/imgui.h"
#include "./ThirdParty/imgui/imgui_impl_sdl.h"
#include "./ThirdParty/imgui/imgui_impl_opengl3.h"
#include "Vector3f.h"
#include "Matrix4f.h"
#include "MiscMath.h"

extern "C"
{
	__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
}

extern "C"
{
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

GraphicsEngine::GraphicsEngine()
	:m_window{ nullptr },
	m_renderer{ nullptr },
	m_glContext{ nullptr },
	m_camera{ nullptr },
	m_graphLib{ GraphicsLibrary::OPENGL },
	m_textureIDs{},
	m_skyboxInitialized{ false },
	m_skyboxTextures{},
	m_clear_color{ 0.45f, 0.55f, 0.60f, 1.00f },
	m_imgui_io{},
	m_windowWidth{},
	m_windowHeight{},
	m_litShader{ nullptr },
	m_debugShader{ nullptr },
	m_unlitShader{ nullptr }
{
}

GraphicsEngine::~GraphicsEngine()
{
	delete m_camera;
	delete m_debugShader;
	delete m_litShader;
	delete m_renderer;
	delete m_unlitShader;
	delete m_window;
}

GraphicsEngine *GraphicsEngine::instance()
{
	static GraphicsEngine *engine = new GraphicsEngine();
	return engine;
}

bool GraphicsEngine::Init(GraphicsLibrary renderer, int windowWidth, int windowHeight)
{
	m_windowWidth = windowWidth;
	m_windowHeight = windowHeight;

	switch (renderer)
	{
	case GraphicsLibrary::OPENGL:
		return InitOpenGL(windowWidth, windowHeight);
	case GraphicsLibrary::DIRECTX:
		return InitDirectX();
	default:
		return false;
	}
}

bool GraphicsEngine::initLighting()
{
	glDisable(GL_LIGHTING);
	return true;
}

void GraphicsEngine::newFrame(bool debugMenu)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GRAPHICS->UpdateViewPos();

	GRAPHICS->m_litShader->SetMat4Uniform("projection", GRAPHICS->GetProjection());
	GRAPHICS->m_litShader->SetMat4Uniform("view", GRAPHICS->GetView());
	GRAPHICS->m_litShader->SetFloatUniform("material.shininess", 16); // TODO move somewhere else

	GRAPHICS->m_unlitShader->SetMat4Uniform("projection", GRAPHICS->GetProjection());
	GRAPHICS->m_unlitShader->SetMat4Uniform("view", GRAPHICS->GetView());


	GRAPHICS->m_debugShader->SetMat4Uniform("projection", GRAPHICS->GetProjection());
	GRAPHICS->m_debugShader->SetMat4Uniform("view", GRAPHICS->GetView());


	if (debugMenu)
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();
	}
}

void GraphicsEngine::UpdateViewPos() const
{
	Vector3f viewPosVec = m_camera->GetTransform().GetWorldTransform().GetRelativePosition();
	GRAPHICS->m_litShader->Use();
	GRAPHICS->m_litShader->SetVec3Uniform(
		"viewPos",
		Vector3f(
			viewPosVec.GetX(),
			viewPosVec.GetY(),
			viewPosVec.GetZ()
		)
	);
}

int GraphicsEngine::AddPointLight(CPointLight *light)
{
	int numpointLights = m_lightManager.AddPointLight(light);

	m_litShader->Use();
	m_litShader->SetIntUniform("numOfPointLights", numpointLights);
	GRAPHICS->m_litShader->SetFloatUniform("pointLights[" + std::to_string(numpointLights - 1) + "].ambientStrength", light->GetAmbientStrength());
	GRAPHICS->m_litShader->SetVec3Uniform("pointLights[" + std::to_string(numpointLights - 1) + "].colour", Vector3f(
		light->GetColour().GetX(),
		light->GetColour().GetY(),
		light->GetColour().GetZ()
	));
	GRAPHICS->m_litShader->SetFloatUniform("pointLights[" + std::to_string(numpointLights - 1) + "].constant", light->GetAttenConstant());
	GRAPHICS->m_litShader->SetFloatUniform("pointLights[" + std::to_string(numpointLights - 1) + "].linear", light->GetLinearAttenutation());
	GRAPHICS->m_litShader->SetFloatUniform("pointLights[" + std::to_string(numpointLights - 1) + "].quadratic", light->GetQuadraticAttenuation());

	return numpointLights;
}

void GraphicsEngine::renderObjects()
{
	skybox.DrawSkybox(GetProjection(), GetView());
	GAMEOBJECT->render();

	if (m_drawDebug)
	{
		DrawDebug();
	}
}

void GraphicsEngine::endFrame(bool debugMenu)
{
	if (debugMenu)
	{
		ImGui::Render();
		glViewport(0, 0, (int)m_imgui_io.DisplaySize.x, (int)m_imgui_io.DisplaySize.y);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
	SDL_GL_SwapWindow(m_window);
}

void GraphicsEngine::SetDisplayCamera(CCamera *camera)
{
	m_camera = camera;
}

CCamera *GraphicsEngine::GetDisplayCamera()
{
	return m_camera;
}

void GraphicsEngine::GenerateTexture(std::string key, unsigned char *image, int width, int height)
{
	switch (m_graphLib)
	{
	case GraphicsLibrary::OPENGL:
		break;
	case GraphicsLibrary::DIRECTX:
		// out of scope for ICT397
		break;
	}
}

void GraphicsEngine::DeleteTexture(std::string key)
{
	if (m_textureIDs.find(key) == m_textureIDs.end())
	{
		return;
	}

	unsigned int texId[] = { m_textureIDs.at(key) };
	glDeleteTextures(1, texId);
}

void GraphicsEngine::DrawModel(AModel *model, const Transform &worldTrans, const Shader *shader)
{
	if (!model)
	{
		return;
	}

	Matrix4f modelTrans;

	modelTrans.Translate(worldTrans.GetRelativePosition());
	modelTrans *= worldTrans.GetRelativeOrientation().Conjugate().Mat4Cast();
	modelTrans.Scale(worldTrans.GetRelativeScale());

	shader->SetMat4Uniform("model", modelTrans);

	glEnable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT, GL_FILL);
	glPolygonMode(GL_BACK, GL_FILL);

	model->Draw(shader);
}

unsigned GraphicsEngine::GetTexID(std::string key) const
{
	return m_textureIDs.at(key);
}

void GraphicsEngine::GetScreenSize(int &w, int &h)
{
	int wBuf, hBuf;
	SDL_GetWindowSize(m_window, &wBuf, &hBuf);
	w = wBuf;
	h = hBuf;
}

void GraphicsEngine::Close()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_GL_DeleteContext(m_glContext);
	SDL_DestroyWindow(m_window);
	SDL_Quit();
}

bool GraphicsEngine::InitOpenGL(int windowWidth, int windowHeight)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		std::cout << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
		return false;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

	if ((m_window = SDL_CreateWindow(
		"ICT398 - FrankEngine",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		windowWidth, windowHeight,
		SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN)) == nullptr)
	{
		std::cout << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
		return false;
	}

	if ((m_glContext = SDL_GL_CreateContext(m_window)) == nullptr)
	{
		std::cout << "OpenGL Context could not be created! SDL Error: " << SDL_GetError() << std::endl;
		return false;
	}

	SDL_WarpMouseInWindow(m_window, windowWidth / 2, windowHeight / 2);
	SDL_GL_MakeCurrent(m_window, m_glContext);
	SDL_GL_SetSwapInterval(0);

	// init imgui
	InitImGui();

	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glEnable(GL_FRAMEBUFFER_SRGB); // gamma correction. looks too washed out
	glClearColor(0.4, 0.2, 0.7, 1);
	std::cout << glGetString(GL_VENDOR) << " : " << glGetString(GL_RENDERER) << std::endl; // GPU used
	std::cerr << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
	std::cerr << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
	std::cerr << "GLEW Version: " << glewGetString(GLEW_VERSION) << std::endl;

	m_litShader = new Shader("./shaders/vertexShader.vert", "./shaders/lit.frag");
	m_unlitShader = new Shader("./shaders/vertexShader.vert", "./shaders/unlit.frag");
	m_debugShader = new Shader("./shaders/simple.vert", "./shaders/debug.frag");

	skybox.CreateSkybox(std::vector<std::string>{
		"../Assets/skybox/right.png",
			"../Assets/skybox/left.png",
			"../Assets/skybox/top.png",
			"../Assets/skybox/bottom.png",
			"../Assets/skybox/front.png",
			"../Assets/skybox/back.png"}
	);

	return true;
}

void GraphicsEngine::InitImGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	m_imgui_io = ImGui::GetIO();
	(void)m_imgui_io;
	ImGui::StyleColorsDark();
	ImGui_ImplSDL2_InitForOpenGL(m_window, m_glContext);
	ImGui_ImplOpenGL3_Init("#version 330 core");
}

bool GraphicsEngine::InitDirectX()
{
	// out of scope for ICT398
	return false;
}

void GraphicsEngine::InitDebug(std::vector <float> &tempVector)
{
	if (!tempVector.empty())
	{
		// create buffers/arrays
		glGenVertexArrays(1, &VAODebug);
		if (VBODebug == 0)
			glGenBuffers(1, &VBODebug);
		glBindVertexArray(VAODebug);
		// load data into vertex buffers
		glBindBuffer(GL_ARRAY_BUFFER, VBODebug);

		glBufferData(GL_ARRAY_BUFFER, sizeof(tempVector.data()[0]) * tempVector.size(), tempVector.data(), GL_DYNAMIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
		glEnableVertexAttribArray(0);
		glBindVertexArray(0);

		initDebug = false;
	}
}

void GraphicsEngine::DrawDebug()
{
	m_debugShader->Use();

	m_debugShader->SetMat4Uniform("model", Matrix4f());
	m_debugShader->SetVec3Uniform("ourColour", Vector3f(1, 0, 0));

	glDisable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT, GL_LINE);
	glPolygonMode(GL_BACK, GL_LINE);

	std::vector <float> tempVector;

	for (int i = 0; i < COLLISION->debugRender->getNbTriangles(); i++)
	{
		tempVector.emplace_back(COLLISION->debugRender->getTrianglesArray()[i].point1.x);
		tempVector.emplace_back(COLLISION->debugRender->getTrianglesArray()[i].point1.y);
		tempVector.emplace_back(COLLISION->debugRender->getTrianglesArray()[i].point1.z);

		tempVector.emplace_back(COLLISION->debugRender->getTrianglesArray()[i].point2.x);
		tempVector.emplace_back(COLLISION->debugRender->getTrianglesArray()[i].point2.y);
		tempVector.emplace_back(COLLISION->debugRender->getTrianglesArray()[i].point2.z);

		tempVector.emplace_back(COLLISION->debugRender->getTrianglesArray()[i].point3.x);
		tempVector.emplace_back(COLLISION->debugRender->getTrianglesArray()[i].point3.y);
		tempVector.emplace_back(COLLISION->debugRender->getTrianglesArray()[i].point3.z);
	}

	if (initDebug)
	{
		InitDebug(tempVector);
	}
	else
	{
		glBufferData(GL_ARRAY_BUFFER, sizeof(tempVector.data()[0]) * tempVector.size(), tempVector.data(), GL_DYNAMIC_DRAW);
	}

	glBindVertexArray(VAODebug);
	glDrawArrays(GL_TRIANGLES, 0, COLLISION->physicsWorld->getDebugRenderer().getNbTriangles() * 3);
	glBindVertexArray(0);

}

Matrix4f GraphicsEngine::GetProjection()
{
	return Perspective(
		m_camera->GetCamera().FOV,
		((float)GRAPHICS->m_windowWidth / GRAPHICS->m_windowHeight),
		m_camera->GetCamera().NearClip,
		m_camera->GetCamera().FarClip
	);
}

Matrix4f GraphicsEngine::GetView()
{
	return LookAt(
		m_camera->GetTransform().GetWorldTransform().GetRelativePosition(),
		m_camera->GetTransform().GetWorldTransform().GetRelativePosition() + m_camera->GetTransform().GetWorldTransform().GetRelativeForward(),
		m_camera->GetTransform().GetWorldTransform().GetRelativeUp()
	);
}
