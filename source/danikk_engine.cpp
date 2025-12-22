#include <cstdlib>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <mutex>
#include <functional>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <danikk_framework/danikk_framework.h>
#include <danikk_framework/path_buffer.h>
#include <danikk_framework/trace.h>
#include <danikk_framework/argv.h>
#include <danikk_framework/filesystem.h>
#include <danikk_framework/log.h>
#include <danikk_framework/datetime.h>
#include <danikk_framework/array.h>
#include <danikk_framework/thread.h>
#include <danikk_framework/memory.h>
#include <danikk_framework/glm.h>

#include <danikk_framework/misc/time_counter.h>
#include <danikk_framework/misc/main.h>

#include <danikk_engine/danikk_engine.h>
#include <danikk_engine/texture.h>
#include <danikk_engine/input.h>
#include <danikk_engine/localization.h>
#include <danikk_engine/internal/data_manager.h>
#include <danikk_engine/internal/gl_object_manager.h>
#include <danikk_engine/internal/error.h>
#include <danikk_engine/font.h>
#include <danikk_engine/sprite.h>
#include <danikk_engine/gui.h>
#include <danikk_engine/dynamic_mesh.h>
#include <danikk_engine/internal/asset.h>

#include <GL/gl.h>

using namespace danikk_framework;
using namespace std::this_thread;
using namespace std::chrono;
using std::thread;
using std::mutex;

namespace danikk_engine_game
{
	void begin();
	void frame();
	void pre_begin();
}

namespace danikk_engine
{
	using namespace internal;

	GLFWwindow* window;

	vec2 cursor_pos;
	vec2 last_cursor_pos;
	vec2 cursor_delta;
	float mouse_sensivity = 10.0f;

	float target_fps = 60.0f;
	float target_fd = 1.0f / target_fps;

    TimeCounter fps_counter;
    static float game_time = 0;
    static uint32 frame_count = 0;
    static bool off_cursor = false;

    thread_local bool is_main_thread = false;
    bool gl_thing_execute_requied = false;
    mutex gl_operation_mutex;
    std::function<void()> gl_thing;

    ivec2 window_size = uvec2(1440, 900);

	float screen_ratio_gz;
	float screen_ratio_lz;

	struct button_state
	{
		uint32 change_frame;
		uint32 state;

		int getState()
		{
	    	if(state)
	    	{
	        	if(change_frame == frame_count)
	        	{
	        		return button_states::press;
	        	}
	        	else
	        	{
	        		return button_states::hold;
	        	}
	    	}
	    	else
	    	{
	        	if(change_frame == frame_count)
	        	{
	        		return button_states::release;
	        	}
	        	else
	        	{
	        		return button_states::free;
	        	}
	    	}
		}
	};

	button_state mouse_button_states[8]{ button_states::free };
	button_state keyboard_button_states[400]{ button_states::free };

	float getMouseSensivity()
	{
		return mouse_sensivity;
	}

	int getKeyboardState(int button)
	{
        return keyboard_button_states[button].getState();
	}

	int getMouseState(int button)
	{
    	return mouse_button_states[button].getState();
	}

    static void cursorPositionCallback(GLFWwindow* window, double x, double y)
    {
    	vec2 new_cursor_pos = vec2(
			cursor_pos.x = x / window_size.x,
			cursor_pos.y = 1 - y / window_size.y);//преобразование из системы координат ввода в систему координат OpenGL
    	cursor_pos = new_cursor_pos;
    }

    void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
    {
    	button_state* currentMouseButton = &mouse_button_states[button];
		currentMouseButton->change_frame = frame_count;
    	if(action == GLFW_RELEASE)
    	{
    		currentMouseButton->state = false;
    	}
    	else if (action == GLFW_PRESS)
    	{
    		currentMouseButton->state = true;
    	}
    	if(button == mouse_buttons::left)
    	{
    		for(GUIElement* ch : gui_root.childs)
    		{
    			ch->handleClick(currentMouseButton->state);
    		}
    	}
    }

    static void errorCallback(int error, const char* description)
    {
        (void)error;
        log("GLFW  ", description);
    }

    static void glDebugMessageCallbackFunc(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
    {
    	(void)source;
    	(void)type;
    	(void)id;
    	(void)severity;
    	(void)length;
    	(void)userParam;
    	log("GL    ", message);
    }

    static void framebufferSizeCallback(GLFWwindow* window, int width, int height)
    {
        (void)window;
        glViewport(0, 0, width, height);
    }

    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
    	button_state* currentMouseButton = &keyboard_button_states[key];
		currentMouseButton->change_frame = frame_count;
    	if(action == GLFW_RELEASE)
    	{
    		currentMouseButton->state = false;
    	}
    	else if (action == GLFW_PRESS)
    	{
    		currentMouseButton->state = true;
    	}
    }

    static void charCallback(GLFWwindow* window, unsigned int codepoint)
    {
        (void)window;
        (void)codepoint;
    }

    static void windowRefreshCallback(GLFWwindow* window)
    {
        (void)window;
        glfwPollEvents();
    }

