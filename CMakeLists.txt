cmake_minimum_required(VERSION 4.0)
project(FileDropOCRTest VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20 )
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package( Tesseract CONFIG REQUIRED )
find_package( Leptonica CONFIG REQUIRED )
find_package( glfw3 CONFIG REQUIRED )
find_package( imgui CONFIG REQUIRED )
find_package( OpenGL REQUIRED )
find_package( Python3 COMPONENTS Interpreter Development REQUIRED)
find_package( nanobind CONFIG REQUIRED )
find_package( duckx CONFIG REQUIRED )
find_package(PkgConfig)
pkg_check_modules(POPPLER_CPP REQUIRED IMPORTED_TARGET poppler-cpp)

execute_process(
   COMMAND "${Python3_EXECUTABLE}" -m venv "${CMAKE_CURRENT_BINARY_DIR}/venv"
   RESULT_VARIABLE VENV_CREATE_RESULT
   OUTPUT_VARIABLE VENV_CREATE_OUTPUT
   ERROR_VARIABLE VENV_CREATE_ERROR
)

if (NOT VENV_CREATE_RESULT EQUAL 0)
   message(FATAL_ERROR "Failed to create virtual environment: ${VENV_CREATE_ERROR}")
endif()

# Define paths to the virtual environment's python and pip
if(CMAKE_HOST_WIN32)  # Windows uses different location
   set(VENV_PYTHON "${CMAKE_CURRENT_BINARY_DIR}/venv/Scripts/python.exe")
   set(VENV_PIP "${CMAKE_CURRENT_BINARY_DIR}/venv/Scripts/pip.exe")
else()  # Linux/macOS
   set(VENV_PYTHON "${CMAKE_CURRENT_BINARY_DIR}/venv/bin/python") 
   set(VENV_PIP "${CMAKE_CURRENT_BINARY_DIR}/venv/bin/pip") # Linux/macOS
endif()

# Install packages using pip within the virtual environment
execute_process(
   COMMAND "${VENV_PIP}" install --upgrade -r "${CMAKE_CURRENT_SOURCE_DIR}/requirements.txt"
   RESULT_VARIABLE PIP_INSTALL_RESULT
   OUTPUT_VARIABLE PIP_INSTALL_OUTPUT
   ERROR_VARIABLE PIP_INSTALL_ERROR
)

if (NOT PIP_INSTALL_RESULT EQUAL 0)
   message(FATAL_ERROR "Failed to install Python packages: ${PIP_INSTALL_ERROR}")
endif()


include(CTest)
enable_testing()


file(DOWNLOAD "https://github.com/tesseract-ocr/tessdata_best/raw/refs/heads/main/eng.traineddata" "${CMAKE_BINARY_DIR}/tessdata/eng.traineddata")

add_executable( FileDropOCRTest )

set_target_properties(FileDropOCRTest PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

target_include_directories( FileDropOCRTest PRIVATE ${PROJECT_SOURCE_DIR}/include )

target_link_libraries(FileDropOCRTest PRIVATE glfw Tesseract::libtesseract leptonica imgui::imgui duckx::duckx PkgConfig::POPPLER_CPP)

if(CMAKE_HOST_WIN32)
   target_link_libraries(FileDropOCRTest PRIVATE opengl32 )
else()
  target_link_libraries(FileDropOCRTest PRIVATE OpenGL::GL )
endif()


target_sources( FileDropOCRTest
  PRIVATE
    src/firstTest.cpp
    src/timekeeper.cpp
    src/ExtractPDFText.cpp 
)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
