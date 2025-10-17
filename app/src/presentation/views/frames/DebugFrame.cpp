#include "DebugFrame.h"
#include "../../../domain/Types.h"  // 4계층 구조: domain의 Types.h
#include <wx/sizer.h>
#include <wx/splitter.h>     // wxSplitterWindow 사용
#include <wx/msgdlg.h>       // wxMessageBox 사용
#include <sstream>
#include <iomanip>
#include <algorithm>         // std::min, std::max, std::count_if 사용
#include <cmath>             // M_PI, std::sin, std::cos 등 수학 함수
#include <memory>            // unique_ptr 사용

// 4계층 구조: presentation 컴포넌트들
#include "../../components/MapOverlayHud.h"
#include "../../components/NavigationProgressBar.h"
#include "../../components/WaypointListPanel.h"
#include "../../components/TurnBanner.h"
#include "../../components/CameraController.h"
#include "../../components/LocationPuck.h"
#include "../../components/PolylineHighlight.h"
// 4계층 구조: infrastructure의 렌더링 파이프라인
#include "../../../infrastructure/rendering/RenderPipeline.h"

// 수학 상수 정의 (가독성 및 유지보수성 향상)
constexpr double DEG_TO_RAD = M_PI / 180.0;

// DataTestPanel 구현 - 기본 개념 학습에 집중
wxBEGIN_EVENT_TABLE(DataTestPanel, wxPanel)
    EVT_BUTTON(ID_ADD_COORD, DataTestPanel::OnAddCoordinate)
    EVT_BUTTON(ID_CLEAR_LIST, DataTestPanel::OnClearList)
    EVT_BUTTON(ID_CALC_DISTANCE, DataTestPanel::OnCalculateDistance)
    EVT_BUTTON(ID_ADD_SAMPLE, DataTestPanel::OnAddSampleData)
wxEND_EVENT_TABLE()

DataTestPanel::DataTestPanel(wxWindow* parent, DebugFrame* debugFrame)
    : wxPanel(parent)
    , debugFrame_(debugFrame)
    , coordinates_(std::make_unique<std::vector<LonLat>>()) // unique_ptr 학습
    , stats_(std::make_unique<CoordinateStats>())           // 통계 객체도 unique_ptr로
{
    CreateUI();
    UpdateDisplay();
    
    // 학습용 초기 메시지
    debugFrame_->AppendLog("=== 1단계: 데이터 구조 학습 시작 ===");
    debugFrame_->AppendLog("📚 학습 목표: LonLat, unique_ptr, 입력검증, JSON 변환");
}

