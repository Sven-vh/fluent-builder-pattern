![Fluent Builder Drawing](images/Drawings.svg)

# FluentBuilderPattern

A modern C++ library implementing a scope-based fluent builder pattern for creating hierarchical, type-safe configuration systems. It allows you to create nested configuration scopes where child scopes inherit settings from their parents, with the ability to override specific values at any level.

## What is FluentBuilderPattern?

This library provides a way to configure complex, nested settings using a fluent API with visual indentation. Think of it as a hierarchical configuration system where:

- **Settings are organized by type** - Each type (int, float, custom structs) has its own configuration space
- **Scopes can be nested** - Create parent-child relationships between configuration scopes
- **Inheritance works automatically** - Child scopes inherit parent settings unless overridden
- **Visual clarity** - Special indentation macros make the configuration structure obvious

## Features

- **Type-safe**: Template-based design ensures compile-time type safety
- **Hierarchical scoping**: Create nested configuration scopes with automatic inheritance
- **Fluent API**: Intuitive method chaining with visual indentation support
- **Automatic fallback**: Configurable behavior for missing settings with parent scope lookup
- **Header-only**: Single header implementation for easy integration
- **Visual clarity**: Built-in indentation macros for readable configuration code

## Adding Custom Settings

To use the library with your own types, you need to specialize the `type_settings` template:

```cpp
#include "scope.hpp"

// Specialize type_settings for your type
template<>
struct type_settings<int> : svh::scope {
    int _min = std::numeric_limits<int>::min();
    int _max = std::numeric_limits<int>::max();
    
    // Fluent API methods
    type_settings& min(const int& v) { _min = v; return *this; }
    type_settings& max(const int& v) { _max = v; return *this; }
    
    // Getter methods
    const int& get_min() const { return _min; }
    const int& get_max() const { return _max; }
};

// For custom structs
struct MyConfig {};

template<>
struct type_settings<MyConfig> : svh::scope {
    std::string name = "default";
    bool enabled = true;
    int priority = 0;
    
    type_settings& set_name(const std::string& n) { name = n; return *this; }
    type_settings& set_enabled(bool e) { enabled = e; return *this; }
    type_settings& set_priority(int p) { priority = p; return *this; }
    
    const std::string& get_name() const { return name; }
    bool is_enabled() const { return enabled; }
    int get_priority() const { return priority; }
};

// Another custom struct example
struct MyStruct {};

template<>
struct type_settings<MyStruct> : svh::scope {
    float value = 1.0f;
    std::string category = "general";
    
    type_settings& set_value(float v) { value = v; return *this; }
    type_settings& set_category(const std::string& c) { category = c; return *this; }
    
    float get_value() const { return value; }
    const std::string& get_category() const { return category; }
};
```

## Quick Start

### Basic Usage with Push/Pop

The core concept involves creating scopes for different types and using `push()` to enter a scope and `pop()` to exit back to the parent:

```cpp
#include "scope.hpp"

// Create a root scope
svh::scope root;

// Push into an int scope, configure it, then pop back
root.push<int>()           // Enter int configuration scope
    ____.min(-50)          // Set minimum value (indentation is visual only)
    ____.max(50)           // Set maximum value
    .pop();                // Return to root scope

// Retrieve configured settings
auto& int_settings = root.get<int>();
std::cout << "Min: " << int_settings.get_min() << std::endl; // Output: Min: -50
std::cout << "Max: " << int_settings.get_max() << std::endl; // Output: Max: 50
```

### Push/Pop Mechanics Explained

- **`push<T>()`**: Enters a configuration scope for type T. If the scope doesn't exist, it creates one. If a parent scope has settings for T, they are inherited.
- **`pop()`**: Returns to the parent scope. You must call `pop()` to return to where you started.
- **Indentation macros** (`____`, `________`, etc.): These are just empty macros that provide visual indentation - they don't affect functionality but make nested configurations easier to read.

### Multiple Types in Sequence

```cpp
svh::scope root;

root.push<int>()
    ____.min(0)
    ____.max(100)
    .pop()                 // Back to root
    .push<float>()         // Now configure float
    ____.min(-1.0f)
    ____.max(1.0f)
    .pop();                // Back to root

auto& int_settings = root.get<int>();
auto& float_settings = root.get<float>();
```

### Nested Scopes and Inheritance

Create hierarchical configurations where child scopes inherit parent settings:

```cpp
svh::scope root;

// Configure root-level int settings
root.push<int>()
    ____.min(-100)
    ____.max(100)
    .pop();

// Create a MyStruct scope with its own int settings
root.push<MyStruct>()
    ____.push<int>()           // This inherits min=-100, max=100 from root
    ________.max(20)           // Override only the max value
    ____.pop()                 // Back to MyStruct scope
    .pop();                    // Back to root

// Access the nested settings
auto& root_int = root.get<int>();                    // min=-100, max=100
auto& nested_int = root.get<MyStruct>().get<int>(); // min=-100, max=20 (inherited + override)
```

