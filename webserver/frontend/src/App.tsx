import { useState, useEffect } from 'react';
import './App.css';

interface SimplifyRequest {
  expression: string;
}
interface SimplifyResponse {
  simplified: string;
}

interface SystemSolveRequest {
  equations: string[];
  variable: string;
}
interface SystemSolveResponse {
  result: string;
}

function App() {
  const [activeTab, setActiveTab] = useState<
    'simplify' | 'evaluate' | 'solve'
  >('solve');
  const [simplifyInput, setSimplifyInput] = useState('');
  const [simplifyResult, setSimplifyResult] = useState('');
  const [simplifyError, setSimplifyError] = useState('');
  const [evaluateInput, setEvaluateInput] = useState('');
  const [evaluateResult, setEvaluateResult] = useState('');
  const [evaluateError, setEvaluateError] = useState('');
  const [systemInput, setSystemInput] = useState('');
  const [systemVariable, setSystemVariable] = useState('x');
  const [systemResult, setSystemResult] = useState('');
  const [systemError, setSystemError] = useState('');

  const [isLoading, setIsLoading] = useState(false);

  useEffect(() => {
    // Warm up the backend server
    fetch('https://algebra-solver.onrender.com/')
  }, []);

  const handleSimplify = async () => {
    setIsLoading(true);
    setSimplifyError('');
    setSimplifyResult('');
    
    try {
      const response = await fetch(
        'https://algebra-solver.onrender.com/simplify',
        {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({
            expression: simplifyInput,
          } as SimplifyRequest),
        },
      );
      
      if (!response.ok) {
        throw new Error(`HTTP error! status: ${response.status}`);
      }
      
      const data: SimplifyResponse = await response.json();
      setSimplifyResult(`Simplified: ${data.simplified}`);
    } catch (error) {
      setSimplifyError(`Error: ${error instanceof Error ? error.message : 'Unknown error occurred'}`);
    } finally {
      setIsLoading(false);
    }
  };

  const handleEvaluate = async () => {
    setIsLoading(true);
    setEvaluateError('');
    setEvaluateResult('');
    
    try {
      // TODO: Call backend API when endpoint is available
      // For now, simulate a delay and show placeholder result
      await new Promise(resolve => setTimeout(resolve, 1000));
      setEvaluateResult(`Result: ${evaluateInput}`);
    } catch (error) {
      setEvaluateError(`Error: ${error instanceof Error ? error.message : 'Unknown error occurred'}`);
    } finally {
      setIsLoading(false);
    }
  };

  const parseEquations = (input: string): string[] => {
    return input
      .split('\n')
      .map(line => line.trim())
      .filter(line => line.length > 0);
  };

  const handleSystemSolve = async () => {
    setIsLoading(true);
    setSystemError('');
    setSystemResult('');
    
    try {
      const equations = parseEquations(systemInput);
      
      if (equations.length === 0) {
        throw new Error('Please enter at least one equation');
      }
      
      if (!systemVariable.trim()) {
        throw new Error('Please specify a variable to solve for');
      }
      
      const response = await fetch(
        'https://algebra-solver.onrender.com/solve-system',
        {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({
            equations: equations,
            variable: systemVariable,
          } as SystemSolveRequest),
        },
      );
      
      if (!response.ok) {
        throw new Error(`HTTP error! status: ${response.status}`);
      }
      
      const data: SystemSolveResponse = await response.json();
      setSystemResult(data.result);
    } catch (error) {
      setSystemError(`Error: ${error instanceof Error ? error.message : 'Unknown error occurred'}`);
    } finally {
      setIsLoading(false);
    }
  };

  return (
    <div className="app">
      <header className="header">
        <h1>Algebra Solver</h1>
        <p>Simplify equations and evaluate expressions</p>
      </header>

      <div className="tabs">
        <button
          className={`tab ${activeTab === 'solve' ? 'active' : ''}`}
          onClick={() => setActiveTab('solve')}
        >
          Solve System of Equations
        </button>
        <button
          className={`tab ${activeTab === 'simplify' ? 'active' : ''}`}
          onClick={() => setActiveTab('simplify')}
        >
          Simplify Equation
        </button>
        <button
          className={`tab ${activeTab === 'evaluate' ? 'active' : ''}`}
          onClick={() => setActiveTab('evaluate')}
        >
          Evaluate Expression
        </button>
      </div>

      <main className="main">
        {activeTab === 'solve' && (
          <div className="feature">
            <div className="input-section">
              <label htmlFor="system-input">
                Enter system of equations (one equation per line):
              </label>
              <textarea
                id="system-input"
                value={systemInput}
                onChange={(e) => setSystemInput(e.target.value)}
                placeholder={`x + a = b * c
a = b + 2
c = 3
b = 4`}
                className="input textarea"
                rows={6}
              />
              <div style={{ display: 'flex', alignItems: 'center', gap: '1rem', marginBottom: '1rem' }}>
                <label htmlFor="system-variable" style={{ marginBottom: 0, minWidth: '120px' }}>
                  Solve for variable:
                </label>
                <input
                  id="system-variable"
                  type="text"
                  value={systemVariable}
                  onChange={(e) => setSystemVariable(e.target.value)}
                  placeholder="x"
                  className="input"
                  style={{ width: '80px', marginBottom: 0 }}
                />
              </div>
              <div className="help-text">
                <p>
                  Enter each equation on a separate line, then specify which variable to solve for. Examples:
                </p>
                <ul>
                  <li>
                    Linear equations: <code>x + y = 5</code>
                  </li>
                  <li>
                    Variable assignments: <code>a = 3</code>
                  </li>
                  <li>
                    Complex expressions: <code>x + a = b * c</code>
                  </li>
                </ul>
                <p>
                  <strong>Note:</strong> The system will solve for the specified variable using all provided equations.
                </p>
              </div>
              <button
                onClick={handleSystemSolve}
                className={`button ${isLoading ? 'loading' : ''}`}
                disabled={isLoading}
              >
                {isLoading ? 'Solving...' : 'Solve System'}
              </button>
            </div>
            {systemError && (
              <div className="result error">
                <h3>Error:</h3>
                <div className="result-content">
                  {systemError}
                </div>
              </div>
            )}
            {systemResult && (
              <div className="result">
                <h3>Result:</h3>
                <div className="result-content">
                  {systemResult}
                </div>
              </div>
            )}
          </div>
        )}

        {activeTab === 'simplify' && (
          <div className="feature">
            <div className="input-section">
              <label htmlFor="simplify-input">
                Enter equation to simplify:
              </label>
              <textarea
                id="simplify-input"
                value={simplifyInput}
                onChange={(e) =>
                  setSimplifyInput(e.target.value)
                }
                placeholder="e.g., 2 + 3 * (4 - 1) - 4 * (a - 2)"
                className="input textarea"
                rows={3}
              />
              <button
                onClick={handleSimplify}
                className={`button ${isLoading ? 'loading' : ''}`}
                disabled={isLoading}
              >
                {isLoading ? 'Simplifying...' : 'Simplify'}
              </button>
            </div>
            {simplifyError && (
              <div className="result error">
                <h3>Error:</h3>
                <div className="result-content">
                  {simplifyError}
                </div>
              </div>
            )}
            {simplifyResult && (
              <div className="result">
                <h3>Result:</h3>
                <div className="result-content">
                  {simplifyResult}
                </div>
              </div>
            )}
          </div>
        )}

        {activeTab === 'evaluate' && (
          <div className="feature">
            <div className="input-section">
              <label htmlFor="evaluate-input">
                Enter expression to evaluate (with variable
                assignments):
              </label>
              <textarea
                id="evaluate-input"
                value={evaluateInput}
                onChange={(e) =>
                  setEvaluateInput(e.target.value)
                }
                placeholder={`e.g., 
x = 10
3 + x * (1 - 2)

or

a = 5
b = 3
2 * a + b - 1`}
                className="input textarea"
                rows={6}
              />
              <div className="help-text">
                <p>
                  You can include variable assignments in the
                  same expression. Each line can be either:
                </p>
                <ul>
                  <li>
                    A variable assignment:{' '}
                    <code>x = 10</code>
                  </li>
                  <li>
                    An expression to evaluate:{' '}
                    <code>3 + x * (1 - 2)</code>
                  </li>
                </ul>
              </div>
              <button 
                onClick={handleEvaluate} 
                className={`button ${isLoading ? 'loading' : ''}`}
                disabled={isLoading}
              >
                {isLoading ? 'Evaluating...' : 'Evaluate'}
              </button>
            </div>
            {evaluateError && (
              <div className="result error">
                <h3>Error:</h3>
                <div className="result-content">
                  {evaluateError}
                </div>
              </div>
            )}
            {evaluateResult && (
              <div className="result">
                <h3>Result:</h3>
                <div className="result-content">
                  {evaluateResult}
                </div>
              </div>
            )}
          </div>
        )}
      </main>
    </div>
  );
}

export default App;
