# Student Management System

A responsive student-management dashboard built with React and Vite, backed by a C++17 REST API. Student records are stored in data/students.json.

## Features
- Dashboard statistics and student directory
- Search and department filtering
- Add, edit, view, and delete student records
- CSV export and responsive layout
- C++ REST API with JSON persistence

## Structure
- backend/CMakeLists.txt and backend/src/main.cpp
- frontend/index.html, package.json, vite.config.js, and src/
- data/students.json

## Requirements
Node.js 18+, CMake 3.20+, a C++17 compiler, and internet access for the first CMake configure.

## Run backend
```bash
cd backend
cmake -S . -B build
cmake --build build --config Release
./build/student_api
```
The API listens on port 8080. On Windows with Visual Studio, run build/Release/student_api.exe.

## Run frontend (second terminal)
```bash
cd frontend
npm install
npm run dev
```
Open http://localhost:5173.

## API
- GET /api/health
- GET /api/students
- POST /api/students
- PUT /api/students/:id
- DELETE /api/students/:id

Run the backend with its working directory set to backend; it reads and writes ../data/students.json.

Educational starter project: add authentication, authorization, backups, and production safeguards before using real student data.
