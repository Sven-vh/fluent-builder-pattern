![Fluent Builder Drawing](images/Drawings.svg)

# FluentBuilderPattern

A modern C++ library implementing a scope-based fluent builder pattern for creating hierarchical, type-safe configuration systems.

## Features

- **Type-safe**: Template-based design ensures compile-time type safety
- **Hierarchical scoping**: Create nested configuration scopes with automatic inheritance
- **Fluent API**: Intuitive method chaining with visual indentation support
- **Automatic fallback**: Configurable behavior for missing settings with parent scope lookup
- **Header-only**: Single header implementation for easy integration
- **Visual clarity**: Built-in indentation macros for readable configuration code

## Quick Start

### Basic Usage

```cpp
#include "scope.hpp"

// Create a root scope
svh::scope root;

// Configure settings using fluent API
root.push<int>()
    ____.min(-50)
    ____.max(50)
    .pop();

// Retrieve configured settings
auto& int_settings = root.get<int>();
std::cout << "Min: " << int_settings.get_min() << std::endl; // Output: Min: -50
std::cout << "Max: " << int_settings.get_max() << std::endl; // Output: Max: 50
```

### Multiple Types

```cpp
svh::scope root;

root.push<int>()
    ____.min(0)
    ____.max(100)
    .pop()
    .push<float>()
    ____.min(-1.0f)
    ____.max(1.0f)
    .pop();

auto& int_settings = root.get<int>();
auto& float_settings = root.get<float>();
```

### Nested Scopes

```cpp
svh::scope root;

root.push<MyStruct>()
    ____.push<int>()
    ________.min(10)
    ________.max(20)
    ____.pop()
    .pop();

// Access nested settings
auto& nested_settings = root.get<MyStruct>().get<int>();
```

### Multi-level Push/Get

```cpp
svh::scope root;

// Push multiple types at once
root.push<MyStruct, bool, float, int>()
    ________.min(-50)
    ________.max(50)
    .pop();

// Get nested settings in one call
auto& settings = root.get<MyStruct, bool, float, int>();
// Equivalent to: root.get<MyStruct>().get<bool>().get<float>().get<int>()
```

## Configuration

The library behavior can be customized using preprocessor definitions:

### SVH_AUTO_INSERT
```cpp
#define SVH_AUTO_INSERT true  // Default: true
```
When `true`, automatically creates default settings when accessing non-existent types at the root level.
When `false`, throws `std::runtime_error` for missing types.

### SVH_RECURSIVE_SEARCH  
```cpp
#define SVH_RECURSIVE_SEARCH true  // Default: true
```
Enables automatic lookup in parent scopes when a setting is not found in the current scope.

## API Reference

### Core Classes

#### `svh::scope`
The main scope class that manages hierarchical settings.

**Key Methods:**
- `push<T>()` - Create or access a settings scope for type T
- `pop()` - Return to the parent scope
- `get<T>()` - Retrieve settings for type T
- `find<T>()` - Find settings for type T (returns nullptr if not found)
- `debug_log()` - Print the scope hierarchy to console

#### `type_settings<T>`
Base template for type-specific settings. Inherit from this to create custom settings:

```cpp
template<>
struct type_settings<int> : svh::scope {
    int _min = std::numeric_limits<int>::min();
    int _max = std::numeric_limits<int>::max();
    
    type_settings& min(const int& v) { _min = v; return *this; }
    type_settings& max(const int& v) { _max = v; return *this; }
    
    const int& get_min() const { return _min; }
    const int& get_max() const { return _max; }
};
```

### Visual Indentation Macros

The library provides macros for visual clarity in nested configurations:

- `____` - 4 spaces indentation
- `________` - 8 spaces indentation  
- `____________` - 12 spaces indentation
- `________________` - 16 spaces indentation

## Building and Testing

This is a header-only library. Simply include `scope.hpp` in your project.

### Requirements
- C++11 or later
- Standard library support for `<unordered_map>`, `<typeindex>`, `<memory>`

### Running Tests

The project includes comprehensive unit tests using Google Test:

```bash
# Build and run tests using Visual Studio
# Open FluentBuilderPattern.sln in Visual Studio
# Build and run the UnitTests project
```

### Integration

1. Copy `scope.hpp` to your project
2. Include the header: `#include "scope.hpp"`
3. Optionally define configuration macros before including
4. Specialize `type_settings<T>` for your types

## Examples

### Function Parameter Passing

```cpp
void configure_system(const svh::scope& config) {
    auto& int_settings = config.get<int>();
    // Use settings...
}

svh::scope root;
root.push<int>()
    ____.min(-50)
    ____.max(50)
    .pop();

configure_system(root);
```

### Default Fallback Behavior

```cpp
svh::scope root;

if (SVH_AUTO_INSERT) {
    // Automatically creates default settings
    auto& settings = root.get<int>();
} else {
    // Throws std::runtime_error if not found
    try {
        auto& settings = root.get<int>();
    } catch (const std::runtime_error& e) {
        // Handle missing settings
    }
}
```

## License

[Add your license information here]

## Contributing

[Add contribution guidelines here]
