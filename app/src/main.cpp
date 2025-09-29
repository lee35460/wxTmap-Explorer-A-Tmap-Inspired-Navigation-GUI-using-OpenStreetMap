#include <wx/wx.h>
#include "AppFrame.h"

class MyApp : public wxApp {
public:
    bool OnInit() override {
        auto* frame = new AppFrame();
        frame->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(MyApp); 