// Testing Base, just want to get file input

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

/*
class DropFolder {
    
    public:
    vector<fs::path> filePaths;
    int numFiles=0;

    

    
    void handle_dropped_file(const char*);
    int getNumFiles() {return numFiles;}
    int getSize() {return filePaths.size();}
    string getPath(int i) {return filePaths.at(i).string(); }
    string getFileName(int i) {return filePaths.at(i).filename().string(); }
    string getExtension(int i) {return filePaths.at(i).extension().string(); }
};
*/


// err callback
static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// handle dropped files, add to parameter vector if not already included
void DropFolder::handle_dropped_file(const char* name) {
    fs::path path(name);
    // fs::u8string()
    
    // Check that dropped folder is accepted filetype, if not, exit without including
    string fpext = path.extension().string();
    if (!(fpext==".txt" || fpext==".doc" || fpext==".docx" || fpext==".pdf" || fpext==".jpg" || fpext==".png" || fpext==".tiff"))
    {
        cout << "Invlaid file type: " << fpext;
        return;
    } 

    // search passed vector for passed path, if not present, push_back
    for (int i=0; i<filePaths.size(); i++)
    {
        if (filePaths.at(i)==path)
            return;
    } 
    cout << "Read " << name << " Made " << path.string() << endl;
    cout << "Parts: " << path.filename() << "  ext  " << path.extension() << endl;
    filePaths.push_back(path);
    numFiles += 1;
}
// file drop callback
static void drop_files(GLFWwindow* window, int count, const char** paths)
{
    DropFolder *dfp = static_cast<DropFolder*>(glfwGetWindowUserPointer(window));
    int i;
    for (i = 0;  i < count;  i++)
    {
        cout << "Before:" << endl;
        cout << (paths[i]) << endl;
        dfp->handle_dropped_file((paths[i]));
        cout << "After:" << endl;
    }
}

// clear color vec
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

// Begin main code loop
// Only one global atm, others are for func handles
int main(int argc, char** argv) {

    // for text to file IO, set locale and save name
    string default_loc = setlocale(LC_ALL, nullptr);

    // Vector of char arrays of filepaths, will be set to window user pointer
    DropFolder df;
    DropFolder *dfp = &df;
    // TimeKeeper object
    TimeKeeper *tk = NULL;
    // Direction drop fun handle;
    //GLFWdropfun fdrop = drop_files;
    
    // bools for use
    static bool show_demo_window = true;     // for reference and debug
    static bool fileDropOn = false;           // if in file drop state
    static bool haveFiles = false;          // turned on at first pushback of a file
    static bool displayFiles = false;       // option to display files, only if haveFiles is true
    static bool displayFileOpenTest = false; // open file testing window, only if haveFiles is true 
    static bool displayExtracted = false;   // open extracted file window, only if haveFiles is true

    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;
    
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // Create window with graphics context
    GLFWwindow *window = glfwCreateWindow(1280, 720, "TimeKeeper", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync


    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontDefault();
    

    // Setup Style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init(glsl_version);
 
    glfwSetWindowUserPointer(window, dfp);

    // Main loop begins
    while (!glfwWindowShouldClose(window)) {

        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();


        if (show_demo_window)
        {
            ImGui::ShowDemoWindow(&show_demo_window);
        }

        if (fileDropOn)
        {
            glfwSetDropCallback(window, drop_files);
        }
        else
        {
            glfwSetDropCallback(window, NULL);
        }

        {
            // Create the base window
            ImGui::Begin("TimeKeeper");

            ImGui::Text("Main page, please select how to upload.");
            ImGui::Text("haveFiles is currently set to %s.", haveFiles ? "true" : "false");
            ImGui::Text("fileDropOn is currently set to %s.", fileDropOn ? "true" : "false");
            ImGui::Text("displayFiles is currently set to %s.", displayFiles ? "true" : "false");
            if (ImGui::Button("Turn on file drop")) 
            {
                fileDropOn = true;
            }
            if (df.hasFiles())
            {
                ImGui::Checkbox("Allow file display", &displayFiles);
                if (ImGui::Button("Extract from current files"))
                {
                    fileDropOn = false;
                    glfwSetDropCallback(window, NULL);
                    if (tk==NULL){
                        tk = new TimeKeeper(df);
                    }
                    displayExtracted = true;
                }
            }
            ImGui::End();
        }

        if (fileDropOn)
        {
            if (ImGui::Begin("File_Drop", &fileDropOn)) 
            {
                ImGui::Text("Time to drag in some files!\nOh yeah!");
                if (df.hasFiles() && ImGui::TreeNode("Attached Files"))
                {
                    if (ImGui::BeginListBox("Files"))
                    {
                        for (int i=0; i<df.getSize(); i++)
                        {
                            ImGui::Text("File %d: %s", i+1, df.getFileName(i).c_str());
                        }
                        ImGui::EndListBox();
                    }
                    ImGui::TreePop();
                }
                if (ImGui::Button("Stop Dropping"))
                {
                    fileDropOn = false;
                }
                ImGui::End();
            }
            else
            {
                ImGui::End();
            }
        }

        if (df.hasFiles() && displayFiles)
        {
            ImGui::Begin("File_Open_Test", &displayFiles);
            ImGui::Text("Test whether open file works");

            static int fileSelected = 0;
            if (ImGui::BeginCombo("Open test", df.getFileName(fileSelected).c_str()))
            {
                for (int i=0; i<df.getSize(); i++)
                {
                    const bool is_selected = (fileSelected == i);
                    if (ImGui::Selectable(df.getFileName(i).c_str(), is_selected))
                    {
                        fileSelected = i;
                        displayFileOpenTest = false;
                    }
                }
                ImGui::EndCombo();
            }
            if (ImGui::Button("Test Opening Current File"))
            {
                std::fstream fop;
                fop.open(df.getPathName(fileSelected));
                if (fop.is_open())
                {
                    displayFileOpenTest = true;
                }
                else
                {
                    displayFileOpenTest = false;
                }
                fop.close();
            }
            ImGui::SameLine(); if (displayFileOpenTest) ImGui::Text("File successfully opened!");
            ImGui::End();
        }

        if (df.hasFiles() && displayExtracted)
        {
            
            {
                tk->ShowFileTexts(&displayExtracted);
            }
        }
        

        // Rendering
        // (Your code clears your framebuffer, renders your other stuff etc.)
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();


    // Test portion to write drag and drop payloads gained to file to check if it worked
    std::ofstream fout;
    fout.open("ReadPayloads.txt");
    if (fout.is_open())
    {
        fout << "Successfully got here" << endl;
        fout << "Number of filepaths: " << std::to_string(df.getNumFiles()) << endl;
        fout << "All values:" << endl;
        for (int i=0; i<df.getSize(); i++) 
        {
            fout << df.getPathName(i) << endl;
        }
        fout << endl << "That's all, folks!";
    }
    fout.close();

    return 0;

}