#include "Engine.h"
#include "ScriptingEngine.h"
#include <iostream>
#include "GameObject.h"
#include "GraphicsEngine.h"
#include "InputManager.h"
#include "GameObjectFactory.h"
#include "MyTime.h"
#include "./ThirdParty/imgui/imgui_impl_sdl.h"
#include "./ThirdParty/imgui/imgui_impl_opengl3.h"
#include <glm/glm/gtc/matrix_transform.hpp>
#include "PhysicsManager.h"

Engine::Engine()
	:m_isRunning{ true }, m_saveState{ false }, m_loadState{ false }, levelLoader{ LevelLoader() }, levelEditor{ LevelEditor() },
	m_debugMenu{ false }, m_editMenu{ true }, m_drawColliders{ false }
{
}

Engine *Engine::Instance()
{
	static Engine engine;
	return &engine;
}

int Engine::Execute(GraphicsLibrary renderer, int windowWidth, int windowHeight)
{
	if (OnInit(renderer, windowWidth, windowHeight) == false)
	{
		return -1;
	}

	SDL_Event event;

	while (m_isRunning)
	{
		//must reset inputs before polling next event, otherwise previous events are retained
		INPUT->ResetInputValues();
		TIME->UpdateDeltaTime();

		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_WINDOWEVENT)
			{
				switch (event.window.event)
				{
				case SDL_WINDOWEVENT_MOVED:
					TIME->CatchupDeltaTime();
					break;
				case SDL_WINDOWEVENT_RESIZED:
					GRAPHICS->SetViewportToWindowSize();
					break;
				}
			}

			if (m_debugMenu)
			{
				ImGui_ImplSDL2_ProcessEvent(&event);
			}
			OnEvent(&event);
		}
		INPUT->CalcDeltaMouse();
		Update();
		Render();
	}

	Cleanup();

	return 0;
}

void Engine::QuitGame()
{
	m_isRunning = false;
	std::cout << "Hope you enjoyed your stay...\n";
}

void Engine::SaveGame()
{
	levelLoader.SaveLevel();
}

void Engine::LoadGame()
{
	levelLoader.LoadLevel();
}

bool Engine::CheckSaveState()
{
	return m_saveState;
}

bool Engine::OnInit(GraphicsLibrary renderer, int windowWidth, int windowHeight)
{
	srand(std::time(NULL));
	COLLISION->Init();

	if (!GRAPHICS->Init(renderer, windowWidth, windowHeight))
	{
		return false;
	}

	SCRIPT->Initialise();
	SCRIPT->RunInitScript();

	GAMEOBJECT->Start();

	return true;
}

void Engine::OnEvent(SDL_Event *e)
{
	switch (e->type)
	{
	case SDL_QUIT:
		m_isRunning = false;
		break;
	}

	INPUT->CheckEvent(e);
}

void Engine::Update()
{
	GAMEOBJECT->Update();
	PHYSICS->IntegrateBodies();
	COLLISION->GenerateContactData();
	PHYSICS->ResolveContactPoints(COLLISION->GetContactCache());
	if (INPUT->GetKeyDown('`'))
	{
		m_debugMenu = !m_debugMenu;
		if (m_debugMenu)
		{
			INPUT->LockCursor(false);
			GAMEOBJECT->GetGameObject("player")->GetComponent<CCharacter>()->SetMouseEnabled(false);
		}
		else
		{
			INPUT->LockCursor(true);
			GAMEOBJECT->GetGameObject("player")->GetComponent<CCharacter>()->SetMouseEnabled(true);
		}
	}

	if (m_saveState)
		levelLoader.SaveLevel();

	if (m_loadState)
		levelLoader.LoadLevel();
}

void Engine::Render()
{
	GRAPHICS->DirLightShadowPass();
	GRAPHICS->PointLightShadowPass();
	GRAPHICS->CameraRenderPass(m_debugMenu);

	if (m_debugMenu) // TEST WINDOW
	{
		levelEditor.DrawEditor();

		ImGui::Begin("Debug Menu");                          // Create a window called "Hello, world!" and append into it.

		ImGui::Checkbox("Draw Colliders", &m_drawColliders);      // Edit bools storing our window open/close state

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		m_saveState = ImGui::Button("Save", ImVec2(100, 30));
		m_loadState = ImGui::Button("Load", ImVec2(100, 30));

		ImGui::End();
	}

	GRAPHICS->endFrame(m_debugMenu);
}

void Engine::Cleanup()
{
	SCRIPT->Close();
	GRAPHICS->Close();
}
