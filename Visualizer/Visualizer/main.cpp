/* Lab 5 base code - transforms using local matrix functions 
   to be written by students - 
   CPE 471 Cal Poly Z. Wood + S. Sueda
*/
#include <iostream>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds

#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "Shape.h"
#include "Audio3.hpp"

#define START_ANGLE 0
#define SPEC_SIZE 1025
#define SPEC_SAMPLES 8043

using namespace std;
using namespace Eigen;

GLFWwindow *window; // Main application window
string RESOURCE_DIR = "../../../resources/"; // Where the resources are loaded from
string AUDIO_INPUT = "";
string AUDIO_ANALYSIS = "";
shared_ptr<Program> prog;
shared_ptr<Program> prog2;
shared_ptr<Shape> shape;

int g_width, g_height;
float sTheta;
float size = 0.5;
sf::Music music;
float g_seconds = 1;
float spectrumBuff[SPEC_SIZE * SPEC_SAMPLES * 3];
vector<essentia::Real> volume;

//global data for ground plane
GLuint SpecBuffObj;

static void error_callback(int error, const char *description)
{
    cerr << description << endl;
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) 
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}


static void mouse_callback(GLFWwindow *window, int button, int action, int mods)
{
    double posX, posY;
    if (action == GLFW_PRESS) {
        glfwGetCursorPos(window, &posX, &posY);
        //cout << "Pos X " << posX <<  " Pos Y " << posY << endl;
    }
}

static void resize_callback(GLFWwindow *window, int width, int height)
{
    g_width = width;
    g_height = height;
    glViewport(0, 0, width, height);
}

/* code to define the ground plane */
static void initGeom()
{
    
    GLuint VertexArrayID;
    //generate the VAO
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
    
    glGenBuffers(1, &SpecBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, SpecBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(spectrumBuff), spectrumBuff, GL_STATIC_DRAW);
    
}

static void init()
{
    GLSL::checkVersion();

    sTheta = START_ANGLE;
    // Set background color.
    glClearColor(.0f, .0f, .0f, 1.0f);
    // Enable z-buffer test.
    //glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    
    // Initialize mesh.
    shape = make_shared<Shape>();
    shape->loadMesh(RESOURCE_DIR + "smoothsphere.obj");
    shape->resize();
    shape->init();
    
    GLSL::printError();
    
    // Initialize the GLSL programs
    prog = make_shared<Program>();
    prog->setVerbose(true);
    prog->setShaderNames(RESOURCE_DIR + "simple_vert.glsl", RESOURCE_DIR + "simple_frag.glsl");
    prog->init();
    prog->addUniform("P");
    prog->addUniform("MV");
    prog->addAttribute("vertPos");
    
    prog2 = make_shared<Program>();
    prog2->setVerbose(true);
    prog2->setShaderNames(RESOURCE_DIR + "phong_vert.glsl", RESOURCE_DIR + "phong_frag.glsl");
    prog2->init();
    prog2->addUniform("P");
    prog2->addUniform("MV");
    prog2->addAttribute("vertPos");
    prog2->addAttribute("vertNor");
    prog2->addUniform("lightPos");
}

