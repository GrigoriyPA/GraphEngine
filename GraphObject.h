#pragma once

#include <iostream>
#include <vector>
#include "Polygon.h"
#include "CommonClasses/Matrix.h"


class GraphObject {
	int free_polygon_id = 0, count_points = 0;
	Vect3 center = Vect3(0, 0, 0), border_color = Vect3(1, 0, 0);
	std::vector < Matrix > models = std::vector < Matrix >(1, one_matrix(4));

	int max_count_models;
	unsigned int matrix_buffer;
	std::vector < Polygon > polygons;
	Shader* shader_program;

	void set_uniforms() {
		if (shader_program == nullptr)
			return;

		shader_program->use();
		glUniform3f(glGetUniformLocation(shader_program->program, "border_color"), border_color.x, border_color.y, border_color.z);
	}

	void draw_polygons(int id) {
		int cnt = models.size();
		if (id != -1) {
			glUniformMatrix4fv(glGetUniformLocation(shader_program->program, "not_instance_model"), 1, GL_FALSE, models[id].value_ptr());
			cnt = 1;
		}

		glUniform1i(glGetUniformLocation(shader_program->program, "use_instance"), id == -1);

		for (Polygon& polygon : polygons) {
			polygon.set_uniforms();
			polygon.draw(cnt);
			polygon.delete_uniforms();
		}
	}

	void draw_border(Vect3 view_pos, int id) {
		if (id != -1) {
			Matrix model_border = models[id] * scale_matrix(1 + border_width * (view_pos - models[id] * center).length());
			glUniformMatrix4fv(glGetUniformLocation(shader_program->program, "not_instance_model"), 1, GL_FALSE, model_border.value_ptr());
		}

		glUniform1i(glGetUniformLocation(shader_program->program, "use_instance"), 0);
		glUniform1i(glGetUniformLocation(shader_program->program, "border"), 1);

		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilMask(0x00);

		for (Polygon& polygon : polygons) {
			polygon.set_uniforms();
			if (id == -1) {
				for (Matrix& model : models) {
					Matrix model_border = model * scale_matrix(1 + border_width * (view_pos - model * center).length());
					glUniformMatrix4fv(glGetUniformLocation(shader_program->program, "not_instance_model"), 1, GL_FALSE, model_border.value_ptr());
					polygon.draw(1);
				}
			}
			else {
				polygon.draw(1);
			}
			polygon.delete_uniforms();
		}

		glStencilFunc(GL_ALWAYS, 0, 0xFF);
		glStencilMask(0xFF);

		glUniform1i(glGetUniformLocation(shader_program->program, "border"), 0);
	}

	void create_matrix_buffer() {
		glGenBuffers(1, &matrix_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, matrix_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 16 * max_count_models, NULL, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		for (Polygon& polygon : polygons)
			polygon.set_matrix_buffer(matrix_buffer);
	}

public:
	bool border = false, transparent = false;
	int id = -1;
	double border_width = 0.003;

	GraphObject(const GraphObject& object) {
		free_polygon_id = object.free_polygon_id;
		count_points = object.count_points;
		center = object.center;
		border_color = object.border_color;
		models = object.models;
		max_count_models = object.max_count_models;
		polygons = object.polygons;
		shader_program = object.shader_program;
		border = object.border;
		transparent = object.transparent;
		border_width = object.border_width;

		create_matrix_buffer();
		set_uniforms();

		glBindBuffer(GL_ARRAY_BUFFER, matrix_buffer);

		glBindBuffer(GL_COPY_READ_BUFFER, object.matrix_buffer);
		glBindBuffer(GL_COPY_WRITE_BUFFER, matrix_buffer);
		glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, sizeof(float) * 16 * max_count_models);
		glBindBuffer(GL_COPY_READ_BUFFER, 0);
		glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	GraphObject(int max_count_models = 0, Shader* shader = nullptr) {
		shader_program = shader;
		this->max_count_models = max_count_models;

		create_matrix_buffer();
		set_uniforms();
	}

	Polygon& operator[](int id) {
		for (Polygon& el : polygons) {
			if (el.id == id)
				return el;
		}
		std::cout << "ERROR::GRAPH_OBJECT::GET_POLYGON\n" << "Polygon with id " << id << " not found.\n";
		return *(new Polygon());
	}

	void set_shader(Shader* shader) {
		shader_program = shader;
		set_uniforms();

		for (Polygon& polygon : polygons)
			polygon.set_shader(shader);
	}

	void set_center() {
		center = Vect3(0, 0, 0);
		for (Polygon polygon : polygons)
			center += polygon.get_center() * polygon.get_count_points();
		center /= count_points;
	}

	std::vector < std::pair < Vect3, int > > get_objects() {
		std::vector < std::pair < Vect3, int > > objects;
		for (int i = 0; i < models.size(); i++)
			objects.push_back({ models[i] * center, i });
		return objects;
	}

	int add_polygon(Polygon polygon) {
		count_points += polygon.get_count_points();

		polygons.push_back(polygon);
		polygons.back().set_shader(shader_program);
		polygons.back().id = free_polygon_id++;
		polygons.back().set_matrix_buffer(matrix_buffer);

		return polygons.back().id;
	}

	int add_polygon(int size) {
		return add_polygon(Polygon(size));
	}

	int add_matrix(Matrix new_matrix = one_matrix(4)) {
		if (models.size() == max_count_models) {
			std::cout << "ERROR::GRAPH_OBJECT::ADD_MATRYX\nToo many instances created.\n";
			return -1;
		}

		models.push_back(new_matrix);
		return models.size() - 1;
	}

	void change_matrix(Matrix trans, int id = 0) {
		if (max_count_models == 0)
			return;

		int sz = models.size();
		id = (id % sz + sz) % sz;

		models[id] = trans * models[id];

		glBindBuffer(GL_ARRAY_BUFFER, matrix_buffer);
		glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * 16 * id, sizeof(float) * 16, models[id].value_ptr());
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void draw(Vect3 view_pos = Vect3(0, 0, 0), int id = -1) {
		if (shader_program == nullptr)
			return;

		if (border) {
			glClear(GL_STENCIL_BUFFER_BIT);
			glStencilFunc(GL_ALWAYS, 1, 0xFF);
		}

		draw_polygons(id);

		if (border)
			draw_border(view_pos, id);
	}

	~GraphObject() {
		glDeleteBuffers(1, &matrix_buffer);
	}
};
