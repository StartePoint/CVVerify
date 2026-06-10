#include "core/pipeline/PipelineEngine.h"

#include <utility>

void PipelineEngine::setSteps(std::vector<std::shared_ptr<IPipelineStep>> steps)
{
    m_steps = std::move(steps);
}

StepResult PipelineEngine::run(FramePacket& frame, const RunContext& context) const
{
    for (const auto& step : m_steps) {
        StepResult result = step->execute(frame, context);
        if (!result.success) {
            return result;
        }
    }

    return {};
}
