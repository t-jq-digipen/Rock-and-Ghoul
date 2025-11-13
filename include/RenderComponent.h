#pragma once
#include "Animator.h"
#include "Sprite.h"
#include <variant>


enum ShapeType {
	SHAPE_RECTANGLE,
	SHAPE_CIRCLE
};

struct ShapeComponent {
	ShapeType shape = SHAPE_RECTANGLE;
	Vec2 dimensions = { 1.f, 1.f };
	Vec3 color = { 1.0f, 1.0f, 1.0f };

	std::string Serialize() const {
		json j = {
			{"shapeType", static_cast<int>(shape)},
			{"dimX", dimensions.x},
			{"dimY", dimensions.y},
			{"colorX", color.x},
			{"colorY", color.y},
			{"colorZ", color.z}
		};
		return j.dump();
	}

	static ShapeComponent DeSerialize(Entity eID, const std::string& data) {
		(void)eID;
		if (data.empty()) {
			return ShapeComponent();
		}
		json j = json::parse(data);

		ShapeComponent s;

		s.shape = static_cast<ShapeType>(static_cast<int>(j["shapeType"]));
		s.dimensions.x = j["dimX"];
		s.dimensions.y = j["dimY"];
		s.color.x = j["colorX"];
		s.color.y = j["colorY"];
		s.color.z = j["colorZ"];

		return s;
	}

	Editor::ComponentData EditorData() {
		Editor::ComponentData ret("Shape");
		Editor::Widget_Dropdown dropdown;
		dropdown.ref = reinterpret_cast<unsigned int*>(&shape);
		dropdown.Add_Option("Rectangle", static_cast<unsigned int>(SHAPE_RECTANGLE));
		dropdown.Add_Option("Circle", static_cast<unsigned int>(SHAPE_CIRCLE));
		ret.Add_Data("Shape", dropdown);

		ret.Add_Data("Width", Editor::Widget_Float{ &dimensions.x, true, 0.f, 100.f });
		ret.Add_Data("Height", Editor::Widget_Float{ &dimensions.y, true, 0.f, 100.f });

		ret.Add_Data("Color:Red", Editor::Widget_Float{ &color.x, true, 0.f, 1.f });
		ret.Add_Data("Color:Green", Editor::Widget_Float{ &color.y, true, 0.f, 1.f });
		ret.Add_Data("Color:Blue", Editor::Widget_Float{ &color.z, true, 0.f, 1.f });

		return ret;
	}
};

enum RenderType {
	RT_SHAPE,
	RT_IMAGE,
	RT_SPRITESHEET
};

enum RenderLayer {
	LAYER_BACKGROUND,
	LAYER_GAMEOBJECT,
	LAYER_FOREGROUND,
	LAYER_UI
};

class IRenderType {

};

class RenderComponent {
private:
	// Type for serialization/editing
	RenderType m_type;
	// For rendering layers
	RenderLayer m_layer;	
	// Actual render data
	std::variant<Sprite::SpriteComponent, Animator::AnimatorComponent, ShapeComponent> m_renderData;

public:
	RenderComponent();
};