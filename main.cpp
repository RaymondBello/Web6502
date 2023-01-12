
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#define GLFW_INCLUDE_ES3
#include <GLES3/gl3.h>
#include <GLFW/glfw3.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <iostream>
#include <stdlib.h>
#include <stdio.h>

#include "config.h"
#include "Bus.h"
#include "R6502.h"

Bus nes;
std::map<uint16_t, std::string> mapAsm;

GLFWwindow* g_window;
ImVec4 clear_color = ImVec4(1.0f, 1.0f, 0.60f, 1.00f);
bool show_demo_window = false;
bool show_another_window = false;
bool show_r6502_window = false;

int g_width;
int g_height;

unsigned char *pixel_data = new unsigned char[100 * 100 * 4];
const char *list_of_scaling_alogritms[] = {"Native 256x240", "512x480", "1024x960"};
static int scaling_algoritm_current_index = 0; // Here we store our selection data as an index.

// for (int y = 0; y < 100; ++y){
//   for (int x = 0; x < 100; x++)
//   {
//     pixel_data[y * 100 + x * 3] = 0xff;
//     pixel_data[y * 100 + x * 3 + 1] = 0x00;
//     pixel_data[y * 100 + x * 3 + 2] = 0x00;
//   }
// }

std::string hex(uint32_t n, uint8_t d)
{
    std::string s(d, '0');
    for (int i = d - 1; i >= 0; i--, n >>= 4)
        s[i] = "0123456789ABCDEF"[n & 0xF];
    return s;
};

EM_JS(int, canvas_get_width, (), {
  return Module.canvas.width;
});

EM_JS(int, canvas_get_height, (), {
  return Module.canvas.height;
});

EM_JS(void, resizeCanvas, (), {
  js_resizeCanvas();
});

void on_size_changed()
{
  glfwSetWindowSize(g_window, g_width, g_height);

  ImGui::SetCurrentContext(ImGui::GetCurrentContext());
}

