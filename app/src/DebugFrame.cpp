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
    
    auto* sizer = new wxBoxSizer(wxVERTICAL);
    auto* label = new wxStaticText(apiTestPanel_, wxID_ANY, 
                                  "🔍 2단계: API Test Panel\n(TmapClient, 경로 탐색 등)\n\n"
                                  "1단계 완료 후 구현 예정");
    label->SetFont(wxFont(14, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    label->SetForegroundColour(wxColour(100, 100, 100));
    sizer->Add(label, 0, wxALL, 20);
    
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