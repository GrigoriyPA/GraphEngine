#pragma once

#include <math.h>
#include <algorithm>
#include <iostream>
#include <vector>
#include "GraphObject.h"
#include "Light.h"
#include "Kernel.h"
#include "CommonClasses/Matrix.h"
#include "CommonClasses/Random.h"


struct TransparentObject {
	int id;
	double dist;
	GraphObject* object;

	TransparentObject(Vect3 cam_position, GraphObject* object, std::pair < Vect3, int > desc) {
		this->object = object;
		dist = (cam_position - desc.first).length();
		id = desc.second;
	}

	bool operator <(TransparentObject other) const {
		return dist < other.dist;
	}
};


class GraphEngine {
	bool grayscale = false;
	int free_object_id = 0;
	double gamma = 2.2, kernel_offset = 1.0 / 300.0;
	Vect3 cam_direction = Vect3(0, 0, 1), cam_horizont = Vect3(1, 0, 0);

	unsigned int framebuffer, tex_color_buffer, screen_coord_vao, screen_coord_vbo;
	double screen_ratio, min_distance, max_distance, fov;
	std::vector < GraphObject > objects;
	std::vector < Light* > lights;
	sf::RenderWindow* window;
	Matrix projection;
	Kernel kernel;
	Shader main_shader, post_shader;

