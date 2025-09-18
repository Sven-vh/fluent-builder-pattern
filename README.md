![Fluent Builder Drawing](images/Drawings.svg)


A modern C++ library implementing a scope-based fluent builder pattern for creating hierarchical configurations.

## What is FluentBuilderPattern?

This library provides a way to configure complex, nested settings using a fluent API with visual indentation. Think of it as a hierarchical configuration system where:

- **Settings are organized by type** - Each type (int, float, custom structs) has its own configuration space
- **Scopes can be nested** - Create parent-child relationships between configuration scopes

## Quick Start

### Basic Usage with Push/Pop

The core concept involves creating scopes for different types and using `push()` to enter a scope and `pop()` to exit back to the parent:

```cpp
#include "scope.hpp"

// Custom int settings
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

// Create a root scope
svh::scope root;

// Push into an int scope, configure it, then pop back
root.push<int>()           // Enter int configuration scope
    	.min(-50)          // Set minimum value
    	.max(50)           // Set maximum value
    .pop();                // Return to root scope

// Retrieve configured settings
int min_value = root.get<int>().get_min(); // -50
int max_value = root.get<int>().get_max(); // 50
```

### Push/Pop Mechanics Explained

- **`push<T>()`**: Enters a configuration scope for type T. If the scope doesn't exist, it creates one. If a parent scope has settings for T, they are inherited.
- **`pop()`**: Returns to the parent scope. You must call `pop()` to return to where you started.
- **Indentation macros** (`____`, `________`, etc.): These are just empty macros that provide visual indentation - they don't affect functionality but make nested configurations easier to read.

### Multiple Types in Sequence

```cpp
svh::scope root;

root.push<int>()
    	.min(0)
    	.max(100)
    .pop()                 // Back to root
    .push<float>()         // Now configure float
    	.min(-1.0f)
    	.max(1.0f)
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
    	.min(-100)
    	.max(100)
    .pop()                     // Back to root
    // Create a MyStruct scope with its own int settings
    .push<MyStruct>()
    	.push<int>()           // This inherits min=-100, max=100 from root
    		.max(20)           // Override only the max value
    	.pop()                 // Back to MyStruct scope
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
    		.min(-50)          // Configure the final int scope
    		.max(50)
    .pop();                    // Pop back through all levels to root

// Access nested settings in one call
auto& settings = root.get<MyStruct, bool, float, int>();
// Equivalent to: 
auto& settings = root.get<MyStruct>().get<bool>().get<float>().get<int>()
```

### Adding Custom Settings

To use the library with your own types, you need to specialize the `type_settings` template with your getters and setters:

```cpp
#include "scope.hpp"

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
```

## Example Use Cases

### 1. Game Configuration System

```cpp
struct Player {};
struct Graphics {};

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
    	.set_name("Hero")
    	.set_level(5)
    	.set_health(150.0f)
    .pop()
    .push<Graphics>()
    	.resolution(2560, 1440)
    	.set_fullscreen(true)
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

### 2. Function Parameter Configuration

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
    	.set_gravity(-9.81f)
    	.set_timestep(0.016f)
    .pop()
    .push<GraphicsSettings>()
    	.enable_shadows(true)
    	.set_quality("high")
    .pop()
    .push<AISettings>()
    	.set_difficulty(0.8f)
    	.set_update_frequency(60)
    .pop();

run_simulation(sim_config);
```

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

The library provides macros for visual clarity in nested configurations. These are completely optional but help make code more readable. It also helps with formatters that apply automatically and remove indentation. 

```cpp
// Without indentation macros:
root.push<MyStruct>().push<int>().min(10).max(20).pop().pop();
// Or:
root.push<MyStruct>()
    .push<int>()
    .min(10)
    .max(20)
    .pop()
    .pop();

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

> [!TIP]
> These macros are purely cosmetic and don't affect functionality. They're defined as empty macros that help visualize the scope nesting structure.

## Building and Testing

This is a header-only library. Simply include `scope.hpp` in your project.

### Requirements
- C++14 or later (uses `auto` return types and `std::make_unique`)
- Standard library support for `<unordered_map>`, `<typeindex>`, `<memory>`

### Running Tests

The project includes unit tests using Google Test:

```bash
# Build and run tests using Visual Studio
# Open FluentBuilderPattern.sln in Visual Studio
# Build and run the UnitTests project (ctrl+F5)
```

### Integration

1. Copy `scope.hpp` to your project
2. Include the header: `#include "scope.hpp"`
3. Optionally define configuration macros before including
4. Specialize `type_settings<T>` for your types

## Examples

### Function Parameter Passing

You can use the scope to pass configuration settings to functions:

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
