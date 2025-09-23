#include "Registry.h"


// ENTITY REGION
#pragma region ENTITY
Entity Registry::Add_Entity()
{
	return entityManager.createEntity();
}

void Registry::Remove_Entity(Entity eID)
{
	// remove all components!
	entityManager.destroyEntity(eID);

	// for CT, call the remove!
	
}
#pragma endregion ENTITY



void Registry::Add_Component(Entity eID, TransformComponent const& comp)
{
	
}