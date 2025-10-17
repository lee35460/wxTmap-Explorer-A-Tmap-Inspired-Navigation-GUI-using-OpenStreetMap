#pragma once

// wxTmap Explorer - 4-Layer Architecture Umbrella Header
// This is the main header that provides access to all layers of the application
// Similar to Python's __init__.py, this creates clean layer boundaries

// === 4-Layer Architecture Headers ===
#include "domain/Domain.h"           // Domain layer: business logic, entities, value objects
#include "infrastructure/Infrastructure.h"  // Infrastructure: rendering, external systems, persistence  
#include "presentation/Presentation.h"      // Presentation: UI components, views, controllers

// Application layer would go here when implemented
// #include "application/Application.h"     // Application: services, commands, events, use cases

// === Layer Access Aliases ===
// These provide convenient shortcuts while maintaining layer clarity

namespace wx_tmap {
    // Domain layer shortcuts
    namespace domain = ::domain;
    
    // Infrastructure layer shortcuts  
    namespace infrastructure = ::infrastructure;
    
    // Presentation layer shortcuts
    namespace presentation = ::presentation;
    
    // Application layer shortcuts (when implemented)
    // namespace application = ::application;
}

// === Common Type Aliases ===
// Most frequently used types across the application
using LonLat = ::LonLat;
using LocationState = ::LocationState;
using RenderPipeline = ::RenderPipeline;