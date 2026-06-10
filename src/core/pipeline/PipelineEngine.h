#pragma once

#include <memory>
#include <vector>

#include "core/pipeline/IPipelineStep.h"

class PipelineEngine
{
public:
    void setSteps(std::vector<std::shared_ptr<IPipelineStep>> steps);
    StepResult run(FramePacket& frame, const RunContext& context) const;

private:
    std::vector<std::shared_ptr<IPipelineStep>> m_steps;
};
