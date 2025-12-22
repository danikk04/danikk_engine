#include <danikk_engine/shader.h>
#include <danikk_engine/internal/data_manager.h>
#include <danikk_engine/internal/glexec.h>
#include <danikk_framework/cstring_functions.h>
#include <danikk_framework/format.h>
#include <glad/glad.h>

namespace danikk_engine
{
	using namespace internal;
	using namespace danikk_framework;

    static String shader_process_buffer;
    static String include_file_buffer;

    static constexpr uint GL_LOG_SIZE = 256;
    static uint last_shader = 0;

    static String shader_layout = "#version 460 core\n";

    struct VirtualIncludeFile
    {
    	String name;
    	String data;
    };

    VirtualIncludeFile virtual_include_files[4];

	namespace internal
	{
		void initShaderCompiler()
		{
			index_t i = 0;
			virtual_include_files[i].name = "vert.glslh";
			virtual_include_files[i].data = formatCreate(
				"layout(location = %) in vec3 vertex_pos;"
				"layout(location = %) in vec3 vertex_normal;"
				"layout(location = %) in vec2 vertex_uv;"
				"out vec3 frag_normal;"
				"out vec2 frag_uv;",
				shader_layout_locations::vertex_pos,
				shader_layout_locations::vertex_normal,
				shader_layout_locations::vertex_uv);
			i++;
			virtual_include_files[i].name = "frag.glslh";
			virtual_include_files[i].data = formatCreate(
				"in vec3 frag_normal;"
				"in vec2 frag_uv;"
				"out vec4 out_color;");
			i++;
			virtual_include_files[i].name = "uniforms.glslh";
			virtual_include_files[i].data = formatCreate(
				"uniform layout(location = %) mat4 world;"
				"uniform layout(location = %) mat4 view;"
				"uniform layout(location = %) mat4 projection;"
				"uniform layout(location = %) mat2 uv;"
				"uniform layout(location = %) vec4 color;"
				"uniform layout(location = %) float time_factor;"
				"uniform sampler2D diffuse_texture;",
				shader_layout_locations::world,
				shader_layout_locations::view,
				shader_layout_locations::projection,
				shader_layout_locations::uv,
				shader_layout_locations::color,
				shader_layout_locations::time_factor);
			i++;
			virtual_include_files[i].name = "2d.glslh";
			virtual_include_files[i].data =
				"vec2 apply_uv(mat2 uv, vec2 vector)"
				"{"
				"return vec2(vector.x * uv[0].x + uv[1].x, vector.y * uv[0].y + uv[1].y);"
				"}"
				"vec4 apply_pos(mat4 pos, vec3 vector)"
				"{"
				"return vec4("
				"vector.x * pos[0].x - (1 - pos[0].x) * 0.5 + pos[1].x, "
				"vector.y * pos[0].y - (1 - pos[0].y) * 0.5 + pos[1].y, 0, 0.5);"
				"}";
			i++;
;
		}
	}

    size_t Shader::handle()
    {
    	return container->shader_data.handle;
    }

    Shader::Shader(Shader& other) : Asset(other.container){}

    Shader::Shader(AssetContainer* container) : Asset(container){}

    static inline const char* glShaderTypeToFileExt(uint type)
    {
    	if(type == GL_VERTEX_SHADER)
    	{
    		return "vert";
    	}
    	else if (type == GL_FRAGMENT_SHADER)
    	{
    		return "frag";
    	}
    	assert(type == GL_VERTEX_SHADER || type == GL_FRAGMENT_SHADER);
    	return "";
    }

