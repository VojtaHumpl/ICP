
#include "ShaderProgram.h"

// set uniform according to name 
// https://docs.gl/gl4/glUniform

ShaderProgram::ShaderProgram(const std::filesystem::path& VS_file, const std::filesystem::path& FS_file) {
	std::vector<GLuint> shader_ids;

	shader_ids.push_back(compile_shader(VS_file, GL_VERTEX_SHADER));
	shader_ids.push_back(compile_shader(FS_file, GL_FRAGMENT_SHADER));

	ID = link_shader(shader_ids);

	GLint modelLoc = glGetUniformLocation(ID, "model");
	GLint viewLoc = glGetUniformLocation(ID, "view");
	GLint projLoc = glGetUniformLocation(ID, "projection");

	std::cout << "Uniform locations - Model: " << modelLoc
		<< ", View: " << viewLoc
		<< ", Projection: " << projLoc << std::endl;
}

void ShaderProgram::setUniform(const std::string& name, const float val) {
	auto loc = glGetUniformLocation(ID, name.c_str());
	if (loc == -1) {
		std::cerr << "no uniform with name:" << name << '\n';
		return;
	}
	glUniform1f(loc, val);
}

void ShaderProgram::setUniform(const std::string& name, const int val) {
	auto loc = glGetUniformLocation(ID, name.c_str());
	if (loc == -1) {
		std::cerr << "No uniform with name: " << name << '\n';
		return;
	}
	glUniform1i(loc, val);
}

void ShaderProgram::setUniform(const std::string& name, const glm::vec3 val) {
	auto loc = glGetUniformLocation(ID, name.c_str());
	if (loc == -1) {
		std::cerr << "No uniform with name: " << name << '\n';
		return;
	}
	glUniform3fv(loc, 1, glm::value_ptr(val));
}

void ShaderProgram::setUniform(const std::string& name, const glm::vec4 in_vec4) {
	auto loc = glGetUniformLocation(ID, name.c_str());
	if (loc == -1) {
		std::cerr << "No uniform with name: " << name << '\n';
		return;
	}
	glUniform4fv(loc, 1, glm::value_ptr(in_vec4));
}

void ShaderProgram::setUniform(const std::string& name, const glm::mat3 val) {
	auto loc = glGetUniformLocation(ID, name.c_str());
	if (loc == -1) {
		std::cerr << "No uniform with name: " << name << '\n';
		return;
	}
	glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(val));
}

void ShaderProgram::setUniform(const std::string& name, const glm::mat4 val) {
	auto loc = glGetUniformLocation(ID, name.c_str());
	if (loc == -1) {
		std::cerr << "No uniform with name: " << name << '\n';
		return;
	}
	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(val));
}

std::string ShaderProgram::getShaderInfoLog(const GLuint shader) {
	GLint log_length = 0;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
	if (log_length > 0) {
		std::vector<char> log(log_length);
		glGetShaderInfoLog(shader, log_length, &log_length, &log[0]);
		return std::string(log.begin(), log.end());
	}
	return "";
}

std::string ShaderProgram::getProgramInfoLog(const GLuint program) {
	GLint log_length = 0;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
	if (log_length > 0) {
		std::vector<char> log(log_length);
		glGetProgramInfoLog(program, log_length, &log_length, &log[0]);
		return std::string(log.begin(), log.end());
	}
	return "";
}

GLuint ShaderProgram::compile_shader(const std::filesystem::path& source_file, const GLenum type) {
	GLuint shader_h = glCreateShader(type);

	std::string shader_source = textFileRead(source_file);
	const char* shader_source_cstr = shader_source.c_str();

	glShaderSource(shader_h, 1, &shader_source_cstr, nullptr);
	glCompileShader(shader_h);

	GLint success;
	glGetShaderiv(shader_h, GL_COMPILE_STATUS, &success);
	if (!success) {
		std::string info_log = getShaderInfoLog(shader_h);
		std::cerr << "Error compiling shader: " << source_file << "\n" << info_log << std::endl;
		glDeleteShader(shader_h);
		throw std::runtime_error("Shader compilation failed.");
	}

	return shader_h;
}

GLuint ShaderProgram::link_shader(const std::vector<GLuint> shader_ids) {
	GLuint prog_h = glCreateProgram();

	for (const auto& id : shader_ids)
		glAttachShader(prog_h, id);

	glLinkProgram(prog_h);

	GLint success;
	glGetProgramiv(prog_h, GL_LINK_STATUS, &success);
	if (!success) {
		std::string info_log = getProgramInfoLog(prog_h);
		std::cerr << "Error linking program:\n" << info_log << std::endl;
		glDeleteProgram(prog_h);
		throw std::runtime_error("Program linking failed.");
	}

	for (const auto& id : shader_ids) {
		glDetachShader(prog_h, id);
		glDeleteShader(id);
	}

	return prog_h;
}

std::string ShaderProgram::textFileRead(const std::filesystem::path& filename) {
	std::ifstream file(filename);
	if (!file.is_open())
		throw std::runtime_error("Error opening file.\n");
	std::stringstream ss;
	ss << file.rdbuf();
	return ss.str();
}
