#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <FreeImage/FreeImage.h>

#include "Shaders/LoadShaders.h"
#include "My_Shading.h"
#include "my_geom_objects.h"
GLuint h_ShaderProgram_simple, h_ShaderProgram_TXPS, h_ShaderProgram_TXGS; // handles to shader programs

// for Phong Shading (Textured) shaders
#define NUMBER_OF_LIGHT_SUPPORTED 4 
GLint loc_global_ambient_color;
loc_light_Parameters loc_light[NUMBER_OF_LIGHT_SUPPORTED];

GLint loc_ModelViewProjectionMatrix_TXPS, loc_ModelViewMatrix_TXPS, loc_ModelViewMatrixInvTrans_TXPS;
GLint loc_ModelViewProjectionMatrix_TXGS, loc_ModelViewMatrix_TXGS, loc_ModelViewMatrixInvTrans_TXGS;
GLint loc_texture, loc_flag_texture_mapping, loc_flag_fog;

// include glm/*.hpp only if necessary
//#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> //translate, rotate, scale, lookAt, perspective, etc.
#include <glm/gtc/matrix_inverse.hpp> // inverseTranspose, etc.
glm::mat4 ModelViewProjectionMatrix, ModelViewMatrix;
glm::mat3 ModelViewMatrixInvTrans;
glm::mat4 ViewProjectionMatrix, ViewMatrix, ProjectionMatrix;

#define TO_RADIAN 0.01745329252f  
#define TO_DEGREE 57.295779513f

// lights in scene
Light_Parameters light[NUMBER_OF_LIGHT_SUPPORTED];

int flag_texture_mapping;
int flag_shading = 0;
int texture_flag = 1;
int spin_flag = 0;

// fog stuffs
// you could control the fog parameters interactively: FOG_COLOR, FOG_NEAR_DISTANCE, FOG_FAR_DISTANCE   
int flag_fog;

// callbacks
float PRP_distance_scale[6] = { 0.5f, 1.0f, 2.5f, 5.0f, 10.0f, 20.0f };

/*********************************  START: camera *********************************/
typedef struct _Camera {
	float pos[3];
	float uaxis[3], vaxis[3], naxis[3];
	float fovy, aspect_ratio, near_c, far_c;
	int move;
} Camera;

Camera camera;

