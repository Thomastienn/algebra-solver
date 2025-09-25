FROM python:3.12-slim-bullseye

WORKDIR /app

# Install build tools and cmake
RUN apt-get update && apt-get install -y \
    g++ \
    cmake \
    build-essential \
    git \
    && rm -rf /var/lib/apt/lists/*

RUN g++ --version
RUN cmake --version

# Install pybind11 via pip (for headers)
RUN pip install pybind11

# Copy C++ CAS source
COPY services/CAS/src cas/

# Copy CMakeLists.txt
COPY services/CAS/CMakeLists.txt .

# Build CAS module
RUN mkdir build && cd build && cmake .. && make

# Copy backend Python dependencies
COPY webserver/backend/requirements.txt backend_requirements.txt
RUN pip install -r backend_requirements.txt

# Copy backend source
COPY webserver/backend/src/ .

# Add build directory to PYTHONPATH so Python can import cas
ENV PYTHONPATH=/app/build:$PYTHONPATH

# Run backend
CMD ["sh", "-c", "uvicorn main:app --host 0.0.0.0 --port ${PORT:-8000}"]
