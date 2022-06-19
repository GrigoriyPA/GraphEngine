#pragma once

#include <math.h>
#include <string>
#include "GraphObject.h"
#include "CommonClasses/Vect3.h"


class Light {
protected:
    Shader* shader_program;

public:
	Vect3 ambient = Vect3(0, 0, 0), diffuse = Vect3(0, 0, 0), specular = Vect3(0, 0, 0);
    int id = -1;

    Light() {
        shader_program = nullptr;
    }

    virtual void draw(int draw_id) = 0;

    virtual void set_shader(Shader* shader) = 0;
};


class DirLight : public Light {
public:
    Vect3 dir;

    DirLight(Vect3 dir = Vect3(1, 0, 0)) {
        shader_program = nullptr;
        this->dir = dir;
    }

    void draw(int draw_id) {
        if (shader_program == nullptr)
            return;

        std::string name = "lights[" + std::to_string(draw_id) + "].";
        glUniform1i(glGetUniformLocation(shader_program->program, (name + "type").c_str()), 0);
        glUniform3f(glGetUniformLocation(shader_program->program, (name + "direction").c_str()), dir.x, dir.y, dir.z);
        glUniform3f(glGetUniformLocation(shader_program->program, (name + "ambient").c_str()), ambient.x, ambient.y, ambient.z);
        glUniform3f(glGetUniformLocation(shader_program->program, (name + "diffuse").c_str()), diffuse.x, diffuse.y, diffuse.z);
        glUniform3f(glGetUniformLocation(shader_program->program, (name + "specular").c_str()), specular.x, specular.y, specular.z);
    }

    void set_shader(Shader* shader) {
        shader_program = shader;
    }
};


class PointLight : public Light {
    bool default_obj = true;

    Vect3 pos;
    GraphObject obj;

    void set_default_object() {
        obj = GraphObject(1, shader_program);
        int pol = obj.add_polygon(4);
        obj[pol].set_positions({
             0.5,  0.5, 0.5,
             0.5, -0.5, 0.5,
            -0.5, -0.5, 0.5,
            -0.5,  0.5, 0.5
        });
        obj[pol].material.emission = Vect3(1, 1, 1);
        obj[pol].material.light = true;

        pol = obj.add_polygon(obj[pol]);
        obj[pol].change_matrix(rotate_matrix(Vect3(0, 1, 0), PI / 2));
        pol = obj.add_polygon(obj[pol]);
        obj[pol].change_matrix(rotate_matrix(Vect3(0, 1, 0), PI / 2));
        pol = obj.add_polygon(obj[pol]);
        obj[pol].change_matrix(rotate_matrix(Vect3(0, 1, 0), PI / 2));
        pol = obj.add_polygon(obj[pol]);
        obj[pol].change_matrix(rotate_matrix(Vect3(0, 0, 1), PI / 2));
        pol = obj.add_polygon(obj[pol]);
        obj[pol].change_matrix(rotate_matrix(Vect3(0, 0, 1), PI));
        obj.change_matrix(scale_matrix(1.0 / 5));
        obj.change_matrix(trans_matrix(pos));
    }

public:
    double constant = 1, linear = 0, quadratic = 0;

    PointLight(Vect3 pos = Vect3(0, 0, 0)) {
        shader_program = nullptr;
        this->pos = pos;
    }

    void draw(int draw_id) {
        if (shader_program == nullptr)
            return;

        obj.draw();

        std::string name = "lights[" + std::to_string(draw_id) + "].";
        glUniform1i(glGetUniformLocation(shader_program->program, (name + "type").c_str()), 1);
        glUniform3f(glGetUniformLocation(shader_program->program, (name + "position").c_str()), pos.x, pos.y, pos.z);
        glUniform1f(glGetUniformLocation(shader_program->program, (name + "constant").c_str()), constant);
        glUniform1f(glGetUniformLocation(shader_program->program, (name + "linear").c_str()), linear);
        glUniform1f(glGetUniformLocation(shader_program->program, (name + "quadratic").c_str()), quadratic);
        glUniform3f(glGetUniformLocation(shader_program->program, (name + "ambient").c_str()), ambient.x, ambient.y, ambient.z);
        glUniform3f(glGetUniformLocation(shader_program->program, (name + "diffuse").c_str()), diffuse.x, diffuse.y, diffuse.z);
        glUniform3f(glGetUniformLocation(shader_program->program, (name + "specular").c_str()), specular.x, specular.y, specular.z);
    }

    void set_position(Vect3 new_pos) {
        obj.change_matrix(trans_matrix(new_pos - pos));
        pos = new_pos;
    }

    void set_shader(Shader* shader) {
        shader_program = shader;

        if (default_obj)
            set_default_object();
        else
            obj.set_shader(shader_program);
    }

    void set_object(GraphObject obj) {
        this->obj = obj;
        obj.set_shader(shader_program);
        default_obj = false;
    }