void DataTestPanel::CreateUI() {
    auto* mainSizer = new wxBoxSizer(wxVERTICAL);
    
    // === 제목 및 설명 ===
    auto* title = new wxStaticText(this, wxID_ANY, "📍 1단계: 기본 데이터 구조 학습");
    title->SetFont(wxFont(16, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    title->SetForegroundColour(wxColour(0, 100, 200));
    mainSizer->Add(title, 0, wxALL, 10);
    
    auto* desc = new wxStaticText(this, wxID_ANY, 
        "🎯 LonLat 구조체, unique_ptr 메모리 관리, 입력 검증을 체험해보세요");
    desc->SetForegroundColour(wxColour(80, 80, 80));
    mainSizer->Add(desc, 0, wxALL | wxEXPAND, 10);
    
    // === 좌표 입력 섹션 ===
    auto* inputBox = new wxStaticBoxSizer(wxVERTICAL, this, "좌표 입력 실습");
    
    // 입력 필드들
    auto* inputRow = new wxBoxSizer(wxHORIZONTAL);
    
    inputRow->Add(new wxStaticText(this, wxID_ANY, "경도:"), 0, 
                  wxALIGN_CENTER_VERTICAL | wxALL, 5);
    lonInput_ = new wxTextCtrl(this, wxID_ANY, "127.0276", wxDefaultPosition, wxSize(120, -1));
    lonInput_->SetToolTip("경도 입력 (-180.0 ~ 180.0)\n예: 서울 127.0276");
    inputRow->Add(lonInput_, 0, wxALL, 5);
    
    inputRow->Add(new wxStaticText(this, wxID_ANY, "위도:"), 0, 
                  wxALIGN_CENTER_VERTICAL | wxALL, 5);
    latInput_ = new wxTextCtrl(this, wxID_ANY, "37.4979", wxDefaultPosition, wxSize(120, -1));
    latInput_->SetToolTip("위도 입력 (-90.0 ~ 90.0)\n예: 서울 37.4979");
    inputRow->Add(latInput_, 0, wxALL, 5);
    
    addCoordBtn_ = new wxButton(this, ID_ADD_COORD, "좌표 추가");
    addCoordBtn_->SetToolTip("입력한 좌표를 벡터에 추가");
    inputRow->Add(addCoordBtn_, 0, wxALL, 5);
    
    inputBox->Add(inputRow, 0, wxEXPAND | wxALL, 5);
    
    // 편의 버튼들
    auto* buttonRow = new wxBoxSizer(wxHORIZONTAL);
    
    auto* sampleBtn = new wxButton(this, ID_ADD_SAMPLE, "샘플 데이터 추가");
    sampleBtn->SetToolTip("서울, 부산, 대구 좌표 자동 추가");
    buttonRow->Add(sampleBtn, 0, wxALL, 5);
    
    auto* calcBtn = new wxButton(this, ID_CALC_DISTANCE, "거리 계산");
    calcBtn->SetToolTip("연속된 좌표간 거리 계산 (하버사인 공식)");
    buttonRow->Add(calcBtn, 0, wxALL, 5);
    
    auto* clearBtn = new wxButton(this, ID_CLEAR_LIST, "전체 초기화");
    clearBtn->SetToolTip("모든 데이터 삭제");
    buttonRow->Add(clearBtn, 0, wxALL, 5);
    
    inputBox->Add(buttonRow, 0, wxEXPAND | wxALL, 5);
    mainSizer->Add(inputBox, 0, wxEXPAND | wxALL, 10);
    
    // === unique_ptr 상태 표시 ===
    auto* memoryBox = new wxStaticBoxSizer(wxVERTICAL, this, "메모리 관리 상태 (unique_ptr)");
    
    memoryStatus_ = new wxStaticText(this, wxID_ANY, "");
    memoryStatus_->SetFont(wxFont(9, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    memoryStatus_->SetForegroundColour(wxColour(0, 128, 0));
    memoryBox->Add(memoryStatus_, 0, wxEXPAND | wxALL, 5);
    
    mainSizer->Add(memoryBox, 0, wxEXPAND | wxALL, 10);
    
    // === 좌표 목록 ===
    auto* listBox = new wxStaticBoxSizer(wxVERTICAL, this, "저장된 좌표 목록");
    coordList_ = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(-1, 180),
                               wxLC_REPORT | wxLC_SINGLE_SEL);
    coordList_->AppendColumn("번호", wxLIST_FORMAT_LEFT, 50);
    coordList_->AppendColumn("경도", wxLIST_FORMAT_LEFT, 120);
    coordList_->AppendColumn("위도", wxLIST_FORMAT_LEFT, 120);
    coordList_->AppendColumn("유효성", wxLIST_FORMAT_LEFT, 70);
    coordList_->AppendColumn("이전 좌표와 거리", wxLIST_FORMAT_LEFT, 120);
    
    listBox->Add(coordList_, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(listBox, 1, wxEXPAND | wxALL, 10);
    
    // === 통계 및 JSON 출력 ===
    auto* splitter = new wxSplitterWindow(this);
    
    // 통계 패널
    auto* statsPanel = new wxPanel(splitter);
    auto* statsBox = new wxStaticBoxSizer(wxVERTICAL, statsPanel, "실시간 통계");
    statsOutput_ = new wxTextCtrl(statsPanel, wxID_ANY, "", 
                                 wxDefaultPosition, wxDefaultSize,
                                 wxTE_MULTILINE | wxTE_READONLY);
    statsOutput_->SetBackgroundColour(wxColour(245, 250, 255));
    statsBox->Add(statsOutput_, 1, wxEXPAND | wxALL, 5);
    statsPanel->SetSizer(statsBox);
    
    // JSON 패널
    auto* jsonPanel = new wxPanel(splitter);
    auto* jsonBox = new wxStaticBoxSizer(wxVERTICAL, jsonPanel, "JSON 출력");
    jsonOutput_ = new wxTextCtrl(jsonPanel, wxID_ANY, "", 
                                wxDefaultPosition, wxDefaultSize,
                                wxTE_MULTILINE | wxTE_READONLY);
    jsonOutput_->SetBackgroundColour(wxColour(250, 250, 250));
    jsonOutput_->SetFont(wxFont(9, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    jsonBox->Add(jsonOutput_, 1, wxEXPAND | wxALL, 5);
    jsonPanel->SetSizer(jsonBox);
    
    splitter->SplitVertically(statsPanel, jsonPanel);
    splitter->SetSashGravity(0.4);
    splitter->SetMinimumPaneSize(200);
    
    mainSizer->Add(splitter, 1, wxEXPAND | wxALL, 10);
    
    SetSizer(mainSizer);
}

void DataTestPanel::OnAddCoordinate(wxCommandEvent& event) {
    wxString lonStr = lonInput_->GetValue().Trim();
    wxString latStr = latInput_->GetValue().Trim();
    
    // 1단계: 입력값 검증 학습
    if (lonStr.IsEmpty() || latStr.IsEmpty()) {
        debugFrame_->AppendLog("❌ 입력 검증 실패: 빈 값");
        wxMessageBox("경도와 위도를 모두 입력하세요", "입력 확인", wxOK | wxICON_WARNING);
        return;
    }
    
    double lon, lat;
    if (!lonStr.ToDouble(&lon) || !latStr.ToDouble(&lat)) {
        debugFrame_->AppendLog("❌ 타입 변환 실패: 숫자가 아닌 값");
        wxMessageBox("올바른 숫자를 입력하세요", "타입 오류", wxOK | wxICON_ERROR);
        return;
    }
    
    // 2단계: LonLat 구조체 생성 및 검증
    LonLat coord(lon, lat);
    if (!coord.IsValid()) {
        debugFrame_->AppendLog(wxString::Format("❌ 범위 검증 실패: (%.6f, %.6f)", lon, lat));
        wxMessageBox("좌표 범위를 벗어났습니다\n경도: -180~180\n위도: -90~90", 
                    "범위 오류", wxOK | wxICON_ERROR);
        return;
    }
    
    // 3단계: unique_ptr 벡터에 추가
    size_t oldSize = coordinates_->size();
    coordinates_->push_back(coord);
    stats_->AddCoordinate(coord);
    
    // 4단계: 로그 및 화면 업데이트
    debugFrame_->AppendLog(wxString::Format("✅ 좌표 추가: %s (벡터 크기: %zu → %zu)", 
                                           coord.ToString(), oldSize, coordinates_->size()));
    
    UpdateDisplay();
    
    // 다음 입력을 위한 필드 정리
    lonInput_->Clear();
    latInput_->Clear();
    lonInput_->SetFocus();
}

void DataTestPanel::OnAddSampleData(wxCommandEvent& event) {
    // 한국 주요 도시 좌표 (학습용)
    std::vector<std::pair<LonLat, std::string>> samples = {
        {{127.0276, 37.4979}, "서울 (강남역)"},
        {{129.0756, 35.1795}, "부산 (부산역)"},  
        {{128.5911, 35.8714}, "대구 (대구역)"},
        {{127.3845, 36.3504}, "대전 (대전역)"},
        {{126.9784, 37.5665}, "서울 (시청)"}
    };
    
    size_t added = 0;
    for (const auto& [coord, name] : samples) {
        coordinates_->push_back(coord);
        stats_->AddCoordinate(coord);
        added++;
    }
    
    debugFrame_->AppendLog(wxString::Format("✅ 샘플 데이터 %zu개 추가 완료", added));
    UpdateDisplay();
}

void DataTestPanel::OnCalculateDistance(wxCommandEvent& event) {
    if (coordinates_->size() < 2) {
        debugFrame_->AppendLog("❌ 거리 계산: 최소 2개 좌표 필요");
        wxMessageBox("거리 계산을 위해 최소 2개의 좌표가 필요합니다", "데이터 부족", wxOK | wxICON_INFORMATION);
        return;
    }
    
    double totalDistance = 0.0;
    for (size_t i = 1; i < coordinates_->size(); ++i) {
        double dist = (*coordinates_)[i-1].DistanceTo((*coordinates_)[i]);
        totalDistance += dist;
    }
    
    stats_->total_distance = totalDistance;
    
    debugFrame_->AppendLog(wxString::Format("📏 총 거리 계산 완료: %.2f km", totalDistance / 1000.0));
    UpdateDisplay();
}

void DataTestPanel::OnClearList(wxCommandEvent& event) {
    size_t oldSize = coordinates_->size();
    
    // unique_ptr 객체들 초기화
    coordinates_->clear();
    stats_ = std::make_unique<CoordinateStats>(); // 새 인스턴스 생성
    
    debugFrame_->AppendLog(wxString::Format("🗑️ 전체 초기화: %zu개 좌표 삭제", oldSize));
    UpdateDisplay();
}

void DataTestPanel::UpdateDisplay() {
    // 1. 메모리 상태 업데이트
    wxString memoryInfo = wxString::Format(
        "Vector 주소: %p | 크기: %zu | 용량: %zu | 메모리 사용량: ~%zu bytes",
        static_cast<void*>(coordinates_.get()),
        coordinates_->size(),
        coordinates_->capacity(),
        coordinates_->capacity() * sizeof(LonLat)
    );
    memoryStatus_->SetLabel(memoryInfo);
    
    // 2. 리스트 업데이트
    coordList_->DeleteAllItems();
    for (size_t i = 0; i < coordinates_->size(); ++i) {
        const auto& coord = (*coordinates_)[i];
        
        long index = coordList_->InsertItem(i, wxString::Format("%zu", i + 1));
        coordList_->SetItem(index, 1, wxString::Format("%.6f", coord.lon));
        coordList_->SetItem(index, 2, wxString::Format("%.6f", coord.lat));
        coordList_->SetItem(index, 3, coord.IsValid() ? "✅" : "❌");
        
        if (i > 0) {
            double dist = (*coordinates_)[i-1].DistanceTo(coord);
            coordList_->SetItem(index, 4, wxString::Format("%.0f m", dist));
        } else {
            coordList_->SetItem(index, 4, "-");
        }
    }
    
    // 3. 통계 업데이트
    UpdateStatsOutput();
    
    // 4. JSON 업데이트
    UpdateJsonOutput();
}

void DataTestPanel::UpdateStatsOutput() {
    std::ostringstream stats;
    stats << "=== 좌표 통계 ===\n";
    stats << "총 개수: " << stats_->total_count << "\n";
    stats << "유효 개수: " << stats_->valid_count << "\n";
    stats << "유효율: " << std::fixed << std::setprecision(1) << (stats_->GetValidRate() * 100) << "%\n\n";
    
    if (stats_->valid_count > 0) {
        stats << "=== 범위 정보 ===\n";
        stats << "경도 범위: " << std::fixed << std::setprecision(6) 
              << stats_->min_lon << " ~ " << stats_->max_lon << "\n";
        stats << "위도 범위: " << std::fixed << std::setprecision(6)
              << stats_->min_lat << " ~ " << stats_->max_lat << "\n\n";
    }
    
    if (stats_->total_distance > 0) {
        stats << "=== 거리 정보 ===\n";
        stats << "총 거리: " << std::fixed << std::setprecision(2) 
              << (stats_->total_distance / 1000.0) << " km\n";
        stats << "평균 구간: " << std::fixed << std::setprecision(0)
              << (stats_->total_distance / std::max(1.0, double(coordinates_->size() - 1))) << " m\n";
    }
    
    statsOutput_->SetValue(stats.str());
}

void DataTestPanel::UpdateJsonOutput() {
    std::ostringstream json;
    json << "{\n";
    json << "  \"meta\": {\n";
    json << "    \"type\": \"std::unique_ptr<std::vector<LonLat>>\",\n";
    json << "    \"size\": " << coordinates_->size() << ",\n";
    json << "    \"capacity\": " << coordinates_->capacity() << ",\n";
    json << "    \"memory_address\": \"" << static_cast<void*>(coordinates_.get()) << "\"\n";
    json << "  },\n";
    json << "  \"statistics\": {\n";
    json << "    \"total_count\": " << stats_->total_count << ",\n";
    json << "    \"valid_count\": " << stats_->valid_count << ",\n";
    json << "    \"valid_rate\": " << std::fixed << std::setprecision(3) << stats_->GetValidRate() << ",\n";
    json << "    \"total_distance_km\": " << std::fixed << std::setprecision(2) << (stats_->total_distance / 1000.0) << "\n";
    json << "  },\n";
    json << "  \"coordinates\": [\n";
    
    for (size_t i = 0; i < coordinates_->size(); ++i) {
        const auto& coord = (*coordinates_)[i];
        json << "    {\n";
        json << "      \"index\": " << i << ",\n";
        json << "      \"lon\": " << std::fixed << std::setprecision(6) << coord.lon << ",\n";
        json << "      \"lat\": " << std::fixed << std::setprecision(6) << coord.lat << ",\n";
        json << "      \"valid\": " << (coord.IsValid() ? "true" : "false") << ",\n";
        json << "      \"toString\": \"" << coord.ToString() << "\"";
        
        if (i > 0) {
            double dist = (*coordinates_)[i-1].DistanceTo(coord);
            json << ",\n      \"distance_from_prev_m\": " << std::fixed << std::setprecision(1) << dist;
        }
        
        json << "\n    }";
        if (i < coordinates_->size() - 1) json << ",";
        json << "\n";
    }
    
    json << "  ]\n";
    json << "}";
    
    jsonOutput_->SetValue(json.str());
}

// DebugFrame 구현
wxBEGIN_EVENT_TABLE(DebugFrame, wxFrame)
wxEND_EVENT_TABLE()

DebugFrame::DebugFrame() 
    : wxFrame(nullptr, wxID_ANY, "wxTmap Debug Console", 
              wxDefaultPosition, wxSize(1200, 800))
    , testCoordinates_(std::make_unique<std::vector<LonLat>>()) // unique_ptr 초기화
{
    CreateUI();
    AppendLog("=== wxTmap Debug Console Started ===");
    AppendLog("🎯 단계적 학습으로 Modern C++ 및 wxWidgets 마스터하기");
}

void DebugFrame::CreateUI() {
    // 메인 스플리터 (상하 분할)
    splitter_ = new wxSplitterWindow(this);
    
    // 상단: 탭 노트북
    notebook_ = new wxNotebook(splitter_, wxID_ANY);
    
    // 하단: 로그 출력창
    logOutput_ = new wxTextCtrl(splitter_, wxID_ANY, "", 
                               wxDefaultPosition, wxDefaultSize,
                               wxTE_MULTILINE | wxTE_READONLY | wxTE_WORDWRAP);
    logOutput_->SetBackgroundColour(wxColour(30, 30, 30));
    logOutput_->SetForegroundColour(wxColour(200, 255, 200));
    logOutput_->SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    
    // 각 테스트 패널 생성
    CreateDataTestPanel();
    CreateApiTestPanel();
    CreateRenderTestPanel();
    
    // 스플리터 설정 (75% 위, 25% 아래)
    splitter_->SplitHorizontally(notebook_, logOutput_);
    splitter_->SetSashGravity(0.75);
    splitter_->SetMinimumPaneSize(150);
    
    // 레이아웃
    auto* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(splitter_, 1, wxEXPAND);
    SetSizer(sizer);
}

void DebugFrame::CreateDataTestPanel() {
    dataTestPanel_ = new DataTestPanel(notebook_, this);
    notebook_->AddPage(dataTestPanel_, "📍 1단계: Data Test", true);
}

void DebugFrame::CreateApiTestPanel() {
    apiTestPanel_ = new wxPanel(notebook_);
    
    // ========================================
    // 🔄 변경점: 단순한 레이블에서 → 실제 기능적인 두 개의 서브패널로 분할
    // - 상단: TmapApiTestPanel (POI 검색 Mock)  
    // - 하단: RoutePlannerTestPanel (경로 생성 알고리즘)
    // ========================================
    
    // 수직 분할을 위한 스플리터 생성
    auto* apiSplitter = new wxSplitterWindow(apiTestPanel_);
    
    // 상단: TmapClient API 테스트 (POI 검색 Mock)
    auto* tmapApiPanel = new TmapApiTestPanel(apiSplitter, this);
    
    // 하단: 경로 계획 알고리즘 테스트 (학습용 Mock 알고리즘들)
    auto* routePlannerPanel = new RoutePlannerTestPanel(apiSplitter, this);
    
    // 스플리터 설정 (50:50 비율로 분할)
    apiSplitter->SplitHorizontally(tmapApiPanel, routePlannerPanel);
    apiSplitter->SetSashGravity(0.5);  // 상하 균등 분할
    apiSplitter->SetMinimumPaneSize(200);  // 최소 200px 확보
    
    // 레이아웃 설정
    auto* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(apiSplitter, 1, wxEXPAND);  // 전체 공간 사용
    apiTestPanel_->SetSizer(sizer);
    
    notebook_->AddPage(apiTestPanel_, "🔍 2단계: API Test");
}

void DebugFrame::CreateRenderTestPanel() {
    // ========================================
    // 🆕 3단계: 실제 지도 렌더링 패널 생성
    // 기존 placeholder를 MapRenderPanel로 교체
    // ========================================
    
    renderTestPanel_ = new MapRenderPanel(notebook_, this);
    notebook_->AddPage(renderTestPanel_, "🗺️ 3단계: Map Render");
    
    AppendLog("🗺️ MapRenderPanel 생성 완료 - OpenStreetMap 타일 렌더링 준비");
}

void DebugFrame::AppendLog(const wxString& message) {
    if (logOutput_) {
        wxDateTime now = wxDateTime::Now();
        wxString timestamped = wxString::Format("[%s] %s\n", 
                                               now.Format("%H:%M:%S"), 
                                               message);
        logOutput_->AppendText(timestamped);
    }
}

// ========================================
// 🆕 추가: 2단계 API Test 패널 구현들
// ========================================

// TmapApiTestPanel 구현 - POI 검색 Mock
wxBEGIN_EVENT_TABLE(TmapApiTestPanel, wxPanel)
    EVT_BUTTON(ID_SEARCH_POI, TmapApiTestPanel::OnSearchPoi)
    EVT_BUTTON(ID_CLEAR_RESULTS, TmapApiTestPanel::OnClearResults)
wxEND_EVENT_TABLE()

TmapApiTestPanel::TmapApiTestPanel(wxWindow* parent, DebugFrame* debugFrame)
    : wxPanel(parent), debugFrame_(debugFrame) {
    
    auto* mainSizer = new wxBoxSizer(wxVERTICAL);
    
    // 제목
    auto* title = new wxStaticText(this, wxID_ANY, "🔍 2단계-A: Tmap API 테스트 (Mock)");
    title->SetFont(wxFont(14, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    title->SetForegroundColour(wxColour(200, 100, 0));
    mainSizer->Add(title, 0, wxALL, 10);
    
    // 설명
    auto* desc = new wxStaticText(this, wxID_ANY, 
        "🎯 POI 검색 API 동작 방식을 Mock 데이터로 학습합니다");
    desc->SetForegroundColour(wxColour(80, 80, 80));
    mainSizer->Add(desc, 0, wxALL | wxEXPAND, 10);
    
    // 검색 입력부
    auto* searchBox = new wxStaticBoxSizer(wxHORIZONTAL, this, "POI 검색 (Mock API)");
    
    searchBox->Add(new wxStaticText(this, wxID_ANY, "검색어:"), 0, 
                   wxALIGN_CENTER_VERTICAL | wxALL, 5);
    searchInput_ = new wxTextCtrl(this, wxID_ANY, "강남역", wxDefaultPosition, wxSize(200, -1));
    searchInput_->SetToolTip("검색할 장소명 입력 (Mock: 강남역, 부산역, 대구역 등)");
    searchBox->Add(searchInput_, 1, wxALL, 5);
    
    searchBtn_ = new wxButton(this, ID_SEARCH_POI, "🔍 검색 실행");
    searchBtn_->SetToolTip("Mock API로 POI 검색 시뮬레이션");
    searchBox->Add(searchBtn_, 0, wxALL, 5);
    
    auto* clearBtn = new wxButton(this, ID_CLEAR_RESULTS, "🗑️ 결과 초기화");
    searchBox->Add(clearBtn, 0, wxALL, 5);
    
    mainSizer->Add(searchBox, 0, wxEXPAND | wxALL, 10);
    
    // 검색 결과 리스트
    auto* resultBox = new wxStaticBoxSizer(wxVERTICAL, this, "검색 결과");
    resultList_ = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(-1, 120),
                                wxLC_REPORT | wxLC_SINGLE_SEL);
    resultList_->AppendColumn("이름", wxLIST_FORMAT_LEFT, 120);
    resultList_->AppendColumn("주소", wxLIST_FORMAT_LEFT, 180);
    resultList_->AppendColumn("경도", wxLIST_FORMAT_LEFT, 100);
    resultList_->AppendColumn("위도", wxLIST_FORMAT_LEFT, 100);
    
    resultBox->Add(resultList_, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(resultBox, 1, wxEXPAND | wxALL, 10);
    
    // Raw API 응답 (학습용)
    auto* responseBox = new wxStaticBoxSizer(wxVERTICAL, this, "Mock API 응답 JSON");
    rawResponseOutput_ = new wxTextCtrl(this, wxID_ANY, "", 
                                       wxDefaultPosition, wxSize(-1, 80),
                                       wxTE_MULTILINE | wxTE_READONLY);
    rawResponseOutput_->SetBackgroundColour(wxColour(40, 40, 40));
    rawResponseOutput_->SetForegroundColour(wxColour(200, 255, 200));
    rawResponseOutput_->SetFont(wxFont(9, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    
    responseBox->Add(rawResponseOutput_, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(responseBox, 0, wxEXPAND | wxALL, 10);
    
    SetSizer(mainSizer);
    
    // 초기 메시지
    debugFrame_->AppendLog("🔍 TmapApiTestPanel 초기화 완료 - Mock API 준비됨");
}

void TmapApiTestPanel::OnSearchPoi(wxCommandEvent& event) {
    wxString query = searchInput_->GetValue().Trim();
    
    if (query.IsEmpty()) {
        debugFrame_->AppendLog("❌ 검색어를 입력하세요");
        return;
    }
    
    debugFrame_->AppendLog(wxString::Format("🔍 POI 검색 시작: '%s' (Mock API 호출)", query));
    
    // ========================================
    // 🎯 학습 포인트: Mock API 응답 생성
    // 실제로는 HTTP 요청 → JSON 응답이지만, 
    // 학습용으로 로컬에서 시뮬레이션
    // ========================================
    
    // Mock 데이터베이스 (한국 주요 장소)
    std::vector<std::tuple<std::string, std::string, double, double>> mockDB = {
        {"강남역", "서울특별시 강남구 강남대로 396", 127.0276, 37.4979},
        {"부산역", "부산광역시 동구 중앙대로 206", 129.0756, 35.1795},
        {"대구역", "대구광역시 동구 동대구로 550", 128.5911, 35.8714},
        {"대전역", "대전광역시 동구 중앙로 215", 127.3845, 36.3504},
        {"광주역", "광주광역시 동구 경양로 264", 126.8895, 35.1595},
        {"인천공항", "인천광역시 중구 공항로 272", 126.4406, 37.4691},
        {"서울역", "서울특별시 중구 한강대로 405", 126.9700, 37.5547}
    };
    
    // 검색어와 매칭되는 결과 찾기 (부분 문자열 매칭)
    std::vector<std::tuple<std::string, std::string, double, double>> results;
    for (const auto& [name, address, lon, lat] : mockDB) {
        if (name.find(query.ToStdString()) != std::string::npos) {
            results.push_back({name, address, lon, lat});
        }
    }
    
    // 결과가 없으면 첫 번째 항목을 기본값으로 (학습용)
    if (results.empty()) {
        results.push_back(mockDB[0]);  // 강남역을 기본값으로
        debugFrame_->AppendLog("⚠️ 검색 결과 없음 - 기본값(강남역) 반환");
    }
    
    // 검색 결과를 리스트에 표시
    resultList_->DeleteAllItems();
    for (size_t i = 0; i < results.size(); ++i) {
        const auto& [name, address, lon, lat] = results[i];
        
        long index = resultList_->InsertItem(i, name);
        resultList_->SetItem(index, 1, address);
        resultList_->SetItem(index, 2, wxString::Format("%.6f", lon));
        resultList_->SetItem(index, 3, wxString::Format("%.6f", lat));
    }
    
    // Mock API 응답 JSON 생성 (실제 Tmap API 형식과 유사하게)
    std::ostringstream json;
    json << "{\n";
    json << "  \"searchPoiInfo\": {\n";
    json << "    \"totalCount\": \"" << results.size() << "\",\n";
    json << "    \"count\": \"" << results.size() << "\",\n";
    json << "    \"page\": \"1\",\n";
    json << "    \"pois\": {\n";
    json << "      \"poi\": [\n";
    
    for (size_t i = 0; i < results.size(); ++i) {
        const auto& [name, address, lon, lat] = results[i];
        json << "        {\n";
        json << "          \"name\": \"" << name << "\",\n";
        json << "          \"fullAddress\": \"" << address << "\",\n";
        json << "          \"frontLat\": \"" << std::fixed << std::setprecision(6) << lat << "\",\n";
        json << "          \"frontLon\": \"" << std::fixed << std::setprecision(6) << lon << "\"\n";
        json << "        }";
        if (i < results.size() - 1) json << ",";
        json << "\n";
    }
    
    json << "      ]\n";
    json << "    }\n";
    json << "  }\n";
    json << "}";
    
    rawResponseOutput_->SetValue(json.str());
    
    debugFrame_->AppendLog(wxString::Format("✅ POI 검색 완료: %zu개 결과 (Mock)", results.size()));
}

void TmapApiTestPanel::OnClearResults(wxCommandEvent& event) {
    resultList_->DeleteAllItems();
    rawResponseOutput_->Clear();
    debugFrame_->AppendLog("🗑️ 검색 결과 초기화됨");
}

// ========================================
// 🆕 추가: 경로 계획 패널 구현
// ========================================

// RoutePlannerTestPanel 구현 - 다양한 경로 알고리즘 Mock
wxBEGIN_EVENT_TABLE(RoutePlannerTestPanel, wxPanel)
    EVT_BUTTON(ID_PLAN_ROUTE, RoutePlannerTestPanel::OnPlanRoute)
    EVT_BUTTON(ID_CLEAR_ROUTE, RoutePlannerTestPanel::OnClearRoute)
wxEND_EVENT_TABLE()

RoutePlannerTestPanel::RoutePlannerTestPanel(wxWindow* parent, DebugFrame* debugFrame)
    : wxPanel(parent), debugFrame_(debugFrame) {
    
    auto* mainSizer = new wxBoxSizer(wxVERTICAL);
    
    // 제목
    auto* title = new wxStaticText(this, wxID_ANY, "🗺️ 2단계-B: 경로 계획 테스트 (Mock)");
    title->SetFont(wxFont(14, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    title->SetForegroundColour(wxColour(0, 150, 200));
    mainSizer->Add(title, 0, wxALL, 10);
    
    // 설명
    auto* desc = new wxStaticText(this, wxID_ANY, 
        "🎯 다양한 경로 계획 알고리즘을 Mock 데이터로 학습합니다");
    desc->SetForegroundColour(wxColour(80, 80, 80));
    mainSizer->Add(desc, 0, wxALL | wxEXPAND, 10);
    
    // 경로 설정부
    auto* routeBox = new wxStaticBoxSizer(wxVERTICAL, this, "경로 설정");
    
    // 출발지/목적지 입력
    auto* coordSizer = new wxFlexGridSizer(2, 4, 5, 10);
    coordSizer->AddGrowableCol(1, 1);
    coordSizer->AddGrowableCol(3, 1);
    
    coordSizer->Add(new wxStaticText(this, wxID_ANY, "출발지 경도:"), 0, wxALIGN_CENTER_VERTICAL);
    startLonInput_ = new wxTextCtrl(this, wxID_ANY, "127.0276");  // 강남역
    coordSizer->Add(startLonInput_, 1, wxEXPAND);
    
    coordSizer->Add(new wxStaticText(this, wxID_ANY, "출발지 위도:"), 0, wxALIGN_CENTER_VERTICAL);
    startLatInput_ = new wxTextCtrl(this, wxID_ANY, "37.4979");
    coordSizer->Add(startLatInput_, 1, wxEXPAND);
    
    coordSizer->Add(new wxStaticText(this, wxID_ANY, "목적지 경도:"), 0, wxALIGN_CENTER_VERTICAL);
    endLonInput_ = new wxTextCtrl(this, wxID_ANY, "126.9700");    // 서울역
    coordSizer->Add(endLonInput_, 1, wxEXPAND);
    
    coordSizer->Add(new wxStaticText(this, wxID_ANY, "목적지 위도:"), 0, wxALIGN_CENTER_VERTICAL);
    endLatInput_ = new wxTextCtrl(this, wxID_ANY, "37.5547");
    coordSizer->Add(endLatInput_, 1, wxEXPAND);
    
    routeBox->Add(coordSizer, 0, wxEXPAND | wxALL, 5);
    
    // 알고리즘 선택
    auto* algoSizer = new wxBoxSizer(wxHORIZONTAL);
    algoSizer->Add(new wxStaticText(this, wxID_ANY, "경로 알고리즘:"), 0, 
                   wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
    
    wxArrayString algorithms;
    algorithms.Add("직선 경로 (Direct)");
    algorithms.Add("지그재그 경로 (Zigzag)");
    algorithms.Add("곡선 경로 (Curved)");
    
    algorithmChoice_ = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, algorithms);
    algorithmChoice_->SetSelection(0);
    algorithmChoice_->SetToolTip("학습용 Mock 알고리즘 선택");
    algoSizer->Add(algorithmChoice_, 1, wxEXPAND);
    
    routeBox->Add(algoSizer, 0, wxEXPAND | wxALL, 5);
    
    // 버튼들
    auto* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    auto* planBtn = new wxButton(this, ID_PLAN_ROUTE, "🗺️ 경로 계획 실행");
    planBtn->SetToolTip("Mock 알고리즘으로 경로 생성");
    btnSizer->Add(planBtn, 0, wxRIGHT, 5);
    
    auto* clearBtn = new wxButton(this, ID_CLEAR_ROUTE, "🗑️ 경로 초기화");
    btnSizer->Add(clearBtn, 0);
    
    routeBox->Add(btnSizer, 0, wxALL, 5);
    mainSizer->Add(routeBox, 0, wxEXPAND | wxALL, 10);
    
    // 경로 결과 표시
    auto* resultBox = new wxStaticBoxSizer(wxVERTICAL, this, "생성된 경로");
    routeOutput_ = new wxTextCtrl(this, wxID_ANY, "", 
                                 wxDefaultPosition, wxSize(-1, 120),
                                 wxTE_MULTILINE | wxTE_READONLY);
    routeOutput_->SetBackgroundColour(wxColour(40, 40, 50));
    routeOutput_->SetForegroundColour(wxColour(200, 200, 255));
    routeOutput_->SetFont(wxFont(9, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    
    resultBox->Add(routeOutput_, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(resultBox, 1, wxEXPAND | wxALL, 10);
    
    SetSizer(mainSizer);
    
    debugFrame_->AppendLog("🗺️ RoutePlannerTestPanel 초기화 완료 - Mock 알고리즘 준비됨");
}

void RoutePlannerTestPanel::OnPlanRoute(wxCommandEvent& event) {
    // 입력값 검증
    double startLon, startLat, endLon, endLat;
    
    if (!startLonInput_->GetValue().ToDouble(&startLon) ||
        !startLatInput_->GetValue().ToDouble(&startLat) ||
        !endLonInput_->GetValue().ToDouble(&endLon) ||
        !endLatInput_->GetValue().ToDouble(&endLat)) {
        debugFrame_->AppendLog("❌ 좌표 입력값이 올바르지 않습니다");
        return;
    }
    
    // 좌표 범위 검증 (한국 대략적 범위)
    if (startLon < 124.0 || startLon > 132.0 || startLat < 33.0 || startLat > 39.0 ||
        endLon < 124.0 || endLon > 132.0 || endLat < 33.0 || endLat > 39.0) {
        debugFrame_->AppendLog("⚠️ 좌표가 한국 범위를 벗어남 - 계속 진행");
    }
    
    LonLat start(startLon, startLat);
    LonLat end(endLon, endLat);
    
    int algorithmIndex = algorithmChoice_->GetSelection();
    wxString algorithmName = algorithmChoice_->GetStringSelection();
    
    debugFrame_->AppendLog(wxString::Format("🗺️ 경로 계획 시작: %s 알고리즘", algorithmName));
    
    // ========================================
    // 🎯 학습 포인트: 다양한 경로 생성 알고리즘
    // 실제로는 도로 네트워크, 교통정보 등을 고려하지만,
    // 학습용으로 기하학적 패턴 생성
    // ========================================
    
    std::vector<LonLat> waypoints;
    std::ostringstream result;
    
    result << "=== 경로 계획 결과 ===\n";
    result << "출발지: " << start.ToString() << "\n";
    result << "목적지: " << end.ToString() << "\n";
    result << "알고리즘: " << algorithmName.ToStdString() << "\n";
    result << "직선 거리: " << std::fixed << std::setprecision(2) << start.DistanceTo(end) << " km\n\n";
    
    waypoints.push_back(start);  // 출발지
    
    switch (algorithmIndex) {
        case 0: {  // 직선 경로
            result << "📍 경로점들 (직선):\n";
            // 중간 지점 몇 개 추가 (직선상의 점들)
            for (int i = 1; i < 4; ++i) {
                double ratio = i / 4.0;
                double midLon = start.lon + (end.lon - start.lon) * ratio;  // longitude → lon으로 수정
                double midLat = start.lat + (end.lat - start.lat) * ratio;  // latitude → lat으로 수정
                LonLat waypoint(midLon, midLat);
                waypoints.push_back(waypoint);
                result << "  " << i << ". " << waypoint.ToString() << "\n";
            }
            break;
        }
        case 1: {  // 지그재그 경로
            result << "📍 경로점들 (지그재그):\n";
            // 지그재그 패턴 생성
            for (int i = 1; i < 6; ++i) {
                double ratio = i / 6.0;
                double midLon = start.lon + (end.lon - start.lon) * ratio;
                double midLat = start.lat + (end.lat - start.lat) * ratio;
                
                // 지그재그 오프셋 추가
                double offset = (i % 2 == 1) ? 0.01 : -0.01;
                midLon += offset;
                
                LonLat waypoint(midLon, midLat);
                waypoints.push_back(waypoint);
                result << "  " << i << ". " << waypoint.ToString() << " (오프셋: " << offset << ")\n";
            }
            break;
        }
        case 2: {  // 곡선 경로
            result << "📍 경로점들 (곡선):\n";
            // 베지어 곡선 근사
            for (int i = 1; i < 8; ++i) {
                double t = i / 8.0;
                // 제어점 설정 (중간지점을 약간 위로)
                double ctrlLon = (start.lon + end.lon) / 2.0;
                double ctrlLat = (start.lat + end.lat) / 2.0 + 0.02;
                
                // 이차 베지어 곡선 공식
                double midLon = (1-t)*(1-t)*start.lon + 2*(1-t)*t*ctrlLon + t*t*end.lon;
                double midLat = (1-t)*(1-t)*start.lat + 2*(1-t)*t*ctrlLat + t*t*end.lat;
                
                LonLat waypoint(midLon, midLat);
                waypoints.push_back(waypoint);
                result << "  " << i << ". " << waypoint.ToString() << " (t=" << std::fixed << std::setprecision(2) << t << ")\n";
            }
            break;
        }
    }
    
    waypoints.push_back(end);  // 목적지
    
    // 총 경로 거리 계산
    double totalDistance = 0.0;
    for (size_t i = 1; i < waypoints.size(); ++i) {
        totalDistance += waypoints[i-1].DistanceTo(waypoints[i]);
    }
    
    result << "\n=== 경로 통계 ===\n";
    result << "총 경로점 수: " << waypoints.size() << "\n";
    result << "총 경로 거리: " << std::fixed << std::setprecision(2) << totalDistance << " km\n";
    result << "직선 대비 비율: " << std::fixed << std::setprecision(1) << (totalDistance / start.DistanceTo(end) * 100.0) << "%\n";
    
    // JSON 형태 경로 데이터 (실제 내비게이션 앱에서 사용하는 형식)
    result << "\n=== Mock 경로 JSON ===\n";
    result << "{\n";
    result << "  \"route\": {\n";
    result << "    \"algorithm\": \"" << algorithmName.ToStdString() << "\",\n";
    result << "    \"totalDistance\": " << totalDistance << ",\n";
    result << "    \"waypoints\": [\n";
    
    for (size_t i = 0; i < waypoints.size(); ++i) {
        result << "      {\"lon\": " << std::fixed << std::setprecision(6) << waypoints[i].lon 
               << ", \"lat\": " << waypoints[i].lat << "}";
        if (i < waypoints.size() - 1) result << ",";
        result << "\n";
    }
    
    result << "    ]\n";
    result << "  }\n";
    result << "}";
    
    routeOutput_->SetValue(result.str());
    
    debugFrame_->AppendLog(wxString::Format("✅ 경로 생성 완료: %zu개 경로점, %.2f km", 
                                          waypoints.size(), totalDistance));
}

void RoutePlannerTestPanel::OnClearRoute(wxCommandEvent& event) {
    routeOutput_->Clear();
    debugFrame_->AppendLog("🗑️ 경로 데이터 초기화됨");
}

// ========================================
// 🆕 3단계: MapRenderPanel 구현
// OpenStreetMap 타일 기반 실제 지도 렌더링
// ========================================

// 이벤트 테이블
wxBEGIN_EVENT_TABLE(MapRenderPanel, wxPanel)
    EVT_PAINT(MapRenderPanel::OnPaint)
    EVT_SIZE(MapRenderPanel::OnSize)
    EVT_LEFT_DOWN(MapRenderPanel::OnLeftDown)
    EVT_LEFT_UP(MapRenderPanel::OnLeftUp)
    EVT_MOTION(MapRenderPanel::OnMouseMove)
    EVT_MOUSEWHEEL(MapRenderPanel::OnMouseWheel)
    EVT_BUTTON(ID_ZOOM_IN, MapRenderPanel::OnZoomIn)
    EVT_BUTTON(ID_ZOOM_OUT, MapRenderPanel::OnZoomOut)
    EVT_BUTTON(ID_RESET_VIEW, MapRenderPanel::OnResetView)
    EVT_BUTTON(ID_LOAD_TILES, MapRenderPanel::OnLoadTiles)
    EVT_BUTTON(ID_SHOW_ROUTE, MapRenderPanel::OnShowRoute)
    // 🆕 4단계: 실시간 UI 업데이트 이벤트
    EVT_BUTTON(ID_START_NAV_SIMULATION, MapRenderPanel::OnStartNavigation)
    EVT_BUTTON(ID_STOP_NAV_SIMULATION, MapRenderPanel::OnStopNavigation)
    EVT_TIMER(ID_UPDATE_TIMER, MapRenderPanel::OnUpdateTimer)
    EVT_TIMER(ID_ANIMATION_TIMER, MapRenderPanel::OnAnimationTimer)
wxEND_EVENT_TABLE()

MapRenderPanel::MapRenderPanel(wxWindow* parent, DebugFrame* debugFrame)
    : wxPanel(parent, wxID_ANY), debugFrame_(debugFrame),
      centerCoord_(126.9780, 37.5665),  // 서울시청 기본값
      zoomLevel_(10),
      isDragging_(false),
      // 🆕 4단계: 시뮬레이션 상태 초기화
      isNavigating_(false),
      currentProgress_(0.0),
      currentSpeed_(0.0),
      remainingDistance_(1200.0) {
    
    // 순수 비즈니스 로직 클래스들 초기화
    cameraController_ = std::make_unique<presentation::components::CameraController>();
    locationPuck_ = std::make_unique<presentation::components::LocationPuck>(this);
    renderPipeline_ = std::make_unique<RenderPipeline>();
    
    // PolylineHighlightRenderer는 RenderPipeline과 테마가 필요
    presentation::components::PolylineTheme defaultTheme;
    polylineHighlight_ = std::make_unique<presentation::components::PolylineHighlightRenderer>(*renderPipeline_, defaultTheme);
    
    // ========================================
    // 🎯 MapPanel.h 구현 가이드: UI 클래스 콜백 패턴
    // ✅ CameraController::SetCameraMoveCallback() 활용
    // ========================================
    cameraController_->SetCameraMoveCallback([this](const LonLat& center, double bearing) {
        centerCoord_ = center;
        debugFrame_->AppendLog(wxString::Format("📍 UI 클래스 콜백: 카메라 이동 %.6f, %.6f (방향: %.1f°)", 
                                               center.lon, center.lat, bearing));
        Refresh(); // 지도 다시 그리기
    });
    
    // ✅ CameraController 클래스 초기 설정 패턴
    cameraController_->SetFollowMode(CameraFollowMode::Location);
    cameraController_->SetFollowThreshold(10.0); // 10m 이상 이동시 follow
    cameraController_->SetAnimationDuration(300); // 300ms 애니메이션
    
    // ========================================
    // 🎯 학습 포인트: 실제 지도 애플리케이션 UI 구성
    // - 지도 영역 + 컨트롤 패널의 분리 설계
    // - 좌표계 변환의 실시간 처리
    // ========================================
    
    auto* mainSizer = new wxBoxSizer(wxVERTICAL);
    
    // 제목 및 설명
    auto* titleBox = new wxBoxSizer(wxHORIZONTAL);
    auto* title = new wxStaticText(this, wxID_ANY, "🗺️ 3단계: OpenStreetMap 렌더링");
    title->SetFont(wxFont(16, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    title->SetForegroundColour(wxColour(0, 120, 200));
    titleBox->Add(title, 0, wxALIGN_CENTER_VERTICAL);
    
    mainSizer->Add(titleBox, 0, wxALL | wxEXPAND, 10);
    
    // 컨트롤 패널 (상단)
    auto* controlBox = new wxStaticBoxSizer(wxHORIZONTAL, this, "지도 컨트롤");
    
    // 줌 컨트롤
    zoomInBtn_ = new wxButton(this, ID_ZOOM_IN, "🔍+");
    zoomInBtn_->SetToolTip("확대 (Zoom In)");
    controlBox->Add(zoomInBtn_, 0, wxRIGHT, 5);
    
    zoomOutBtn_ = new wxButton(this, ID_ZOOM_OUT, "🔍-");
    zoomOutBtn_->SetToolTip("축소 (Zoom Out)");
    controlBox->Add(zoomOutBtn_, 0, wxRIGHT, 10);
    
    // 뷰 리셋
    resetBtn_ = new wxButton(this, ID_RESET_VIEW, "🏠 서울");
    resetBtn_->SetToolTip("서울 중심으로 리셋");
    controlBox->Add(resetBtn_, 0, wxRIGHT, 10);
    
    // 타일 로드 (학습용)
    loadTilesBtn_ = new wxButton(this, ID_LOAD_TILES, "📥 타일 로드");
    loadTilesBtn_->SetToolTip("OpenStreetMap 타일 다운로드 시뮬레이션");
    controlBox->Add(loadTilesBtn_, 0, wxRIGHT, 10);
    
    // 경로 표시
    showRouteBtn_ = new wxButton(this, ID_SHOW_ROUTE, "🛣️ 경로 표시");
    showRouteBtn_->SetToolTip("2단계에서 생성한 경로를 지도에 표시");
    controlBox->Add(showRouteBtn_, 0, wxRIGHT, 10);
    
    // 🆕 4단계: 실시간 UI 시뮬레이션 컨트롤
    startNavBtn_ = new wxButton(this, ID_START_NAV_SIMULATION, "▶️ 네비게이션 시작");
    startNavBtn_->SetToolTip("실시간 UI 업데이트 시뮬레이션 시작");
    startNavBtn_->SetBackgroundColour(wxColour(0, 200, 0));
    controlBox->Add(startNavBtn_, 0, wxRIGHT, 5);
    
    stopNavBtn_ = new wxButton(this, ID_STOP_NAV_SIMULATION, "⏹️ 정지");
    stopNavBtn_->SetToolTip("네비게이션 시뮬레이션 정지");
    stopNavBtn_->SetBackgroundColour(wxColour(200, 0, 0));
    stopNavBtn_->Enable(false);  // 초기에는 비활성화
    controlBox->Add(stopNavBtn_, 0, wxRIGHT, 10);
    
    // 좌표 및 줌 정보 표시
    coordLabel_ = new wxStaticText(this, wxID_ANY, 
                                   wxString::Format("중심: %.4f, %.4f", centerCoord_.lon, centerCoord_.lat));
    coordLabel_->SetFont(wxFont(9, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    controlBox->Add(coordLabel_, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
    
    zoomLabel_ = new wxStaticText(this, wxID_ANY, wxString::Format("줌: %d", zoomLevel_));
    zoomLabel_->SetFont(wxFont(9, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    controlBox->Add(zoomLabel_, 0, wxALIGN_CENTER_VERTICAL);
    
    mainSizer->Add(controlBox, 0, wxEXPAND | wxALL, 10);
    
    // 지도 렌더링 영역 (메인)
    // 이 패널 자체가 지도 캔버스 역할을 함
    auto* mapInfo = new wxStaticText(this, wxID_ANY, 
        "💡 지도 영역: 마우스로 드래그하여 이동, 휠로 줌 조절\n"
        "현재는 Mock 구현 - 실제로는 OSM 타일 이미지가 표시됩니다");
    mapInfo->SetForegroundColour(wxColour(100, 100, 100));
    mainSizer->Add(mapInfo, 0, wxALL, 10);
    
    // ========================================
    // 🆕 4단계: 실제 UI 컴포넌트 초기화
    // ========================================
    
    // HUD 오버레이 생성 (지도 위에 떠있는 정보 표시)
    hudOverlay_ = new presentation::components::MapOverlayHud(this);  // wxWidgets 자동 관리
    presentation::components::HudState initialHudState;
    initialHudState.visible = true;
    initialHudState.speed_kmh = 0.0;
    initialHudState.distance_remain_m = 1200.0;  // 1.2km 예시
    hudOverlay_->SetState(initialHudState);
    
    // 웨이포인트 패널을 별도 창이 아닌 하단에 추가 (구현 완성됨)
    waypointPanel_ = new presentation::components::WaypointListPanel(this);  // wxWidgets 자동 관리
    waypointPanel_->SetMinSize(wxSize(-1, 150));  // 최소 높이 150px
    
    // 네비게이션 진행도 바
    progressBar_ = new presentation::components::NavigationProgressBar(this);  // wxWidgets 자동 관리
    presentation::components::NavigationProgress initialProgress;
    initialProgress.total_distance = 1000.0;
    initialProgress.remaining_distance = 1000.0;
    initialProgress.completion_ratio = 0.0;
    progressBar_->UpdateProgress(initialProgress);
    progressBar_->SetMinSize(wxSize(-1, 20));
    
    // 🆕 4단계: 실시간 HUD 상태 표시 패널 (GUI)
    auto* hudBox = new wxStaticBoxSizer(wxVERTICAL, this, "🚗 실시간 네비게이션 상태");
    
    // HUD 상태 라벨
    hudStatusLabel_ = new wxStaticText(this, wxID_ANY, 
        "⏸️ 네비게이션 대기중 | 속도: 0 km/h | 남은 거리: 1200m");
    hudStatusLabel_->SetFont(wxFont(11, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    hudStatusLabel_->SetForegroundColour(wxColour(50, 50, 50));
    hudBox->Add(hudStatusLabel_, 0, wxEXPAND | wxALL, 5);
    
    // 진행도 게이지 (시각적 프로그레스 바)
    progressGauge_ = new wxGauge(this, wxID_ANY, 100);  // 0-100%
    progressGauge_->SetValue(0);
    progressGauge_->SetMinSize(wxSize(-1, 25));
    hudBox->Add(progressGauge_, 0, wxEXPAND | wxALL, 5);
    
    // 타이머 초기화
    updateTimer_ = new wxTimer(this, ID_UPDATE_TIMER);  // 네비게이션 업데이트용
    animationTimer_ = new wxTimer(this, ID_ANIMATION_TIMER);  // 애니메이션 업데이트용 (60fps)
    
    // 기존 sizer에 새 UI 컴포넌트들 추가
    mainSizer->Add(hudBox, 0, wxEXPAND | wxALL, 10);
    mainSizer->Add(progressBar_, 0, wxEXPAND | wxALL, 5);
    mainSizer->Add(waypointPanel_, 0, wxEXPAND | wxALL, 10);  // 웨이포인트 패널 활성화
    
    SetSizer(mainSizer);
    SetBackgroundColour(wxColour(240, 248, 255));  // 연한 파란색 배경
    
    // 초기 샘플 경로 데이터 (2단계 연동 준비)
    currentRoute_ = {
        LonLat(126.9780, 37.5665),  // 서울시청
        LonLat(127.0276, 37.4979),  // 강남역
        LonLat(126.9700, 37.5547)   // 서울역
    };
    
    debugFrame_->AppendLog("🗺️ MapRenderPanel 초기화 완료");
    debugFrame_->AppendLog("🆕 4단계: HUD, 웨이포인트 패널, 진행도바 추가됨");
    debugFrame_->AppendLog(wxString::Format("📍 초기 중심 좌표: %.4f, %.4f (줌 %d)", 
                                          centerCoord_.lon, centerCoord_.lat, zoomLevel_));
}

// ========================================
// 🎯 핵심 학습: 지도 렌더링 및 인터랙션
// ========================================

void MapRenderPanel::OnPaint(wxPaintEvent& event) {
    wxPaintDC dc(this);
    RenderMap(dc);
}

void MapRenderPanel::OnSize(wxSizeEvent& event) {
    panelSize_ = GetSize();
    debugFrame_->AppendLog(wxString::Format("🖥️ 지도 패널 크기 변경: %dx%d", 
                                          panelSize_.GetWidth(), panelSize_.GetHeight()));
    Refresh();  // 화면 다시 그리기
    event.Skip();
}

void MapRenderPanel::OnLeftDown(wxMouseEvent& event) {
    isDragging_ = true;
    lastMousePos_ = event.GetPosition();
    CaptureMouse();  // 마우스 캡처로 드래그 추적
    
    // 클릭한 지점의 좌표 계산 (학습용)
    LonLat clickedCoord = ScreenToLatLon(event.GetPosition());
    debugFrame_->AppendLog(wxString::Format("🖱️ 지도 클릭: %.6f, %.6f", 
                                          clickedCoord.lon, clickedCoord.lat));
}

void MapRenderPanel::OnLeftUp(wxMouseEvent& event) {
    if (isDragging_) {
        isDragging_ = false;
        ReleaseMouse();
        debugFrame_->AppendLog("🖱️ 지도 드래그 완료");
    }
}

void MapRenderPanel::OnMouseMove(wxMouseEvent& event) {
    if (isDragging_) {
        // ========================================
        // 🎯 학습 포인트: 지도 패닝 (이동) 알고리즘
        // 마우스 이동량을 지리 좌표 이동량으로 변환
        // ========================================
        
        wxPoint currentPos = event.GetPosition();
        wxPoint delta = currentPos - lastMousePos_;
        
        // 픽셀 이동량을 경위도 이동량으로 변환 (간단한 선형 변환)
        // 실제로는 머케이터 투영법 고려 필요
        double lonDelta = -delta.x * 0.001 * (18 - zoomLevel_);  // 줌 레벨에 따라 민감도 조정
        double latDelta = delta.y * 0.001 * (18 - zoomLevel_);
        
        centerCoord_.lon += lonDelta;
        centerCoord_.lat += latDelta;
        
        // 좌표 범위 제한 (전 세계)
        centerCoord_.lon = std::max(-180.0, std::min(180.0, centerCoord_.lon));
        centerCoord_.lat = std::max(-85.0, std::min(85.0, centerCoord_.lat));
        
        lastMousePos_ = currentPos;
        
        // UI 업데이트
        coordLabel_->SetLabel(wxString::Format("중심: %.4f, %.4f", centerCoord_.lon, centerCoord_.lat));
        Refresh();  // 지도 다시 그리기
    }
}

void MapRenderPanel::OnMouseWheel(wxMouseEvent& event) {
    // ========================================
    // 🎯 학습 포인트: 지도 줌 기능 구현
    // 마우스 휠로 줌 레벨 조정
    // ========================================
    
    int wheelRotation = event.GetWheelRotation();
    int oldZoom = zoomLevel_;
    
    if (wheelRotation > 0 && zoomLevel_ < 18) {
        zoomLevel_++;
        debugFrame_->AppendLog(wxString::Format("🔍 줌 인: %d → %d", oldZoom, zoomLevel_));
    } else if (wheelRotation < 0 && zoomLevel_ > 1) {
        zoomLevel_--;
        debugFrame_->AppendLog(wxString::Format("🔍 줌 아웃: %d → %d", oldZoom, zoomLevel_));
    }
    
    zoomLabel_->SetLabel(wxString::Format("줌: %d", zoomLevel_));
    Refresh();
}

void MapRenderPanel::OnZoomIn(wxCommandEvent& event) {
    if (zoomLevel_ < 18) {
        zoomLevel_++;
        zoomLabel_->SetLabel(wxString::Format("줌: %d", zoomLevel_));
        debugFrame_->AppendLog(wxString::Format("🔍+ 버튼 줌 인: %d", zoomLevel_));
        Refresh();
    }
}

void MapRenderPanel::OnZoomOut(wxCommandEvent& event) {
    if (zoomLevel_ > 1) {
        zoomLevel_--;
        zoomLabel_->SetLabel(wxString::Format("줌: %d", zoomLevel_));
        debugFrame_->AppendLog(wxString::Format("🔍- 버튼 줌 아웃: %d", zoomLevel_));
        Refresh();
    }
}

void MapRenderPanel::OnResetView(wxCommandEvent& event) {
    centerCoord_ = LonLat(126.9780, 37.5665);  // 서울시청으로 리셋
    zoomLevel_ = 10;
    
    coordLabel_->SetLabel(wxString::Format("중심: %.4f, %.4f", centerCoord_.lon, centerCoord_.lat));
    zoomLabel_->SetLabel(wxString::Format("줌: %d", zoomLevel_));
    
    debugFrame_->AppendLog("🏠 지도 뷰 리셋: 서울시청 중심");
    Refresh();
}

void MapRenderPanel::OnLoadTiles(wxCommandEvent& event) {
    // ========================================
    // 🎯 학습 포인트: 지도 타일 시스템 이해
    // 실제로는 OSM 타일 서버에서 이미지 다운로드
    // ========================================
    
    debugFrame_->AppendLog("📥 OpenStreetMap 타일 로드 시뮬레이션 시작...");
    
    // Mock: 필요한 타일 계산
    int tilesPerSide = 1 << zoomLevel_;  // 2^zoomLevel
    debugFrame_->AppendLog(wxString::Format("📊 줌 %d: %dx%d 총 타일", 
                                          zoomLevel_, tilesPerSide, tilesPerSide));
    
    // Mock: 현재 뷰포트에 필요한 타일들 계산
    UpdateTileList();
    
    debugFrame_->AppendLog("✅ 타일 로드 완료 (Mock)");
}

void MapRenderPanel::OnShowRoute(wxCommandEvent& event) {
    debugFrame_->AppendLog("🛣️ 4단계: 실제 경로 하이라이트 시스템 활용");
    
    if (currentRoute_.empty()) {
        debugFrame_->AppendLog("⚠️ 표시할 경로가 없습니다");
        return;
    }
    
    // ========================================
    // 🆕 4단계: 실제 UI 알고리즘 적용
    // ========================================
    
    debugFrame_->AppendLog(wxString::Format("📍 경로점 수: %zu개", currentRoute_.size()));
    
    // 웨이포인트 패널에 경로 추가
    std::vector<Waypoint> waypoints;
    for (size_t i = 0; i < currentRoute_.size(); ++i) {
        Waypoint wp;
        wp.coordinates = currentRoute_[i];
        wp.name = wxString::Format("지점 %zu", i+1).ToStdString();
        waypoints.push_back(wp);
        
        debugFrame_->AppendLog(wxString::Format("  %zu. %.6f, %.6f (%s)", 
                                              i+1, currentRoute_[i].lon, currentRoute_[i].lat, wp.name));
    }
    
    // 웨이포인트 패널 업데이트
    // 웨이포인트 패널에 경로 데이터 전달
    if (waypointPanel_) {
        waypointPanel_->ClearWaypoints();
        for (const auto& coord : currentRoute_) {
            Waypoint wp;
            wp.coordinates = coord;
            wp.name = wxString::Format("지점-%zu", waypointPanel_->GetWaypoints().size() + 1).ToStdString();
            waypointPanel_->AddWaypoint(wp);
        }
    }
    
    // 진행도바 업데이트 (30% 진행 시뮬레이션)
    presentation::components::NavigationProgress updatedProgress;
    updatedProgress.total_distance = 1000.0;
    updatedProgress.remaining_distance = 700.0;
    updatedProgress.completion_ratio = 0.3;
    progressBar_->UpdateProgress(updatedProgress);
    progressBar_->Refresh();
    
    // HUD 상태 업데이트
    presentation::components::HudState hudState;
    hudState.visible = true;
    hudState.speed_kmh = 45.0;       // 45km/h 시뮬레이션
    hudState.distance_remain_m = 850.0; // 남은 거리 850m
    hudOverlay_->SetState(hudState);
    
    debugFrame_->AppendLog("✅ 4단계 UI 컴포넌트들 업데이트 완료");
    debugFrame_->AppendLog("  - 웨이포인트 패널: 경로점 표시");
    debugFrame_->AppendLog("  - 진행도바: 30% 진행");
    debugFrame_->AppendLog("  - HUD 오버레이: 속도 45km/h, 남은 거리 850m");
    
    // LocationPuck 초기 위치 설정 (경로 시작점)
    if (!currentRoute_.empty() && locationPuck_) {
        LonLat startPos = currentRoute_[0];
        double bearing = 0.0;
        bool hasBearing = false;
        
        // 시작점에서 다음 점으로의 방향 계산
        if (currentRoute_.size() > 1) {
            bearing = CalculateBearing(startPos, currentRoute_[1]);
            hasBearing = true;
        }
        
        LocationState initialState(startPos, 10.0, bearing, hasBearing, true); // 10m 정확도
        locationPuck_->UpdateLocation(initialState);
        
        // CameraController에도 전달
        if (cameraController_) {
            cameraController_->UpdateLocation(initialState);
        }
        
        debugFrame_->AppendLog(wxString::Format("📍 LocationPuck 초기 위치: %.6f, %.6f (방향: %.1f°)", 
                                              startPos.lon, startPos.lat, bearing));
    }
    
    Refresh();  // 지도 다시 그려서 경로 표시
}

// ========================================
// 🎯 핵심 학습: 좌표계 변환 (머케이터 투영법)
// ========================================

wxPoint MapRenderPanel::LatLonToScreen(const LonLat& coord) const {
    // ========================================
    // 🎓 학습 포인트: 웹 머케이터 투영법 (EPSG:3857)
    // 위경도를 화면 픽셀 좌표로 변환하는 핵심 알고리즘
    // ========================================
    
    if (!panelSize_.IsFullySpecified()) {
        return wxPoint(0, 0);
    }
    
    // 1. 경도는 선형 변환 (-180~180 → 0~1)
    double lonNormalized = (coord.lon + 180.0) / 360.0;
    
    // 2. 위도는 머케이터 투영법 적용
    double latRad = coord.lat * DEG_TO_RAD;
    double latNormalized = (1.0 - std::log(std::tan(latRad) + 1.0/std::cos(latRad)) / M_PI) / 2.0;
    
    // 3. 줌 레벨 적용 (2^zoom 배율)
    double scale = std::pow(2.0, zoomLevel_);
    
    // 4. 중심 좌표 기준으로 상대 위치 계산
    double centerLonNormalized = (centerCoord_.lon + 180.0) / 360.0;
    double centerLatRad = centerCoord_.lat * DEG_TO_RAD;
    double centerLatNormalized = (1.0 - std::log(std::tan(centerLatRad) + 1.0/std::cos(centerLatRad)) / M_PI) / 2.0;
    
    // 5. 화면 중심을 기준으로 픽셀 좌표 계산
    int screenX = static_cast<int>(panelSize_.GetWidth() / 2 + 
                                  (lonNormalized - centerLonNormalized) * scale * 256);
    int screenY = static_cast<int>(panelSize_.GetHeight() / 2 + 
                                  (latNormalized - centerLatNormalized) * scale * 256);
    
    return wxPoint(screenX, screenY);
}

LonLat MapRenderPanel::ScreenToLatLon(const wxPoint& point) const {
    // ========================================
    // 🎓 학습 포인트: 역 머케이터 투영법
    // 화면 픽셀 좌표를 위경도로 변환
    // ========================================
    
    if (!panelSize_.IsFullySpecified()) {
        return centerCoord_;
    }
    
    double scale = std::pow(2.0, zoomLevel_);
    
    // 1. 화면 중심 기준 상대 픽셀 좌표
    double deltaX = (point.x - panelSize_.GetWidth() / 2.0) / (scale * 256.0);
    double deltaY = (point.y - panelSize_.GetHeight() / 2.0) / (scale * 256.0);
    
    // 2. 중심 좌표의 정규화된 좌표
    double centerLonNormalized = (centerCoord_.lon + 180.0) / 360.0;
    double centerLatRad = centerCoord_.lat * DEG_TO_RAD;
    double centerLatNormalized = (1.0 - std::log(std::tan(centerLatRad) + 1.0/std::cos(centerLatRad)) / M_PI) / 2.0;
    
    // 3. 클릭 지점의 정규화된 좌표
    double lonNormalized = centerLonNormalized + deltaX;
    double latNormalized = centerLatNormalized + deltaY;
    
    // 4. 정규화된 좌표를 위경도로 변환
    double lon = lonNormalized * 360.0 - 180.0;
    double latRad = std::atan(std::sinh(M_PI * (1.0 - 2.0 * latNormalized)));
    double lat = latRad * 180.0 / M_PI;
    
    return LonLat(lon, lat);
}

void MapRenderPanel::UpdateTileList() {
    // ========================================
    // 🎓 학습 포인트: 지도 타일 시스템 이해
    // 현재 뷰포트에 필요한 타일들을 계산
    // ========================================
    
    debugFrame_->AppendLog("🧮 필요한 지도 타일 계산 중...");
    
    // 화면 네 모서리의 좌표를 구해서 필요한 타일 범위 계산
    LonLat topLeft = ScreenToLatLon(wxPoint(0, 0));
    LonLat bottomRight = ScreenToLatLon(wxPoint(panelSize_.GetWidth(), panelSize_.GetHeight()));
    
    debugFrame_->AppendLog(wxString::Format("📐 뷰포트: (%.4f,%.4f) ~ (%.4f,%.4f)", 
                                          topLeft.lon, topLeft.lat, 
                                          bottomRight.lon, bottomRight.lat));
    
    // 타일 인덱스 계산 (OSM 타일 명명 규칙)
    int tilesPerSide = 1 << zoomLevel_;
    
    int tileXMin = static_cast<int>((topLeft.lon + 180.0) / 360.0 * tilesPerSide);
    int tileXMax = static_cast<int>((bottomRight.lon + 180.0) / 360.0 * tilesPerSide);
    
    debugFrame_->AppendLog(wxString::Format("🗂️ 필요 타일 X 범위: %d ~ %d", tileXMin, tileXMax));
    debugFrame_->AppendLog("💡 실제 구현시 여기서 HTTP 요청으로 타일 다운로드");
}

// ========================================
// 🎯 MapPanel.h 구현 가이드: 지리 계산 유틸리티 함수
// 📌 실제 MapPanel 구현 시 Types.h로 이동하여 전역 함수로 사용 권장
// ========================================
double MapRenderPanel::CalculateBearing(const LonLat& from, const LonLat& to) const {
    // ========================================
    // 🎓 학습 포인트: 두 지점 간 방향(베어링) 계산
    // 구면 삼각법을 사용한 진북 기준 각도 계산
    // 📌 MapPanel.h에서는 Types.h의 전역 함수로 구현할 것
    // ========================================
    
    double lat1 = from.lat * DEG_TO_RAD;
    double lat2 = to.lat * DEG_TO_RAD;
    double deltaLon = (to.lon - from.lon) * DEG_TO_RAD;
    
    double y = std::sin(deltaLon) * std::cos(lat2);
    double x = std::cos(lat1) * std::sin(lat2) - std::sin(lat1) * std::cos(lat2) * std::cos(deltaLon);
    
    double bearing = std::atan2(y, x) * 180.0 / M_PI;
    
    // 0-360도 범위로 정규화
    bearing = std::fmod(bearing + 360.0, 360.0);
    
    return bearing;
}

// ========================================
// 🎯 MapPanel.h 구현 가이드: 경로 보간 함수
// ❌ 이 방식보다는 ✅ LocationPuck::InterpolateLocation() 활용 권장
// ========================================
LonLat MapRenderPanel::CalculateRoutePosition(double progress) const {
    // ========================================
    // 🎯 MapPanel.h 구현 가이드: 경로 상 위치 계산
    // 📌 실제 구현에서는 LocationPuck의 InterpolateLocation 메서드 활용
    // 📌 이 함수는 데모용이며, 실제로는 UI 클래스에 위임할 것
    // ========================================
    
    if (currentRoute_.empty()) return LonLat();
    if (currentRoute_.size() == 1) return currentRoute_[0];
    
    // 진행률을 경로 인덱스로 변환
    double routeProgress = progress * (currentRoute_.size() - 1);
    size_t segmentIndex = static_cast<size_t>(routeProgress);
    double segmentProgress = routeProgress - segmentIndex;
    
    // 마지막 점 처리
    if (segmentIndex >= currentRoute_.size() - 1) {
        return currentRoute_.back();
    }
    
    // ✅ LocationPuck::InterpolateLocation() 방식과 유사한 선형 보간
    const LonLat& from = currentRoute_[segmentIndex];
    const LonLat& to = currentRoute_[segmentIndex + 1];
    
    LonLat result;
    result.lon = from.lon + (to.lon - from.lon) * segmentProgress;
    result.lat = from.lat + (to.lat - from.lat) * segmentProgress;
    
    return result;
}

void MapRenderPanel::RenderMap(wxDC& dc) {
    // ========================================
    // 🎓 학습 포인트: 지도 렌더링 파이프라인
    // 1. 배경 → 2. 지도 타일 → 3. 경로 → 4. UI 오버레이
    // ========================================
    
    wxSize size = GetSize();
    
    // 1. 배경 그리기 (바다색)
    dc.SetBrush(wxBrush(wxColour(170, 211, 223)));  // 연한 파란색
    dc.DrawRectangle(0, 0, size.GetWidth(), size.GetHeight());
    
    // 2. 지도 타일 영역 표시 (Mock)
    dc.SetPen(wxPen(wxColour(100, 100, 100), 1, wxPENSTYLE_DOT));
    dc.SetBrush(wxBrush(wxColour(245, 245, 220)));  // 베이지색 (육지)
    
    // 타일 격자 그리기 (학습용 시각화)
    int gridSize = 50;
    for (int x = 0; x < size.GetWidth(); x += gridSize) {
        for (int y = 0; y < size.GetHeight(); y += gridSize) {
            dc.DrawRectangle(x, y, gridSize, gridSize);
        }
    }
    
    // 3. 경로 렌더링 (4단계 개선)
    RenderRouteAdvanced(dc);
    
    // ========================================
    // 🎯 MapPanel.h 구현 가이드: UI 클래스 렌더링 패턴
    // ✅ LocationPuck 클래스의 Render() 메서드 활용
    // ========================================
    if (locationPuck_ && locationPuck_->IsVisible()) {
        // 좌표 변환 함수를 람다로 전달 (의존성 주입 패턴)
        std::function<wxPoint(const LonLat&)> coordToPixel = [this](const LonLat& coord) -> wxPoint {
            return LatLonToScreen(coord);
        };
        locationPuck_->Render(dc, coordToPixel); // ✅ UI 클래스 메서드 호출
    }
    
    // 4. 기본 UI 오버레이
    RenderUI(dc);
    
    // 🆕 5. HUD 오버레이 렌더링 (4단계 추가)
    if (hudOverlay_) {
        // HUD는 별도 패널이므로 여기서는 상태만 확인
        // 실제 렌더링은 HUD 자체의 OnPaint에서 처리됨
    }
}

void MapRenderPanel::RenderRoute(wxDC& dc) {
    if (currentRoute_.size() < 2) return;
    
    // ========================================
    // 🎓 학습 포인트: 지도 위 경로 시각화
    // 지리 좌표를 화면 좌표로 변환하여 선 그리기
    // ========================================
    
    dc.SetPen(wxPen(wxColour(255, 0, 0), 3));  // 빨간색 굵은 선
    
    for (size_t i = 1; i < currentRoute_.size(); ++i) {
        wxPoint startPoint = LatLonToScreen(currentRoute_[i-1]);
        wxPoint endPoint = LatLonToScreen(currentRoute_[i]);
        
        // 화면 영역 내에 있는 경우만 그리기
        wxSize screenSize = GetSize();
        if (startPoint.x >= -50 && startPoint.x <= screenSize.GetWidth() + 50 &&
            startPoint.y >= -50 && startPoint.y <= screenSize.GetHeight() + 50) {
            dc.DrawLine(startPoint, endPoint);
        }
    }
    
    // 경로점 마커 그리기
    dc.SetPen(wxPen(wxColour(0, 0, 0), 2));
    dc.SetBrush(wxBrush(wxColour(255, 255, 0)));  // 노란색
    
    for (size_t i = 0; i < currentRoute_.size(); ++i) {
        wxPoint point = LatLonToScreen(currentRoute_[i]);
        dc.DrawCircle(point, 5);
        
        // 점 번호 표시
        dc.SetTextForeground(wxColour(0, 0, 0));
        dc.DrawText(wxString::Format("%zu", i+1), point.x + 8, point.y - 8);
    }
}

void MapRenderPanel::RenderUI(wxDC& dc) {
    // ========================================
    // 🎓 학습 포인트: 지도 UI 오버레이
    // 십자선, 스케일바, 좌표 정보 등
    // ========================================
    
    wxSize size = GetSize();
    
    // 중심점 십자선 그리기
    dc.SetPen(wxPen(wxColour(0, 0, 0), 1));
    int centerX = size.GetWidth() / 2;
    int centerY = size.GetHeight() / 2;
    
    dc.DrawLine(centerX - 10, centerY, centerX + 10, centerY);
    dc.DrawLine(centerX, centerY - 10, centerX, centerY + 10);
    
    // 현재 중심 좌표 표시
    dc.SetTextForeground(wxColour(0, 0, 0));
    dc.SetTextBackground(wxColour(255, 255, 255));
    wxString coordText = wxString::Format("중심: %.4f, %.4f", centerCoord_.lon, centerCoord_.lat);
    dc.DrawText(coordText, 10, size.GetHeight() - 30);
    
    // 줌 레벨 표시
    wxString zoomText = wxString::Format("줌: %d", zoomLevel_);
    dc.DrawText(zoomText, 10, size.GetHeight() - 50);
    
    // Mock 타일 정보
    dc.SetTextForeground(wxColour(100, 100, 100));
    dc.DrawText("💡 Mock 지도 - 실제로는 OSM 타일 이미지", 10, 10);
    
    // 4단계 정보 표시
    dc.SetTextForeground(wxColour(0, 150, 0));
    dc.DrawText("🆕 4단계: UI 알고리즘 적용됨 (HUD, 웨이포인트, 진행도)", 10, 30);
}

// ========================================
// 🆕 4단계: 향상된 경로 렌더링 (PolylineHighlight 알고리즘 적용)
// ========================================
void MapRenderPanel::RenderRouteAdvanced(wxDC& dc) {
    if (currentRoute_.size() < 2) return;
    
    // ========================================
    // 🎓 학습 포인트: 실제 네비게이션 앱의 경로 시각화
    // - 진행도에 따른 색상 변화
    // - 향상된 시각적 효과
    // - 성능 최적화된 렌더링
    // ========================================
    
    // 진행도 기반 렌더링 (30% 진행 시뮬레이션)
    double progress = 0.3;
    size_t progressIndex = static_cast<size_t>(progress * currentRoute_.size());
    
    // 완료된 구간 (초록색 굵은 선)
    dc.SetPen(wxPen(wxColour(0, 200, 0), 5));
    for (size_t i = 1; i <= progressIndex && i < currentRoute_.size(); ++i) {
        wxPoint startPoint = LatLonToScreen(currentRoute_[i-1]);
        wxPoint endPoint = LatLonToScreen(currentRoute_[i]);
        
        wxSize screenSize = GetSize();
        if (startPoint.x >= -50 && startPoint.x <= screenSize.GetWidth() + 50 &&
            startPoint.y >= -50 && startPoint.y <= screenSize.GetHeight() + 50) {
            dc.DrawLine(startPoint, endPoint);
        }
    }
    
    // 남은 구간 (회색 선)
    dc.SetPen(wxPen(wxColour(150, 150, 150), 3));
    for (size_t i = progressIndex + 1; i < currentRoute_.size(); ++i) {
        wxPoint startPoint = LatLonToScreen(currentRoute_[i-1]);
        wxPoint endPoint = LatLonToScreen(currentRoute_[i]);
        
        wxSize screenSize = GetSize();
        if (startPoint.x >= -50 && startPoint.x <= screenSize.GetWidth() + 50 &&
            startPoint.y >= -50 && startPoint.y <= screenSize.GetHeight() + 50) {
            dc.DrawLine(startPoint, endPoint);
        }
    }
    
    // 현재 위치 마커 (큰 파란색 원)
    if (progressIndex < currentRoute_.size()) {
        wxPoint currentPos = LatLonToScreen(currentRoute_[progressIndex]);
        dc.SetPen(wxPen(wxColour(0, 0, 255), 3));
        dc.SetBrush(wxBrush(wxColour(100, 150, 255)));
        dc.DrawCircle(currentPos, 8);
        
        // 현재 위치 텍스트
        dc.SetTextForeground(wxColour(0, 0, 255));
        dc.DrawText("📍 현재 위치", currentPos.x + 15, currentPos.y - 10);
    }
    
    // 경로점 마커들 (기존 코드 유지하되 스타일 개선)
    dc.SetPen(wxPen(wxColour(50, 50, 50), 2));
    
    for (size_t i = 0; i < currentRoute_.size(); ++i) {
        wxPoint point = LatLonToScreen(currentRoute_[i]);
        
        // 진행도에 따른 색상 변경
        if (i <= progressIndex) {
            dc.SetBrush(wxBrush(wxColour(100, 255, 100)));  // 완료: 밝은 녹색
        } else {
            dc.SetBrush(wxBrush(wxColour(220, 220, 220)));  // 미완료: 회색
        }
        
        dc.DrawCircle(point, 6);
        
        // 웨이포인트 번호
        dc.SetTextForeground(wxColour(0, 0, 0));
        dc.DrawText(wxString::Format("%zu", i+1), point.x + 10, point.y - 8);
    }
}

// ========================================
// 🆕 4단계: 실시간 UI 업데이트 구현
// ========================================

void MapRenderPanel::OnStartNavigation(wxCommandEvent& event) {
    if (isNavigating_) return;  // 이미 실행 중이면 무시
    
    isNavigating_ = true;
    currentProgress_ = 0.0;
    currentSpeed_ = 0.0;
    remainingDistance_ = 1200.0;
    
    // UI 상태 업데이트
    startNavBtn_->Enable(false);
    stopNavBtn_->Enable(true);
    
    // 타이머 시작
    updateTimer_->Start(500);   // 네비게이션 업데이트: 500ms마다
    animationTimer_->Start(16); // 애니메이션 업데이트: 60fps (16ms마다)
    
    debugFrame_->AppendLog("🚗 네비게이션 시뮬레이션 시작!");
    debugFrame_->AppendLog("[실시간 UI] 60fps 애니메이션으로 마커 움직임 시뮬레이션");
    
    UpdateHudDisplay();
}

void MapRenderPanel::OnStopNavigation(wxCommandEvent& event) {
    if (!isNavigating_) return;  // 실행 중이 아니면 무시
    
    isNavigating_ = false;
    updateTimer_->Stop();
    animationTimer_->Stop();
    
    // UI 상태 리셋
    startNavBtn_->Enable(true);
    stopNavBtn_->Enable(false);
    
    currentProgress_ = 0.0;
    currentSpeed_ = 0.0;
    remainingDistance_ = 1200.0;
    
    UpdateHudDisplay();
    UpdateProgressBar();
    
    debugFrame_->AppendLog("⏹️ 네비게이션 시뮬레이션 정지");
}

void MapRenderPanel::OnUpdateTimer(wxTimerEvent& event) {
    if (!isNavigating_) return;
    
    // ========================================
    // 🎯 핵심 학습: 실제 네비게이션 앱의 실시간 데이터 처리
    // - GPS 위치 업데이트 시뮬레이션
    // - 속도 및 거리 계산
    // - UI 컴포넌트 동기화
    // ========================================
    
    // 시뮬레이션: 진행도 증가 (500ms마다 1~2.5% 진행 = 초당 2~5%)
    double increment = (1.0 + (rand() % 15) * 0.1);  // 1.0-2.5% 랜덤
    currentProgress_ += increment;
    
    if (currentProgress_ >= 100.0) {
        currentProgress_ = 100.0;
        
        // 자동 정지: 직접 정지 로직 실행 (이벤트 객체 없이)
        isNavigating_ = false;
        updateTimer_->Stop();
        animationTimer_->Stop();
        startNavBtn_->Enable(true);
        stopNavBtn_->Enable(false);
        
        debugFrame_->AppendLog("🎉 목적지 도착! 네비게이션 완료");
        UpdateHudDisplay();
        UpdateProgressBar();
        return;
    }
    
    // 시뮬레이션: 속도 변화 (30-80 km/h 랜덤)
    currentSpeed_ = 30.0 + (rand() % 51);
    
    // 시뮬레이션: 남은 거리 감소
    remainingDistance_ = 1200.0 * (100.0 - currentProgress_) / 100.0;
    
    // ========================================
    // 🎯 MapPanel.h 구현 가이드: UI 클래스 활용 패턴
    // ✅ LocationPuck::InterpolateLocation() 및 CameraController::UpdateLocation() 활용
    // ❌ 직접 보간 로직 구현하지 말고 UI 클래스 메서드 사용
    // ========================================
    if (!currentRoute_.empty() && locationPuck_) {
        // ✅ UI 클래스 활용: 경로 상 현재 위치 계산
        LonLat currentPos = CalculateRoutePosition(currentProgress_ / 100.0);
        
        // ========================================
        // 🎯 MapPanel.h 구현 가이드: 방향 계산 로직
        // Types.h의 CalculateDistance() 함수 활용
        // ========================================
        double bearing = 0.0;
        bool hasBearing = false;
        
        // 현재 위치에서 다음 위치로의 방향을 계산
        double segmentProgress = currentProgress_ / 100.0 * (currentRoute_.size() - 1);
        size_t currentSegment = static_cast<size_t>(segmentProgress);
        
        if (currentSegment < currentRoute_.size() - 1) {
            bearing = CalculateBearing(currentRoute_[currentSegment], currentRoute_[currentSegment + 1]);
            hasBearing = true;
        }
        
        // ✅ LocationPuck 클래스 활용: LocationState 생성 및 위치 업데이트
        LocationState currentState(currentPos, 5.0, bearing, hasBearing, true); // 5m 정확도
        locationPuck_->UpdateLocation(currentState); // UI 클래스 메서드 호출
        
        // ✅ CameraController 클래스 활용: 카메라 follow 모드
        if (cameraController_) {
            cameraController_->UpdateLocation(currentState); // UI 클래스 메서드 호출
        }
        
        debugFrame_->AppendLog(wxString::Format("📍 UI 클래스 활용 위치 업데이트: %.6f, %.6f (%.1f%% 진행)", 
                                              currentPos.lon, currentPos.lat, currentProgress_));
    }
    
    // UI 업데이트
    UpdateHudDisplay();
    UpdateProgressBar();
    
    // 지도 다시 그리기 (경로 진행도 반영)
    Refresh();
    
    // 로그 출력 (사용자 요청 형식)
    wxDateTime now = wxDateTime::Now();
    wxString timeStr = now.Format("[%H:%M:%S]");
    
    debugFrame_->AppendLog(wxString::Format("%s ✅ 4단계 UI 컴포넌트들 업데이트 완료", timeStr));
    debugFrame_->AppendLog(wxString::Format("%s   - 웨이포인트 패널: 경로점 표시", timeStr));
    debugFrame_->AppendLog(wxString::Format("%s   - 진행도바: %.0f%% 진행", timeStr, currentProgress_));
    debugFrame_->AppendLog(wxString::Format("%s   - HUD 오버레이: 속도 %.0fkm/h, 남은 거리 %.0fm", 
                                          timeStr, currentSpeed_, remainingDistance_));
}

void MapRenderPanel::UpdateHudDisplay() {
    // HUD 상태 라벨 업데이트
    wxString status;
    if (isNavigating_) {
        status = wxString::Format("▶️ 네비게이션 진행중 | 속도: %.0f km/h | 남은 거리: %.0fm | 진행: %.1f%%", 
                                currentSpeed_, remainingDistance_, currentProgress_);
        hudStatusLabel_->SetForegroundColour(wxColour(0, 150, 0));  // 녹색
    } else {
        status = "⏸️ 네비게이션 대기중 | 속도: 0 km/h | 남은 거리: 1200m";
        hudStatusLabel_->SetForegroundColour(wxColour(100, 100, 100));  // 회색
    }
    
    hudStatusLabel_->SetLabel(status);
    
    // 실제 HUD 컴포넌트 업데이트
    if (hudOverlay_) {
        presentation::components::HudState hudState;
        hudState.visible = true;
        hudState.speed_kmh = currentSpeed_;
        hudState.distance_remain_m = remainingDistance_;
        hudOverlay_->SetState(hudState);
    }
}

void MapRenderPanel::UpdateProgressBar() {
    // 시각적 진행도 게이지 업데이트
    progressGauge_->SetValue(static_cast<int>(currentProgress_));
    
    // 진행도에 따른 색상 변경 (wxWidgets는 게이지 색상 변경이 제한적이므로 시뮬레이션)
    if (currentProgress_ < 30.0) {
        // 초기 단계: 기본 파란색
    } else if (currentProgress_ < 70.0) {
        // 중간 단계: 노란색 효과 (실제로는 색상 변경 어려움)
    } else {
        // 후반 단계: 녹색 효과
    }
    
    // 실제 NavigationProgressBar 컴포넌트 업데이트
    if (progressBar_) {
        presentation::components::NavigationProgress progress;
        progress.total_distance = 1200.0;
        progress.remaining_distance = remainingDistance_;
        progress.completion_ratio = currentProgress_ / 100.0;
        progressBar_->UpdateProgress(progress);
    }
}

void MapRenderPanel::OnAnimationTimer(wxTimerEvent& event) {
    // ========================================
    // 🎯 MapPanel.h 구현 가이드: 애니메이션 업데이트 패턴
    // ✅ LocationPuck::UpdateAnimation() 메서드 활용
    // 📌 60fps 고빈도 업데이트로 부드러운 애니메이션 구현
    // ========================================
    if (!isNavigating_) return;
    
    // ✅ UI 클래스 애니메이션 업데이트 (위치는 OnUpdateTimer에서 설정됨)
    if (locationPuck_) {
        locationPuck_->UpdateAnimation(); // UI 클래스 메서드 호출
    }
    
    // 📌 MapPanel에서는 이 패턴을 그대로 활용할 것
    Refresh(); // 지도 다시 그리기 (애니메이션만)
}