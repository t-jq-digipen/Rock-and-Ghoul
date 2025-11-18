#include "RenderComponent.h"


RenderComponent::RenderComponent() : m_type{ RT_IMAGE }, m_layer{ LAYER_GAMEOBJECT }, m_renderData{Sprite::SpriteComponent()} {}

void Fix(Sprite::SpriteComponent& spriteComp) {
	spriteComp.Update_ImageSize();
}

void Fix(Sprite::SpriteSheet& spriteSheet) {
	spriteSheet.Sprite().Update_ImageSize();
}

void Fix(ShapeComponent&) { /*do nothing*/ }


void RenderComponent::UpdateData(float dt) {

	//switch (m_type)
	//{
	//case RT_SHAPE:
	//{
	//	return;
	//}
	//break; // end RT_SHAPE

	//case RT_IMAGE:
	//{
	//	std::get<Sprite::SpriteComponent>(m_renderData).Update_ImageSize();
	//	return;
	//}
	//break; // end RT_IMAGE

	//case RT_SPRITESHEET:
	//{
	//	Sprite::SpriteSheet& spritesheet = std::get<Sprite::SpriteSheet>(m_renderData);
	//	spritesheet.Sprite().Update_ImageSize();
	//	spritesheet.UpdateFrame(dt);
	//	return;
	//}
	//break; // end RT_SPRITESHEET

	//default:
	//	break;
	//}

	std::visit([&](auto& data)
		{ Fix(data); }, m_renderData);

	if (m_type == RT_SPRITESHEET)
	{
		std::get<Sprite::SpriteSheet>(m_renderData).UpdateFrame(dt);
		
	}

}

ShapeComponent& RenderComponent::GetShape() {
	if (m_type != RT_SHAPE) {
		std::cout << "Render component type wrong :( not shape!\n";
		ShapeComponent temp{};
		return temp; 
	}
	return std::get<ShapeComponent>(m_renderData);
}

Sprite::SpriteComponent& RenderComponent::GetSprite() {
	if (m_type != RT_IMAGE) {
		Sprite::SpriteComponent temp{};
		std::cout << "Render component type wrong :( not image!\n"; 
		return temp; 
	}
	return std::get<Sprite::SpriteComponent>(m_renderData);
}

Sprite::SpriteSheet& RenderComponent::GetSpriteSheet() {
	if (m_type != RT_SPRITESHEET) {
		Sprite::SpriteSheet temp{};
		std::cout << "Render component type wrong :( not spritesheet!\n";
		return temp;
	}
	return std::get<Sprite::SpriteSheet>(m_renderData);
}

std::string RenderComponent::Serialize() const {
	json j;

	j["renderType"] = static_cast<int>(m_type);
	j["renderLayer"] = static_cast<int>(m_layer);

	std::string renderDataString{};

	std::visit([&](auto&& renderData) {
		renderDataString = renderData.Serialize();
		}, m_renderData);

	j["renderData"] = renderDataString;

	return j.dump();
}

RenderComponent RenderComponent::DeSerialize(Entity eID, const std::string& data) {

	if (data.empty()) {
		return RenderComponent();
	}

	json j = json::parse(data);

	RenderComponent rc;

	rc.m_type = static_cast<RenderType>(static_cast<int>(j["renderType"]));
	rc.m_layer = static_cast<RenderLayer>(static_cast<int>(j["renderLayer"]));

	std::string renderDataString = j["renderData"];

	switch (rc.m_type)
	{
	case RT_SHAPE:
	{
		rc.m_renderData = ShapeComponent::DeSerialize(eID, renderDataString);
	}
	break; // end RT_SHAPE
	case RT_IMAGE:
	{
		rc.m_renderData = Sprite::SpriteComponent::DeSerialize(eID, renderDataString);
	}
	break; // end RT_IMAGE
	case RT_SPRITESHEET:
	{
		rc.m_renderData = Sprite::SpriteSheet::DeSerialize(eID, renderDataString);
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
	{
		Editor::Widget_Dropdown dropdown;
		dropdown.ref = reinterpret_cast<unsigned int*>(&m_type);
		dropdown.Add_Option("Shape", static_cast<unsigned int>(RT_SHAPE));
		dropdown.Add_Option("Image", static_cast<unsigned int>(RT_IMAGE));
		dropdown.Add_Option("Spritesheet", static_cast<unsigned int>(RT_SPRITESHEET));
		ret.Add_Data("Type", dropdown);
	}

	// Data to be determined in imguisystem...

	return ret;
}