void set_ViewMatrix_from_camera_frame(void) {
	ViewMatrix = glm::mat4(camera.uaxis[0], camera.vaxis[0], camera.naxis[0], 0.0f,
		camera.uaxis[1], camera.vaxis[1], camera.naxis[1], 0.0f,
		camera.uaxis[2], camera.vaxis[2], camera.naxis[2], 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
	ViewMatrix = glm::translate(ViewMatrix, glm::vec3(-camera.pos[0], -camera.pos[1], -camera.pos[2]));
}

void initialize_camera(void) {
	camera.pos[0] = 1500.0f; camera.pos[1] = 300.0f;  camera.pos[2] = 0.0f;
	camera.uaxis[0] = camera.uaxis[1] = 0.0f; camera.uaxis[2] = -1.0f;
	camera.vaxis[0] = camera.vaxis[2] = 0.0f; camera.vaxis[1] = 1.0f;
	camera.naxis[1] = camera.naxis[2] = 0.0f; camera.naxis[0] = 1.0f;

	camera.move = 0;
	camera.fovy =50.0f, camera.aspect_ratio = 1.0f; camera.near_c = 0.1f; camera.far_c = 10000.0f;

	set_ViewMatrix_from_camera_frame();

	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
}

enum axes {X_AXIS, Y_AXIS, Z_AXIS };
int flag_translation_axis;
int flag_trans_ro_po;
#define CAM_TSPEED 0.3f

void renew_cam_position(int del) {
	switch (flag_translation_axis) {
	case X_AXIS:
		camera.pos[0] += CAM_TSPEED * del * (camera.uaxis[0]);
		camera.pos[1] += CAM_TSPEED * del * (camera.uaxis[1]);
		camera.pos[2] += CAM_TSPEED * del * (camera.uaxis[2]);
		break;
	case Y_AXIS:
		camera.pos[0] += CAM_TSPEED * del * (camera.vaxis[0]);
		camera.pos[1] += CAM_TSPEED * del * (camera.vaxis[1]);
		camera.pos[2] += CAM_TSPEED * del * (camera.vaxis[2]);
		break;
	case Z_AXIS:
		camera.pos[0] += CAM_TSPEED * del * (-camera.naxis[0]);
		camera.pos[1] += CAM_TSPEED * del * (-camera.naxis[1]);
		camera.pos[2] += CAM_TSPEED * del * (-camera.naxis[2]);
		break;
	}
}

#define CAM_RSPEED 0.1f
void renew_cam_orientation_rotation_around_v_axis(int angle) {
	// let's get a help from glm
	glm::mat3 RotationMatrix;
	glm::vec3 direction;

	RotationMatrix = glm::mat3(glm::rotate(glm::mat4(1.0), CAM_RSPEED * TO_RADIAN * angle,
		glm::vec3(camera.vaxis[0], camera.vaxis[1], camera.vaxis[2])));

	direction = RotationMatrix * glm::vec3(camera.uaxis[0], camera.uaxis[1], camera.uaxis[2]);
	camera.uaxis[0] = direction.x; camera.uaxis[1] = direction.y; camera.uaxis[2] = direction.z;
	direction = RotationMatrix * glm::vec3(camera.naxis[0], camera.naxis[1], camera.naxis[2]);
	camera.naxis[0] = direction.x; camera.naxis[1] = direction.y; camera.naxis[2] = direction.z;
}

void renew_rotation(int angle) {
	glm::mat3 RotationMatrix;
	glm::vec3 direction;

	switch (flag_translation_axis) {
	case X_AXIS:
		RotationMatrix = glm::mat3(glm::rotate(glm::mat4(1.0), CAM_RSPEED * TO_RADIAN * angle,
			glm::vec3(camera.uaxis[0], camera.uaxis[1], camera.uaxis[2])));
		direction = RotationMatrix * glm::vec3(camera.vaxis[0], camera.vaxis[1], camera.vaxis[2]);
		camera.vaxis[0] = direction.x; camera.vaxis[1] = direction.y; camera.vaxis[2] = direction.z;
		direction = RotationMatrix * glm::vec3(camera.naxis[0], camera.naxis[1], camera.naxis[2]);
		camera.naxis[0] = direction.x; camera.naxis[1] = direction.y; camera.naxis[2] = direction.z;
		break;
	case Y_AXIS:
		RotationMatrix = glm::mat3(glm::rotate(glm::mat4(1.0), CAM_RSPEED * TO_RADIAN * angle,
			glm::vec3(camera.vaxis[0], camera.vaxis[1], camera.vaxis[2])));
		direction = RotationMatrix * glm::vec3(camera.uaxis[0], camera.uaxis[1], camera.uaxis[2]);
		camera.uaxis[0] = direction.x; camera.uaxis[1] = direction.y; camera.uaxis[2] = direction.z;
		direction = RotationMatrix * glm::vec3(camera.naxis[0], camera.naxis[1], camera.naxis[2]);
		camera.naxis[0] = direction.x; camera.naxis[1] = direction.y; camera.naxis[2] = direction.z;
		break;
	case Z_AXIS:
		RotationMatrix = glm::mat3(glm::rotate(glm::mat4(1.0), CAM_RSPEED * TO_RADIAN * angle,
			glm::vec3(camera.naxis[0], camera.naxis[1], camera.naxis[2])));
		direction = RotationMatrix * glm::vec3(camera.uaxis[0], camera.uaxis[1], camera.uaxis[2]);
		camera.uaxis[0] = direction.x; camera.uaxis[1] = direction.y; camera.uaxis[2] = direction.z;
		direction = RotationMatrix * glm::vec3(camera.vaxis[0], camera.vaxis[1], camera.vaxis[2]);
		camera.vaxis[0] = direction.x; camera.vaxis[1] = direction.y; camera.vaxis[2] = direction.z;
		break;
	}
}


/*********************************  END: camera *********************************/


void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(h_ShaderProgram_simple);
	ModelViewMatrix = glm::scale(ViewMatrix, glm::vec3(50.0f, 50.0f, 50.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_simple, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glLineWidth(2.0f);
	draw_axes();
	glLineWidth(1.0f);


	//수정시작

	if (flag_shading == 0) glUseProgram(h_ShaderProgram_TXPS);
	else glUseProgram(h_ShaderProgram_TXGS);

	set_material_floor();

	if(spin_flag==40) glUniform1i(loc_texture, TEXTURE_ID_FLOOR2);
	else glUniform1i(loc_texture, TEXTURE_ID_FLOOR);

	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(-500.0f, 0.0f, 500.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(1000.0f, 1000.0f, 1000.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));

	if (flag_shading == 0) {
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	}
	else {
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXGS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_TXGS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXGS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	}

	draw_floor();
	

	if (flag_draw_screen==1) {
		set_material_screen();
		ModelViewMatrix = glm::scale(ViewMatrix, glm::vec3(350.0f, 350.0f, 350.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
		ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
		ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));

		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);

		glUniform1i(loc_screen_effect, flag_screen_effect);
		draw_screen();
		glUniform1i(loc_screen_effect, 0);
	}

 	set_material_tiger();
	glUniform1i(loc_texture, TEXTURE_ID_TIGER);

	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(-200.0f, 0.0f, -400.0f));
	ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(((rotation_angle_tiger * TO_RADIAN) -180*TO_RADIAN)*100, 0.0f, sin(rotation_angle_tiger * TO_RADIAN)*100));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, -fabs((rotation_angle_tiger * TO_RADIAN) - (180 * TO_RADIAN)) - (180.0f * TO_RADIAN), glm::vec3(0.0f, 1.0f, 0.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(0.5f, 0.5f, 0.5f));

	ModelViewMatrix = glm::rotate(ModelViewMatrix, t_stamp * TO_RADIAN * spin_flag, glm::vec3(1.0f, 0.0f, 0.0f));

	ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));

	if (flag_shading == 0) {
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	}
	else {
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXGS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_TXGS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXGS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	}
	
	draw_tiger();

	glUseProgram(h_ShaderProgram_simple);
	ModelViewProjectionMatrix = glm::scale(ModelViewProjectionMatrix, glm::vec3(20.0f, 20.0f, 20.0f));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_simple, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_axes();

	if (flag_shading == 0) glUseProgram(h_ShaderProgram_TXPS);
	else glUseProgram(h_ShaderProgram_TXGS);
	set_material_ben();
	glUniform1i(loc_texture, TEXTURE_ID_BEN);

	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(t_stamp - 500.0f, 0.0f,0.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(100.0f*t_stamp*0.005f+50.0f, -100.0f * t_stamp * 0.005f - 50.0f, -100.0f * t_stamp * 0.005f- 50.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, t_stamp * TO_RADIAN * spin_flag, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 270.0f * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));

	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));

	if (flag_shading == 0) {
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	}
	else {
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXGS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_TXGS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXGS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	}
	draw_ben();


	set_material_tiger();
	glUniform1i(loc_texture, TEXTURE_ID_SPIDER);
	if(spider_flag==0) ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(150.0f, spider_stamp, 150.0f));
	else if(spider_flag == 1) ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(-150.0f, spider_stamp, 150.0f));
	else if(spider_flag == 2) ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(-150.0f, spider_stamp, -150.0f));
	else ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(150.0f, spider_stamp, -150.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(50.0f, -50.0f, 50.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f * TO_RADIAN * spider_flag + 45.0f * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, t_stamp * TO_RADIAN * spin_flag, glm::vec3(0.0f, 1.0f, 0.0f));

	ModelViewMatrix = glm::rotate(ModelViewMatrix, 90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));

	if (flag_shading == 0) {
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	}
	else {
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXGS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_TXGS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXGS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	}
	
	draw_spider();

	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(-150.0f, 50.0f, 250.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, t_stamp * TO_RADIAN * spin_flag, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, -45.0f*TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(250.0f, 250.0f, 250.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));

	if (flag_shading == 0) {
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	}
	else {
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXGS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_TXGS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXGS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	}
	draw_cow();


	if (texture_flag == 1) {
		set_material_tiger();
		glUniform1i(loc_texture, TEXTURE_ID_TIGER);
	}
	else glUniform1i(loc_texture, TEXTURE_ID_BIKE);
	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(300.0f, 0.0f, 300.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, t_stamp * TO_RADIAN * spin_flag, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, -45.0f * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(60.0f, 60.0f, 60.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));

	if (flag_shading == 0) {
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	}
	else {
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXGS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_TXGS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXGS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	}
	draw_bike();



	glm::vec4 position_EC;
	glm::vec3 direction_EC;

	set_material_tiger();
	glUniform1i(loc_texture, TEXTURE_ID_TIGER);
	
	direction_EC = glm::mat3(ModelViewMatrix) * glm::vec3(light[3].spot_direction[0], light[3].spot_direction[1],
		light[3].spot_direction[2]);
	glUniform3fv(loc_light[3].spot_direction, 1, &direction_EC[0]);
	position_EC = ModelViewMatrix * glm::vec4(light[3].position[0], light[3].position[1],
		light[3].position[2], light[3].position[3]);
	glUniform4fv(loc_light[3].position, 1, &position_EC[0]);
	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(500.0f, 0.0f, 0.0f));
	
	direction_EC = glm::mat3(ModelViewMatrix) * glm::vec3(light[3].spot_direction[0], light[3].spot_direction[1],
		light[3].spot_direction[2]);
	glUniform3fv(loc_light[3].spot_direction, 1, &direction_EC[0]);
	position_EC = ModelViewMatrix * glm::vec4(light[3].position[0], light[3].position[1],
		light[3].position[2], light[3].position[3]);
	glUniform4fv(loc_light[3].position, 1, &position_EC[0]);
	ModelViewMatrix = glm::rotate(ModelViewMatrix, -rotation_algle_bus, glm::vec3(0.0f, 1.0f, 0.0f));
	
	direction_EC = glm::mat3(ModelViewMatrix) * glm::vec3(light[3].spot_direction[0], light[3].spot_direction[1],
		light[3].spot_direction[2]);
	glUniform3fv(loc_light[3].spot_direction, 1, &direction_EC[0]);
	position_EC = ModelViewMatrix * glm::vec4(light[3].position[0], light[3].position[1],
		light[3].position[2], light[3].position[3]);
	glUniform4fv(loc_light[3].position, 1, &position_EC[0]);
	ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(-500.0f, 0.0f, 0.0f));
	
	direction_EC = glm::mat3(ModelViewMatrix) * glm::vec3(light[3].spot_direction[0], light[3].spot_direction[1],
		light[3].spot_direction[2]);
	glUniform3fv(loc_light[3].spot_direction, 1, &direction_EC[0]);
	position_EC = ModelViewMatrix * glm::vec4(light[3].position[0], light[3].position[1],
		light[3].position[2], light[3].position[3]);
	glUniform4fv(loc_light[3].position, 1, &position_EC[0]);
	ModelViewMatrix = glm::rotate(ModelViewMatrix, t_stamp * TO_RADIAN * spin_flag, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(3.0f, 3.0f, 3.0f));

	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	if (flag_shading == 0) {
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	}
	else {
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXGS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_TXGS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXGS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	}
	draw_bus();
	



	glUseProgram(0);

	glutSwapBuffers();
}

