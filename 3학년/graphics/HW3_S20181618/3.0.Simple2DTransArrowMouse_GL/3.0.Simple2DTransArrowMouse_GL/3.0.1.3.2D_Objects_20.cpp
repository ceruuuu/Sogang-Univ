#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "Shaders/LoadShaders.h"
GLuint h_ShaderProgram; // handle to shader program
GLint loc_ModelViewProjectionMatrix, loc_primitive_color; // indices of uniform variables

														  // include glm/*.hpp only if necessary
														  //#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> //translate, rotate, scale, ortho, etc.
glm::mat4 ModelViewProjectionMatrix;
glm::mat4 ViewMatrix, ProjectionMatrix, ViewProjectionMatrix;

#define TO_RADIAN 0.01745329252f  
#define TO_DEGREE 57.295779513f
#define BUFFER_OFFSET(offset) ((GLvoid *) (offset))

#define LOC_VERTEX 0

int win_width = 0, win_height = 0;
float centerx = 0.0f, centery = 0.0f, rotate_angle = 0.0f;

#include "objects.h"

//-------------집과 차 좌표 저장하는 구조체
int cur_houseCnt = 0;
typedef struct HOUSE_CAR_LOCATION {
	int houseFlag;
	glm::vec2 h_loc;
	int carFlag;
	glm::vec2 car_loc; // 귀속되는 차의 좌표
}house_car_loc;
house_car_loc hc_loc[5] = { 0, glm::vec2(0.0f,0.0f),0,glm::vec2(0.0f,0.0f) };

int clock1 = 0;
int clock2 = 0;
int clock3 = 0;
int clock4 = 0;
int clock5 = 0;
void timer(int value) {
	clock1 = (clock1 + 5) % 800;
	clock2 = (clock2 + 2) % 502;
	clock3 = (clock3 + 4) % 1004;
	clock4 = (clock4 + 1) % 1200;
	clock5 = (clock5 + 1) % 360;
	glutPostRedisplay();
	glutTimerFunc(10, timer, 0);
}

float tempX = 400.0f;
float tempY = 200.0f;

int randFlag = -1;
int bgFlag = 0;

void display(void) {
	glm::mat4 ModelMatrix;

	glClear(GL_COLOR_BUFFER_BIT);
	
	if (bgFlag == 1) { // 밤
		ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-300.0f, 200.0f, 0.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.0f, 2.0f, 1.0f));
		ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_moon();

	}
	else { // 낮
		ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-300.0f, 200.0f, 0.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.0f, 2.0f, 1.0f));

		ModelMatrix = glm::rotate(ModelMatrix, sin((clock5 * 0.5f) * TO_RADIAN)+100.0f, glm::vec3(0.0f, 0.0f, 1.0f));

		ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_sun();
	
		for (int i = 0; i < 10; i++)
		{
			float c = sin(clock5 * TO_RADIAN);

			ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-300.0f, 200.0f, 0.0f));
	
			ModelMatrix = glm::rotate(ModelMatrix, (i * 40 * TO_RADIAN) + (float)clock5 / 100, glm::vec3(0.0f, 0.0f, 1.0f));
			ModelMatrix = glm::translate(ModelMatrix, glm::vec3( (180 - (float)clock5)*0.3f , 70.0f, 0.0f));
			ModelMatrix = glm::scale(ModelMatrix, glm::vec3(abs(c) / 5 + 0.2f, abs(c) / 5 + 0.2f, abs(c) / 5 + 0.2f));

			ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
			glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
			draw_rnd();
		}

	
	}

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(250.0f, -450.0f, -50.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(140.0f,140.0f, 110.0f));
	ModelMatrix = glm::rotate(ModelMatrix, 5.0f*TO_RADIAN,  glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_road();


	for (int i = 1; i < 11 ; i++) {
		ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-400 + (clock4 * 1.3f) + i, -200 + (80 * sin(clock4 * 0.05f) + clock4 * 0.3) - i*2, 0.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(3.0f, 3.0f, 1.0f));
		ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_car2();
	}


	for (int i = 0; i < 5; i++) {

		if (hc_loc[i].houseFlag == 0) break;
		ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(hc_loc[i].h_loc.x, hc_loc[i].h_loc.y, 0.0f));
		if (bgFlag == 0 || hc_loc[i].carFlag == 0) {
			ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.0f, 2.0f, 1.0f));
		}
		else if(bgFlag == 1 && hc_loc[i].carFlag == 1) {
			ModelMatrix = glm::scale(ModelMatrix, glm::vec3(abs(180.0f-(float)clock5)/100 + 1.2f, abs(180.0f - (float)clock5) / 100 + 1.2f, 1.0f));
		}
	
		ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_house();

		if (hc_loc[i].carFlag == 1) {
			ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(hc_loc[i].car_loc.x, hc_loc[i].car_loc.y, 0.0f));
			ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.0f, 2.0f, 1.0f));
			ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
			glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
			draw_car();
		}
	

		if (clock3 == 0) {
			randFlag = rand() % 5;
		}
	
		if(randFlag == i && clock3 < 800) {

			ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(hc_loc[i].h_loc.x, hc_loc[i].h_loc.y + 30, 0.0f));
			ModelMatrix = glm::scale(ModelMatrix, glm::vec3(abs(clock3/2) * 0.005f,abs(clock3/2) * 0.005f, abs(clock3/2) * 0.005));
			ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.0f, 2.0f, 1.0f));
			ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
			glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
			draw_cake();

		}
		else if(clock3 >= 800){
			randFlag = -1;
		}

	
	}

	float airX = 400.0f - clock1;
	float airY = 200.0f;


	//초기단계. 자동차 안떨어질때
	if (tempY == 200.0f) {
		tempX = airX;
	}
	else if (tempY <= -300.0f) { //끝까지 떨어졌을때 다시 비행기 보급
		tempX = airX;
		tempY = 200.0f;
	}
	tempY = 200.0f - clock2;

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(airX, airY, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.0f, 2.0f, 1.0f));
	ModelMatrix = glm::rotate(ModelMatrix, sin((clock1*0.5f) * TO_RADIAN)+300.0f, glm::vec3(0.0f, 0.0f, 1.0f)); //회전하면서 이동하게끔 설정
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_airplane();

	
	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(tempX, tempY, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.0f, 2.0f, 1.0f));
	ModelMatrix = glm::rotate(ModelMatrix, sin(clock2 * TO_RADIAN), glm::vec3(0.0f, 0.0f, 1.0f)); //회전하면서 이동하게끔 설정
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_car();

	for (int i = 0; i < 5; i++)
	{
		if (abs(hc_loc[i].h_loc.x - tempX) < 40 && abs(hc_loc[i].h_loc.y - tempY) < 10) { //car와 house가 접촉했을 때
			if (hc_loc[i].houseFlag == 1 && hc_loc[i].carFlag == 0) { // house가 있고 해당 house에 car는 없으면
				hc_loc[i].car_loc.x = hc_loc[i].h_loc.x + 30.0f; // car를 귀속시킨다.
				hc_loc[i].car_loc.y = hc_loc[i].h_loc.y - 5.0f;
				hc_loc[i].carFlag = 1;

				tempY = -300.0f;
				clock2 = 0;
			}
		}
	}
	glFlush();
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 27: // ESC key
		glutLeaveMainLoop(); // Incur destuction callback for cleanups.
		break;
	case 'x':
		if (bgFlag == 0) { //밤으로 바뀐다
			glClearColor(0 / 255.0f, 0 / 255.0f, 0 / 255.0f, 1.0f);
			glutPostRedisplay();
			bgFlag = 1;
		}
		else { //낮으로 바뀐다
			glClearColor(102 / 255.0f, 204 / 255.0f, 255 / 255.0f, 1.0f);
			glutPostRedisplay();
			bgFlag = 0;
		}
	}

}

