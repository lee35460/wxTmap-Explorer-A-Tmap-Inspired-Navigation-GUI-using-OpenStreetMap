#pragma once

// ============================================================================
// 🗺️ wxTmap-Explorer Project File Map (최상단 버전)
// ============================================================================
// 프로젝트 최상단에서 모든 파일 경로를 관리하는 중앙화된 매크로 시스템
// 사용법: #include "ProjectMap.h" (프로젝트 루트 기준)
// ============================================================================

// === 🏢 DOMAIN LAYER (비즈니스 로직, 엔티티, 값 객체) ===
#define DOMAIN_TYPES                    "include/domain/Types.h"
#define DOMAIN_ENTITIES_LOCATION        "include/domain/entities/Location.h"
#define DOMAIN_ENTITIES_ROUTE           "include/domain/entities/Route.h"
#define DOMAIN_ENTITIES_MAPVIEW         "include/domain/entities/MapView.h"
#define DOMAIN_SERVICES_ROUTE_PLANNER   "include/domain/services/IRoutePlanner.h"
#define DOMAIN_SERVICES_ROUTE_PROGRESS  "include/domain/services/RouteProgressCalculator.h"
#define DOMAIN_STATE_NAVIGATION         "include/domain/state/NavigationState.h"
#define DOMAIN_STATE_HUD_STORE          "include/domain/state/HudStateStore.h"
#define DOMAIN_STATE_ROUTE_PROGRESS     "include/domain/state/RouteProgressState.h"

// === 🏗️ INFRASTRUCTURE LAYER (외부 시스템, 렌더링, 영속성) ===
#define INFRASTRUCTURE_RENDER_PIPELINE      "include/infrastructure/rendering/RenderPipeline.h"
#define INFRASTRUCTURE_RENDER_METRICS       "include/infrastructure/rendering/RenderMetricsExporter.h"
#define INFRASTRUCTURE_EXTERNAL_TMAP        "include/infrastructure/external/TmapClient.h"
#define INFRASTRUCTURE_EXTERNAL_BRIDGE      "include/infrastructure/external/IsimBridge.h"
#define INFRASTRUCTURE_PERSISTENCE_CACHE    "include/infrastructure/persistence/CacheManager.h"

// === 🎨 PRESENTATION LAYER (UI 컴포넌트, 뷰, 컨트롤러) ===
// UI Components
#define PRESENTATION_BASE_COMPONENT         "include/presentation/components/BaseComponent.h"
#define PRESENTATION_CAMERA_CONTROLLER      "include/presentation/components/CameraController.h"
#define PRESENTATION_CAMERA_PRESET          "include/presentation/components/CameraPreset.h"
#define PRESENTATION_TRANSITION_ANIMATOR    "include/presentation/components/TransitionAnimator.h"
#define PRESENTATION_LOCATION_PUCK          "include/presentation/components/LocationPuck.h"
#define PRESENTATION_MAP_OVERLAY_HUD        "include/presentation/components/MapOverlayHud.h"
#define PRESENTATION_NAVIGATION_PROGRESS    "include/presentation/components/NavigationProgressBar.h"
#define PRESENTATION_TURN_BANNER            "include/presentation/components/TurnBanner.h"
#define PRESENTATION_WAYPOINT_LIST          "include/presentation/components/WaypointListPanel.h"
#define PRESENTATION_POLYLINE_HIGHLIGHT     "include/presentation/components/PolylineHighlight.h"
#define PRESENTATION_POLYLINE_STYLER        "include/presentation/components/PolylineStyler.h"
#define PRESENTATION_MAP_OVERLAY_THEME      "include/presentation/components/MapOverlayTheme.h"
#define PRESENTATION_UI_DISPATCHER          "include/presentation/components/UiDispatcher.h"
#define PRESENTATION_STATE_SUBSCRIBER       "include/presentation/components/StateSubscriberComponent.h"
#define PRESENTATION_VISUAL_FEEDBACK        "include/presentation/components/VisualFeedbackManager.h"
#define PRESENTATION_ENHANCED_PROGRESS      "include/presentation/components/EnhancedProgressBar.h"
#define PRESENTATION_ROUTE_PROGRESS_PIPELINE "include/presentation/components/RouteProgressPipeline.h"

