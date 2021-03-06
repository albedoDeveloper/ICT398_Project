#include "CollisionManager.h"
#include "GameObject.h"

CollisionManager::CollisionManager()
{}

void CollisionManager::Init()
{
	physicsWorld = physicsCommon.createPhysicsWorld();
	physicsWorld->setIsDebugRenderingEnabled(true);
	debugRender = &physicsWorld->getDebugRenderer();
	debugRender->setIsDebugItemDisplayed(reactphysics3d::DebugRenderer::DebugItem::COLLISION_SHAPE, true);
	debugRender->setIsDebugItemDisplayed(reactphysics3d::DebugRenderer::DebugItem::CONTACT_POINT, true);
}

CollisionManager *CollisionManager::Instance()
{
	static CollisionManager instance;
	return &instance;
}

bool CollisionManager::CheckCollision(CCollider &myCollider)
{
	m_collision = false;

	physicsWorld->testCollision(myCollider.colBody, *COLLISION);

	return m_collision;
}

void CollisionManager::onContact(const CallbackData &callbackData)
{
	m_collision = true;
	for (int i = 0; i < callbackData.getContactPair(0).getNbContactPoints(); i++)
	{
		reactphysics3d::Vector3 points(callbackData.getContactPair(0).getContactPoint(i).getLocalPointOnCollider1());

		//std::cout << "Contact Points: " << points.x << " " << points.y << " " << points.z << '\n';
	}
}