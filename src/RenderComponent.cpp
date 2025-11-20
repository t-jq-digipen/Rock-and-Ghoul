#include "RenderComponent.h"


RenderComponent::RenderComponent() : m_layer{ LAYER_GAMEOBJECT } {
	Sprite::SpriteComponent temp{};
	m_renderData = std::make_unique<Sprite::SpriteComponent>(temp);
}

RenderComponent::RenderComponent(const RenderComponent& rhs)
	: m_layer(rhs.m_layer)
{
	m_renderData = nullptr;

	if (rhs.m_renderData)
		m_renderData = rhs.m_renderData->Clone();
}

RenderComponent& RenderComponent::operator=(const RenderComponent& rhs) {
	if (this != &rhs) {
		m_layer = rhs.m_layer;
		m_renderData = rhs.m_renderData ? rhs.m_renderData->Clone() : nullptr;
	}
	return *this;
}

// default RT_IMAGE if nothing
RenderType RenderComponent::GetRenderType() {
	return m_renderData ? m_renderData->GetType() : RT_IMAGE;
}

const RenderType RenderComponent::GetRenderType() const {
	return m_renderData ? m_renderData->GetType() : RT_IMAGE;
}

void RenderComponent::SetRenderType(RenderType type) {
	switch (type)
	{
	case RT_SHAPE:
	{
		m_renderData = std::make_unique<ShapeComponent>();
	}
	break; // end RT_SHAPE

	case RT_IMAGE:
	{
		m_renderData = std::make_unique<Sprite::SpriteComponent>();
	}
	break; // end RT_IMAGE

	case RT_SPRITESHEET:
	{
		m_renderData = std::make_unique<Sprite::SpriteSheet>();
	}
	break; // end RT_SPRITESHEET
	}
}

void Fix(Sprite::SpriteComponent& spriteComp) {
	spriteComp.Update_ImageSize();
}

void Fix(Sprite::SpriteSheet& spriteSheet) {
	spriteSheet.Sprite().Update_ImageSize();
}

void Fix(ShapeComponent&) { /*do nothing*/ }


void RenderComponent::UpdateData(float dt) {
	m_renderData->Update(dt);
}

ShapeComponent& RenderComponent::GetShape() {
	if (m_renderData->GetType() != RT_SHAPE) {
		throw std::runtime_error("trying to get rendercomp shape but is not shape");
	}
	return *static_cast<ShapeComponent*>(m_renderData.get());
}

const ShapeComponent& RenderComponent::GetShape() const {
	if (m_renderData->GetType() != RT_SHAPE) {
		throw std::runtime_error("trying to get rendercomp shape but is not shape");
	}
	return *static_cast<ShapeComponent*>(m_renderData.get());
}

Sprite::SpriteComponent& RenderComponent::GetSprite() {
	if (m_renderData->GetType() != RT_IMAGE) {
		throw std::runtime_error("trying to get rendercomp sprite but is not sprite");
	}
	return *static_cast<Sprite::SpriteComponent*>(m_renderData.get());
}

const Sprite::SpriteComponent& RenderComponent::GetSprite() const {
	if (m_renderData->GetType() != RT_IMAGE) {
		throw std::runtime_error("trying to get rendercomp sprite but is not sprite");
	}
	return *static_cast<Sprite::SpriteComponent*>(m_renderData.get());
}

Sprite::SpriteSheet& RenderComponent::GetSpriteSheet() {
	if (m_renderData->GetType() != RT_SPRITESHEET) {
		throw std::runtime_error("trying to get rendercomp spritesheet but is not spritesheet");
	}
	return *static_cast<Sprite::SpriteSheet*>(m_renderData.get());
}

const Sprite::SpriteSheet& RenderComponent::GetSpriteSheet() const {
	if (m_renderData->GetType() != RT_SPRITESHEET) {
		throw std::runtime_error("trying to get rendercomp spritesheet but is not spritesheet");
	}
	return *static_cast<Sprite::SpriteSheet*>(m_renderData.get());
}

void RenderComponent::SetShape(ShapeComponent const& shape) {
	m_renderData = std::make_unique<ShapeComponent>(shape);
}

void RenderComponent::SetSprite(Sprite::SpriteComponent const& sprite) {
	m_renderData = std::make_unique<Sprite::SpriteComponent>(sprite);
}

void RenderComponent::SetSpriteSheet(Sprite::SpriteSheet const& spritesheet) {
	m_renderData = std::make_unique<Sprite::SpriteSheet>(spritesheet);
}

std::string RenderComponent::Serialize() const {
	json j;

	j["renderLayer"] = static_cast<int>(m_layer);
	j["renderType"] = static_cast<int>(GetRenderType());

	std::string renderDataString{};
	if (m_renderData) { renderDataString = m_renderData->Serialize(); }

	j["renderData"] = renderDataString;

	return j.dump();
}

RenderComponent RenderComponent::DeSerialize(Entity eID, const std::string& data) {

	if (data.empty()) {
		return RenderComponent();
	}

	json j = json::parse(data);

	RenderComponent rc;

	rc.m_layer = static_cast<RenderLayer>(static_cast<int>(j["renderLayer"]));

	RenderType type = static_cast<RenderType>(static_cast<int>(j["renderType"]));

	std::string renderDataString = j["renderData"];

	switch (type)
	{
	case RT_SHAPE:
	{
		ShapeComponent shape = ShapeComponent::DeSerialize(eID, renderDataString);
		rc.m_renderData = std::make_unique<ShapeComponent>(shape);
	}
	break; // end RT_SHAPE
	case RT_IMAGE:
	{
		Sprite::SpriteComponent sprite = Sprite::SpriteComponent::DeSerialize(eID, renderDataString);
		rc.m_renderData = std::make_unique<Sprite::SpriteComponent>(sprite);
	}
	break; // end RT_IMAGE
	case RT_SPRITESHEET:
	{
		Sprite::SpriteSheet spritesheet = Sprite::SpriteSheet::DeSerialize(eID, renderDataString);
		rc.m_renderData = std::make_unique<Sprite::SpriteSheet>(spritesheet);
	}
	break; // end RT_SPRITESHEET:
	default:
		throw std::runtime_error("Rendercomp deserialize break, check the loading");
	}

	return rc;
}

Editor::ComponentData RenderComponent::EditorData() {
	Editor::ComponentData ret("Render");

	// RenderLayer dropdown
	{
		Editor::Widget_Dropdown dropdown;
		dropdown.ref = reinterpret_cast<unsigned int*>(&m_layer);
		dropdown.Add_Option("Background", LAYER_BACKGROUND);
		dropdown.Add_Option("GameObject", LAYER_GAMEOBJECT);
		dropdown.Add_Option("Foreground", LAYER_FOREGROUND);
		dropdown.Add_Option("UI", LAYER_UI);
		ret.Add_Data("Layer", dropdown);
	}

	// RenderType dropdown
	//{
	//	Editor::Widget_Dropdown dropdown;
	//	dropdown.ref = reinterpret_cast<unsigned int*>(&m_type);
	//	dropdown.Add_Option("Shape", static_cast<unsigned int>(RT_SHAPE));
	//	dropdown.Add_Option("Image", static_cast<unsigned int>(RT_IMAGE));
	//	dropdown.Add_Option("Spritesheet", static_cast<unsigned int>(RT_SPRITESHEET));
	//	ret.Add_Data("Type", dropdown);
	//}

	// Data to be determined in imguisystem...

	return ret;
}