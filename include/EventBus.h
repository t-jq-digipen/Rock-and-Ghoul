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
#include <stdexcept>

// Try to convert to queue system after M2
struct IEvent {
	virtual ~IEvent() = default;
};

class EventBus {
private:
	using BaseHandler = std::function<void(const IEvent&)>;

	struct Subscriber {
		size_t id;
		BaseHandler handler;
	};

	size_t nextID = 0;
	std::vector<size_t> freeID{};

	template<typename EventType>
	using HandlerFunction = std::function<void(const EventType&)>;


	// Store a vector of functions to be executed when x event happens
	// Keyed by type_index of EVENT TYPE!
	std::unordered_map<std::type_index, std::vector<Subscriber>> subscribers{};


	
private:
	EventBus() = default;

	EventBus(const EventBus&) = delete;
	EventBus& operator=(const EventBus&) = delete;

	~EventBus() = default;

	size_t AssignID() {
		if (freeID.empty())
		{
			return ++nextID;
		}
		size_t temp = freeID.back();
		freeID.pop_back();
		return temp;
	}

public:


	static EventBus& Get() {
		static EventBus instance;
		return instance;
	}
	
	template<typename EventType>
	size_t Subscribe(HandlerFunction<EventType> func) {
		auto& handlers = subscribers[typeid(EventType)];
		size_t id = AssignID();
		
		auto& vec = subscribers[typeid(EventType)];

		vec.push_back({ id, [func](const IEvent& event) {
			func(static_cast<const EventType&>(event));
			} });

		return id;

	}

	template<typename EventType>
	void PushEvent(const EventType& event) {
		auto it = subscribers.find(typeid(EventType));
		if (it == subscribers.end()) { return; }

		for (auto& subscriber : it->second)
		{
			subscriber.handler(event);
		}
	}

	template<typename EventType>
	void Unsubscribe(size_t id) {
		auto it = subscribers.find(typeid(EventType));
		if (it == subscribers.end()) { throw std::runtime_error("EVENT DOES NOT EXIST\n"); return; } // This event dont exist...?

		std::vector<Subscriber>& vec = it->second;
		vec.erase(std::remove_if(vec.begin(), vec.end(),
			[id](const Subscriber& s) { return s.id == id; }),
			vec.end());

		freeID.push_back(id);
	}

	void Clear() {
		subscribers.clear();
	}

};