#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <string>
#include <memory>
#include <variant>
#include <vector>
#include <functional> // For std::function
#include <array>      // For std::array
#include <algorithm>  // For std::find_if

#include "model/Node.hpp" // Include our data model

#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

// --- Application Modes ---
enum class AppMode {
    TREE,
    NORMAL, // Viewing an entry, fields are read-only, can navigate between fields
    EDIT    // Editing a field within an entry
};

static AppMode current_app_mode = AppMode::TREE;
static int focused_field_index = 0; // For NORMAL/EDIT mode: 0=title, 1=username, etc.

// Global variables to store the selected node and entry for interaction
static ArcaneLock::Node* g_selected_node = nullptr; // Points to a Node within children vector (never to root_folder itself)
static ArcaneLock::Entry* g_selected_entry = nullptr; // Pointer to the selected Entry
static ArcaneLock::Folder* g_selected_folder = nullptr; // Pointer to the selected Folder (can be root_folder)

// Global root folder for the password database
static std::unique_ptr<ArcaneLock::Folder> g_root_folder;

// Function to create some mock data for the tree view
std::unique_ptr<ArcaneLock::Folder> CreateMockData() {
    auto root = std::make_unique<ArcaneLock::Folder>();
    root->name = "Root";

    ArcaneLock::Folder work_folder;
    work_folder.name = "Work";
    work_folder.children.push_back(std::make_unique<ArcaneLock::Node>(
        ArcaneLock::Entry{"SSH Key", "user", "password123", "", ""}));
    work_folder.children.push_back(std::make_unique<ArcaneLock::Node>(
        ArcaneLock::Entry{"Database", "admin", "db_pass", "", ""}));

    ArcaneLock::Folder personal_folder;
    personal_folder.name = "Personal";
    personal_folder.children.push_back(std::make_unique<ArcaneLock::Node>(
        ArcaneLock::Entry{"Email", "my.email@example.com", "my_pass", "", ""}));

    root->children.push_back(
        std::make_unique<ArcaneLock::Node>(std::move(work_folder)));
    root->children.push_back(
        std::make_unique<ArcaneLock::Node>(std::move(personal_folder)));
    root->children.push_back(std::make_unique<ArcaneLock::Node>(
        ArcaneLock::Entry{"Root Entry", "root_user", "root_pass", "", ""}));

    return root;
}

// Function to add a new folder to the currently selected folder
void AddNewFolder(ArcaneLock::Folder* parent_folder) {
    if (!parent_folder) return;
    ArcaneLock::Folder new_folder;
    new_folder.name = "New Folder " + std::to_string(parent_folder->children.size() + 1);
    parent_folder->children.push_back(std::make_unique<ArcaneLock::Node>(std::move(new_folder)));
}

// Function to add a new entry to the currently selected folder
void AddNewEntry(ArcaneLock::Folder* parent_folder) {
    if (!parent_folder) return;
    ArcaneLock::Entry new_entry;
    new_entry.title = "New Entry " + std::to_string(parent_folder->children.size() + 1);
    parent_folder->children.push_back(std::make_unique<ArcaneLock::Node>(std::move(new_entry)));
}


// Recursive function to draw tree items and handle selection logic
void DrawSelectableTreeItem(ArcaneLock::Folder* current_drawn_parent_folder, ArcaneLock::Node* node_in_folder, int& current_item_idx, int& selected_item_idx) {
    std::visit([&](auto& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, ArcaneLock::Folder>) {
            ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
            
            bool is_selected = (current_item_idx == selected_item_idx);
            if (is_selected) {
                node_flags |= ImGuiTreeNodeFlags_Selected;
                g_selected_node = node_in_folder; 
                g_selected_entry = nullptr;
                g_selected_folder = &arg;
            }

            if (arg.is_open) node_flags |= ImGuiTreeNodeFlags_DefaultOpen;

            ImGui::PushID(node_in_folder); 
            bool node_open = ImGui::TreeNodeEx((void*)(intptr_t)current_item_idx, node_flags, "📁 %s", arg.name.c_str());
            ImGui::PopID();

            if (node_open != arg.is_open) { // Update folder's open state if user clicked arrow
                arg.is_open = node_open;
            }

            current_item_idx++;

            if (node_open) {
                for (auto& child : arg.children) {
                    DrawSelectableTreeItem(&arg, child.get(), current_item_idx, selected_item_idx);
                }
                ImGui::TreePop();
            }
        } else if constexpr (std::is_same_v<T, ArcaneLock::Entry>) {
            ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanAvailWidth;
            bool is_selected = (current_item_idx == selected_item_idx);
            if (is_selected) {
                node_flags |= ImGuiTreeNodeFlags_Selected;
                g_selected_node = node_in_folder; 
                g_selected_entry = &arg;
                g_selected_folder = nullptr;
            }
            ImGui::PushID(node_in_folder);
            ImGui::TreeNodeEx((void*)(intptr_t)current_item_idx, node_flags, "📄 %s", arg.title.c_str());
            ImGui::PopID();
            current_item_idx++;
        }
    }, *node_in_folder);
}