void show_menubar()
{
  static ImGuiComboFlags scaling_algo_flags = 0;
  const char *combo_scaling_preview = list_of_scaling_alogritms[scaling_algoritm_current_index]; // Pass in the preview value visible before opening the combo (it could be anything)

  if (ImGui::BeginMainMenuBar())
  {
    if (ImGui::BeginMenu("Settings"))
    {
      if (ImGui::BeginCombo("Select Resolution", combo_scaling_preview, scaling_algo_flags))
      {
        for (int n = 0; n < IM_ARRAYSIZE(list_of_scaling_alogritms); n++)
        {
          const bool is_selected = (scaling_algoritm_current_index == n);
          if (ImGui::Selectable(list_of_scaling_alogritms[n], is_selected))
            scaling_algoritm_current_index = n;

          // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
          if (is_selected)
            ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }

      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Windows")){
      ImGui::Checkbox("NES Emulator", &show_r6502_window);
      ImGui::Checkbox("Demo Window", &show_demo_window); // Edit bools storing our windows open/close state
      ImGui::EndMenu();
    }

    ImGui::Separator();

    ImGui::Text("%s", list_of_scaling_alogritms[scaling_algoritm_current_index]);

    ImGui::Separator();

    if (ImGui::Button("Clock CPU"))
    {
        nes.cpu.clock();
        ImGui::LogText("Clocked CPU");
    }

    ImGui::EndMainMenuBar();
  }
}

void loop()
{
  int width = canvas_get_width();
  int height = canvas_get_height();

  if (width != g_width || height != g_height)
  {
    g_width = width;
    g_height = height;
    on_size_changed();
  }

  glfwPollEvents();

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();



  // 1. Show a simple window.
  // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets automatically appears in a window called "Debug".
  {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

    ImGui::Begin("NES Emulator", &show_r6502_window,  ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus);

    show_menubar();

    
    ImGuiIO io = ImGui::GetIO();
    ImTextureID my_tex_id = io.Fonts->TexID;
    float my_tex_w = (float)io.Fonts->TexWidth;
    float my_tex_h = (float)io.Fonts->TexHeight;

    // ImGui::SetCursorPos((ImGui::GetWindowSize() - ImVec2(my_tex_w, my_tex_h)) * 0.5f);
    ImGui::SetCursorPos(ImGui::GetCursorPos() + (ImGui::GetContentRegionAvail() - ImVec2(my_tex_w, my_tex_h-100)) * 0.5f);
    {
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImVec2 uv_min = ImVec2(0.0f, 0.0f);                 // Top-left
        ImVec2 uv_max = ImVec2(1.0f, 1.0f);                 // Lower-right
        ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
        ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white
        ImGui::Image(my_tex_id, ImVec2(my_tex_w, my_tex_h), uv_min, uv_max, tint_col, border_col);
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            float region_sz = 32.0f;
            float region_x = io.MousePos.x - pos.x - region_sz * 0.5f;
            float region_y = io.MousePos.y - pos.y - region_sz * 0.5f;
            float zoom = 4.0f;
            if (region_x < 0.0f)
            {
                region_x = 0.0f;
            }
            else if (region_x > my_tex_w - region_sz)
            {
                region_x = my_tex_w - region_sz;
            }
            if (region_y < 0.0f)
            {
                region_y = 0.0f;
            }
            else if (region_y > my_tex_h - region_sz)
            {
                region_y = my_tex_h - region_sz;
            }
            ImGui::Text("Min: (%.2f, %.2f)", region_x, region_y);
            ImGui::Text("Max: (%.2f, %.2f)", region_x + region_sz, region_y + region_sz);
            ImVec2 uv0 = ImVec2((region_x) / my_tex_w, (region_y) / my_tex_h);
            ImVec2 uv1 = ImVec2((region_x + region_sz) / my_tex_w, (region_y + region_sz) / my_tex_h);
            ImGui::Image(my_tex_id, ImVec2(region_sz * zoom, region_sz * zoom), uv0, uv1, tint_col, border_col);
            ImGui::EndTooltip();
      }
    }

    ImGui::End();
  }

  //std::cout << "2nd window" << std::endl;

  // 2. Show another simple window. In most cases you will use an explicit Begin/End pair to name your windows.
  if (show_r6502_window)
  {
    ImGui::SetNextWindowPos(ImVec2(30, 550), ImGuiCond_FirstUseEver);
    ImGui::Begin("NES Debugger", &show_r6502_window);
    ImGui::Text("720 x 720 Dynamic Texture");
    ImGui::End();
  }

  // 3. Show the ImGui demo window. Most of the sample code is in ImGui::ShowDemoWindow(). Read its code to learn more about Dear ImGui!
  if (show_demo_window)
  {
      ImGui::SetNextWindowPos(ImVec2(400, 20), ImGuiCond_FirstUseEver); // Normally user code doesn't need/want to call this because positions are saved in .ini file anyway. Here we just want to make the demo initial state a bit more friendly!
      ImGui::ShowDemoWindow(&show_demo_window);
  }

  ImGui::Render();

  int display_w, display_h;
  glfwMakeContextCurrent(g_window);
  // glfwGetFramebufferSize(g_window, &display_w, &display_h);
  // glViewport(0, 0, display_w, display_h);
  // glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
  // glDrawPixels()
  // glClear(GL_COLOR_BUFFER_BIT);
  

  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  glfwMakeContextCurrent(g_window);
}


int init_gl()
{
  if( !glfwInit() )
  {
      fprintf( stderr, "Failed to initialize GLFW\n" );
      return 1;
  }

  //glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL

  // Open a window and create its OpenGL context
  int canvasWidth = 800;
  int canvasHeight = 600;

  g_window = glfwCreateWindow(canvasWidth, canvasHeight, "R6502 Emulator", NULL, NULL);
  if( g_window == NULL )
  {
      fprintf( stderr, "Failed to open GLFW window.\n" );
      glfwTerminate();
      return -1;
  }
  glfwMakeContextCurrent(g_window); // Initialize GLEW

  return 0;
}


int init_imgui()
{
  // Setup Dear ImGui binding
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui_ImplGlfw_InitForOpenGL(g_window, true);
  ImGui_ImplOpenGL3_Init();

  // Setup style
  ImGui::StyleColorsDark();

  ImGuiIO& io = ImGui::GetIO();

  // Load Fonts
  io.Fonts->AddFontFromFileTTF("data/SF-Pro.otf", 23.0f);
  io.Fonts->AddFontFromFileTTF("data/xkcd-script.ttf", 23.0f);
  io.Fonts->AddFontFromFileTTF("data/xkcd-script.ttf", 18.0f);
  io.Fonts->AddFontFromFileTTF("data/xkcd-script.ttf", 26.0f);
  io.Fonts->AddFontFromFileTTF("data/xkcd-script.ttf", 32.0f);
  io.Fonts->AddFontDefault();

  resizeCanvas();

  return 0;
}


int init()
{
  init_gl();
  init_imgui();
  return 0;
}


void quit()
{
  glfwTerminate();
}


extern "C" int main(int argc, char** argv)
{
  if (init() != 0) return 1;

  #ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(loop, 0, 1);
  #endif

  quit();

  return 0;
}
