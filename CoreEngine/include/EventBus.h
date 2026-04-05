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

/**
 * @struct IEvent
 * @brief Base interface for all events published through the EventBus.
 *        Derive from this struct to define a custom event type.
 */
// Try to convert to queue system after M2
struct IEvent {
	virtual ~IEvent() = default;
};

/**
 * @class EventBus
 * @brief Singleton publish/subscribe event dispatcher.
 *
 * Listeners subscribe to a specific event type with a callback.
 * When an event is pushed, all registered handlers for that type
 * are invoked immediately (synchronous dispatch).
 */
class EventBus {
private:
	using BaseHandler = std::function<void(const IEvent&)>;

	/**
	 * @struct Subscriber
	 * @brief Stores a subscriber's unique ID and its type-erased handler callback.
	 */
	struct Subscriber {
		size_t id;          // Unique ID used to identify and remove this subscriber.
		BaseHandler handler; // Type-erased callback invoked when the event fires.
	};

	size_t nextID = 0;
	std::vector<size_t> freeID{};

	template<typename EventType>
	using HandlerFunction = std::function<void(const EventType&)>;

	// Store a vector of functions to be executed when x event happens
	// Keyed by type_index of EVENT TYPE!
	std::unordered_map<std::type_index, std::vector<Subscriber>> subscribers{};


private:
	/** @brief Private constructor — use Get() to access the singleton. */
	EventBus() = default;

	/** @brief Deleted copy constructor to enforce singleton pattern. */
	EventBus(const EventBus&) = delete;

	/** @brief Deleted copy-assignment to enforce singleton pattern. */
	EventBus& operator=(const EventBus&) = delete;

	~EventBus() = default;

	/**
	 * @brief Returns the next available subscriber ID, reusing freed IDs when possible.
	 * @return A unique size_t ID for the new subscriber.
	 */
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

	/**
	 * @brief Returns the singleton instance of EventBus.
	 * @return Reference to the single EventBus instance.
	 */
	static EventBus& Get() {
		static EventBus instance;
		return instance;
	}

	/**
	 * @brief Subscribes a callback to receive events of a specific type.
	 * @tparam EventType The event type to listen for (must derive from IEvent).
	 * @param func Callback function invoked with a const reference to the event.
	 * @return A unique subscriber ID that can be passed to Unsubscribe() to remove the listener.
	 */
	template<typename EventType>
	size_t Subscribe(HandlerFunction<EventType> func) {

		size_t id = AssignID();

		auto& vec = subscribers[typeid(EventType)];

		vec.push_back({ id, [func](const IEvent& event) {
			func(static_cast<const EventType&>(event));
			} });

		return id;

	}

	/**
	 * @brief Immediately dispatches an event to all subscribers of its type.
	 * @tparam EventType The event type to dispatch (must derive from IEvent).
	 * @param event The event instance to broadcast to all registered handlers.
	 */
	template<typename EventType>
	void PushEvent(const EventType& event) {
		auto it = subscribers.find(typeid(EventType));
		if (it == subscribers.end()) { return; }

		for (auto& subscriber : it->second)
		{
			subscriber.handler(event);
		}
	}

	/**
	 * @brief Removes a previously registered subscriber by ID.
	 * @tparam EventType The event type the subscriber was registered for.
	 * @param id The subscriber ID returned by Subscribe().
	 * @throws std::runtime_error If no subscribers are registered for EventType.
	 */
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

	/**
	 * @brief Removes all subscribers for all event types.
	 */
	void Clear() {
		subscribers.clear();
	}

};
