#pragma once
#include <unordered_map>
#include <vector>
#include <typeindex>
#include <any>
#include <stdexcept>
#include <memory>

/* Whether to insert a default object at root level if not found in any scope*/
#ifndef SVH_AUTO_INSERT
#define SVH_AUTO_INSERT true
#endif

// Forward declare
template<class T>
struct type_settings;

namespace svh {

	// Polymorphic base for type-erasure
	//struct scope {
	//	virtual ~scope() = default;
	//};

	struct scope {
	private:
		scope* parent = nullptr; /* Root level */
		std::unordered_map<std::type_index, std::shared_ptr<scope>> children; /* shared since we need to copy the base*/

		inline bool is_root() const { return parent == nullptr; }
		inline bool has_parent() const { return parent != nullptr; }

		template<typename T>
		constexpr std::type_index get_type_key() const { return std::type_index{ typeid(std::decay_t<T>) }; }

		template<typename T>
		type_settings<T>& emplace_new() {
			const std::type_index key = get_type_key<T>();
			auto child = std::make_unique<type_settings<T>>();
			auto& ref = *child;
			child->parent = this;
			children.emplace(key, std::move(child));
			return ref;
		}

	public:
		virtual ~scope() = default; // Needed for dynamic_cast
		scope() = default;

		/// <summary>
		/// Push a new scope for type T. If one already exists, it is returned.
		/// Else if a parent has one, it is copied.
		/// Else, a new one is created.
		/// </summary>
		/// <typeparam name="T">The type of the scope to push</typeparam>
		/// <returns>Reference to the pushed scope</returns>
		template<class T>
		type_settings<T>& push() {
			const std::type_index key = get_type_key<T>();

			/* Reuse if present */
			auto it = children.find(key);
			if (it != children.end()) {
				auto* found = dynamic_cast<type_settings<T>*>(it->second.get());
				if (!found) {
					throw std::runtime_error("Existing child has unexpected type");
				}
				return *found;
			}

			/* copy if found recursive */
			if (has_parent()) {
				auto* found = find<T>();
				if (found) {
					auto child = std::make_unique<type_settings<T>>(*found); // Copy construct
					auto& ref = *child;
					child->parent = this;
					child->children.clear(); // Clear children to avoid copying them
					children.emplace(key, std::move(child));
					return ref;
				}
			}

			/* Else create new */
			return emplace_new<T>();
		}

		/// <summary>
		/// Push a new scope for type T with default values.
		/// </summary>
		/// <typeparam name="T">The type of the scope to push</typeparam>
		/// <returns>Reference to the pushed scope</returns>
		template<class T>
		type_settings<T>& push_default() {
			const std::type_index key = get_type_key<T>();

			/* reset if present */
			auto it = children.find(key);
			if (it != children.end()) {
				auto* found = dynamic_cast<type_settings<T>*>(it->second.get());
				if (!found) {
					throw std::runtime_error("Existing child has unexpected type");
				}
				*found = type_settings<T>{}; // Reset to default
				return *found;
			}

			/* Else create new */
			return emplace_new<T>();
		}

		scope& pop() const {
			if (!has_parent()) {
				throw std::runtime_error("No parent to pop to");
			}
			return *parent;
		}

		/// <summary>
		/// Get the scope for type T. If not found, recurse to parent.
		/// If not found and at root, optionally insert a default one depending on ``SVH_AUTO_INSERT``.
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <returns></returns>
		template<typename T>
		type_settings<T>& get() {

			auto* found = find<T>();
			if (found) {
				return *found;
			}

			if (is_root() && SVH_AUTO_INSERT) {
				return emplace_new<T>();
			}

			throw std::runtime_error("Type not found");
		}

		template <typename T>
		type_settings<T>* find() const {
			const std::type_index key = get_type_key<T>();
			auto it = children.find(key);
			if (it != children.end()) {
				auto* found = dynamic_cast<type_settings<T>*>(it->second.get());
				if (!found) {
					throw std::runtime_error("Existing child has unexpected type");
				}
				return found;
			}
			if (has_parent()) {
				return parent->find<T>();
			}
			return nullptr; // Not found
		}
	};
} // namespace svh

template<class T>
struct type_settings : svh::scope {};

/* Macros for indenting */
#define ____
#define ________