void timer_scene(int value) {
	timestamp_scene = (timestamp_scene + 1) % UINT_MAX;
	cur_frame_tiger = timestamp_scene % N_TIGER_FRAMES;
	cur_frame_ben = timestamp_scene % N_BEN_FRAMES;
	cur_frame_spider = timestamp_scene % N_SPIDER_FRAMES;

	rotation_angle_tiger = (timestamp_scene % 360);
	rotation_algle_bus = (timestamp_scene % 90) * TO_RADIAN;

	t_stamp = timestamp_scene % 500;
	spider_stamp = timestamp_scene % 100;
	if (spider_stamp == 0) spider_flag = (spider_flag + 1) % 4;

	glutPostRedisplay();
	if (flag_tiger_animation)
		glutTimerFunc(10, timer_scene, 0);
}



int prevx, prevy;

void motion(int x, int y) {
	if (!camera.move) return;

	if(flag_trans_ro_po == 0) renew_cam_position(prevx - x);
	else renew_rotation(prevx - x);
	//renew_cam_orientation_rotation_around_v_axis(prevx - x);
	

	prevx = x; prevy = y;

	set_ViewMatrix_from_camera_frame();
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

	glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {
	if ((button == GLUT_LEFT_BUTTON)) {
		if (state == GLUT_DOWN) {
			camera.move = 1;
			prevx = x; prevy = y;
		}
		else if (state == GLUT_UP) camera.move = 0;
	}

}

void initialize_lights_and_material();
void prepare_shader_program();
void set_up_scene_lights();

void keyboard(unsigned char key, int x, int y) {
	static int flag_cull_face = 0;
	static int PRP_distance_level = 4;

	static int flag_blind_effect = 0;

	glm::vec4 position_EC;
	glm::vec3 direction_EC;

	if ((key >= '0') && (key <= '0' + NUMBER_OF_LIGHT_SUPPORTED - 1)) {
		int light_ID = (int)(key - '0');

		if (flag_shading == 0) glUseProgram(h_ShaderProgram_TXPS);
		else glUseProgram(h_ShaderProgram_TXGS);
		light[light_ID].light_on = 1 - light[light_ID].light_on;
		glUniform1i(loc_light[light_ID].light_on, light[light_ID].light_on);
		glUseProgram(0);

		glutPostRedisplay();
		return;
	}

	switch (key) {
	case 'x':
		flag_translation_axis = X_AXIS;
		break;
	case 'y':
		flag_translation_axis = Y_AXIS;
		break;
	case 'z':
		flag_translation_axis = Z_AXIS;
		break;
	case 32:
		flag_trans_ro_po = (flag_trans_ro_po + 1) % 2;
		break;
	case 'a': // toggle the animation effect.
		flag_tiger_animation = 1 - flag_tiger_animation;
		if (flag_tiger_animation) {
			glutTimerFunc(100, timer_scene, 0);
			fprintf(stdout, "^^^ Animation mode ON.\n");
		}
		else
			fprintf(stdout, "^^^ Animation mode OFF.\n");
		break;
	case '`':
		flag_texture_mapping = 1 - flag_texture_mapping;
		glUseProgram(h_ShaderProgram_TXPS);
		glUniform1i(loc_flag_texture_mapping, flag_texture_mapping);
		glUseProgram(0);
		glutPostRedisplay();
		break;
	case 'q':
		flag_shading = 1 - flag_shading;

		prepare_shader_program();
		initialize_lights_and_material();
		set_up_scene_lights();
		glutPostRedisplay();
	
	case 'w':
		flag_draw_screen = 1;
		flag_blind_effect = 1 - flag_blind_effect;

		if (flag_shading == 0) glUseProgram(h_ShaderProgram_TXPS);
		else glUseProgram(h_ShaderProgram_TXGS);
		glUniform1i(loc_blind_effect, flag_blind_effect);
		glUseProgram(0);

		glutPostRedisplay();
	case '4':
		flag_draw_screen = 1 - flag_draw_screen;

		glutPostRedisplay();
		break;
	case '5':
		if (flag_draw_screen) {
			flag_screen_effect = 1 - flag_screen_effect;
			glutPostRedisplay();
		}

		break;
#define SCEEN_MAX_FREQUENCY 50.0f
	case 'e':
		flag_draw_screen = 1;

		if (flag_draw_screen) {
			screen_frequency += 1.0f;
			if (screen_frequency > SCEEN_MAX_FREQUENCY)
				screen_frequency = SCEEN_MAX_FREQUENCY;
			glUseProgram(h_ShaderProgram_TXPS);
			glUniform1f(loc_screen_frequency, screen_frequency);
			glUseProgram(0);
			glutPostRedisplay();
		}
		break;
#define SCEEN_MIN_FREQUENCY 1.0f
	case 'f':
		if (flag_draw_screen) {
			screen_frequency -= 1.0f;
			if (screen_frequency < SCEEN_MIN_FREQUENCY)
				screen_frequency = SCEEN_MIN_FREQUENCY;
			glUseProgram(h_ShaderProgram_TXPS);
			glUniform1f(loc_screen_frequency, screen_frequency);
			glUseProgram(0);
			glutPostRedisplay();
		}
		break;
	case 'r':
		spin_flag = (spin_flag + 10) % 50;
		break;
	case 't':
		texture_flag = 1 - texture_flag;

		glutPostRedisplay();
		break;
		/*
	case 'c':
		flag_cull_face = (flag_cull_face + 1) % 3;
		switch (flag_cull_face) {
		case 0:
			glDisable(GL_CULL_FACE);
			glutPostRedisplay();
			break;
		case 1: // cull back faces;
			glCullFace(GL_BACK);
			glEnable(GL_CULL_FACE);
			glutPostRedisplay();
			break;
		case 2: // cull front faces;
			glCullFace(GL_FRONT);
			glEnable(GL_CULL_FACE);
			glutPostRedisplay();
			break;
		}
		break;
		*/
	case 'u':
		if (camera.fovy < 110.0f) camera.fovy += 10.0f;

		ProjectionMatrix = glm::perspective(camera.fovy * TO_RADIAN, camera.aspect_ratio, camera.near_c, camera.far_c);
		ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

		glutPostRedisplay();
		break;
	case 'd':
		if(camera.fovy >20.0f) camera.fovy -= 10.0f;

		ProjectionMatrix = glm::perspective(camera.fovy * TO_RADIAN, camera.aspect_ratio, camera.near_c, camera.far_c);
		ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

		glutPostRedisplay();
		break;
		/*
	case 'p':
		flag_polygon_fill = 1 - flag_polygon_fill;
		if (flag_polygon_fill)
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glutPostRedisplay();
		break;
		*/
	case 27: // ESC key
		glutLeaveMainLoop(); // Incur destuction callback for cleanups
		break;
	}
}

//----시작
void reshape(int width, int height) {
	//float aspect_ratio;
	
	camera.aspect_ratio = (float)width / height;
	glViewport(0, 0, width, height);
	
	//aspect_ratio = (float) width / height;
	ProjectionMatrix = glm::perspective(camera.fovy*TO_RADIAN, camera.aspect_ratio, camera.near_c, camera.far_c);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

	glutPostRedisplay();
}

void cleanup(void) {
	glDeleteVertexArrays(1, &axes_VAO); 
	glDeleteBuffers(1, &axes_VBO);

	glDeleteVertexArrays(1, &rectangle_VAO);
	glDeleteBuffers(1, &rectangle_VBO);

	glDeleteVertexArrays(1, &tiger_VAO);
	glDeleteBuffers(1, &tiger_VBO);

	glDeleteTextures(N_TEXTURES_USED, texture_names);
}

void register_callbacks(void) {
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutReshapeFunc(reshape);
	glutTimerFunc(100, timer_scene, 0);
	glutCloseFunc(cleanup);
}

void prepare_shader_program(void) {
	int i;
	char string[256];
	ShaderInfo shader_info_simple[3] = {
		{ GL_VERTEX_SHADER, "Shaders/simple.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/simple.frag" },
		{ GL_NONE, NULL }
	};
	ShaderInfo shader_info_TXPS[3] = {
		{ GL_VERTEX_SHADER, "Shaders/Phong_Tx.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/Phong_Tx.frag" },
		{ GL_NONE, NULL }
	};
	ShaderInfo shader_info_TXGS[3] = {
		{ GL_VERTEX_SHADER, "Shaders/Gouraud_Tx.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/Gouraud_Tx.frag" },
		{ GL_NONE, NULL }
	};

	h_ShaderProgram_simple = LoadShaders(shader_info_simple);
	loc_primitive_color = glGetUniformLocation(h_ShaderProgram_simple, "u_primitive_color");
	loc_ModelViewProjectionMatrix_simple = glGetUniformLocation(h_ShaderProgram_simple, "u_ModelViewProjectionMatrix");

	if (flag_shading == 0) {
		h_ShaderProgram_TXPS = LoadShaders(shader_info_TXPS);
		loc_ModelViewProjectionMatrix_TXPS = glGetUniformLocation(h_ShaderProgram_TXPS, "u_ModelViewProjectionMatrix");
		loc_ModelViewMatrix_TXPS = glGetUniformLocation(h_ShaderProgram_TXPS, "u_ModelViewMatrix");
		loc_ModelViewMatrixInvTrans_TXPS = glGetUniformLocation(h_ShaderProgram_TXPS, "u_ModelViewMatrixInvTrans");

		loc_global_ambient_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_global_ambient_color");
		for (i = 0; i < NUMBER_OF_LIGHT_SUPPORTED; i++) {
			sprintf(string, "u_light[%d].light_on", i);
			loc_light[i].light_on = glGetUniformLocation(h_ShaderProgram_TXPS, string);
			sprintf(string, "u_light[%d].position", i);
			loc_light[i].position = glGetUniformLocation(h_ShaderProgram_TXPS, string);
			sprintf(string, "u_light[%d].ambient_color", i);
			loc_light[i].ambient_color = glGetUniformLocation(h_ShaderProgram_TXPS, string);
			sprintf(string, "u_light[%d].diffuse_color", i);
			loc_light[i].diffuse_color = glGetUniformLocation(h_ShaderProgram_TXPS, string);
			sprintf(string, "u_light[%d].specular_color", i);
			loc_light[i].specular_color = glGetUniformLocation(h_ShaderProgram_TXPS, string);
			sprintf(string, "u_light[%d].spot_direction", i);
			loc_light[i].spot_direction = glGetUniformLocation(h_ShaderProgram_TXPS, string);
			sprintf(string, "u_light[%d].spot_exponent", i);
			loc_light[i].spot_exponent = glGetUniformLocation(h_ShaderProgram_TXPS, string);
			sprintf(string, "u_light[%d].spot_cutoff_angle", i);
			loc_light[i].spot_cutoff_angle = glGetUniformLocation(h_ShaderProgram_TXPS, string);
			sprintf(string, "u_light[%d].light_attenuation_factors", i);
			loc_light[i].light_attenuation_factors = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		}

		loc_material.ambient_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.ambient_color");
		loc_material.diffuse_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.diffuse_color");
		loc_material.specular_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.specular_color");
		loc_material.emissive_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.emissive_color");
		loc_material.specular_exponent = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.specular_exponent");

		loc_texture = glGetUniformLocation(h_ShaderProgram_TXPS, "u_base_texture");

		loc_flag_texture_mapping = glGetUniformLocation(h_ShaderProgram_TXPS, "u_flag_texture_mapping");
		loc_flag_fog = glGetUniformLocation(h_ShaderProgram_TXPS, "u_flag_fog");

		loc_screen_effect = glGetUniformLocation(h_ShaderProgram_TXPS, "screen_effect");
		loc_screen_frequency = glGetUniformLocation(h_ShaderProgram_TXPS, "screen_frequency");
		loc_screen_width = glGetUniformLocation(h_ShaderProgram_TXPS, "screen_width");


		loc_blind_effect = glGetUniformLocation(h_ShaderProgram_TXPS, "u_blind_effect");
	}
	else {
		h_ShaderProgram_TXGS = LoadShaders(shader_info_TXGS);
		loc_ModelViewProjectionMatrix_TXGS = glGetUniformLocation(h_ShaderProgram_TXGS, "u_ModelViewProjectionMatrix");
		loc_ModelViewMatrix_TXGS = glGetUniformLocation(h_ShaderProgram_TXGS, "u_ModelViewMatrix");
		loc_ModelViewMatrixInvTrans_TXGS = glGetUniformLocation(h_ShaderProgram_TXGS, "u_ModelViewMatrixInvTrans");
		loc_global_ambient_color = glGetUniformLocation(h_ShaderProgram_TXGS, "u_global_ambient_color");
	
		for (i = 0; i < NUMBER_OF_LIGHT_SUPPORTED; i++) {
			sprintf(string, "u_light[%d].light_on", i);
			loc_light[i].light_on = glGetUniformLocation(h_ShaderProgram_TXGS, string);
			sprintf(string, "u_light[%d].position", i);
			loc_light[i].position = glGetUniformLocation(h_ShaderProgram_TXGS, string);
			sprintf(string, "u_light[%d].ambient_color", i);
			loc_light[i].ambient_color = glGetUniformLocation(h_ShaderProgram_TXGS, string);
			sprintf(string, "u_light[%d].diffuse_color", i);
			loc_light[i].diffuse_color = glGetUniformLocation(h_ShaderProgram_TXGS, string);
			sprintf(string, "u_light[%d].specular_color", i);
			loc_light[i].specular_color = glGetUniformLocation(h_ShaderProgram_TXGS, string);
			sprintf(string, "u_light[%d].spot_direction", i);
			loc_light[i].spot_direction = glGetUniformLocation(h_ShaderProgram_TXGS, string);
			sprintf(string, "u_light[%d].spot_exponent", i);
			loc_light[i].spot_exponent = glGetUniformLocation(h_ShaderProgram_TXGS, string);
			sprintf(string, "u_light[%d].spot_cutoff_angle", i);
			loc_light[i].spot_cutoff_angle = glGetUniformLocation(h_ShaderProgram_TXGS, string);
			sprintf(string, "u_light[%d].light_attenuation_factors", i);
			loc_light[i].light_attenuation_factors = glGetUniformLocation(h_ShaderProgram_TXGS, string);
		}

		loc_material.ambient_color = glGetUniformLocation(h_ShaderProgram_TXGS, "u_material.ambient_color");
		loc_material.diffuse_color = glGetUniformLocation(h_ShaderProgram_TXGS, "u_material.diffuse_color");
		loc_material.specular_color = glGetUniformLocation(h_ShaderProgram_TXGS, "u_material.specular_color");
		loc_material.emissive_color = glGetUniformLocation(h_ShaderProgram_TXGS, "u_material.emissive_color");
		loc_material.specular_exponent = glGetUniformLocation(h_ShaderProgram_TXGS, "u_material.specular_exponent");

		loc_texture = glGetUniformLocation(h_ShaderProgram_TXGS, "u_base_texture");

		loc_flag_texture_mapping = glGetUniformLocation(h_ShaderProgram_TXGS, "u_flag_texture_mapping");
		
		loc_screen_effect = glGetUniformLocation(h_ShaderProgram_TXGS, "screen_effect");
		loc_screen_frequency = glGetUniformLocation(h_ShaderProgram_TXGS, "screen_frequency");
		loc_screen_width = glGetUniformLocation(h_ShaderProgram_TXGS, "screen_width");


		loc_blind_effect = glGetUniformLocation(h_ShaderProgram_TXGS, "u_blind_effect");
	}

}

void initialize_lights_and_material(void) { // follow OpenGL conventions for initialization
	int i;
	if(flag_shading==0) glUseProgram(h_ShaderProgram_TXPS);
	else glUseProgram(h_ShaderProgram_TXGS);

	glUniform4f(loc_global_ambient_color, 0.115f, 0.115f, 0.115f, 1.0f);
	for (i = 0; i < NUMBER_OF_LIGHT_SUPPORTED; i++) {
		glUniform1i(loc_light[i].light_on, 0); // turn off all lights initially
		glUniform4f(loc_light[i].position, 0.0f, 0.0f, 1.0f, 0.0f);
		glUniform4f(loc_light[i].ambient_color, 0.0f, 0.0f, 0.0f, 1.0f);
		if (i == 0) {
			glUniform4f(loc_light[i].diffuse_color, 1.0f, 1.0f, 1.0f, 1.0f);
			glUniform4f(loc_light[i].specular_color, 1.0f, 1.0f, 1.0f, 1.0f);
		}
		else if (i == 1) {
			glUniform4f(loc_light[i].diffuse_color, 0.0f, 1.0f, 1.0f, 1.0f);
			glUniform4f(loc_light[i].specular_color, 0.0f, 1.0f, 1.0f, 1.0f);
		}
		else {
			glUniform4f(loc_light[i].diffuse_color, 1.0f, 0.0f, 0.0f, 1.0f);
			glUniform4f(loc_light[i].specular_color, 0.0f, 0.0f, 0.0f, 1.0f);
		}
		glUniform3f(loc_light[i].spot_direction, 0.0f, 0.0f, -1.0f);
		glUniform1f(loc_light[i].spot_exponent, 0.0f); // [0.0, 128.0]
		glUniform1f(loc_light[i].spot_cutoff_angle, 180.0f); // [0.0, 90.0] or 180.0 (180.0 for no spot light effect)
		glUniform4f(loc_light[i].light_attenuation_factors, 1.0f, 0.0f, 0.0f, 0.0f); // .w != 0.0f for no ligth attenuation
	}

	glUniform4f(loc_material.ambient_color, 0.2f, 0.2f, 0.2f, 1.0f);
	glUniform4f(loc_material.diffuse_color, 0.8f, 0.8f, 0.8f, 1.0f);
	glUniform4f(loc_material.specular_color, 0.0f, 0.0f, 0.0f, 1.0f);
	glUniform4f(loc_material.emissive_color, 0.0f, 0.0f, 0.0f, 1.0f);
	glUniform1f(loc_material.specular_exponent, 0.0f); // [0.0, 128.0]


	glUniform1i(loc_screen_effect, 0);
	glUniform1f(loc_screen_frequency, 25.0f);
	glUniform1f(loc_screen_width, 0.125f);

	glUniform1i(loc_blind_effect, 0);

	glUseProgram(0);
}

void initialize_flags(void) {
	flag_tiger_animation = 1;
	flag_polygon_fill = 1;
	flag_texture_mapping = 1;
	flag_shading = 0;
	flag_fog = 0;
	flag_translation_axis = Z_AXIS;
	flag_trans_ro_po = 0;

	if(flag_shading==0) glUseProgram(h_ShaderProgram_TXPS);
	else glUseProgram(h_ShaderProgram_TXGS);
	
	glUniform1i(loc_flag_fog, flag_fog);
	glUniform1i(loc_flag_texture_mapping, flag_texture_mapping);
	glUseProgram(0);
}

void initialize_OpenGL(void) {

	glEnable(GL_MULTISAMPLE);

  	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

	ViewMatrix = glm::mat4(1.0f);
	ProjectionMatrix = glm::mat4(1.0f);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;


	//ViewMatrix = glm::lookAt(PRP_distance_scale[0] * glm::vec3(500.0f, 300.0f, 500.0f),
	//	glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	ViewMatrix = glm::lookAt(4.0f/6.0f * glm::vec3(500.0f, 600.0f, 500.0f),
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	initialize_lights_and_material();
	initialize_flags();

	glGenTextures(N_TEXTURES_USED, texture_names);
}

void set_up_scene_lights(void) {


	// spot_light_WC: use light 0
	light[0].light_on = 1;
	light[0].position[0] = 0.0f; light[0].position[1] = 2000.0f; // spot light position in WC
	light[0].position[2] = 0.0f; light[0].position[3] = 1.0f;
		  
	light[0].ambient_color[0] = 0.152f; light[0].ambient_color[1] = 0.152f;
	light[0].ambient_color[2] = 0.152f; light[0].ambient_color[3] = 1.0f;
		  
	light[0].diffuse_color[0] = 0.3f; light[0].diffuse_color[1] = 0.3f;
	light[0].diffuse_color[2] = 0.3f; light[0].diffuse_color[3] = 1.0f;
		  
	light[0].specular_color[0] = 0.3f; light[0].specular_color[1] = 0.3f;
	light[0].specular_color[2] = 0.3f; light[0].specular_color[3] = 1.0f;
		 

	// spot_light_WC: use light 1
	light[1].light_on = 1;
	light[1].position[0] = -250.0f; light[1].position[1] = 630.0f; // spot light position in WC
	light[1].position[2] = 0.0f; light[1].position[3] = 1.0f;

	light[1].ambient_color[0] = 0.152f; light[1].ambient_color[1] = 0.152f;
	light[1].ambient_color[2] = 0.152f; light[1].ambient_color[3] = 1.0f;

	light[1].diffuse_color[0] = 0.0f; light[1].diffuse_color[1] = 0.3f;
	light[1].diffuse_color[2] = 0.0f; light[1].diffuse_color[3] = 1.0f;

	light[1].specular_color[0] = 0.0f; light[1].specular_color[1] = 0.3f;
	light[1].specular_color[2] = 0.0f; light[1].specular_color[3] = 1.0f;



	// spot_light_EC: use light 2
	light[2].light_on = 0;
	light[2].position[0] = 0.0f; light[2].position[1] = 700.0f; 	// point light position in EC
	light[2].position[2] = 0.0f; light[2].position[3] = 1.0f;

	light[2].ambient_color[0] = 0.13f; light[2].ambient_color[1] = 0.13f;
	light[2].ambient_color[2] = 0.13f; light[2].ambient_color[3] = 1.0f;

	light[2].diffuse_color[0] = 0.0f; light[2].diffuse_color[1] = 0.5f;
	light[2].diffuse_color[2] = 0.5f; light[2].diffuse_color[3] = 1.5f;

	light[2].specular_color[0] = 0.0f; light[2].specular_color[1] = 0.8f;
	light[2].specular_color[2] = 0.8f; light[2].specular_color[3] = 1.0f;

	light[2].spot_direction[0] = 0.0f; light[2].spot_direction[1] = -1.0f; // spot light direction in WC
	light[2].spot_direction[2] = 0.0f;
	light[2].spot_cutoff_angle = 20.0f;
	light[2].spot_exponent = 8.0f;

	// spot_light_MC: use light 3
	light[3].light_on = 0;
	light[3].position[0] = 20.0f; light[3].position[1] = 70.0f; 	// point light position in EC
	light[3].position[2] = -20.0f; light[3].position[3] = 1.0f;

	light[3].ambient_color[0] = 0.13f; light[3].ambient_color[1] = 0.13f;
	light[3].ambient_color[2] = 0.13f; light[3].ambient_color[3] = 1.0f;

	light[3].diffuse_color[0] = 1.0f; light[3].diffuse_color[1] = 0.0f;
	light[3].diffuse_color[2] = 1.0f; light[3].diffuse_color[3] = 1.5f;

	light[3].specular_color[0] = 1.0f; light[3].specular_color[1] = 0.0f;
	light[3].specular_color[2] = 1.0f; light[3].specular_color[3] = 1.0f;

	light[3].spot_direction[0] = 0.0f; light[3].spot_direction[1] = -1.0f; // spot light direction in WC
	light[3].spot_direction[2] = 0.0f;
	light[3].spot_cutoff_angle = 40.0f;
	light[3].spot_exponent = 8.0f;

	if(flag_shading == 0) glUseProgram(h_ShaderProgram_TXPS);
	else glUseProgram(h_ShaderProgram_TXGS);
	glUniform1i(loc_light[2].light_on, light[2].light_on);

	//glUniform4fv(loc_light[2].position, 1, light[2].position);
	glm::vec4 position_EC = ViewMatrix * glm::vec4(light[2].position[0], light[2].position[1],
		light[2].position[2], light[2].position[3]);

	glUniform4fv(loc_light[2].position, 1, &position_EC[0]);
	glUniform4fv(loc_light[2].ambient_color, 1, light[2].ambient_color);
	glUniform4fv(loc_light[2].diffuse_color, 1, light[2].diffuse_color);
	glUniform4fv(loc_light[2].specular_color, 1, light[2].specular_color);

	glm::vec3 direction_EC = glm::mat3(ViewMatrix) * glm::vec3(light[2].spot_direction[0], light[2].spot_direction[1],
		light[2].spot_direction[2]);
	glUniform3fv(loc_light[2].spot_direction, 1, &direction_EC[0]);
	glUniform1f(loc_light[2].spot_cutoff_angle, light[2].spot_cutoff_angle);
	glUniform1f(loc_light[2].spot_exponent, light[2].spot_exponent);


	glUniform1i(loc_light[3].light_on, light[3].light_on);
	position_EC = ViewMatrix * glm::vec4(light[3].position[0], light[3].position[1],
		light[3].position[2], light[3].position[3]);
	glUniform4fv(loc_light[3].position, 1, &position_EC[0]);
	glUniform4fv(loc_light[3].ambient_color, 1, light[3].ambient_color);
	glUniform4fv(loc_light[3].diffuse_color, 1, light[3].diffuse_color);
	glUniform4fv(loc_light[3].specular_color, 1, light[3].specular_color);
	direction_EC = glm::mat3(ViewMatrix) * glm::vec3(light[3].spot_direction[0], light[3].spot_direction[1],
		light[3].spot_direction[2]);
	glUniform3fv(loc_light[3].spot_direction, 1, &direction_EC[0]);
	glUniform1f(loc_light[3].spot_cutoff_angle, light[3].spot_cutoff_angle);
	glUniform1f(loc_light[3].spot_exponent, light[3].spot_exponent);


	glUniform1i(loc_light[0].light_on, light[0].light_on);
	glUniform4fv(loc_light[0].position, 1, light[0].position);
	glUniform4fv(loc_light[0].ambient_color, 1, light[0].ambient_color);
	glUniform4fv(loc_light[0].diffuse_color, 1, light[0].diffuse_color);
	glUniform4fv(loc_light[0].specular_color, 1, light[0].specular_color);

	glUniform1i(loc_light[1].light_on, light[1].light_on);
	glUniform4fv(loc_light[1].position, 1, light[1].position);
	glUniform4fv(loc_light[1].ambient_color, 1, light[1].ambient_color);
	glUniform4fv(loc_light[1].diffuse_color, 1, light[1].diffuse_color);
	glUniform4fv(loc_light[1].specular_color, 1, light[1].specular_color);


	glUseProgram(0);
}

void prepare_scene(void) {
	prepare_axes();
	prepare_floor();

	prepare_tiger();
	prepare_ben();
	prepare_spider();

	prepare_cow();
	prepare_bus();
	prepare_bike();
	initialize_screen();

	set_up_scene_lights();
}

void initialize_renderer(void) {
	register_callbacks();
	prepare_shader_program();
	initialize_OpenGL();
	prepare_scene();
	initialize_camera();


}

void initialize_glew(void) {
	GLenum error;

	glewExperimental = GL_TRUE;

	error = glewInit();
	if (error != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(error));
		exit(-1);
	}
	fprintf(stdout, "*********************************************************\n");
	fprintf(stdout, " - GLEW version supported: %s\n", glewGetString(GLEW_VERSION));
	fprintf(stdout, " - OpenGL renderer: %s\n", glGetString(GL_RENDERER));
	fprintf(stdout, " - OpenGL version supported: %s\n", glGetString(GL_VERSION));
	fprintf(stdout, "*********************************************************\n\n");
}

void greetings(char *program_name, char messages[][256], int n_message_lines) {
	fprintf(stdout, "**************************************************************\n\n");
	fprintf(stdout, "  PROGRAM NAME: %s\n\n", program_name);
	fprintf(stdout, "    This program was coded for CSE4170 students\n");
	fprintf(stdout, "      of Dept. of Comp. Sci. & Eng., Sogang University.\n\n");

	for (int i = 0; i < n_message_lines; i++)
		fprintf(stdout, "%s\n", messages[i]);
	fprintf(stdout, "\n**************************************************************\n\n");

	initialize_glew();
}

#define N_MESSAGE_LINES 1
void main(int argc, char *argv[]) {
	char program_name[64] = "Sogang CSE4170 3D Objects";
	char messages[N_MESSAGE_LINES][256] = { "    - Keys used: '0', '1', 'a', 't', 'f', 'c', 'd', 'y', 'u', 'i', 'o', 'ESC'"  };

	glutInit(&argc, argv);
  	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glutInitWindowSize(800, 800);
	glutInitContextVersion(3, 2);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_renderer();

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutMainLoop();
}