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

/* Whether to search parent scopes if not found in current scope*/
#ifndef SVH_RECURSIVE_SEARCH
#define SVH_RECURSIVE_SEARCH true
#endif

/* Whether to copy settings from parent scope when pushing a new scope*/
#ifndef SVH_COPY_OVERRIDE
#define SVH_COPY_OVERRIDE true
#endif

// Forward declare
template<class T>
struct type_settings;

namespace svh {

	// Polymorphic base for type-erasure
	//struct scope_base {
	//	virtual ~scope_base() = default;
	//};

	struct scope_base : std::enable_shared_from_this<scope_base> {
	private:
		std::weak_ptr<scope_base> parent; /* Root level */
		std::unordered_map<std::type_index, std::unique_ptr<scope_base>> children;

		inline bool is_root() const { return parent.expired(); }
		inline bool has_parent() const { return !parent.expired(); }

	public:
		virtual ~scope_base() = default; // Needed for dynamic_cast
		scope_base() = default;
		scope_base(const scope_base&) = delete;
		scope_base& operator=(const scope_base&) = delete;

		template<class T>
		type_settings<T>& push() {
			const std::type_index key{ typeid(T) };

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
			if (has_parent() && SVH_COPY_OVERRIDE) {
				//TODO
			}

			/* Create new */
			auto child = std::make_unique<type_settings<T>>();
			auto& ref = *child;
			child->parent = shared_from_this();
			children.emplace(key, std::move(child));
			return ref;
		}

		scope_base& pop() {
			if (!has_parent()) {
				throw std::runtime_error("No parent to pop to");
			}
			auto* p = dynamic_cast<scope_base*>(parent.lock().get());
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
				auto* p = dynamic_cast<scope_base*>(parent.lock().get());
				if (!p) {
					throw std::runtime_error("Parent has unexpected type");
				}
				return p->get<T>();
			}

			throw std::runtime_error("Type not found");
		}
	};

	template<typename T>
	struct scope : scope_base {
		type_settings<T> settings;

		void copy_settings_from(const scope_base& other) {
			const type_settings<T>& other_settings = other.get<T>();
			settings = other_settings; // Requires copy assignment
		}
	};

} // namespace svh

template<class T>
struct type_settings : svh::scope_base {};

/* Macros for indenting */
#define ____
#define ________