### Multiple Push/Pop Operations

You can push multiple levels at once and pop multiple levels:

```cpp
svh::scope root;

// Push multiple types at once: MyStruct -> bool -> float -> int
root.push<MyStruct, bool, float, int>()
    ________.min(-50)          // Configure the final int scope
    ________.max(50)
    .pop();                    // Pop back through all levels to root

// Access nested settings in one call
auto& settings = root.get<MyStruct, bool, float, int>();
// Equivalent to: root.get<MyStruct>().get<bool>().get<float>().get<int>()
```

## Example Use Cases

### 1. Game Configuration System

```cpp
struct Player {};
struct Graphics {};
struct Audio {};

// Set up type_settings for each
template<>
struct type_settings<Player> : svh::scope {
    std::string name = "Player1";
    int level = 1;
    float health = 100.0f;
    
    type_settings& set_name(const std::string& n) { name = n; return *this; }
    type_settings& set_level(int l) { level = l; return *this; }
    type_settings& set_health(float h) { health = h; return *this; }
};

template<>
struct type_settings<Graphics> : svh::scope {
    int width = 1920, height = 1080;
    bool fullscreen = false;
    
    type_settings& resolution(int w, int h) { width = w; height = h; return *this; }
    type_settings& set_fullscreen(bool f) { fullscreen = f; return *this; }
};

// Usage:
svh::scope game_config;

game_config.push<Player>()
    ____.set_name("Hero")
    ____.set_level(5)
    ____.set_health(150.0f)
    .pop()
    .push<Graphics>()
    ____.resolution(2560, 1440)
    ____.set_fullscreen(true)
    .pop();

// Pass configuration to different systems
void init_player(const svh::scope& config) {
    auto& player = config.get<Player>();
    // Initialize player with player.name, player.level, etc.
}

void init_graphics(const svh::scope& config) {
    auto& gfx = config.get<Graphics>();
    // Set up graphics with gfx.width, gfx.height, etc.
}

init_player(game_config);
init_graphics(game_config);
```

### 2. Validation Rules System

```cpp
struct ValidationRules {};
struct EmailField {};
struct PasswordField {};

template<>
struct type_settings<ValidationRules> : svh::scope {
    bool required = false;
    int min_length = 0;
    int max_length = INT_MAX;
    
    type_settings& require() { required = true; return *this; }
    type_settings& length_range(int min, int max) { 
        min_length = min; max_length = max; return *this; 
    }
};

template<>
struct type_settings<EmailField> : svh::scope {
    std::string pattern = ".*@.*";
    bool validate_domain = false;
    
    type_settings& set_pattern(const std::string& p) { pattern = p; return *this; }
    type_settings& enable_domain_validation(bool v) { validate_domain = v; return *this; }
};

template<>
struct type_settings<PasswordField> : svh::scope {
    bool require_uppercase = false;
    bool require_numbers = false;
    bool require_symbols = false;
    
    type_settings& require_uppercase_letters(bool r) { require_uppercase = r; return *this; }
    type_settings& require_numeric_digits(bool r) { require_numbers = r; return *this; }
    type_settings& require_special_symbols(bool r) { require_symbols = r; return *this; }
};

// Create different validation contexts
svh::scope validation_config;

// Default validation rules
validation_config.push<ValidationRules>()
    ____.length_range(1, 100)
    .pop();

// Email field with stricter rules  
validation_config.push<EmailField>()
    ____.push<ValidationRules>()
    ________.require()                    // Inherits length_range(1,100)
    ________.length_range(5, 255)        // Override length limits
    ____.pop()
    .pop();

// Password field with different rules
validation_config.push<PasswordField>()
    ____.push<ValidationRules>()
    ________.require()                    // Inherits length_range(1,100)
    ________.length_range(8, 50)         // Override for passwords
    ____.pop()
    .pop();
```

### 3. API Configuration with Environment Overrides

