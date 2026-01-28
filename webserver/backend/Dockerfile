# Stage 1: Build the C++ CAS module
FROM python:3.12-slim-bullseye AS builder

WORKDIR /app

# Install only what's needed for building
RUN apt-get update && apt-get install -y --no-install-recommends \
    g++ \
    cmake \
    make \
    git \
    && rm -rf /var/lib/apt/lists/*

# Install pybind11 for C++ binding headers
RUN pip install --no-cache-dir pybind11

# Copy C++ CAS source and build it
COPY services/CAS/src src/
COPY services/CAS/CMakeLists.txt .

# Build CAS module
RUN mkdir build && cd build && cmake .. && make

# Stage 2: Install Python dependencies
FROM python:3.12-slim-bullseye AS deps

WORKDIR /app

# Copy and install Python dependencies
COPY webserver/backend/requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# Stage 3: Final minimal runtime image
FROM python:3.12-slim-bullseye

WORKDIR /app

# Copy pre-built CAS module from builder
COPY --from=builder /app/build/cas*.so /app/build/
COPY --from=builder /app/build/libalgebra_core.so /app/build/

# Copy installed Python packages from deps stage
COPY --from=deps /usr/local/lib/python3.12/site-packages /usr/local/lib/python3.12/site-packages
COPY --from=deps /usr/local/bin /usr/local/bin

# Copy backend source
COPY webserver/backend/src/ .

# Add build directory to PYTHONPATH and LD_LIBRARY_PATH
ENV PYTHONPATH=/app/build
ENV LD_LIBRARY_PATH=/app/build

# Pre-compile Python bytecode for faster startup
RUN python -m compileall -q .

# Run backend
CMD ["sh", "-c", "uvicorn main:app --host 0.0.0.0 --port ${PORT:-8000}"]
