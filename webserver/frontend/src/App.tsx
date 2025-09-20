import { useState } from 'react'
import './App.css'

function App() {
  const [activeTab, setActiveTab] = useState<'simplify' | 'evaluate'>('simplify')
  const [simplifyInput, setSimplifyInput] = useState('')
  const [simplifyResult, setSimplifyResult] = useState('')
  const [evaluateInput, setEvaluateInput] = useState('')
  const [evaluateResult, setEvaluateResult] = useState('')

  const handleSimplify = () => {
    // TODO: Call backend API
    setSimplifyResult(`Simplified: ${simplifyInput}`)
  }

  const handleEvaluate = () => {
    // TODO: Call backend API
    setEvaluateResult(`Result: ${evaluateInput}`)
  }

  return (
    <div className="app">
      <header className="header">
        <h1>Algebra Solver</h1>
        <p>Simplify equations and evaluate expressions</p>
      </header>

      <div className="tabs">
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
        {activeTab === 'simplify' && (
          <div className="feature">
            <div className="input-section">
              <label htmlFor="simplify-input">Enter equation to simplify:</label>
              <textarea
                id="simplify-input"
                value={simplifyInput}
                onChange={(e) => setSimplifyInput(e.target.value)}
                placeholder="e.g., 2 + 3 * (4 - 1) - 4 * (a - 2)"
                className="input textarea"
                rows={3}
              />
              <button onClick={handleSimplify} className="button">
                Simplify
              </button>
            </div>
            {simplifyResult && (
              <div className="result">
                <h3>Result:</h3>
                <div className="result-content">{simplifyResult}</div>
              </div>
            )}
          </div>
        )}

        {activeTab === 'evaluate' && (
          <div className="feature">
            <div className="input-section">
              <label htmlFor="evaluate-input">Enter expression to evaluate (with variable assignments):</label>
              <textarea
                id="evaluate-input"
                value={evaluateInput}
                onChange={(e) => setEvaluateInput(e.target.value)}
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
                <p>You can include variable assignments in the same expression. Each line can be either:</p>
                <ul>
                  <li>A variable assignment: <code>x = 10</code></li>
                  <li>An expression to evaluate: <code>3 + x * (1 - 2)</code></li>
                </ul>
              </div>
              <button onClick={handleEvaluate} className="button">
                Evaluate
              </button>
            </div>
            {evaluateResult && (
              <div className="result">
                <h3>Result:</h3>
                <div className="result-content">{evaluateResult}</div>
              </div>
            )}
          </div>
        )}
      </main>
    </div>
  )
}

export default App