	void init_gl() {
		glewInit();
		glViewport(0, 0, window->getSize().x, window->getSize().y);
		glClearColor(0.2, 0.3, 0.3, 1.0);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_STENCIL_TEST);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_CULL_FACE);
	}

	void set_uniforms() {
		main_shader.use();
		glUniformMatrix4fv(glGetUniformLocation(main_shader.program, "projection"), 1, GL_FALSE, projection.value_ptr());
		glUniform1i(glGetUniformLocation(main_shader.program, "diffuse_map"), 0);
		glUniform1i(glGetUniformLocation(main_shader.program, "specular_map"), 1);
		glUniform1i(glGetUniformLocation(main_shader.program, "emission_map"), 2);
		glUniform1f(glGetUniformLocation(main_shader.program, "gamma"), gamma);

		post_shader.use();
		kernel.use(&post_shader);
		glUniform1i(glGetUniformLocation(post_shader.program, "grayscale"), grayscale);
		glUniform1f(glGetUniformLocation(post_shader.program, "offset"), kernel_offset);
	}

	void create_screen_coord() {
		glGenVertexArrays(1, &screen_coord_vao);
		glBindVertexArray(screen_coord_vao);

		glGenBuffers(1, &screen_coord_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, screen_coord_vbo);

		float vertices[] = {
			 1,  1, 1, 1,
			-1,  1, 0, 1,
			-1, -1, 0, 0,
			-1, -1, 0, 0,
			 1, -1, 1, 0,
			 1,  1, 1, 1
		};
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid*)(2 * sizeof(float)));
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void create_framebuffer() {
		glGenFramebuffers(1, &framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

		glGenTextures(1, &tex_color_buffer);
		glBindTexture(GL_TEXTURE_2D, tex_color_buffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window->getSize().x, window->getSize().y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_color_buffer, 0);

		unsigned int rbo;
		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, window->getSize().x, window->getSize().y);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "ERROR::GRAPH_ENGINE::FRAMEBUFFER::\nFramebuffer is not complete.\n";
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void draw_lights() {
		for (int i = 0; i < lights.size(); i++) {
			if (lights[i] == nullptr) {
				draw_default_light(i, &main_shader);
				continue;
			}

			lights[i]->draw(i);
		}
	}

	void draw_objects() {
		std::vector < TransparentObject > transparent_objects;
		for (GraphObject& object : objects) {
			if (object.transparent) {
				for (std::pair < Vect3, int > el : object.get_objects())
					transparent_objects.push_back(TransparentObject(cam_position, &object, el));
				continue;
			}

			object.draw(cam_position);
		}

		std::sort(transparent_objects.rbegin(), transparent_objects.rend());
		for (TransparentObject object : transparent_objects)
			object.object->draw(cam_position, object.id);
	}

	void draw_framebuffer() {
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		main_shader.use();

		Matrix view = Matrix(cam_horizont, cam_direction ^ cam_horizont, cam_direction).transp() * trans_matrix(-cam_position);
		glUniformMatrix4fv(glGetUniformLocation(main_shader.program, "view"), 1, GL_FALSE, view.value_ptr());
		
		glUniform3f(glGetUniformLocation(main_shader.program, "view_pos"), cam_position.x, cam_position.y, cam_position.z);

		draw_lights();
		draw_objects();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void draw_mainbuffer() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		post_shader.use();
		glDisable(GL_DEPTH_TEST);

		glBindVertexArray(screen_coord_vao);
		glBindTexture(GL_TEXTURE_2D, tex_color_buffer);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glEnable(GL_DEPTH_TEST);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(0);
	}

public:
	Vect3 cam_position = Vect3(0, 0, 0);

	GraphEngine(const GraphEngine& object) {
		free_object_id = object.free_object_id;
		cam_position = object.cam_position;
		cam_direction = object.cam_direction;
		cam_horizont = object.cam_horizont;
		screen_ratio = object.screen_ratio;
		min_distance = object.min_distance;
		max_distance = object.max_distance;
		objects = object.objects;
		lights = object.lights;
		window = object.window;
		main_shader = object.main_shader;
		post_shader = object.post_shader;
		grayscale = object.grayscale;
		gamma = object.gamma;
		kernel_offset = object.kernel_offset;
		fov = object.fov;
		kernel = object.kernel;
		projection = object.projection;

		create_framebuffer();
		create_screen_coord();
		set_uniforms();
	}

	GraphEngine(sf::RenderWindow* window, double fov, double min_distance, double max_distance) {
		this->window = window;
		window->setActive(true);
		init_gl();

		screen_ratio = ((double)window->getSize().x) / ((double)window->getSize().y);
		this->fov = fov;
		this->min_distance = min_distance;
		this->max_distance = max_distance;
		main_shader = Shader("GraphEngine/Shaders/MainShader", "GraphEngine/Shaders/MainShader");
		post_shader = Shader("GraphEngine/Shaders/PostShader", "GraphEngine/Shaders/PostShader");
		lights.resize(main_shader.get_count_lights(), nullptr);

		projection = scale_matrix(Vect3(1 / tan(fov / 2), screen_ratio / tan(fov / 2), (min_distance + max_distance) / (max_distance - min_distance))) * trans_matrix(Vect3(0, 0, -2 * min_distance * max_distance / (min_distance + max_distance)));
		projection[3][3] = 0;
		projection[3][2] = 1;

		create_framebuffer();
		create_screen_coord();
		set_uniforms();
	}

	GraphObject& operator[](int id) {
		for (GraphObject& el : objects) {
			if (el.id == id)
				return el;
		}
		std::cout << "ERROR::GRAPH_ENGINE::GET_OBJECT\n" << "Object with id " << id << " not found.\n";
		return *(new GraphObject());
	}

	void set_clear_color(Vect3 color) {
		glClearColor(color.x, color.y, color.z, color.w);
	}

	void set_light(int id, Light* new_light) {
		int sz = lights.size();
		lights[(id % sz + sz) % sz] = new_light;
		if (new_light != nullptr)
			lights[(id % sz + sz) % sz]->set_shader(&main_shader);
	}

	void set_kernel(Kernel new_kernel) {
		kernel = new_kernel;
		post_shader.use();
		kernel.use(&post_shader);
	}

	void set_grayscale(bool grayscale) {
		this->grayscale = grayscale;
		post_shader.use();
		glUniform1i(glGetUniformLocation(post_shader.program, "grayscale"), grayscale);
	}

	void set_kernel_offset(double kernel_offset) {
		this->kernel_offset = kernel_offset;
		post_shader.use();
		glUniform1f(glGetUniformLocation(post_shader.program, "offset"), kernel_offset);
	}

	Shader* get_main_shader() {
		return &main_shader;
	}

	Light* get_light(int id) {
		int sz = lights.size();
		return lights[(id % sz + sz) % sz];
	}

	int get_count_lights() {
		return lights.size();
	}

	Vect3 get_cam_direction() {
		return cam_direction;
	}

	Vect3 get_cam_horizont() {
		return cam_horizont;
	}

	Vect3 get_cam_vertical() {
		return cam_direction ^ cam_horizont;
	}

	int add_object(GraphObject object) {
		objects.push_back(object);
		objects.back().set_shader(&main_shader);
		objects.back().id = free_object_id++;
		return objects.back().id;
	}

	void draw() {
		window->setActive(true);
		draw_framebuffer();
		draw_mainbuffer();
	}

	void rotate_cam(Vect3 axis, double angle) {
		Matrix rotate = rotate_matrix(axis, angle);
		cam_direction = rotate * cam_direction;
		cam_horizont = rotate * cam_horizont;
	}

	~GraphEngine() {
		glDeleteVertexArrays(1, &screen_coord_vao);
		glDeleteBuffers(1, &screen_coord_vbo);
		glDeleteFramebuffers(1, &framebuffer);
		glDeleteTextures(1, &tex_color_buffer);
	}
};
