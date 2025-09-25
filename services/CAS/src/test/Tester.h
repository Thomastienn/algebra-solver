#include "../core/solver/EquationSolver.h"
#include "../core/solver/Simplifier.h"
#include "../core/solver/Isolator.h"

class Tester: public Simplifier, public EquationSolver, public Isolator {
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

    using Isolator::transferAdditives;
    using Isolator::isolateVariable;
};
