#include "ScriptingEngine.h"
#include "GameAssetFactory.h"
#include "GameObjectFactory.h"
#include "CStaticMesh.h"
#include "CUserInterface.h"
#include "CScript.h"
#include "CCharacterComponent.h"
#include "InputManager.h"
#include "CCollider.h"
#include "Engine.h"
#include <iostream>

Engine* ScriptingEngine::m_engine = nullptr;

using namespace luabridge;

ScriptingEngine* ScriptingEngine::Instance()
{
	static ScriptingEngine instance;
	return &instance;
}

void ScriptingEngine::Initialise(Engine &engine)
{
    m_L = NewState();
    m_engine = &engine;
}

lua_State* ScriptingEngine::NewState()
{
    lua_State* Lbuff = luaL_newstate();
    if (!Lbuff)
    {
        std::cout << "lua_open() call failed\n";
        return nullptr;
    }

    luaL_openlibs(Lbuff);

    getGlobalNamespace(Lbuff).addFunction("LoadModel", LoadModel);
    getGlobalNamespace(Lbuff).addFunction("LoadScript", LoadScript);
    getGlobalNamespace(Lbuff).addFunction("UnloadTexture", UnloadTexture);
    getGlobalNamespace(Lbuff).addFunction("LoadHeightMap", LoadHeightMap);
    getGlobalNamespace(Lbuff).addFunction("SpawnGameObject", SpawnGameObject);
    getGlobalNamespace(Lbuff).addFunction("GetGameObject", GetGameObject);
    getGlobalNamespace(Lbuff).addFunction("QuitGame", QuitGame);
    getGlobalNamespace(Lbuff).addFunction("SaveGame", SaveGame);
    getGlobalNamespace(Lbuff).addFunction("LoadGame", LoadGame);
    getGlobalNamespace(Lbuff).addFunction("CheckSaveState", CheckSaveState);
    getGlobalNamespace(Lbuff).addFunction("InitSkybox", InitSkybox);
    getGlobalNamespace(Lbuff).addFunction("GetInput", GetInputManager);
    
    getGlobalNamespace(Lbuff)
        .beginClass<Vector3f>("Vector3f")
            .addFunction("GetX", &Vector3f::GetX)
            .addFunction("GetY", &Vector3f::GetY)
            .addFunction("GetZ", &Vector3f::GetZ)
        .endClass();

    getGlobalNamespace(Lbuff)
        .beginClass<Transform>("Transform")
            .addFunction("GetWorldTransform", &Transform::GetWorldTransform)
            .addFunction("GetPosition", &Transform::GetPosition)
            .addFunction("SetPosition", &Transform::SetPosition)
            .addFunction("RotateLocal", &Transform::RotateLocal)
            .addFunction("RotateLocalX", &Transform::RotateLocalX)
            .addFunction("RotateLocalY", &Transform::RotateLocalY)
            .addFunction("RotateLocalZ", &Transform::RotateLocalZ)
            .addFunction("Translate", &Transform::Translate)
            .addFunction("ScaleLocal", &Transform::ScaleLocal)
            .addFunction("GetDistance", &Transform::GetDistance)
            .addFunction("GetDistance3f", &Transform::GetDistance3f)
            .addFunction("MoveTowards", &Transform::MoveTowards)
            .addFunction("MoveTowards3f", &Transform::MoveTowards3f)
            .addFunction("RotateTowards", &Transform::RotateTowards)
            .addFunction("SetParent", &Transform::SetParent)
        .endClass();

    getGlobalNamespace(Lbuff)
        .beginClass<InputManager>("InputManager")
            .addFunction("GetKey", &InputManager::GetKey)
            .addFunction("GetKeyDown", &InputManager::GetKeyDown)
            .addFunction("GetKeyUp", &InputManager::GetKeyUp)
            .addFunction("GetMouseButton", &InputManager::GetMouseButton)
            .addFunction("GetMouseButtonDown", &InputManager::GetMouseButtonDown)
            .addFunction("GetMouseButtonUp", &InputManager::GetMouseButtonUp)
            .addFunction("GetAxis", &InputManager::GetAxis)
            .addFunction("LockCursor", &InputManager::LockCursor)
            .addFunction("CheckCursorLock", &InputManager::CheckCursorLock)
            .addFunction("QuitGame", &InputManager::QuitGame)
        .endClass();

    getGlobalNamespace(Lbuff)
        .beginClass<GameObject>("GameObject")
            .addConstructor <void (*) (void)>()
            .addFunction("GetKey", &GameObject::getFactoryKey)
            .addFunction("AddCStaticMesh", &GameObject::AddCStaticMesh)
            .addFunction("GetCStaticMesh", &GameObject::GetCStaticMesh)
            .addFunction("AddCScript", &GameObject::AddCScript)
            .addFunction("AddCUserInterface", &GameObject::AddCUserInterface)
            .addFunction("GetCUserInterface", &GameObject::GetCUserInterface)
            .addFunction("AddCCharacter", &GameObject::AddCCharacter)
            .addFunction("AddCCameraComponent", &GameObject::AddCCameraComponent)
            .addFunction("GetTransform", &GameObject::GetTransform)
            .addFunction("GetClosestObject", &GameObject::GetClosestObject)
            .addFunction("GetCCamera", &GameObject::GetCCamera)
            .addFunction("GetCCharacter", &GameObject::GetCCharacter)
            .addFunction("AddCCollider", &GameObject::AddCCollider)
            .addFunction("AddCPointLight", &GameObject::AddCPointLight)
            .addFunction("GetCPointLight", &GameObject::GetCPointLight)
            .addFunction("SetActive", &GameObject::SetActive)
            .addFunction("SetDifficulty", &GameObject::SetDifficulty)
            .addFunction("GetDifficulty", &GameObject::GetDifficulty)
            .addFunction("SetParentObject", &GameObject::SetParentObject)
            .addFunction("AddCSound", &GameObject::AddCSound)
            .addFunction("GetCSound", &GameObject::GetCSound)
            .addFunction("SetStatic", &GameObject::SetStatic)
            .addFunction("GetCCollider", &GameObject::GetCCollider)
        .endClass();

    getGlobalNamespace(Lbuff)
        .beginClass<Component>("Component")
            .addFunction("GetTransform", &Component::GetTransform)
        .endClass()
        .deriveClass<CCharacter, Component>("CCharacter")
            .addFunction("Move", &CCharacter::Move)
            .addFunction("Jump", &CCharacter::Jump)
            .addFunction("GetHitpoints", &CCharacter::GetHitpoints)
            .addFunction("SetHitpoints", &CCharacter::SetHitpoints)
            .addFunction("SetPlayerControlled", &CCharacter::SetPlayerControlled)
        .endClass();

    getGlobalNamespace(Lbuff)
        .beginClass<Component>("Component")
            .addFunction("GetTransform", &Component::GetTransform)
        .endClass()
        .deriveClass<CCamera, Component>("CCameraComponent.h")
            .addFunction("SetAsCurrentCamera", &CCamera::SetAsCurrentCamera)
        .endClass();


    getGlobalNamespace(Lbuff)
        .beginClass<Component>("Component")
        .endClass()
        .deriveClass<CStaticMesh,Component>("CStaticMesh")
            .addFunction("AssignModel", &CStaticMesh::AssignModelByKey)
            .addFunction("GetModel", &CStaticMesh::GetModel)
            .addFunction("AssignShader", &CStaticMesh::AssignShader)
        .endClass();

    getGlobalNamespace(Lbuff)
        .beginClass<Component>("Component")
        .endClass()
        .deriveClass<CScript, Component>("CScript")
            .addFunction("AssignScript", &CScript::AssignScriptByKey)
        .endClass();

    getGlobalNamespace(Lbuff)
        .beginClass<Component>("Component")
        .endClass()
        .deriveClass<CPointLight, Component>("CPointLight")
            .addFunction("AssignColour", &CPointLight::AssignColour)
            .addFunction("AssignAmbientStrength", &CPointLight::AssignAmbientStrength)
        .endClass();

    getGlobalNamespace(Lbuff)
        .beginClass<Component>("Component")
        .endClass()
        .deriveClass<CSound, Component>("CSound")
        .addFunction("LoadSound", &CSound::LoadSound)
        .addFunction("PlaySound", &CSound::PlaySound)
        .endClass();

    getGlobalNamespace(Lbuff)
        .beginClass<Component>("Component")
        .endClass()
        .deriveClass<CUserInterface, Component>("CUserInterface")
        .addFunction("SetSize", &CUserInterface::SetSize)
        .addFunction("SetFullscreen", &CUserInterface::SetFullscreen)
        .addFunction("MouseClicked", &CUserInterface::MouseClicked)
        .addFunction("SetPosition", &CUserInterface::SetPosition)
        .endClass();

   getGlobalNamespace(Lbuff)
       .beginClass<Component>("Component")
       .endClass()
       .deriveClass<CCollider, Component>("CCollider")
       .addFunction("AddConvexCollider", &CCollider::AddConvexCollider)
       .addFunction("AddBoxCollider", &CCollider::AddBoxCollider)
       .addFunction("AddConcaveCollider", &CCollider::AddConcaveCollider)
       .addFunction("CollideWith", &CCollider::CollideWith)
       .endClass();

    getGlobalNamespace(Lbuff)
        .beginClass<Asset>("Asset")
        .endClass()
        .deriveClass<Model, Component>("AModel")
        .endClass();

    return Lbuff;
}

