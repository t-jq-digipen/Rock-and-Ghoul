#include "RenderComponent.h"


RenderComponent::RenderComponent() : m_type{ RT_IMAGE }, m_layer{ LAYER_GAMEOBJECT }, m_renderData{Sprite::SpriteComponent()} {}