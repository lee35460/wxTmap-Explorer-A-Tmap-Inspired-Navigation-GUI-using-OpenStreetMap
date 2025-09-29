#include "render/RenderMetricsExporter.h"
#include <fstream>

void exportMetrics(const RenderPipeline& rp, const std::string& path) {
    std::ofstream out(path, std::ios::app);
    out << "fps_avg," << rp.fpsAverage() << "\n";
    out.close();
}