void ScriptingEngine::RunInitScript()
{
    if (luaL_dofile(m_L, "../Assets/init.lua") != LUA_OK)
    {
        std::cout << lua_tostring(m_L, -1);
    }
}

void ScriptingEngine::RunStateFromScript(lua_State* luaState, std::string AIscript, std::string AIstate, GameObject* parent)
{
    LuaRef State = getGlobal(luaState, AIstate.c_str());
    State(parent, INPUT);
}

void ScriptingEngine::DoFile(std::string filePath)
{
    luaL_dostring(m_L, filePath.c_str());
}

void ScriptingEngine::Close()
{
    lua_close(m_L);
}

void ScriptingEngine::LoadModel(std::string name, std::string filePath)
{
	ASSET->LoadModel(name, "../Assets/Models/"+filePath);
}

void ScriptingEngine::LoadScript(std::string key, std::string filePath)
{
    ASSET->LoadScript(key, "../Assets/Scripts/"+filePath);
}

void ScriptingEngine::UnloadTexture(std::string key)
{
    ASSET->UnloadTexture(key);
}

void ScriptingEngine::LoadHeightMap(std::string key, std::string filePath)
{
    ASSET->LoadHeightMap(key, "../Assets/HeightMaps/" + filePath);
}

GameObject* ScriptingEngine::SpawnGameObject(std::string key)
{
    return GAMEOBJECT->SpawnGameObject(key);
}

GameObject* ScriptingEngine::GetGameObject(std::string objectKey)
{
    return GAMEOBJECT->GetGameObject(objectKey);
}

void ScriptingEngine::QuitGame()
{
    m_engine->QuitGame();
}

void ScriptingEngine::SaveGame()
{
    m_engine->SaveGame();
}

void ScriptingEngine::LoadGame()
{
    m_engine->LoadGame();
}

bool ScriptingEngine::CheckSaveState()
{
    return m_engine->CheckSaveState();
}

void ScriptingEngine::InitSkybox(
    std::string negx,
    std::string negy,
    std::string negz,
    std::string posx,
    std::string posy,
    std::string posz
)
{
    GRAPHICS->InitSkybox(
        negx,
        negy,
        negz,
        posx,
        posy,
        posz
    );
}

InputManager* ScriptingEngine::GetInputManager()
{
    return InputManager::Instance();
}
