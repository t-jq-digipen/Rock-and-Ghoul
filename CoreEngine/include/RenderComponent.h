/* ========================================================
 * @file	    RenderComponent.h
 * @author	    Tan Jun Quan          | t.junquan@digipen.edu 60%
 * @co-author   Kervin Ong Guan Cheng | kervinguancheng.ong@digipen.edu 40%
 * @date	    19 NOV 2025
 *
 * @brief	Render Component for rendering logic
 * 
 * This file implements the RenderComponent, which encapsulates rendering 
 * data and logic for entities in the ECS.
 * 
 *
 * Copyright © 2026 DigiPen, All rights reserved.
 * ========================================================
 */

#pragma once
#include "DataDefs.h"	// For rendertype, renderlayer, IRenderData
#include "Animator.h"
#include "Sprite.h"
#include <variant>
#include <optional>


enum ShapeType {
	SHAPE_RECTANGLE,
	SHAPE_CIRCLE
};

struct ShapeComponent : public IRenderData {
	ShapeType shape = SHAPE_RECTANGLE;
	Vec2 dimensions = { 1.f, 1.f };
	Vec3 color = { 1.0f, 1.0f, 1.0f };

	void Update(float) override {}

	std::string Serialize() const override {
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

	RenderType GetType() const override { return RT_SHAPE; }

	std::unique_ptr<IRenderData> Clone() const override {
		return std::make_unique<ShapeComponent>(*this);
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




class RenderComponent {
private:
	// Type for serialization/editing
	//RenderType m_type;
	// For rendering layers
	RenderLayer m_layer;


public:
	float alpha{ 1.f };
	// Actual render data
	//std::variant<Sprite::SpriteComponent, Sprite::SpriteSheet, ShapeComponent> m_renderData;
	std::unique_ptr<IRenderData> m_renderData;

public:
	RenderComponent();

	RenderComponent(const RenderComponent& rhs);

	RenderComponent& operator=(const RenderComponent& rhs);

	RenderType GetRenderType();

	const RenderType GetRenderType() const;

	void SetRenderType(RenderType rt);

	RenderLayer GetRenderLayer() { return m_layer; }

	const RenderLayer GetRenderLayer() const { return m_layer; }

	void SetRenderLayer(RenderLayer rl) { m_layer = rl; }

	const float GetAlpha() const { return alpha; }

	void SetAlpha(float a) { alpha = a; }

	void UpdateData(float dt);

	ShapeComponent& GetShape();

	const ShapeComponent& GetShape() const;

	Sprite::SpriteComponent& GetSprite();

	const Sprite::SpriteComponent& GetSprite() const;

	Sprite::SpriteSheet& GetSpriteSheet();

	const Sprite::SpriteSheet& GetSpriteSheet() const;

	void SetShape(ShapeComponent const& shape);

	void SetSprite(Sprite::SpriteComponent const& sprite);

	void SetSpriteSheet(Sprite::SpriteSheet const& spritesheet);

	std::string Serialize() const;

	static RenderComponent DeSerialize(Entity eID, const std::string& data);

	Editor::ComponentData EditorData();
};