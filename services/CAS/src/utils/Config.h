#pragma once

class Config {
    
public:
    static const int MAX_ITERATIONS_EXECUTE_STEPS = 100;
    static const int MAX_ITERATIONS_CONVERGE_SOLVE = 1000;
    static const int MAX_ITERATIONS_WITHOUT_IMPROVEMENT = 100;
    static constexpr float LIMIT_RATIO_NEW_DISTINCT_VARS = 1.2;
};
