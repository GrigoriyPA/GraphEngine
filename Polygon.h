#pragma once

#include <cassert>
#include <vector>
#include "Shader.h"
#include "Texture.h"
#include "CommonClasses/Matrix.h"


class Material {
public:
	bool light = false;
	double shininess = 1, alpha = 1;
	Vect3 ambient = Vect3(0, 0, 0), diffuse = Vect3(0, 0, 0), specular = Vect3(0, 0, 0), emission = Vect3(0, 0, 0);

	void use(Shader* shader_program) {
		glUniform3f(glGetUniformLocation(shader_program->program, "object_material.ambient"), ambient.x, ambient.y, ambient.z);
		glUniform3f(glGetUniformLocation(shader_program->program, "object_material.diffuse"), diffuse.x, diffuse.y, diffuse.z);
		glUniform3f(glGetUniformLocation(shader_program->program, "object_material.specular"), specular.x, specular.y, specular.z);
		glUniform3f(glGetUniformLocation(shader_program->program, "object_material.emission"), emission.x, emission.y, emission.z);
		glUniform1f(glGetUniformLocation(shader_program->program, "object_material.shininess"), shininess);
		glUniform1f(glGetUniformLocation(shader_program->program, "object_material.alpha"), alpha);
		glUniform1i(glGetUniformLocation(shader_program->program, "object_material.light"), light);
	}
};


class Polygon {
	unsigned int matrix_buffer = 0;
	Matrix polygon = one_matrix(4);
	Shader* shader_program = nullptr;

	int count_points;
	unsigned int vertex_array, vertex_buffer, index_buffer;
	std::vector < float > positions;
	Vect3 center;

	void create_vertex_array() {
		glGenVertexArrays(1, &vertex_array);
		glBindVertexArray(vertex_array);

		glGenBuffers(1, &vertex_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8 * count_points, NULL, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(sizeof(float) * 3 * count_points));
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)(sizeof(float) * 6 * count_points));
		glEnableVertexAttribArray(2);

		glGenBuffers(1, &index_buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);

		unsigned int* indices = new unsigned int[(count_points - 2) * 3];
		for (int i = 0; i < count_points - 2; i++) {
			indices[3 * i] = 0;
			indices[3 * i + 1] = i + 1;
			indices[3 * i + 2] = i + 2;
		}
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * (count_points - 2) * 3, indices, GL_STATIC_DRAW);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

public:
	int id = -1;

	Texture diffuse_map, specular_map, emission_map;
	Material material;

	Polygon(const Polygon& object) {
		polygon = object.polygon;
		shader_program = object.shader_program;
		count_points = object.count_points;
		center = object.center;
		diffuse_map = object.diffuse_map;
		specular_map = object.specular_map;
		emission_map = object.emission_map;
		material = object.material;
		positions = object.positions;

		create_vertex_array();
		set_matrix_buffer(object.matrix_buffer);

		glBindVertexArray(vertex_array);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

		glBindBuffer(GL_COPY_READ_BUFFER, object.vertex_buffer);
		glBindBuffer(GL_COPY_WRITE_BUFFER, vertex_buffer);
		glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, sizeof(float) * 8 * count_points);
		glBindBuffer(GL_COPY_READ_BUFFER, 0);
		glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	Polygon(int count_points = 0, Shader* shader = nullptr) {
		this->count_points = count_points;
		this->shader_program = shader;

		create_vertex_array();
	}

	void set_positions(std::vector < float > positions, bool update_normals = true) {
		this->positions = positions;
		positions = get_positions();

		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * 3 * count_points, &positions[0]);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		center = Vect3(0, 0, 0);
		for (int i = 0; i < count_points; i++)
			center += Vect3(positions[3 * i], positions[3 * i + 1], positions[3 * i + 2]);
		center /= count_points;

		if (update_normals) {
			Vect3 p0(positions[0], positions[1], positions[2]);
			Vect3 p1(positions[3], positions[4], positions[5]);
			Vect3 p2(positions[6], positions[7], positions[8]);
			Vect3 normal = (p2 - p0) ^ (p1 - p0);

			std::vector < float > normals(3 * count_points);
			for (int i = 0; i < count_points; i++) {
				normals[3 * i] = normal.x;
				normals[3 * i + 1] = normal.y;
				normals[3 * i + 2] = normal.z;
			}
			set_normals(normals);
		}
	}

	void set_normals(std::vector < float > normals) {
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

		glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * 3 * count_points, sizeof(float) * 3 * count_points, &normals[0]);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void set_tex_coords(std::vector < float > tex_coords) {
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

		glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * 6 * count_points, sizeof(float) * 2 * count_points, &tex_coords[0]);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void set_shader(Shader* shader) {
		shader_program = shader;
	}

	void set_uniforms() {
		if (shader_program == nullptr)
			return;

		shader_program->use();
		glUniform1i(glGetUniformLocation(shader_program->program, "use_diffuse_map"), diffuse_map.texture_id);
		glUniform1i(glGetUniformLocation(shader_program->program, "use_specular_map"), specular_map.texture_id);
		glUniform1i(glGetUniformLocation(shader_program->program, "use_emission_map"), emission_map.texture_id);

		material.use(shader_program);

		diffuse_map.active(0);
		specular_map.active(1);
		emission_map.active(2);
	}

	void set_matrix_buffer(unsigned int matrix_buffer) {
		if (matrix_buffer == 0)
			return;

		this->matrix_buffer = matrix_buffer;

		glBindBuffer(GL_ARRAY_BUFFER, matrix_buffer);
		glBindVertexArray(vertex_array);

		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 16, (void*)0);
		glEnableVertexAttribArray(3);

		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 16, (void*)(sizeof(float) * 4));
		glEnableVertexAttribArray(4);

		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 16, (void*)(sizeof(float) * 8));
		glEnableVertexAttribArray(5);

		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 16, (void*)(sizeof(float) * 12));
		glEnableVertexAttribArray(6);

		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	int get_count_points() {
		return count_points;
	}

	std::vector < float > get_positions() {
		std::vector < float > global_positions;
		for (int i = 0; i < count_points; i++) {
			Vect3 pos(positions[3 * i], positions[3 * i + 1], positions[3 * i + 2]);
			pos = polygon * pos;
			for (int j = 0; j < 3; j++)
				global_positions.push_back(pos[j]);
		}
		return global_positions;
	}

	Vect3 get_center() {
		return polygon * center;
	}

	int get_vao() {
		return vertex_array;
	}

	void change_matrix(Matrix trans) {
		polygon = trans * polygon;
		set_positions(positions);
	}

	void delete_uniforms() {
		if (shader_program == nullptr)
			return;

		shader_program->use();
		emission_map.deactive(2);
		specular_map.deactive(1);
		diffuse_map.deactive(0);
	}

	void draw(int count) {
		if (shader_program == nullptr)
			return;

		glBindVertexArray(vertex_array);
		glDrawElementsInstanced(GL_TRIANGLES, (count_points - 2) * 3, GL_UNSIGNED_INT, 0, count);
		glBindVertexArray(0);
	}

	~Polygon() {
		glDeleteVertexArrays(1, &vertex_array);
		glDeleteBuffers(1, &vertex_buffer);
		glDeleteBuffers(1, &index_buffer);
	}
};
