#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <GL/glew.h>


class Shader {
	std::string vertex_shader_code, fragment_shader_code;

	unsigned int load_vertex_shader(std::string vertex_shader_path) {
		std::ifstream vertex_shader_file(vertex_shader_path + ".vert_sh");

		for (std::string line; std::getline(vertex_shader_file, line); )
			vertex_shader_code += line + "\n";

		const char* vertex_shader_code_c = vertex_shader_code.c_str();

		unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex_shader, 1, &vertex_shader_code_c, NULL);
		glCompileShader(vertex_shader);

		int success;
		glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
		if (!success) {
			char info_log[512];
			glGetShaderInfoLog(vertex_shader, 512, NULL, info_log);

			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << info_log << "\n";
		}

		return vertex_shader;
	}

	unsigned int load_fragment_shader(std::string fragment_shader_path) {
		std::ifstream fragment_shader_file(fragment_shader_path + ".frag_sh");

		for (std::string line; std::getline(fragment_shader_file, line); )
			fragment_shader_code += line + "\n";

		const char* fragment_shader_code_c = fragment_shader_code.c_str();

		unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment_shader, 1, &fragment_shader_code_c, NULL);
		glCompileShader(fragment_shader);

		int success;
		glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
		if (!success) {
			char info_log[512];
			glGetShaderInfoLog(fragment_shader, 512, NULL, info_log);

			std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << info_log << "\n";
		}

		return fragment_shader;
	}

	void link_shaders(unsigned int vertex_shader, unsigned int fragment_shader) {
		program = glCreateProgram();
		glAttachShader(program, vertex_shader);
		glAttachShader(program, fragment_shader);
		glLinkProgram(program);

		int success;
		glGetProgramiv(program, GL_LINK_STATUS, &success);
		if (!success) {
			GLchar info_log[512];
			glGetProgramInfoLog(program, 512, NULL, info_log);

			std::cout << "ERROR::PROGRAM::LINKING_FAILED\n" << info_log << "\n";
		}
	}

public:
	unsigned int program;

	Shader() {
		program = 0;
	}

	Shader(std::string vertex_shader_path, std::string fragment_shader_path) {
		unsigned int vertex_shader = load_vertex_shader(vertex_shader_path);
		unsigned int fragment_shader = load_fragment_shader(fragment_shader_path);
		link_shaders(vertex_shader, fragment_shader);
		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);
	}

	void use() {
		glUseProgram(program);
	}

	int get_count_lights() {
		std::vector < std::string > split_string(1);
		for (char el : fragment_shader_code) {
			if (el == ' ' || el == '\n')
				split_string.push_back("");
			else
				split_string.back() += el;
		}

		for (int i = 0; i < split_string.size(); i++) {
			if (split_string[i] == "#define" && split_string[i + 1] == "NR_LIGHTS")
				return std::stoi(split_string[i + 2]);
		}

		return 0;
	}
};
