/**
 * @file include/common/timer.h
 * @author work(@baidu.com)
 * @date 2018/06/11 17:48:53
 * @version $Revision$ 
 * @brief 
 *  
 **/
#pragma once
#include <memory>
#include <unordered_map>
#include "glog/logging.h"
#include "butil/time.h"
#include "bvar/latency_recorder.h"

namespace paddle {
namespace ps {
    
    struct CostProfilerNode {
        std::shared_ptr<bvar::LatencyRecorder> recorder;
    };
    
    class CostProfiler {
    public:
        ~CostProfiler() {}
        static CostProfiler& instance() {
            static CostProfiler profiler;
            return profiler;
        }

        void register_profiler(const std::string& label) {
            if (_cost_profiler_map.find(label) != _cost_profiler_map.end()) {
                return;
            }
            auto profiler_node = std::make_shared<CostProfilerNode>();
            profiler_node->recorder.reset(new bvar::LatencyRecorder("cost_profiler", label));
            _cost_profiler_map[label] = profiler_node; 
        }
        
        CostProfilerNode* profiler(const std::string& label) {
            auto itr = _cost_profiler_map.find(label);
            if (itr != _cost_profiler_map.end()) {
                return itr->second.get();
            }
            return NULL;
        }
        
    private:
        CostProfiler() {}
        std::unordered_map<std::string, std::shared_ptr<CostProfilerNode>> _cost_profiler_map;
    };
    
    class CostTimer {
    public:
        CostTimer(const std::string& label) {
            _label = label;
            auto& profiler = CostProfiler::instance();
            _profiler_node = profiler.profiler(label);
            //如果不在profiler中，则使用log输出耗时信息
            _is_print_cost = _profiler_node == NULL;
            _start_time_ms = butil::gettimeofday_ms();
            
        }
        CostTimer(CostProfilerNode& profiler_node) {
            _is_print_cost = false;
            _profiler_node = &profiler_node;
            _start_time_ms = butil::gettimeofday_ms();
        }
        ~CostTimer() {
            if (_is_print_cost) {
                LOG(INFO) << "CostTimer label:" << _label 
                    << ", cost:" << butil::gettimeofday_ms() - _start_time_ms << "ms";
            } else {
                *(_profiler_node->recorder) << butil::gettimeofday_ms() - _start_time_ms;
            }
        }

    private:
        std::string _label;
        bool _is_print_cost;
        uint64_t _start_time_ms;
        CostProfilerNode* _profiler_node;
    };
}
}
/* vim: set ts=4 sw=4 sts=4 tw=100 */
