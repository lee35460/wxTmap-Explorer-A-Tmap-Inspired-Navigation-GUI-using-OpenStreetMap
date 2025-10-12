#pragma once
#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/textctrl.h>
#include <wx/listctrl.h>
#include <wx/splitter.h>
#include <wx/choice.h>
#include <wx/slider.h>
#include <wx/gauge.h>
#include <wx/timer.h>
#include <wx/checkbox.h>
#include <wx/clrpicker.h>
#include <wx/spinctrl.h>
#include <memory>
#include <vector>

// Forward declarations
struct LonLat;
class RenderPipeline;

// 디버그용 메인 프레임 - Modern C++ unique_ptr 적용
class DebugFrame : public wxFrame {
public:
    DebugFrame();
    virtual ~DebugFrame() = default; // unique_ptr이 자동 정리
    
    // 로그 함수 (다른 패널에서 접근 가능하도록 public)
    void AppendLog(const wxString& message);
    
private:
    // UI 컴포넌트들 (wxWidgets가 자동 관리)
    wxNotebook* notebook_;
    wxTextCtrl* logOutput_;
    wxSplitterWindow* splitter_;
    
    // 각 탭 패널들 (wxWidgets가 자동 관리)
    wxPanel* dataTestPanel_;
    wxPanel* apiTestPanel_;
    wxPanel* renderTestPanel_;
    
    // 비즈니스 로직 객체들 (unique_ptr로 안전한 메모리 관리) - 1단계용
    std::unique_ptr<std::vector<LonLat>> testCoordinates_;
    
    void CreateUI();
    void CreateDataTestPanel();
    void CreateApiTestPanel();
    void CreateRenderTestPanel();
    
    wxDECLARE_EVENT_TABLE();
};

// Forward declaration for CoordinateStats
struct CoordinateStats;

// 데이터 테스트 패널 - unique_ptr로 데이터 안전성 확보
class DataTestPanel : public wxPanel {
public:
    DataTestPanel(wxWindow* parent, DebugFrame* debugFrame);
    virtual ~DataTestPanel() = default;
    
private:
    DebugFrame* debugFrame_;
    
    // UI 컴포넌트들 (wxWidgets가 자동 관리)
    wxTextCtrl* lonInput_;
    wxTextCtrl* latInput_;
    wxButton* addCoordBtn_;
    wxListCtrl* coordList_;
    wxTextCtrl* jsonOutput_;
    
    // 추가 UI 컴포넌트들 (Step 1-2)
    wxStaticText* memoryStatus_;
    wxTextCtrl* statsOutput_;
    
    // 데이터 저장소 (unique_ptr로 안전한 메모리 관리)
    std::unique_ptr<std::vector<LonLat>> coordinates_;
    std::unique_ptr<CoordinateStats> stats_;
    
    // 이벤트 핸들러들
    void OnAddCoordinate(wxCommandEvent& event);
    void OnClearList(wxCommandEvent& event);
    void OnCalculateDistance(wxCommandEvent& event);
    void OnAddSampleData(wxCommandEvent& event);
    
    // UI 업데이트 메서드들
    void CreateUI();
    void UpdateDisplay();
    void UpdateStatsOutput();
    void UpdateJsonOutput();
    
    wxDECLARE_EVENT_TABLE();
};

// API 테스트용 개별 패널들
class TmapApiTestPanel : public wxPanel {
public:
    TmapApiTestPanel(wxWindow* parent, DebugFrame* debugFrame);
    virtual ~TmapApiTestPanel() = default;
    
private:
    DebugFrame* debugFrame_;
    wxTextCtrl* searchInput_;
    wxButton* searchBtn_;
    wxListCtrl* resultList_;
    wxTextCtrl* rawResponseOutput_;
    
    void OnSearchPoi(wxCommandEvent& event);
    void OnClearResults(wxCommandEvent& event);
    
    wxDECLARE_EVENT_TABLE();
};

class RoutePlannerTestPanel : public wxPanel {
public:
    RoutePlannerTestPanel(wxWindow* parent, DebugFrame* debugFrame);
    virtual ~RoutePlannerTestPanel() = default;
    
private:
    DebugFrame* debugFrame_;
    wxChoice* plannerChoice_;
    wxTextCtrl* startLonInput_;
    wxTextCtrl* startLatInput_;
    wxTextCtrl* endLonInput_;
    wxTextCtrl* endLatInput_;
    wxButton* calculateBtn_;
    wxListCtrl* routePointsList_;
    wxTextCtrl* metricsOutput_;
    
    // 경로 데이터 (unique_ptr로 관리)
    std::unique_ptr<std::vector<LonLat>> calculatedRoute_;
    
    void OnCalculateRoute(wxCommandEvent& event);
    void OnPlannerChanged(wxCommandEvent& event);
    
    wxDECLARE_EVENT_TABLE();
};

// WXT-4: 렌더링 성능 테스트 패널
class RenderPipelineTestPanel : public wxPanel {
public:
    RenderPipelineTestPanel(wxWindow* parent, DebugFrame* debugFrame);
    virtual ~RenderPipelineTestPanel();
    
private:
    DebugFrame* debugFrame_;
    wxButton* startRenderBtn_;
    wxButton* submitPolylineBtn_;
    wxSlider* pointCountSlider_;
    wxGauge* fpsGauge_;
    wxTextCtrl* metricsOutput_;
    
    // 타이머 (unique_ptr로 안전한 관리)
    std::unique_ptr<wxTimer> updateTimer_;
    
