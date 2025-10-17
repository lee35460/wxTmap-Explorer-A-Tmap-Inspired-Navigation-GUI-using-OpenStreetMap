#include "RenderMetricsExporter.h"
#include <fstream>

void exportMetrics(const RenderPipeline& rp, const std::string& path) {
    std::ofstream out(path, std::ios::app);
    out << "fps_avg," << rp.fpsAverage() << "\n";
    out << "polyline_draw_ms," << rp.lastPolylineDrawMs() << "\n";
    out << "draw_calls," << rp.drawCalls() << "\n";
    out.close();
}

// 테스트 결과를 txt로 기록하는 함수 (예시)
void exportTestResults(const std::string& testName, bool passed, double duration, const std::string& path) {
    std::ofstream out(path, std::ios::app);
    out << "Test: " << testName << "\n";
    out << "Result: " << (passed ? "Passed" : "Failed") << "\n";
    out << "Duration: " << duration << " sec\n";
    out << "--------------------------\n";
    out.close();
}