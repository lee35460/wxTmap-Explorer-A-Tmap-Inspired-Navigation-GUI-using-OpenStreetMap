#include <wx/wx.h>
#include "ProjectMap.h"  // 🗺️ 프로젝트 파일 지도 사용
#include PRESENTATION_APP_FRAME  // "presentation/views/frames/AppFrame.h"로 자동 확장
#include PRESENTATION_DEBUG_FRAME  // "presentation/views/frames/DebugFrame.h"로 자동 확장

class MyApp : public wxApp {
public:
    bool OnInit() override {
        // 커맨드라인 인자 확인
        bool debugMode = false;
        for (int i = 1; i < argc; ++i) {
            wxString arg = argv[i];
            if (arg == "--debug" || arg == "-D") {
                debugMode = true;
                break;
            }
        }
        
        if (debugMode) {
            // 디버그 모드
            auto* debugFrame = new DebugFrame();
            debugFrame->Show(true);
            SetTopWindow(debugFrame);
        } else {
            // 일반 모드
            auto* frame = new AppFrame();
            frame->Show(true);
            SetTopWindow(frame);
        }
        
        return true;
    }
};

wxIMPLEMENT_APP(MyApp);