#pragma once

#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>

#include <GL/glew.h> 

#include <glm/glm.hpp>
#include <glm/ext.hpp>

class ShaderProgram {
public:

	ShaderProgram(void) = default; //does nothing
	ShaderProgram(const std::filesystem::path& VS_file, const std::filesystem::path& FS_file); // TODO: load, compile, and link shader

	void activate(void) { 
		if (ID == currently_used)
			return;
		else {
			glUseProgram(ID);
			currently_used = ID;
		}
	};    // activate shader
	void deactivate(void) {
		glUseProgram(0);
		currently_used = 0;
	};   // deactivate current shader program (i.e. activate shader no. 0)

	void clear(void) { 	//deallocate shader program
		deactivate();
		glDeleteProgram(ID);
		ID = 0;
	}

	// set uniform according to name 
	// https://docs.gl/gl4/glUniform
	void setUniform(const std::string& name, const float val);
	void setUniform(const std::string& name, const int val);
	void setUniform(const std::string& name, const glm::vec3 val);
	void setUniform(const std::string& name, const glm::vec4 val);
	void setUniform(const std::string& name, const glm::mat3 val);
	void setUniform(const std::string& name, const glm::mat4 val); 

private:
	inline static GLuint currently_used { 0 };

	GLuint ID { 0 }; // default = 0, empty shader
	std::string getShaderInfoLog(const GLuint obj);   // TODO: check for shader compilation error; if any, print compiler output  
	std::string getProgramInfoLog(const GLuint obj);  // TODO: check for linker error; if any, print linker output

	GLuint compile_shader(const std::filesystem::path& source_file, const GLenum type); // TODO: try to load and compile shader
	GLuint link_shader(const std::vector<GLuint> shader_ids);                            // TODO: try to link all shader IDs to final program
	std::string textFileRead(const std::filesystem::path& filename);                    // TODO: load text file
};

