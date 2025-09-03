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

#ifndef SVH_RECURSIVE_SEARCH
#define SVH_RECURSIVE_SEARCH true
#endif

// Forward declare
template<class T>
struct type_settings;

namespace svh {

	// Polymorphic base for type-erasure
	struct scope_base {
		virtual ~scope_base() = default;
	};

	struct scope : scope_base {
	private:
		scope_base* parent = nullptr; /* Root level */
		std::unordered_map<std::type_index, std::unique_ptr<scope_base>> children;

		inline bool is_root() const { return parent == nullptr; }
		inline bool has_parent() const { return parent != nullptr; }

	public:
		scope() = default;
		scope(const scope&) = delete;
		scope& operator=(const scope&) = delete;
		scope(scope&&) = default;
		scope& operator=(scope&&) = default;

		template<class T>
		type_settings<T>& push() {
			const std::type_index key{ typeid(T) };

			// Reuse if present
			if (auto it = children.find(key); it != children.end()) {
				auto* found = dynamic_cast<type_settings<T>*>(it->second.get());
				if (!found) {
					throw std::runtime_error("Existing child has unexpected type");
				}
				return *found;
			}

			auto child = std::make_unique<type_settings<T>>();
			child->parent = this;
			auto& ref = *child;
			children.emplace(key, std::move(child));
			return ref;
		}

		scope& pop() {
			if (!parent) {
				throw std::runtime_error("No parent to pop to");
			}
			auto* p = dynamic_cast<scope*>(parent);
			if (!p) {
				throw std::runtime_error("Parent has unexpected type");
			}
			return *p;
		}

		template<typename T>
		type_settings<T>& get() {
			const std::type_index key{ typeid(T) };
			auto it = children.find(key);
			if (it != children.end()) {
				auto* found = dynamic_cast<type_settings<T>*>(it->second.get());
				if (!found) {
					throw std::runtime_error("Existing child has unexpected type");
				}
				return *found;
			}

			if (is_root() && SVH_AUTO_INSERT) {
				return push<T>();
			}

			if (has_parent() && SVH_RECURSIVE_SEARCH) {
				// Recurse to parent
				auto* p = dynamic_cast<scope*>(parent);
				if (!p) {
					throw std::runtime_error("Parent has unexpected type");
				}
				return p->get<T>();
			}

			throw std::runtime_error("Type not found");
		}
	};

} // namespace svh

template<class T>
struct type_settings : svh::scope {};

/* Macros for indenting */
#define ____
#define ________
