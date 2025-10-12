#include <wx/wx.h>
#include "AppFrame.h"
#include "DebugFrame.h"  // 새로 추가

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