// Function to draw the details of a selected entry
void DrawEntryDetails(ArcaneLock::Entry* entry, AppMode current_mode) {
    if (!entry) {
        ImGui::Text("Select a password entry to view its details.");
        return;
    }

    // InputText needs char* buffer. For prototyping, we use temporary buffers.
    // In a real app, Entry struct might hold fixed-size char arrays or custom string widgets.
    std::array<char, 256> title_buffer;
    strncpy(title_buffer.data(), entry->title.c_str(), title_buffer.size());
    title_buffer[title_buffer.size() - 1] = '\0';

    std::array<char, 256> username_buffer;
    strncpy(username_buffer.data(), entry->username.c_str(), username_buffer.size());
    username_buffer[username_buffer.size() - 1] = '\0';

    std::array<char, 256> password_buffer;
    strncpy(password_buffer.data(), entry->password.c_str(), password_buffer.size());
    password_buffer[password_buffer.size() - 1] = '\0';

    std::array<char, 256> url_buffer;
    strncpy(url_buffer.data(), entry->url.c_str(), url_buffer.size());
    url_buffer[url_buffer.size() - 1] = '\0';

    std::array<char, 1024> notes_buffer; // Larger buffer for notes
    strncpy(notes_buffer.data(), entry->notes.c_str(), notes_buffer.size());
    notes_buffer[notes_buffer.size() - 1] = '\0';

    ImGuiInputTextFlags input_flags = (current_mode == AppMode::EDIT) ? 0 : ImGuiInputTextFlags_ReadOnly;

    // Title
    ImGui::Text("Title:");
    if (focused_field_index == 0 && current_mode == AppMode::EDIT) ImGui::SetKeyboardFocusHere();
    if (ImGui::InputText("##Title", title_buffer.data(), title_buffer.size(), input_flags)) {
        entry->title = title_buffer.data();
    }
    if (ImGui::IsItemDeactivatedAfterEdit() && current_mode == AppMode::EDIT) current_app_mode = AppMode::NORMAL;


    // Username
    ImGui::Text("Username:");
    if (focused_field_index == 1 && current_mode == AppMode::EDIT) ImGui::SetKeyboardFocusHere();
    if (ImGui::InputText("##Username", username_buffer.data(), username_buffer.size(), input_flags)) {
        entry->username = username_buffer.data();
    }
    if (ImGui::IsItemDeactivatedAfterEdit() && current_mode == AppMode::EDIT) current_app_mode = AppMode::NORMAL;


    // Password
    ImGui::Text("Password:");
    if (focused_field_index == 2 && current_mode == AppMode::EDIT) ImGui::SetKeyboardFocusHere();
    if (ImGui::InputText("##Password", password_buffer.data(), password_buffer.size(), input_flags | ImGuiInputTextFlags_Password)) {
        entry->password = password_buffer.data();
    }
    if (ImGui::IsItemDeactivatedAfterEdit() && current_mode == AppMode::EDIT) current_app_mode = AppMode::NORMAL;

    // URL
    ImGui::Text("URL:");
    if (focused_field_index == 3 && current_mode == AppMode::EDIT) ImGui::SetKeyboardFocusHere();
    if (ImGui::InputText("##URL", url_buffer.data(), url_buffer.size(), input_flags)) {
        entry->url = url_buffer.data();
    }
    if (ImGui::IsItemDeactivatedAfterEdit() && current_mode == AppMode::EDIT) current_app_mode = AppMode::NORMAL;

    // Notes
    ImGui::Text("Notes:");
    if (focused_field_index == 4 && current_mode == AppMode::EDIT) ImGui::SetKeyboardFocusHere();
    if (ImGui::InputTextMultiline("##Notes", notes_buffer.data(), notes_buffer.size(), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 8), input_flags)) {
        entry->notes = notes_buffer.data();
    }
    if (ImGui::IsItemDeactivatedAfterEdit() && current_mode == AppMode::EDIT) current_app_mode = AppMode::NORMAL;
}


