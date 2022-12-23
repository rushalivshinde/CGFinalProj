#include <windows.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <GL/glext.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include "InitShader.h"    //Functions for loading shaders from text files
#include "LoadMesh.h"      //Functions for creating OpenGL buffers from mesh files
#include "LoadTexture.h"   //Functions for creating OpenGL textures from image files


static const std::string vertex_shader("FP_vs.glsl");
static const std::string fragment_shader("FP_fs.glsl");
GLuint shader_program = -1;

static const std::string quad_name = "quad.obj";
static const std::string mesh_name = "Amago0.obj";

const int num_textures = 11;

static const std::string shader_tex[num_textures] = { "tex_10.jpg", "tex_01.jpg", "tex_02.jpg", "tex_03.jpg", "tex_04.jpg", "tex_05.jpg", "tex_06.jpg", "tex_07.jpg", "tex_08.jpg", "tex_09.jpg", "tex_11.jpg"};

GLuint texture_id[num_textures] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}; //Texture map for mesh
MeshData mesh_data;
MeshData quad_data;


bool use_quats = true;
bool auto_rotate = true;


//for M
float angle = 0.0f;

bool texChecker = true;

//for V
float cam_angle = 30.0f;
glm::vec3 cam_pos = glm::vec3(0.0f, 0.5f, 2.0f);

namespace Uniforms
{
   glm::mat4 PV; 
   glm::vec4 Lw(-2.0f, 1.0f, 0.8f, 1.0);
   glm::vec4 kd;
   glm::vec4 Ld;
   glm::vec4 ka;
   glm::vec4 La;
   glm::vec4 ks;
   glm::vec4 Ls;
};

namespace UniformLocs
{
   int PV;
   int M;
   int kd;
   int tex[11];
   int draw_id; //draw_id = 0 for mesh, 1 for quad, 2 for shadow
   int time;
   int Lw;
   int texCheck;
   int selectedTex;
   float texScale, ki= 2.0, ka = 0.1f, ke = 0.5f;
};

float aspectRatio = 1.0f;
float texture_scale = 4.799f;
float diff_int = 2.0f, amb_int = 0.2f, smooth_int = 0.1f;


glm::vec4 groundPlane(0.0f, 1.0f, 0.0f, +0.5f);

glm::vec4 kd = glm::vec4(0.0f, 0.3f, 0.57f, 1.0f);
glm::vec4 Ld = glm::vec4(0);
glm::vec4 ka = glm::vec4(0.0, 0.5, 0.50, 1.0);
glm::vec4 La = glm::vec4(0.0f, 0.32f, 0.57f, 1.0f);


void draw_scene();

glm::mat4 ShadowMatrix(glm::vec4 Lw, glm::vec4 plane)//The shadow object
{
   const glm::mat4 I(1.0f);
   return glm::dot(plane, Lw) * I - glm::outerProduct(Lw, plane);
}