    // 렌더링 파이프라인 (unique_ptr로 관리)
    std::unique_ptr<RenderPipeline> renderPipeline_;
    
    void OnStartRender(wxCommandEvent& event);
    void OnSubmitPolyline(wxCommandEvent& event);
    void OnUpdateMetrics(wxTimerEvent& event);
    void OnPointCountChanged(wxCommandEvent& event);
    
    wxDECLARE_EVENT_TABLE();
};

// WXT-55: HUD 컴포넌트 테스트 패널
class HudComponentTestPanel : public wxPanel {
public:
    HudComponentTestPanel(wxWindow* parent, DebugFrame* debugFrame);
    virtual ~HudComponentTestPanel() = default;
    
private:
    DebugFrame* debugFrame_;
    wxSlider* speedSlider_;
    wxSlider* speedLimitSlider_;
    wxTextCtrl* distanceInput_;
    wxChoice* themeChoice_;
    wxCheckBox* visibleCheckbox_;
    wxPanel* hudPreview_;
    
    // HUD 상태 데이터 (unique_ptr로 관리)
    struct HudTestData;
    std::unique_ptr<HudTestData> hudData_;
    
    void OnSpeedChanged(wxCommandEvent& event);
    void OnThemeChanged(wxCommandEvent& event);
    void OnVisibilityChanged(wxCommandEvent& event);
    void UpdateHudPreview();
    
    wxDECLARE_EVENT_TABLE();
};

// WXT-56: Turn Banner 및 Progress Bar 테스트 패널
class TurnBannerTestPanel : public wxPanel {
public:
    TurnBannerTestPanel(wxWindow* parent, DebugFrame* debugFrame);
    virtual ~TurnBannerTestPanel();
    
private:
    DebugFrame* debugFrame_;
    wxChoice* turnDirectionChoice_;
    wxTextCtrl* distanceInput_;
    wxSlider* progressSlider_;
    wxButton* startAnimBtn_;
    wxButton* stopAnimBtn_;
    wxPanel* bannerPreview_;
    wxGauge* progressBar_;
    
    // 애니메이션 타이머 (unique_ptr로 안전한 관리)
    std::unique_ptr<wxTimer> animationTimer_;
    
    // Turn Banner 데이터
    struct TurnBannerData;
    std::unique_ptr<TurnBannerData> bannerData_;
    
    void OnTurnDirectionChanged(wxCommandEvent& event);
    void OnProgressChanged(wxCommandEvent& event);
    void OnStartAnimation(wxCommandEvent& event);
    void OnStopAnimation(wxCommandEvent& event);
    void OnAnimationTimer(wxTimerEvent& event);
    void UpdateBannerPreview();
    
    wxDECLARE_EVENT_TABLE();
};

// WXT-57: Polyline Highlight 테스트 패널  
class PolylineHighlightTestPanel : public wxPanel {
public:
    PolylineHighlightTestPanel(wxWindow* parent, DebugFrame* debugFrame);
    virtual ~PolylineHighlightTestPanel() = default;
    
private:
    DebugFrame* debugFrame_;
    wxButton* generateRouteBtn_;
    wxSlider* progressSlider_;
    wxChoice* highlightStyleChoice_;
    wxColourPickerCtrl* completedColorPicker_;
    wxColourPickerCtrl* remainingColorPicker_;
    wxSpinCtrl* lineWidthSpin_;
    wxPanel* polylinePreview_;
    
    // 경로 및 스타일 데이터 (unique_ptr로 관리)
    struct PolylineData;
    std::unique_ptr<PolylineData> polylineData_;
    
    void OnGenerateRoute(wxCommandEvent& event);
    void OnProgressChanged(wxCommandEvent& event);
    void OnStyleChanged(wxCommandEvent& event);
    void OnColorChanged(wxColourPickerEvent& event);
    void UpdatePolylinePreview();
    
    wxDECLARE_EVENT_TABLE();
};

// 이벤트 ID들
enum {
    // Data Test IDs (1000번대)
    ID_ADD_COORD = 1001,
    ID_CLEAR_LIST = 1002,
    ID_CALC_DISTANCE = 1003,
    ID_ADD_SAMPLE = 1004,
    
    // API Test IDs (2000번대)
    ID_SEARCH_POI = 2001,
    ID_CLEAR_RESULTS = 2002,
    ID_CALCULATE_ROUTE = 2003,
    ID_PLANNER_CHOICE = 2004,
    
    // Render Test IDs (3000번대)  
    ID_START_RENDER = 3001,
    ID_SUBMIT_POLYLINE = 3002,
    ID_UPDATE_TIMER = 3003,
    ID_POINT_COUNT_SLIDER = 3004,
    
    // HUD Test IDs (4000번대)
    ID_SPEED_SLIDER = 4001,
    ID_THEME_CHOICE = 4002,
    ID_VISIBILITY_CHECKBOX = 4003,
    
    // Turn Banner Test IDs (5000번대)
    ID_TURN_DIRECTION_CHOICE = 5001,
    ID_PROGRESS_SLIDER = 5002,
    ID_START_ANIM = 5003,
    ID_STOP_ANIM = 5004,
    ID_ANIM_TIMER = 5005,
    
    // Polyline Highlight Test IDs (6000번대)
    ID_GENERATE_ROUTE = 6001,
    ID_HIGHLIGHT_PROGRESS = 6002,
    ID_HIGHLIGHT_STYLE = 6003,
    ID_COMPLETED_COLOR = 6004,
    ID_REMAINING_COLOR = 6005
};