    static void initGraphics()
    {
    	assert(glfwInit());

        glfwSetErrorCallback(errorCallback);

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, gl_version_major);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, gl_version_minor);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
        glfwWindowHint(GLFW_DECORATED, false);

        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);

        window_size = ivec2(mode->width / 2 - 1, mode->height / 2);
        window = glfwCreateWindow(window_size.x, window_size.y, "danikk engine game", NULL, NULL);

        if (!window)
        {
        	fatalError("Cant create window");
        }

        //const GLFWvidmode* mode;
        //mode = glfwGetVideoMode(NULL);
        //window_size = uvec2(mode->width, mode->height);

        glfwGetWindowSize(window, &window_size.x, &window_size.y);

    	screen_ratio_gz = (float)window_size.x / (float)window_size.y;
    	screen_ratio_lz = (float)window_size.y / (float)window_size.x;

        glfwMakeContextCurrent((GLFWwindow*)window);
        glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
        glfwSetKeyCallback(window, keyCallback);
        glfwSetWindowRefreshCallback(window, windowRefreshCallback);
        glfwSetCharCallback(window, charCallback);
        glfwSetCursorPosCallback(window, cursorPositionCallback);
        glfwSetMouseButtonCallback(window, mouseButtonCallback);
        if(off_cursor)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
        	fatalError("Cant inir glad");
        }

		#define display_gl_thing(name) formatLogInfo("% : %", #name , (char*)glGetString(name));
			display_gl_thing(GL_VENDOR)
			display_gl_thing(GL_VERSION)
			display_gl_thing(GL_RENDERER)
		#undef display_gl_thing

	    glDebugMessageCallback(glDebugMessageCallbackFunc, 0);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		//glEnable(GL_TEXTURE_2D);
		//glEnable(GL_TEXTURE0);
		glEnable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glEnable(GL_CULL_FACE_MODE);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        //glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glfwSwapBuffers(window);

		initGlObjectManager();
		initBuiltInMeshes();
		initTextureRenderer();

		gui_root.absolute_size = vec2(1.0f);

		glfwSetWindowSize(window, window_size.x+1, window_size.y);
        //glViewport(0, 0, window_size.x, window_size.y);
    }

    void exit()
    {
        glfwSetWindowShouldClose(window, true);
    }

    void setWindowTitle(const String& title)
    {
    	glfwSetWindowTitle(window, title.c_string());
    }

    float gameTime()
    {
    	return game_time;
    }

    float getTimeFactor()
    {
    	return glm::cos(game_time * 2.0f * pi);
    }

    float getTargetFPS()
    {
    	return target_fps;
    }

    float getTargetFrameDelay()
    {
    	return target_fd;
    }

    uint64 frameCount()
    {
    	return frame_count;
    }

    vec2 getPixelSize()
    {
    	return vec2(1.0f / window_size.x, 1.0f / window_size.y);
    }

    vec2 getCursorPos()
    {
    	return cursor_pos;
    }

    vec2 getCursorDelta()
    {
    	return cursor_delta;
    }

    namespace internal
	{
		void _glexec(std::function<void()> func)
		{
			if(is_main_thread)
			{
				func();
			}
			else
			{
				gl_operation_mutex.lock();
				gl_thing = func;
				gl_thing_execute_requied = true;
				gl_operation_mutex.lock();
				gl_operation_mutex.unlock();
			}
		}
	}

    static void baseInit()
    {
    	traceSegfault();

    	log_config.log_to_dir = false;
    	log_config.log_to_file = true;
        configureLog();
        argv::log();
        formatLogInfo("BIT : %", sizeof(size_t) * 8);

    	danikk_framework::setLocale("ru_RU");
	    danikk_framework::initLocalization();
	    danikk_framework::default_random.init();
    	danikk_engine::initLocaliztion();


    	cwdToExd();
        formatLogInfo("CWD : %", getExecutableDirectory().c_string());
        is_main_thread = true;
    }

    static void mainLoop()
    {
        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();
        	glUniform1f(shader_layout_locations::time_factor, game_time);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            danikk_engine_game::frame();

            cursor_delta = cursor_pos - last_cursor_pos;
            last_cursor_pos = cursor_pos;

    		for(GUIElement* ch : gui_root.childs)
    		{
    			ch->recursiveDraw();
    		}
            glfwSwapBuffers(window);
            fps_counter.check();
            glUniform1f(shader_layout_locations::time_factor, sin(game_time));
            if(gl_thing_execute_requied)
            {
            	gl_thing();
            	gl_thing_execute_requied = false;
    			gl_operation_mutex.unlock();
            }
            game_time += target_fd;
            frame_count++;
        }
    }
}

using namespace danikk_engine;
using namespace danikk_engine_game;

DECLARE_MAIN
{
	baseInit();
	pre_begin();
	initAssetTypes();
	initDataManager();
	initShaderCompiler();
	initFontRenderer();
	initGraphics();
	begin();
	mainLoop();
    glfwDestroyWindow(window);
    //glfwTerminate();
    return 0;
}