glm::mat4 Mplane(glm::vec4 plane)//The seabed
{
   //Transforms plane through origin with n=0.0,0.0,1.0 into given plane
   glm::vec3 n = glm::vec3(plane);
   n = glm::normalize(n);
   //glm::mat4 R = RotateVecToVec(glm::vec3(0.0f, 0.0f, 1.0f), n);
   glm::mat4 R = glm::rotate(-90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
   glm::mat4 T = glm::translate(-plane.w * n);
   glm::mat4 S = glm::scale(glm::vec3(3*quad_data.mScaleFactor));
   return T * R * S;
}

void draw_gui(GLFWwindow* window)
{
   //Begin ImGui Frame
   ImGui_ImplOpenGL3_NewFrame();
   ImGui_ImplGlfw_NewFrame();
   ImGui::NewFrame();

   int w, h;
   glfwGetFramebufferSize(window, &w, &h);
   glViewport(0, 0, w, h);
   aspectRatio = (float)w / h;

   //Draw Gui
   ImGui::Begin("Final Project");                       

   ImGui::SliderFloat("mesh angle", &angle, -180.0f, +180.0f);
   ImGui::SliderFloat("cam angle", &cam_angle, -180.0f, +180.0f);
   ImGui::SliderFloat3("cam pos", &cam_pos.x, -5.0f, +5.0f);

   if (ImGui::SliderFloat3("light pos", &Uniforms::Lw.x, -10.0f, +10.0f))
   {
       glUniform4fv(UniformLocs::Lw, 1, glm::value_ptr(Uniforms::Lw));
   }
   ImGui::SliderFloat("Diffuse Intensity", &diff_int, 0.0f, +5.0f);
   glUniform1f(UniformLocs::ki, diff_int);
   ImGui::SliderFloat("Ambient Intensity", &amb_int, 0.0f, 1.0f);
   glUniform1f(UniformLocs::ka, amb_int);
   ImGui::SliderFloat("Smoothness", &smooth_int, 0.0f, +5.0f);
   glUniform1f(UniformLocs::ke, smooth_int);
   ImGui::SliderFloat("Shader Scale", &texture_scale, 0.1f, 5.0f);
   glUniform1f(UniformLocs::texScale, texture_scale);

   static int selected = 4;
   for (int n = 0; n < num_textures; n++)
   {
       char buf[11];
       sprintf(buf, "Texture %d", n);
       if (ImGui::Selectable(buf, selected == n))
           selected = n;
   }
   glUniform1i(UniformLocs::selectedTex, selected);

   
   ImGui::Checkbox("Enable Texture?",&texChecker);
   glUniform1i(UniformLocs::texCheck,texChecker);
 
   ImGui::End();

   static bool show_test = false;
   if(show_test)
   {
      ImGui::ShowDemoWindow(&show_test);
   }

   //End ImGui Frame
   ImGui::Render();
   ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void draw_scene()
{
   //draw_id = 0 for mesh, 1 for quad, 2 for shadow
 
   //draw mesh
   glUniform1i(UniformLocs::draw_id, 1);
   glm::mat4 Mmesh = glm::translate(glm::vec3(0.0f, -0.25f, 0.0f))*glm::rotate(angle, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::vec3(mesh_data.mScaleFactor));
   glUniformMatrix4fv(UniformLocs::M, 1, false, glm::value_ptr(Mmesh));
   glBindVertexArray(mesh_data.mVao);
   mesh_data.DrawMesh();

   //Setup stencil for ground shadows
   int ground_stencil_value = 200;
   int stencil_mask = -1;
   glEnable(GL_STENCIL_TEST);
   glStencilFuncSeparate(GL_FRONT_AND_BACK, GL_ALWAYS, ground_stencil_value, stencil_mask); //always pass the stencil test
   glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_REPLACE); //replace stencil value with 255 when depth test pass

   //draw ground to color and stencil buffers
   glUniform1i(UniformLocs::draw_id, 0);
   glm::mat4 Mground = Mplane(groundPlane);
   glUniformMatrix4fv(UniformLocs::M, 1, false, glm::value_ptr(Mground));
   glUniform4f(UniformLocs::kd, 0.5f, 0.5f, 0.5f, 1.0f);
   glBindVertexArray(quad_data.mVao);
   quad_data.DrawMesh();

   //draw shadow where stencil value = shadow stencil value
   glStencilFuncSeparate(GL_FRONT_AND_BACK, GL_EQUAL, ground_stencil_value, stencil_mask);
   glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_KEEP); //don't change stencil values
   glUniform1i(UniformLocs::draw_id, 2);
   glEnable(GL_POLYGON_OFFSET_FILL);
   glPolygonOffset(-1.0, -2.0);
   glm::mat4 Shad = ShadowMatrix(Uniforms::Lw, groundPlane);
   glm::mat4 Mshad = Shad * Mmesh;
   glUniformMatrix4fv(UniformLocs::M, 1, false, glm::value_ptr(Mshad));
   glUniform4f(UniformLocs::kd, 0.1,0.1,0.1, 1.0f);
   glBindVertexArray(mesh_data.mVao);
   mesh_data.DrawMesh();
   glDisable(GL_POLYGON_OFFSET_FILL);

   glDisable(GL_STENCIL_TEST);
}


// This function gets called every time the scene gets redisplayed
void display(GLFWwindow* window)
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

   glm::vec3 we = glm::mat3(glm::rotate(cam_angle, glm::vec3(0.0f, 1.0f, 0.0f))) * cam_pos; //world-space eye position

   glm::mat4 V = glm::lookAt(we, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
   glm::mat4 P = glm::perspective(40.0f, aspectRatio, 0.1f, 100.0f);


   glUseProgram(shader_program);
   
   Uniforms::PV = P * V;
   glUniformMatrix4fv(UniformLocs::PV, 1, false, glm::value_ptr(Uniforms::PV));
  

   for (int i = 0; i < num_textures; i++)
   {
       glActiveTexture(GL_TEXTURE0+i);
       glBindTexture(GL_TEXTURE_2D, texture_id[i]);
       glUniform1i(UniformLocs::tex[i], i);
   }

   draw_scene();

   draw_gui(window);

   glfwSwapBuffers(window);
}

void idle()
{
   float time_sec = static_cast<float>(glfwGetTime());
   glUniform1f(UniformLocs::time, time_sec);
}

