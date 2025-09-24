#include "../core/solver/EquationSolver.h"
#include "../core/solver/Simplifier.h"

class Tester: public Simplifier, public EquationSolver {
public:
    using Simplifier::combineLikeTerms;
    using Simplifier::distributeMinusUnaryInBinary;
    using Simplifier::distributeMultiplyBinary;
    using Simplifier::evaluateConstantBinary;
    using Simplifier::evaluateSpecialCases;
    using Simplifier::flattenNode;
    using Simplifier::reduceUnary;
    using Simplifier::seperateIntoUnary;
    using Simplifier::mergeBinaryWithRightUnary;

    using EquationSolver::isIsolated;
    using EquationSolver::reorderConstants;
};
