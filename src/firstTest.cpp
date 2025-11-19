#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <vector>
#include <algorithm>
#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <locale>

#include "timekeeper.h"

namespace fs = std::filesystem;
using std::cout;
using std::endl;
using std::string;


static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

static void drop_files(GLFWwindow* window, int count, const char** paths) {
    DropFolder* df = static_cast<DropFolder*>(glfwGetWindowUserPointer(window));
    for (int i = 0; i < count; i++) {
        df->handle_dropped_file(paths[i]);
    }
}

int main(int argc, char** argv) {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return 1;

    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow* window = glfwCreateWindow(1280, 800, "TimeKeeper", nullptr, nullptr);
    if (window == nullptr) return 1;
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Data Containers
    DropFolder df;
    TimeKeeper* tk = new TimeKeeper(df);
    glfwSetWindowUserPointer(window, &df);
    
    // ALWAYS listen for drops
    glfwSetDropCallback(window, drop_files); 

    // ImGui Setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Modern Dark Style
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 5.0f;
    style.FrameRounding = 5.0f;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // App State
    int selectedFileIndex = -1;
    std::string currentRawText = "";
    std::string currentParsedResult = "No file processed yet.";
    std::string statusMessage = "Ready for files.";

    // --- EDITABLE BUFFERS ---
    static char editTitle[128] = "New Event";
    static char editDate[128] = "";
    static char editEndDate[128] = "";
    static char editDesc[1024] = "";

    // Main Loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // UI LAYOUT
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("MainDashboard", nullptr, 
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

        // Header
        ImGui::Text("TimeKeeper v1.0 - Extract Dates from Documents");
        ImGui::Separator();

        ImGui::Columns(2, "MainCols", true); 
        
        // --------------------------
        // LEFT COLUMN: File Queue
        // --------------------------
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "1. DRAG FILES HERE");
        
        ImGui::BeginChild("FileQueue", ImVec2(0, 300), true);
        if (df.getSize() == 0) {
            ImGui::TextDisabled("Drag and drop PDFs/Images here...");
        } else {
            for (int i = 0; i < df.getSize(); i++) {
                bool isSelected = (selectedFileIndex == i);
                if (ImGui::Selectable(df.getFileName(i).c_str(), isSelected)) {
                    selectedFileIndex = i;
                }
            }
        }
        ImGui::EndChild();

        if (ImGui::Button("Clear List")) {
            df = DropFolder();
            selectedFileIndex = -1;
            statusMessage = "List cleared.";
        }
        
        ImGui::SameLine();

        ImGui::BeginDisabled(df.getSize() == 0);
        if (ImGui::Button("PROCESS FILES", ImVec2(150, 0))) {
            statusMessage = "Processing...";
            
            if (tk != nullptr) delete tk;
            tk = new TimeKeeper(df); 
            
            if (selectedFileIndex >= 0 && selectedFileIndex < tk->extractedText.size()) {
                 currentRawText = tk->extractedText.at(selectedFileIndex);
                 
                 // ---------------------------------------------------------
                 // FIXED: Python Execution Was Missing Here
                 // ---------------------------------------------------------
                 // 1. Write temp file
                 std::ofstream out("temp_input.txt");
                 out << currentRawText;
                 out.close();

                 // 2. Run Python
                 // Use a full path if necessary, otherwise assumes date_parser.py is in the same folder
                 std::string cmd = "python date_parser.py \"temp_input.txt\""; 
                 currentParsedResult = tk->execPython(cmd.c_str()); 
                 // ---------------------------------------------------------

                 // 3. Reset buffers
                 strncpy(editTitle, "Scanned Event", sizeof(editTitle));
                 
                // 4. Parse Start Date (PRETTY)
                 std::string dateKey = "\"start_date_pretty\": \""; // CHANGED
                 size_t startPos = currentParsedResult.find(dateKey);
                 if (startPos != std::string::npos) {
                     startPos += dateKey.length();
                     size_t endPos = currentParsedResult.find("\"", startPos);
                     std::string foundDate = currentParsedResult.substr(startPos, endPos - startPos);
                     strncpy(editDate, foundDate.c_str(), sizeof(editDate));
                 } else {
                     strncpy(editDate, "", sizeof(editDate));
                 }

                 // 5. Parse End Date (PRETTY)
                 std::string endKey = "\"end_date_pretty\": \""; // CHANGED
                 size_t sPos2 = currentParsedResult.find(endKey);
                 if (sPos2 != std::string::npos) {
                     sPos2 += endKey.length();
                     size_t ePos2 = currentParsedResult.find("\"", sPos2);
                     std::string foundEnd = currentParsedResult.substr(sPos2, ePos2 - sPos2);
                     strncpy(editEndDate, foundEnd.c_str(), sizeof(editEndDate));
                 } else {
                     strncpy(editEndDate, "", sizeof(editEndDate));
                 }

                 // 6. Fill Description
                 strncpy(editDesc, currentRawText.substr(0, 1023).c_str(), sizeof(editDesc));
                 
                 statusMessage = "Done. You can now edit the details.";
            } else {
                statusMessage = "Error: Selected file not found.";
            }
        }
        ImGui::EndDisabled();

        ImGui::Spacing();
        ImGui::TextWrapped("Status: %s", statusMessage.c_str());

        ImGui::NextColumn(); 

        // --------------------------
        // RIGHT COLUMN: Results
        // --------------------------
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "2. RESULTS");
        
        if (ImGui::BeginTabBar("ResultTabs")) {
            
            // Tab 1: Raw Text
            if (ImGui::BeginTabItem("Extracted Text")) {
                ImGui::BeginChild("RawView", ImVec2(0, 0), true); 
                ImGui::TextWrapped("%s", currentRawText.empty() ? "No text extracted yet." : currentRawText.c_str());
                ImGui::EndChild();
                ImGui::EndTabItem();
            }

            // Tab 2: Smart Data (The Edit Form)
            if (ImGui::BeginTabItem("Smart Data (Edit Event)")) {
                ImGui::BeginChild("ParseView", ImVec2(0, -40), true); 
                
                ImGui::TextColored(ImVec4(1,1,0,1), "Event Details");
                ImGui::Text("Review and edit the information below before saving.");
                ImGui::Separator();
                ImGui::Spacing();


                ImGui::InputText("Event Title", editTitle, IM_ARRAYSIZE(editTitle));
                ImGui::InputText("Start (YYYYMMDDTHHMMSS)", editDate, IM_ARRAYSIZE(editDate));
                ImGui::InputText("End   (YYYYMMDDTHHMMSS)", editEndDate, IM_ARRAYSIZE(editEndDate));
                
                ImGui::Spacing();
                ImGui::Text("Description:");
                ImGui::InputTextMultiline("##desc", editDesc, IM_ARRAYSIZE(editDesc), ImVec2(-FLT_MIN, 150));

                ImGui::EndChild();

                // ACTION BUTTON
                if (ImGui::Button("CREATE RSVP (.ics)", ImVec2(200, 30))) {
                    
                    // Helper Lambda to convert "2025-10-28 17:00:00" -> "20251028T170000"
                    auto toICS = [](std::string input) -> std::string {
                        std::string digits = "";
                        for (char c : input) {
                            if (isdigit(c)) digits += c;
                        }
                        // If we have enough digits (YYYYMMDDHHMMSS), insert the T
                        if (digits.size() >= 14) {
                            return digits.substr(0, 8) + "T" + digits.substr(8, 6);
                        }
                        return digits; // Fallback or empty
                    };

                    // Find Desktop Path
                    std::string desktopPath;
                    const char* userProfile = std::getenv("USERPROFILE");
                    if (userProfile) desktopPath = std::string(userProfile) + "\\Desktop\\invite.ics";
                    else desktopPath = "invite.ics";

                    // CONVERT PRETTY UI STRINGS BACK TO ICS FORMAT
                    std::string sDate = toICS(std::string(editDate));
                    std::string eDate = toICS(std::string(editEndDate));
                    
                    tk->generateICS(desktopPath, editTitle, sDate, eDate, editDesc);
                    
                    statusMessage = "Saved to Desktop: " + desktopPath;
                    
                    #ifdef _WIN32
                    std::string openCmd = "start \"\" \"" + desktopPath + "\"";
                    system(openCmd.c_str());
                    #endif
                }
                ImGui::EndTabItem();
            }
            
            ImGui::EndTabBar();
        }

        ImGui::End(); 

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        ImVec4 clear_color = ImVec4(0.15f, 0.16f, 0.21f, 1.00f); 
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}