void reload_shader()
{
   GLuint new_shader = InitShader(vertex_shader.c_str(), fragment_shader.c_str());

   if (new_shader == -1) // loading failed
   {
      glClearColor(1.0f, 0.0f, 1.0f, 0.0f); //change clear color if shader can't be compiled
   }
   else
   {
      glClearColor(64.0f/255.0f, 64.0f / 255.0f, 64.0f / 255.0f, 0.0f);

      if (shader_program != -1)
      {
         glDeleteProgram(shader_program);
      }
      shader_program = new_shader;

      //glUniform linking with shader code
      UniformLocs::PV = glGetUniformLocation(shader_program, "PV");
      UniformLocs::M = glGetUniformLocation(shader_program, "M");
      UniformLocs::time = glGetUniformLocation(shader_program, "time");
      UniformLocs::draw_id = glGetUniformLocation(shader_program, "draw_id");
      UniformLocs::ka = glGetUniformLocation(shader_program, "ka");
      UniformLocs::kd = glGetUniformLocation(shader_program, "kd");
      UniformLocs::ke = glGetUniformLocation(shader_program, "ke");
      UniformLocs::ki = glGetUniformLocation(shader_program, "ki");
      UniformLocs::tex[0] = glGetUniformLocation(shader_program, "diffuse_tex[0]");
      UniformLocs::tex[1] = glGetUniformLocation(shader_program, "diffuse_tex[1]");
      UniformLocs::tex[2] = glGetUniformLocation(shader_program, "diffuse_tex[2]");
      UniformLocs::tex[3] = glGetUniformLocation(shader_program, "diffuse_tex[3]");
      UniformLocs::tex[4] = glGetUniformLocation(shader_program, "diffuse_tex[4]");
      UniformLocs::tex[5] = glGetUniformLocation(shader_program, "diffuse_tex[5]");
      UniformLocs::tex[6] = glGetUniformLocation(shader_program, "diffuse_tex[6]");
      UniformLocs::tex[7] = glGetUniformLocation(shader_program, "diffuse_tex[7]");
      UniformLocs::tex[8] = glGetUniformLocation(shader_program, "diffuse_tex[8]");
      UniformLocs::tex[9] = glGetUniformLocation(shader_program, "diffuse_tex[9]");
      UniformLocs::tex[10] = glGetUniformLocation(shader_program, "diffuse_tex[10]");
      UniformLocs::Lw = glGetUniformLocation(shader_program, "Lw");
      UniformLocs::texCheck = glGetUniformLocation(shader_program, "texCheck");
      UniformLocs::texScale = glGetUniformLocation(shader_program, "texScale");
      UniformLocs::selectedTex = glGetUniformLocation(shader_program, "selectedTex");
   }
}

//This function gets called when a key is pressed
void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
   std::cout << "key : " << key << ", " << char(key) << ", scancode: " << scancode << ", action: " << action << ", mods: " << mods << std::endl;

   if(action == GLFW_PRESS)
   {
      switch(key)
      {
         case 'r':
         case 'R':
            reload_shader();     
         break;

         case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
         break;     
      }
   }
}

void mouse_cursor(GLFWwindow* window, double x, double y)
{
    //std::cout << "cursor pos: " << x << ", " << y << std::endl;
}

void mouse_button(GLFWwindow* window, int button, int action, int mods)
{
    //std::cout << "button : "<< button << ", action: " << action << ", mods: " << mods << std::endl;
}

void initOpenGL()
{
   glewInit();

   glEnable(GL_DEPTH_TEST);


   reload_shader();
   mesh_data = LoadMesh(mesh_name);
   for (int i = 0; i < num_textures; i++)
   {
       texture_id[i] = LoadTexture(shader_tex[i]);
   }

   quad_data = LoadMesh(quad_name);
}



int main(void)
{
   GLFWwindow* window;

   if (!glfwInit())
   {
      return -1;
   }

   //request stencil buffer
   //glfwWindowHint(GLFW_STENCIL_BITS, 0); //to disable stencil
   glfwWindowHint(GLFW_STENCIL_BITS, 8);  //request 8 bit stencil buffer

   window = glfwCreateWindow(1024, 1024, "CS 535 - Final Project", NULL, NULL);
   if (!window)
   {
      glfwTerminate();
      return -1;
   }

   //Register callback functions with glfw. 
   glfwSetKeyCallback(window, keyboard);
   glfwSetCursorPosCallback(window, mouse_cursor);
   glfwSetMouseButtonCallback(window, mouse_button);

   /* Make the window's context current */
   glfwMakeContextCurrent(window);

   initOpenGL();
   
   //Init ImGui
   IMGUI_CHECKVERSION();
   ImGui::CreateContext();
   ImGui_ImplGlfw_InitForOpenGL(window, true);
   ImGui_ImplOpenGL3_Init("#version 150");

   /* Loop until the user closes the window */
   while (!glfwWindowShouldClose(window))
   {
      idle();
      display(window);

      /* Poll for and process events */
      glfwPollEvents();
   }

    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

   glfwTerminate();
   return 0;
}