// Views
#define PRESENTATION_APP_FRAME              "include/presentation/views/frames/AppFrame.h"
#define PRESENTATION_DEBUG_FRAME            "include/presentation/views/frames/DebugFrame.h"
#define PRESENTATION_MAP_PANEL              "include/presentation/views/panels/MapPanel.h"
#define PRESENTATION_CONTROL_PANEL          "include/presentation/views/panels/ControlPanel.h"

// Controllers
#define PRESENTATION_MAIN_CONTROLLER        "include/presentation/controllers/MainController.h"
#define PRESENTATION_MAP_CONTROLLER         "include/presentation/controllers/MapController.h"
#define PRESENTATION_NAVIGATION_CONTROLLER  "include/presentation/controllers/NavigationController.h"

// === � APPLICATION LAYER (애플리케이션 서비스, 유스케이스) ===
// Services
#define APPLICATION_HUD_STATE_STORE         "include/application/services/HudStateStore.h"
#define APPLICATION_ROUTE_PLANNER_SERVICE   "include/application/services/RoutePlannerService.h"
#define APPLICATION_NAVIGATION_SERVICE      "include/application/services/NavigationService.h"
#define APPLICATION_CAMERA_SERVICE          "include/application/services/CameraService.h"
#define APPLICATION_MAP_STYLE_SERVICE       "include/application/services/MapStyleService.h"
#define APPLICATION_WAYPOINT_SERVICE        "include/application/services/WaypointService.h"
#define APPLICATION_LOCATION_SERVICE        "include/application/services/LocationService.h"
#define APPLICATION_PROGRESS_CALCULATOR     "include/application/services/RouteProgressCalculator.h"
#define DOMAIN_SERVICES_ROUTE_PROGRESS_CALCULATOR "include/domain/services/RouteProgressCalculator.h"

// Use Cases
#define APPLICATION_NAVIGATION_USECASE      "include/application/useCases/NavigationUseCase.h"
#define APPLICATION_MAP_INTERACTION_USECASE "include/application/useCases/MapInteractionUseCase.h"
#define APPLICATION_ROUTE_PLANNING_USECASE  "include/application/useCases/RoutePlanningUseCase.h"

// ============================================================================
// 🚀 편의 매크로들 (자주 사용되는 조합)
// ============================================================================

// 모든 Domain 타입
#define INCLUDE_ALL_DOMAIN_TYPES \
    #include DOMAIN_TYPES

// 모든 UI 컴포넌트
#define INCLUDE_ALL_UI_COMPONENTS \
    #include PRESENTATION_CAMERA_CONTROLLER \
    #include PRESENTATION_LOCATION_PUCK \
    #include PRESENTATION_MAP_OVERLAY_HUD \
    #include PRESENTATION_NAVIGATION_PROGRESS \
    #include PRESENTATION_TURN_BANNER \
    #include PRESENTATION_WAYPOINT_LIST \
    #include PRESENTATION_POLYLINE_HIGHLIGHT \
    #include PRESENTATION_POLYLINE_STYLER \
    #include PRESENTATION_MAP_OVERLAY_THEME

// 모든 렌더링 관련
#define INCLUDE_ALL_RENDERING \
    #include INFRASTRUCTURE_RENDER_PIPELINE \
    #include INFRASTRUCTURE_RENDER_METRICS

// 모든 Views
#define INCLUDE_ALL_VIEWS \
    #include PRESENTATION_APP_FRAME \
    #include PRESENTATION_DEBUG_FRAME \
    #include PRESENTATION_MAP_PANEL \
    #include PRESENTATION_CONTROL_PANEL

// ============================================================================
// 📝 참고: namespace 별칭은 각 파일에서 필요시 직접 정의
// ============================================================================
// 예시: 
// namespace wx_map = presentation::components;
// namespace core = domain;
// namespace infra = infrastructure;