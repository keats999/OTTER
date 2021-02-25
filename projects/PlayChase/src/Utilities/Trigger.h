#pragma once
#include <entt.hpp>
#include <typeindex>
#include <memory>

struct TriggerBinding;
class Trigger
{
public:
	bool    Enabled = true;
	virtual ~Trigger() = default;

	virtual void OnTrigger(entt::handle handle) {}

	bool Triggered = false;

};
struct TriggerBinding {
	std::vector<std::shared_ptr<Trigger>> Triggers;

	template <typename T, typename ... TArgs, typename = typename std::enable_if<std::is_base_of<Trigger, T>::value>::type>
	static std::shared_ptr<T> Bind(entt::handle entity, TArgs&&... args) {
		// Get the binding component
		TriggerBinding& binding = entity.get_or_emplace<TriggerBinding>();
		// Make a new Trigger, forwarding the arguments
		const std::shared_ptr<Trigger> trigger = std::make_shared<T>(std::forward<TArgs>(args)...);
		// Append it to the binding component's storage
		binding.Triggers.push_back(trigger);
		return std::dynamic_pointer_cast<T>(trigger);
	}

	template <typename T, typename ... TArgs, typename = typename std::enable_if<std::is_base_of<Trigger, T>::value>::type>
	static std::shared_ptr<T> BindDisabled(entt::handle entity, TArgs&&... args) {
		// Get the binding component
		TriggerBinding& binding = entity.get_or_emplace<TriggerBinding>();
		// Make a new Trigger, forwarding the arguments
		const std::shared_ptr<Trigger> trigger = std::make_shared<T>(std::forward<TArgs>(args)...);
		trigger->Enabled = false;
		// Append it to the binding component's storage
		binding.Triggers.push_back(trigger);
		return std::dynamic_pointer_cast<T>(trigger);
	}

	template <typename T, typename = typename std::enable_if<std::is_base_of<Trigger, T>::value>::type>
	static bool Has(entt::handle entity) {
		// Check to see if the entity has a Trigger binding attached
		if (entity.has<TriggerBinding>()) {
			// Get the binding component
			const auto& binding = entity.get<TriggerBinding>();
			// Iterate over all the pointers in the binding list
			for (const auto& ptr : binding.Triggers) {
				// If the pointer type matches T, we return true
				if (std::type_index(typeid(*ptr.get())) == std::type_index(typeid(T))) {
					return true;
				}
			}
			return false;
		}
		else return false;
	}

	template <typename T, typename = typename std::enable_if<std::is_base_of<Trigger, T>::value>::type>
	static std::shared_ptr<T> Get(entt::handle entity) {
		// Check to see if the entity has a Trigger binding attached
		if (entity.has<TriggerBinding>()) {
			// Get the binding component
			const auto& binding = entity.get<TriggerBinding>();
			// Iterate over all the pointers in the binding list
			for (const auto& ptr : binding.Triggers) {
				// If the pointer type matches T, we return that Trigger, making sure to cast it back to the requested type
				if (std::type_index(typeid(*ptr.get())) == std::type_index(typeid(T))) {
					return std::dynamic_pointer_cast<T>(ptr);
				}
			}
			return nullptr;
		}
		else return nullptr;
	}
};