void mousepress(int button, int state, int x, int y) {

	if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN)) {
		float relative_x = float(x) - win_width / 2;
		float relative_y = win_height / 2 - float(y);

		hc_loc[cur_houseCnt].h_loc.x = relative_x;
		hc_loc[cur_houseCnt].h_loc.y = relative_y;
		hc_loc[cur_houseCnt].houseFlag = 1;
		hc_loc[cur_houseCnt].carFlag = 0;

		(++cur_houseCnt) %= 5;
	}

}


void reshape(int width, int height) {
	win_width = width, win_height = height;

	glViewport(0, 0, win_width, win_height);
	ProjectionMatrix = glm::ortho(-win_width / 2.0, win_width / 2.0,
		-win_height / 2.0, win_height / 2.0, -1000.0, 1000.0);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

	update_axes();
	update_line();

	glutPostRedisplay();
}

void cleanup(void) {
	glDeleteVertexArrays(1, &VAO_axes);
	glDeleteBuffers(1, &VBO_axes);

	glDeleteVertexArrays(1, &VAO_line);
	glDeleteBuffers(1, &VBO_line);

	glDeleteVertexArrays(1, &VAO_airplane);
	glDeleteBuffers(1, &VBO_airplane);

	glDeleteVertexArrays(1, &VAO_rnd);
	glDeleteBuffers(1, &VBO_rnd);

	glDeleteVertexArrays(1, &VAO_moon);
	glDeleteBuffers(1, &VBO_moon);

	glDeleteVertexArrays(1, &VAO_sun);
	glDeleteBuffers(1, &VBO_sun);

	glDeleteVertexArrays(1, &VAO_road);
	glDeleteBuffers(1, &VBO_road);

	// Delete others here too!!!
}

void register_callbacks(void) {
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(reshape);
	glutCloseFunc(cleanup);

	glutMouseFunc(mousepress);
	glutTimerFunc(10, timer, 0);
}

void prepare_shader_program(void) {
	ShaderInfo shader_info[3] = {
		{ GL_VERTEX_SHADER, "Shaders/simple.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/simple.frag" },
		{ GL_NONE, NULL }
	};

	h_ShaderProgram = LoadShaders(shader_info);
	glUseProgram(h_ShaderProgram);

	loc_ModelViewProjectionMatrix = glGetUniformLocation(h_ShaderProgram, "u_ModelViewProjectionMatrix");
	loc_primitive_color = glGetUniformLocation(h_ShaderProgram, "u_primitive_color");
}

void initialize_OpenGL(void) {
	glEnable(GL_MULTISAMPLE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glClearColor(102 / 255.0f, 204 / 255.0f, 255 / 255.0f, 1.0f);
	ViewMatrix = glm::mat4(1.0f);
}

void prepare_scene(void) {
	prepare_axes();
	prepare_line();
	prepare_airplane();
	prepare_shirt();
	prepare_house();
	prepare_car();
	prepare_cocktail();
	prepare_car2();
	prepare_hat();
	prepare_cake();
	prepare_sword();
	prepare_rnd();
	prepare_moon();
	prepare_sun();
	prepare_road();
}

void initialize_renderer(void) {
	register_callbacks();
	prepare_shader_program();
	initialize_OpenGL();
	prepare_scene();
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
	char program_name[64] = "Sogang CSE4170 2DObjects_GLSL_3.0.1.3";
	char messages[N_MESSAGE_LINES][256] = {
		"    - Keys used: 'ESC' "
	};

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_MULTISAMPLE);
	glutInitWindowSize(800, 600);
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_renderer();

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutMainLoop();
}