    void delete_object() {
        set_default_object();
        default_obj = true;
    }
};


class SpotLight : public Light {
    bool default_obj = true;

    Vect3 pos;
    GraphObject obj;

    void set_default_object() {
        obj = GraphObject(1, shader_program);
        int pol = obj.add_polygon(4);
        obj[pol].set_positions({
             0.5,  0.5, 0.5,
             0.5, -0.5, 0.5,
            -0.5, -0.5, 0.5,
            -0.5,  0.5, 0.5
        });
        obj[pol].material.emission = Vect3(1, 1, 1);
        obj[pol].material.light = true;

        pol = obj.add_polygon(obj[pol]);
        obj[pol].change_matrix(rotate_matrix(Vect3(0, 1, 0), PI / 2));
        pol = obj.add_polygon(obj[pol]);
        obj[pol].change_matrix(rotate_matrix(Vect3(0, 1, 0), PI / 2));
        pol = obj.add_polygon(obj[pol]);
        obj[pol].change_matrix(rotate_matrix(Vect3(0, 1, 0), PI / 2));
        pol = obj.add_polygon(obj[pol]);
        obj[pol].change_matrix(rotate_matrix(Vect3(0, 0, 1), PI / 2));
        pol = obj.add_polygon(obj[pol]);
        obj[pol].change_matrix(rotate_matrix(Vect3(0, 0, 1), PI));
        obj.change_matrix(rotate_matrix(Vect3(0, 0, 1), PI / 4));
        obj.change_matrix(scale_matrix(1.0 / 5));
        obj.change_matrix(trans_matrix(pos));
    }

public:
    double constant = 1, linear = 0, quadratic = 0;

    double cut_in, cut_out;
    Vect3 dir;

    SpotLight(Vect3 pos = Vect3(0, 0, 0), Vect3 dir = Vect3(1, 0, 0), double cut_in = 0, double cut_out = 0) {
        shader_program = nullptr;
        this->pos = pos;
        this->dir = dir;
        this->cut_in = cut_in;
        this->cut_out = cut_out;
    }

    void draw(int draw_id) {
        if (shader_program == nullptr)
            return;

        obj.draw();

        std::string name = "lights[" + std::to_string(draw_id) + "].";
        glUniform1i(glGetUniformLocation(shader_program->program, (name + "type").c_str()), 2);
        glUniform3f(glGetUniformLocation(shader_program->program, (name + "position").c_str()), pos.x, pos.y, pos.z);
        glUniform3f(glGetUniformLocation(shader_program->program, (name + "direction").c_str()), dir.x, dir.y, dir.z);
        glUniform1f(glGetUniformLocation(shader_program->program, (name + "cut_in").c_str()), cos(cut_in));
        glUniform1f(glGetUniformLocation(shader_program->program, (name + "cut_out").c_str()), cos(cut_out));
        glUniform1f(glGetUniformLocation(shader_program->program, (name + "constant").c_str()), constant);
        glUniform1f(glGetUniformLocation(shader_program->program, (name + "linear").c_str()), linear);
        glUniform1f(glGetUniformLocation(shader_program->program, (name + "quadratic").c_str()), quadratic);
        glUniform3f(glGetUniformLocation(shader_program->program, (name + "ambient").c_str()), ambient.x, ambient.y, ambient.z);
        glUniform3f(glGetUniformLocation(shader_program->program, (name + "diffuse").c_str()), diffuse.x, diffuse.y, diffuse.z);
        glUniform3f(glGetUniformLocation(shader_program->program, (name + "specular").c_str()), specular.x, specular.y, specular.z);
    }

    void set_position(Vect3 new_pos) {
        obj.change_matrix(trans_matrix(new_pos - pos));
        pos = new_pos;
    }

    void set_shader(Shader* shader) {
        shader_program = shader;

        if (default_obj)
            set_default_object();
        else
            obj.set_shader(shader_program);
    }

    void set_object(GraphObject obj) {
        this->obj = obj;
        obj.set_shader(shader_program);
        default_obj = false;
    }

    void delete_object() {
        set_default_object();
        default_obj = true;
    }
};


void draw_default_light(int draw_id, Shader* shader_program) {
    std::string name = "lights[" + std::to_string(draw_id) + "].";
    glUniform1i(glGetUniformLocation(shader_program->program, (name + "type").c_str()), 0);
    glUniform3f(glGetUniformLocation(shader_program->program, (name + "direction").c_str()), 1, 0, 0);
    glUniform3f(glGetUniformLocation(shader_program->program, (name + "ambient").c_str()), 0, 0, 0);
    glUniform3f(glGetUniformLocation(shader_program->program, (name + "diffuse").c_str()), 0, 0, 0);
    glUniform3f(glGetUniformLocation(shader_program->program, (name + "specular").c_str()), 0, 0, 0);
}
