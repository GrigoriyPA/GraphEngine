#pragma once

#include <fstream>
#include <vector>
#include <string>
#include "Shader.h"
#include "CommonClasses/Matrix.h"


class Kernel {
	Matrix kernel = Matrix(3, 3, 0);

public:
	Kernel() {
		kernel[1][1] = 1;
	}

	Kernel(std::vector < std::vector < double > > init) {
		kernel = Matrix(init);
	}

	Kernel(std::string kernel_path) {
		std::ifstream kernel_file(kernel_path + ".kernel");

		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++)
				kernel_file >> kernel[i][j];
		}
	}

	void use(Shader* shader) {
		glUniform1fv(glGetUniformLocation(shader->program, "kernel"), 9, kernel.value_ptr());
	}
};