```cpp
struct Database {};
struct Cache {};
struct API {};

template<>
struct type_settings<Database> : svh::scope {
    std::string host = "localhost";
    int port = 5432;
    int timeout = 30;
    std::string username = "user";
    
    type_settings& set_host(const std::string& h) { host = h; return *this; }
    type_settings& set_port(int p) { port = p; return *this; }
    type_settings& set_timeout(int t) { timeout = t; return *this; }
    type_settings& set_username(const std::string& u) { username = u; return *this; }
};

template<>
struct type_settings<Cache> : svh::scope {
    int ttl = 3600;  // Time to live in seconds
    int max_size = 1000;
    bool enabled = true;
    
    type_settings& set_ttl(int t) { ttl = t; return *this; }
    type_settings& set_max_size(int s) { max_size = s; return *this; }
    type_settings& enable(bool e) { enabled = e; return *this; }
};

template<>
struct type_settings<API> : svh::scope {
    std::string base_url = "https://api.example.com";
    int rate_limit = 1000;
    int timeout = 10;
    
    type_settings& set_base_url(const std::string& url) { base_url = url; return *this; }
    type_settings& set_rate_limit(int limit) { rate_limit = limit; return *this; }
    type_settings& set_timeout(int t) { timeout = t; return *this; }
};

svh::scope production_config;

// Production defaults
production_config.push<Database>()
    ____.set_host("prod-db.company.com")
    ____.set_port(5432)
    ____.set_timeout(30)
    .pop()
    .push<Cache>()
    ____.set_ttl(3600)
    ____.set_max_size(1000)
    .pop();

// Development environment inherits but overrides some settings
svh::scope dev_config = production_config; // Copy base config

dev_config.push<Database>()
    ____.set_host("localhost")            // Override host
    ____.set_port(5433)                   // Override port
    // timeout remains 30 (inherited)
    .pop()
    .push<Cache>()
    ____.set_ttl(60)                      // Shorter TTL for dev
    // max_size remains 1000 (inherited)
    .pop();
```

### 4. Function Parameter Configuration

```cpp
struct PhysicsSettings {};
struct GraphicsSettings {};
struct AISettings {};

template<>
struct type_settings<PhysicsSettings> : svh::scope {
    float gravity = -9.81f;
    float timestep = 0.016f;
    bool collision_detection = true;
    
    type_settings& set_gravity(float g) { gravity = g; return *this; }
    type_settings& set_timestep(float t) { timestep = t; return *this; }
    type_settings& enable_collision(bool c) { collision_detection = c; return *this; }
};

template<>
struct type_settings<GraphicsSettings> : svh::scope {
    bool shadows = false;
    std::string quality = "medium";
    int fps_limit = 60;
    bool vsync = true;
    
    type_settings& enable_shadows(bool s) { shadows = s; return *this; }
    type_settings& set_quality(const std::string& q) { quality = q; return *this; }
    type_settings& set_fps_limit(int fps) { fps_limit = fps; return *this; }
    type_settings& enable_vsync(bool v) { vsync = v; return *this; }
};

template<>
struct type_settings<AISettings> : svh::scope {
    float difficulty = 0.5f;
    int update_frequency = 30;
    bool pathfinding = true;
    
    type_settings& set_difficulty(float d) { difficulty = d; return *this; }
    type_settings& set_update_frequency(int freq) { update_frequency = freq; return *this; }
    type_settings& enable_pathfinding(bool p) { pathfinding = p; return *this; }
};
// Configure a complex algorithm
void run_simulation(const svh::scope& config) {
    auto& physics = config.get<PhysicsSettings>();
    auto& graphics = config.get<GraphicsSettings>();
    auto& ai = config.get<AISettings>();
    
    // Use the configurations...
}

svh::scope sim_config;
sim_config.push<PhysicsSettings>()
    ____.set_gravity(-9.81f)
    ____.set_timestep(0.016f)
    .pop()
    .push<GraphicsSettings>()
    ____.enable_shadows(true)
    ____.set_quality("high")
    .pop()
    .push<AISettings>()
    ____.set_difficulty(0.8f)
    ____.set_update_frequency(60)
    .pop();

run_simulation(sim_config);
```

## Configuration Options

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

The library provides macros for visual clarity in nested configurations. These are completely optional but help make code more readable:

```cpp
// Without indentation macros:
root.push<MyStruct>().push<int>().min(10).max(20).pop().pop();

// With indentation macros for clarity:
root.push<MyStruct>()
    ____.push<int>()
    ________.min(10)
    ________.max(20)
    ____.pop()
    .pop();
```

Available macros:
- `____` - 4 spaces indentation (single level)
- `________` - 8 spaces indentation (two levels)
- `____________` - 12 spaces indentation (three levels)
- `________________` - 16 spaces indentation (four levels)

**Important**: These macros are purely cosmetic and don't affect functionality. They're defined as empty macros that help visualize the scope nesting structure.

## Building and Testing

This is a header-only library. Simply include `scope.hpp` in your project.

### Requirements
- C++14 or later (uses `auto` return types and `std::make_unique`)
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

The library excels at passing complex configurations to functions:

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

Control what happens when accessing non-existent settings:

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