static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

// Function to calculate total items (visible items, including root)
int CalculateTotalVisibleItems(ArcaneLock::Folder* folder) {
    if (!folder) return 0;
    int count = 1; // Count the folder itself
    if (folder->is_open) { // Only count children if folder is open
        for (const auto& child_ptr : folder->children) {
            std::visit([&](auto& arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, ArcaneLock::Folder>) {
                    count += CalculateTotalVisibleItems(&arg);
                } else if constexpr (std::is_same_v<T, ArcaneLock::Entry>) {
                    count++;
                }
            }, *child_ptr);
        }
    }
    return count;
}


int main(int, char**) {
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "ArcaneLock", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Apply TUI-like style
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.FrameRounding = 0.0f;
    style.GrabRounding = 0.0f;
    style.WindowRounding = 0.0f;
    style.ChildRounding = 0.0f;
    style.ScrollbarRounding = 0.0f;
    style.TabRounding = 0.0f;
    style.PopupRounding = 0.0f;
    style.WindowBorderSize = 0.0f; // No window borders
    style.ChildBorderSize = 0.0f;  // No child borders
    style.PopupBorderSize = 0.0f;  // No popup borders
    style.FrameBorderSize = 1.0f; // Simple frame border for input fields, etc.
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.00f); // Dark background
    style.Colors[ImGuiCol_Header] = ImVec4(0.2f, 0.2f, 0.2f, 1.00f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.3f, 0.3f, 0.3f, 1.00f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.4f, 0.4f, 0.4f, 1.00f);
    style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.8f, 0.6f, 0.0f, 1.00f); // Highlight color
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.3f, 0.3f, 0.3f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.4f, 0.4f, 0.4f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.5f, 0.5f, 0.5f, 1.00f);

    // Load a monospaced font
    io.Fonts->AddFontDefault(); // Fallback, could load a specific one later
    // ImFont* font = io.Fonts->AddFontFromFileTTF("path/to/your/monospace_font.ttf", 16.0f);
    // if (font) io.FontDefault = font;

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Our state
    ImVec4 clear_color = ImVec4(0.1f, 0.1f, 0.1f, 1.00f);

    g_root_folder = CreateMockData();
    int selected_item_index = 0; // Index of the currently selected item in the tree (for navigation) 
    

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // --- Keyboard Input Handling ---
        
        // Always allow 'q' to quit
        if (ImGui::IsKeyPressed(ImGuiKey_Q)) { 
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        switch (current_app_mode) {
            case AppMode::TREE: {
                int total_visible_items = CalculateTotalVisibleItems(g_root_folder.get());
                
                // Handle J/K (move selection)
                if (ImGui::IsKeyPressed(ImGuiKey_DownArrow) || ImGui::IsKeyPressed(ImGuiKey_J, false) && !ImGui::GetIO().KeyShift) {
                    selected_item_index++;
                }
                if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) || ImGui::IsKeyPressed(ImGuiKey_K, false) && !ImGui::GetIO().KeyShift) {
                    selected_item_index--;
                }

                // Handle A/a (add new item)
                if (ImGui::IsKeyPressed(ImGuiKey_A, false)) { // 'a' or 'A'
                    ArcaneLock::Folder* target_folder = g_selected_folder ? g_selected_folder : g_root_folder.get();
                    if (target_folder) {
                        if (ImGui::GetIO().KeyShift) { // Shift+A for Folder
                            AddNewFolder(target_folder);
                        } else { // 'a' for Entry
                            AddNewEntry(target_folder);
                        }
                        // After adding, recalculate total items and potentially adjust selected_item_index
                        total_visible_items = CalculateTotalVisibleItems(g_root_folder.get()); 
                        selected_item_index = total_visible_items - 1; // Select the newly added item
                    }
                }

                // Handle H/L (collapse/expand) - already handled directly in DrawSelectableTreeItem based on is_selected
                // But if they are for global operations:

                if (ImGui::IsKeyPressed(ImGuiKey_I)) { // 'i' to enter NORMAL mode if an entry is selected
                    if (g_selected_entry != nullptr) {
                        current_app_mode = AppMode::NORMAL;
                        focused_field_index = 0; // Focus on the first field (Title)
                    }
                }

                // Clamp selection index within bounds
                if (selected_item_index < 0) selected_item_index = total_visible_items - 1;
                if (selected_item_index >= total_visible_items) selected_item_index = 0;
                break;
            }
            case AppMode::NORMAL: {
                if (ImGui::IsKeyPressed(ImGuiKey_Escape)) { // ESC to return to TREE mode
                    current_app_mode = AppMode::TREE;
                }
                if (ImGui::IsKeyPressed(ImGuiKey_DownArrow) || ImGui::IsKeyPressed(ImGuiKey_J, false) && !ImGui::GetIO().KeyShift) {
                    focused_field_index++;
                    if (focused_field_index > 4) focused_field_index = 0; // Cycle through fields (0-4)
                }
                if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) || ImGui::IsKeyPressed(ImGuiKey_K, false) && !ImGui::GetIO().KeyShift) {
                    focused_field_index--;
                    if (focused_field_index < 0) focused_field_index = 4; // Cycle through fields (0-4)
                }
                if (ImGui::IsKeyPressed(ImGuiKey_I)) { // 'i' to enter EDIT mode
                    current_app_mode = AppMode::EDIT;
                }
                break;
            }
            case AppMode::EDIT: {
                // Check for Escape only if not currently editing an ImGui::InputText.
                // ImGui::InputText itself will handle Escape for its own purposes (e.g., losing focus).
                // We want a *second* Escape to return to NORMAL mode if already out of direct text edit.
                if (ImGui::IsKeyPressed(ImGuiKey_Escape) && !ImGui::IsAnyItemActive()) { 
                    current_app_mode = AppMode::NORMAL;
                    ImGui::SetWindowFocus(nullptr); // Clear focus for any ImGui window
                }
                // Typing is handled by ImGui::InputText directly
                break;
            }
        }
        
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Set the main window to fill the entire application window
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
        ImGui::Begin("ArcaneLockMain", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
        ImGui::PopStyleVar(); // Pop WindowPadding

        // Render the tree view (left pane)
        ImGui::BeginChild("TreeView", ImVec2(ImGui::GetWindowWidth() * 0.3f, 0), true); // 30% width for tree
            ImGui::Text("ArcaneLock (Mode: %s)", 
                        current_app_mode == AppMode::TREE ? "TREE" : 
                        (current_app_mode == AppMode::NORMAL ? "NORMAL" : "EDIT"));
            ImGui::Separator();
            
            int current_item_idx = 0; // Correctly declared and initialized here
            g_selected_entry = nullptr; // Reset selected entry before drawing
            g_selected_node = nullptr;
            g_selected_folder = nullptr; // Reset selected folder before drawing

            // --- Draw Root Folder ---
            bool is_root_selected = (current_item_idx == selected_item_index);
            if (is_root_selected) {
                g_selected_folder = g_root_folder.get();
            }
            ImGui::Text("📁 %s", g_root_folder->name.c_str());
            current_item_idx++;

            // --- Draw Children ---
            if (g_root_folder->is_open) { // Only draw children if root is open
                for (auto& child : g_root_folder->children) {
                    DrawSelectableTreeItem(g_root_folder.get(), child.get(), current_item_idx, selected_item_index);
                }
            }
        ImGui::EndChild();

        ImGui::SameLine();

        // Render the entry details (right pane)
        ImGui::BeginChild("EntryDetails", ImVec2(0, 0), true); // Remaining width
            DrawEntryDetails(g_selected_entry, current_app_mode);
        ImGui::EndChild();

        ImGui::End();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
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