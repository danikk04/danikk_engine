#include <danikk_engine/localization.h>

#define _ set_localization

using namespace danikk_framework;

namespace danikk_engine
{

	danikk_framework::Localization localization;

	void initLocaliztion()
	{
		_("failed",								"Failed to",						"Не удалось")
		_("mismatch",							"Mismatch of",						"Нехватка")
		_("cant_load",							"Cant load",						"Не удаётся загрузить")

		_("info_out/font_name", 				"font name", 						"имя шрифта")
		_("info_out/texture_name", 				"texture name", 					"имя текстуры")
		_("info_out/shader_name", 				"shader name", 						"имя шейдера")
		_("info_out/shader_file", 				"shader file", 						"файл шейдера")
		_("info_out/error_code", 				"error code", 						"код ошибки")
		_("info_out/char_code", 				"char code", 						"код символа")
		_("info_out/log", 						"log", 								"лог")
		_("info_out/code", 						"code", 							"код")
		_("info_out/reason", 					"reason", 							"причина")
		_("info_out/path", 						"path", 							"путь")

		_("initGraphics::create_window", 		"create window.", 					"создать окно")
		_("initGraphics::init_glfw", 			"initialize GLFW", 					"инициализировать GLFW")
		_("initGraphics::init_glad", 			"initialize GLAD", 					"инициализировать GLAD")

		_("initDataManager::init", 				"init data manager", 				"не удалось инициализировать менеджер данных")
		_("initDataManager::find_game_data", 	"find game data", 					"найти данные игры")

		_("Shader::Shader::load_shader", 		"load shader", 						"загрузить шейдер")
		_("compileShader::link", 				"shader link", 						"связать шейдер")
		_("compileShader::compile", 			"compile shader", 					"скомпилировать шейдер")
		_("compileShader::preproc",				"preprocess shader",				"обработать шейдер")
		_("compileShader::include_file",		"Include file",						"Заголовочный файл")
		_("compileShader::not_exits",			"not exits",						"не существует")
		_("compileShader::wrong_directive",		"wrong directive",					"неизвестная директива")

		_("Texture::Texture/color_channels",	"color channels",					"цветовых каналов")
		_("Texture::Texture/load", 				"load texture", 					"загрузить")
		_("Texture::Texture/color_channels",	"unexpected texture channels count","неправильное количество каналов текстуры")

		_("Font::Font/load", 					"load font", 						"загрузить шрифт")
	}
}
