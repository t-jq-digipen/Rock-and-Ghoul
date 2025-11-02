/* ========================================================
 * @file		EventBus.h
 * @author		Tan Jun Quan		| t.junquan@digipen.edu
 * @date		02 NOV 2025
 *
 * @brief	Event System to manage events
 * 
 * Singleton Event System
 * Follows a subscriber-listener system, which immediately calls
 * the functions subscribed when an event is pushed
 * 
 * Ideally to be converted to an event bus where events are
 * stored in a queue and dispatched later but I don't understand
 * the concept enough to implement that yet :') it's 6AM im tired
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#pragma once

#include <string>
#include <map>
#include <functional>
#include <typeindex>

// Try to convert to queue system after M2
struct IEvent {
	virtual ~IEvent() = default;
};

class EventBus {
private:
	using BaseHandler = std::function<void(const IEvent&)>;

	template<typename EventType>
	using HandlerFunction = std::function<void(const EventType&)>;


	// Store a vector of functions to be executed when x event happens
	// Keyed by type_index of EVENT TYPE!
	std::unordered_map<std::type_index, std::vector<BaseHandler>> subscribers;
	
private:
	EventBus() = default;

	EventBus(const EventBus&) = delete;
	EventBus& operator=(const EventBus&) = delete;

	~EventBus() = default;

public:

	static EventBus& Get() {
		static EventBus instance;
		return instance;
	}
	
	template<typename EventType>
	void Subscribe(HandlerFunction<EventType> func) {
		auto& handlers = subscribers[typeid(EventType)];
		handlers.push_back([func](const IEvent& event) {
			func(static_cast<const EventType&>(event));
			});
	}

	template<typename EventType>
	void PushEvent(const EventType& event) {
		auto it = subscribers.find(typeid(EventType));
		if (it == subscribers.end()) { return; }

		for (auto& eventhandler : it->second)
		{
			eventhandler(event);
		}
	}

	void Clear() {
		subscribers.clear();
	}

};