static void render()
{
    // Get current frame buffer size.
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    float aspect = width/(float)height;
    glViewport(0, 0, width, height);
    
    float loudness = volume[0] * 100;
    size = loudness;
    volume.erase(volume.begin());
    
    // Clear framebuffer.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Create the matrix stacks
    auto P = make_shared<MatrixStack>();
    auto MV = make_shared<MatrixStack>();
    P->pushMatrix();
    P->perspective(45.0f, aspect, 0.01f, 100.0f);
    
    //draw the ground plane
    prog->bind();
        glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, P->topMatrix().data());
        
        MV->pushMatrix();
            MV->loadIdentity();
            MV->translate(Vector3f(0, -1.0, -g_seconds));
            MV->scale(Vector3f(1.2, 1.0, 1));
    
            glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, MV->topMatrix().data());
            
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, SpecBuffObj);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
            for (int i = 0; i < g_seconds; i++)
            {
                glDrawArrays(GL_LINE_STRIP, i * SPEC_SIZE * 3, SPEC_SIZE);
            }
            
            glDisableVertexAttribArray(0);
        
        MV->popMatrix();
        MV->pushMatrix();
            MV->loadIdentity();
            MV->translate(Vector3f(0, -1.0, -g_seconds));
            MV->scale(Vector3f(1.2, -1.0, 1));
    
            glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, MV->topMatrix().data());
            
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, SpecBuffObj);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
            
            for (int i = 0; i < g_seconds; i++)
            {
                glDrawArrays(GL_LINE_STRIP, i * SPEC_SIZE * 3, SPEC_SIZE);
            }
            
            glDisableVertexAttribArray(0);
        
        MV->popMatrix();
        MV->pushMatrix();
            MV->loadIdentity();
            MV->translate(Vector3f(0, -1.0, -g_seconds));
            MV->scale(Vector3f(-1.2, 1, 1));
            
            glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, MV->topMatrix().data());
            
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, SpecBuffObj);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
            
            for (int i = 0; i < g_seconds; i++)
            {
                glDrawArrays(GL_LINE_STRIP, i * SPEC_SIZE * 3, SPEC_SIZE);
            }
            
            glDisableVertexAttribArray(0);
            
        MV->popMatrix();
        MV->pushMatrix();
            MV->loadIdentity();
            MV->translate(Vector3f(0, -1.0, -g_seconds));
            MV->scale(Vector3f(-1.2, -1.0, 1));
            
            glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, MV->topMatrix().data());
            
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, SpecBuffObj);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
            
            for (int i = 0; i < g_seconds; i++)
            {
                glDrawArrays(GL_LINE_STRIP, i * SPEC_SIZE * 3, SPEC_SIZE);
            }
            
            glDisableVertexAttribArray(0);
            
        MV->popMatrix();
    prog->unbind();
    
    prog2->bind();
        glUniformMatrix4fv(prog2->getUniform("P"), 1, GL_FALSE, P->topMatrix().data());
        glUniformMatrix4fv(prog2->getUniform("MV"), 1, GL_FALSE, MV->topMatrix().data());
        if (loudness > 0.5)
        {
            glUniform3f(prog2->getUniform("lightPos"), 0, 0, -50);
        }
        else
        {
            glUniform3f(prog2->getUniform("lightPos"), 0, 0, 50);
        }
    
        /* draw left sphere */
        MV->pushMatrix();
            MV->loadIdentity();
            
            MV->pushMatrix();
            MV->translate(Vector3f(1.5, 1, -5));
            MV->rotate(sTheta, Vector3f(0, 1, 0));
            MV->scale(loudness * Vector3f(0.75, 0.75, 0.75));
            glUniformMatrix4fv(prog2->getUniform("MV"), 1, GL_FALSE, MV->topMatrix().data());
            shape->draw(prog2);
        MV->popMatrix();
    
        /* draw right sphere */
        MV->pushMatrix();
            MV->loadIdentity();
            
            MV->pushMatrix();
            MV->translate(Vector3f(-1.5, 1, -5));
            MV->rotate(-sTheta, Vector3f(0, 1, 0));
            MV->scale(loudness * Vector3f(0.75, 0.75, 0.75));
    
            glUniformMatrix4fv(prog2->getUniform("MV"), 1, GL_FALSE, MV->topMatrix().data());
            shape->draw(prog2);
        MV->popMatrix();
    prog2->unbind();
    
    // Pop matrix stacks.
    P->popMatrix();
    
    sTheta += 0.5;
}

int main(int argc, char **argv)
{
    if(argc < 3)
    {
        cout << "Please specify the resource directory." << endl;
        return 0;
    }
    AUDIO_INPUT = argv[1];
    AUDIO_ANALYSIS = argv[2];
    
    Audio3 processor = *new Audio3();
    vector<vector<essentia::Real>> spectrum = *processor.audio3(AUDIO_INPUT, AUDIO_ANALYSIS);
    volume = processor.getVolume();
    
    int seconds = 0;
    int count = 0;
    for (const auto& x : spectrum)
    {
        int specPos = 1;
        for (const auto& data : x)
        {
            //cout << count << endl;
            //spectrumBuff[3 * count] = ((float)((specPos * (2.0/(SPEC_SIZE + 1)))-1));
            spectrumBuff[3 * count] = ((float)((specPos * (1.0/(SPEC_SIZE + 1)))));
            spectrumBuff[(3*count) + 1] = ((float)(data * 10));
            spectrumBuff[(3*count) + 2] = ((float)seconds);
            specPos++;
            count++;
        }
        seconds++;
    }
    
    music.openFromFile(AUDIO_INPUT);

    // Set error callback.
    glfwSetErrorCallback(error_callback);
    // Initialize the library.
    if(!glfwInit())
    {
        return -1;
    }
    //request the highest possible version of OGL - important for mac
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

    // Create a windowed mode window and its OpenGL context.
    window = glfwCreateWindow(640, 480, "Kyle Reis", NULL, NULL);
    if(!window)
    {
        glfwTerminate();
        return -1;
    }
    // Make the window's context current.
    glfwMakeContextCurrent(window);
    // Initialize GLEW.
    glewExperimental = true;
    if(glewInit() != GLEW_OK)
    {
        cerr << "Failed to initialize GLEW" << endl;
        return -1;
    }
    //weird bootstrap of glGetError
    glGetError();
    cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

    // Set vsync.
    glfwSwapInterval(1);
    // Set keyboard callback.
    glfwSetKeyCallback(window, key_callback);
    //set the mouse call back
    glfwSetMouseButtonCallback(window, mouse_callback);
    //set the window resize call back
    glfwSetFramebufferSizeCallback(window, resize_callback);

    // Initialize scene. Note geometry initialized in init now
    init();
    initGeom();

    // Loop until the user closes the window.
    music.play();
    while (size != -1)
    {
        // Render scene.
        render();
        
        if(glfwWindowShouldClose(window))
        {
            break;
        }
        
        // Swap front and back buffers.
        glfwSwapBuffers(window);
        // Poll for and process events.
        glfwPollEvents();
        //std::this_thread::sleep_for(std::chrono::microseconds((int) (21.35*1000))); // Aproximated time of the sample rate
        std::this_thread::sleep_for(std::chrono::microseconds((int) (20.158*1000)));
        g_seconds++;
    }
    // Quit program.
    glfwDestroyWindow(window);
    glfwTerminate();
    cout << "Closing Audio" << endl;
    return 0;
}
