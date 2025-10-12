#include "DebugFrame.h"
#include "Types.h"           // 새로 만든 Types.h (LonLat, CoordinateStats 등)
#include <wx/sizer.h>
#include <wx/splitter.h>     // wxSplitterWindow 사용
#include <wx/msgdlg.h>       // wxMessageBox 사용
#include <sstream>
#include <iomanip>
#include <algorithm>         // std::min, std::max, std::count_if 사용
#include <cmath>             // M_PI, std::sin, std::cos 등 수학 함수
#include <memory>            // unique_ptr 사용

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
    renderTestPanel_ = new wxPanel(notebook_);
    
    auto* sizer = new wxBoxSizer(wxVERTICAL);
    auto* label = new wxStaticText(renderTestPanel_, wxID_ANY, 
                                  "🎨 3단계: Render Test Panel\n(RenderPipeline, HUD 등)\n\n"
                                  "1-2단계 완료 후 구현 예정");
    label->SetFont(wxFont(14, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    label->SetForegroundColour(wxColour(100, 100, 100));
    sizer->Add(label, 0, wxALL, 20);
    
    renderTestPanel_->SetSizer(sizer);
    notebook_->AddPage(renderTestPanel_, "🎨 3단계: Render Test");
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