    static uint compileShader(const char* shader_name, uint type)
    {
    	uint shader;
    	//указатель на необработанный glsl код.
    	//Обработчик раскрывает include директивы.
    	const char* raw_code = asset_load_buffer.c_string();

    	shader_process_buffer.clear();
    	shader_process_buffer << shader_layout;

    	while(true)
    	{
    		while(strstartswith(raw_code, "//"))
    		{
    			//Пропускаем комментарии
    			for(;*raw_code!='\n';raw_code++);
    			raw_code++;
    		}
    		if(*raw_code == '#')
    		{
    			raw_code++;
    			if(strmifsw(raw_code, "include<"))
    			{
    				include_file_buffer.clear();
    				while(*raw_code != '>')
    				{
    					include_file_buffer << *raw_code;
    					raw_code++;
    				}
		    		raw_code += 2;
    				bool find_flag = false;

    				for(const VirtualIncludeFile& vfile : virtual_include_files)
    				{
        				if (include_file_buffer == vfile.name)
        				{
        					for(char chr : vfile.data)//Поспроцессинг файла шейдеров для удобной читаемости в дебаггере.
        					{
            	    			if(chr == ';' || chr == '}')
            	    			{
            	        			shader_process_buffer << chr << '\n';
            	    			}
            	    			else if (chr == '{')
            	    			{
            	        			shader_process_buffer << '\n' << chr << '\n';
            	    			}
            	    			else
            	    			{
            	        			shader_process_buffer << chr;
            	    			}
        					}
        					find_flag = true;
            				break;
        		    	}
    				}

    				if(!find_flag)
    				{
    					String error_buffer;

    					formatFatalError("Cant find file for shader % with file %", shader_name, glShaderTypeToFileExt(type));
        				logFatal(error_buffer);
        				fatalError();
    				}
    			}
    			else
    			{
					formatFatalError("Wrong directive for shader % with file %", shader_name, glShaderTypeToFileExt(type));
    			}
    		}
    		else if(strstartswith(raw_code, "void main()"))
    		{
    			break;//Препроцессинг идёт только до объявления main.
    		}
    		else
    		{
    			shader_process_buffer << *raw_code;
    			raw_code++;
    		}
    	}

    	shader_process_buffer << raw_code;

    	int success;
        char infoLog[GL_LOG_SIZE];
        const char* code = shader_process_buffer.c_string();

        glexec
		(
			shader = glCreateShader(type);
            glShaderSource(shader, 1, (const GLchar* const*)&code, NULL);
            glCompileShader(shader);
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		)

        if (!success)
        {
        	glexec
        	(
                glGetShaderInfoLog(shader, GL_LOG_SIZE, NULL, infoLog);
            )

			formatLogFatal("log:%", infoLog);
			formatLogFatal("code:%", code);
			formatFatalError("Cant compile shader  %.%\n", shader_name, glShaderTypeToFileExt(type));
        }

    	return shader;
    }

    void Shader::load(const char* name)
    {
    	AssetContainer** container_ptr = assets.get(name);
    	AssetContainer* container;

    	formatLogInfo("Loading shader: %", name);

    	if(container_ptr == NULL)
    	{
        	int vertexShader;
        	int fragmentShader;
        	int program;

			if(!loadDataToBuffer("shaders", name, "vert", true))
			{
				formatFatalError("Cant load shader file  %.%\n", name, "vert");
			}

            vertexShader = compileShader(name, GL_VERTEX_SHADER);

			if(!loadDataToBuffer("shaders", name, "frag", true))
			{
				formatFatalError("Cant load shader file  %.%\n", name, "frag");
			}

            fragmentShader = compileShader(name, GL_FRAGMENT_SHADER);

            int success;
            char infoLog[GL_LOG_SIZE];

            glexec
        	(
                program = glCreateProgram();
                glAttachShader(program, vertexShader);
                glAttachShader(program, fragmentShader);
                glLinkProgram(program);
                glGetProgramiv(program, GL_LINK_STATUS, &success);
            	glDetachShader(program, vertexShader);
            	glDetachShader(program, fragmentShader);
                glDeleteShader(vertexShader);
                glDeleteShader(fragmentShader);
            )

            if (!success)
            {
                glexec
            	(
            		glGetProgramInfoLog(program, GL_LOG_SIZE, NULL, infoLog);
                )
                formatLogFatal("log:%", infoLog);
				formatFatalError("Cant link shader  %\n", name);
            }

    		container = new (structalloc<AssetContainer>()) AssetContainer(asset_type::texture, name);
            container->shader_data.handle = program;
        	assets.insert(name, container);
    	}
    	else
    	{
    		container = *container_ptr;
        	container->ref_count++;
    	}

    	new (this) Asset(container);
    }

    void Shader::operator=(Shader& other)
    {
    	new (this) Shader(other);
    }

    void Shader::operator=(Shader&& other)
	{
    	new (this) Shader(other);
	}

    void Shader::use(bool force)
    {
    	uint current_handle = handle();
    	if(last_shader != current_handle || force)
    	{
            glUseProgram(current_handle);
            last_shader = current_handle;
    	}
    }

    void Shader::unuse()
    {
    	last_shader = 0;
		glUseProgram(0);
    }
}
