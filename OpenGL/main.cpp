/*Chương trình chiếu sáng Blinn-Phong (Phong sua doi) cho hình lập phương đơn vị, điều khiển quay bằng phím x, y, z, X, Y, Z.*/

#include "Angel.h"  /* Angel.h là file tự phát triển (tác giả Prof. Angel), có chứa cả khai báo includes glew và freeglut*/


// remember to prototype
void generateGeometry(void);
void initGPUBuffers(void);
void shaderSetup(void);
void display(void);
void keyboard(unsigned char key, int x, int y);

typedef vec4 point4;
typedef vec4 color4;
using namespace std;

// Số các đỉnh của các tam giác
const int NumPoints = 36;

point4 points[NumPoints]; /* Danh sách các đỉnh của các tam giác cần vẽ*/
color4 colors[NumPoints]; /* Danh sách các màu tương ứng cho các đỉnh trên*/
vec3 normals[NumPoints]; /*Danh sách các vector pháp tuyến ứng với mỗi đỉnh*/

point4 vertices[8]; /* Danh sách 8 đỉnh của hình lập phương*/
color4 vertex_colors[8]; /*Danh sách các màu tương ứng cho 8 đỉnh hình lập phương*/

GLuint program;

GLfloat theta[3] = { 0, 0, 0 };
GLfloat dr = 5;
GLint viewkt= 0;
GLfloat xx=0, yy=1, zz=1;
static GLint carDisplayList;

mat4 model;
GLuint model_loc;
mat4 projection;
GLuint projection_loc;
mat4 view;
GLuint view_loc;


void initCube()
{
	// Gán giá trị tọa độ vị trí cho các đỉnh của hình lập phương
	vertices[0] = point4(-0.5, -0.5, 0.5, 1.0);
	vertices[1] = point4(-0.5, 0.5, 0.5, 1.0);
	vertices[2] = point4(0.5, 0.5, 0.5, 1.0);
	vertices[3] = point4(0.5, -0.5, 0.5, 1.0);
	vertices[4] = point4(-0.5, -0.5, -0.5, 1.0);
	vertices[5] = point4(-0.5, 0.5, -0.5, 1.0);
	vertices[6] = point4(0.5, 0.5, -0.5, 1.0);
	vertices[7] = point4(0.5, -0.5, -0.5, 1.0);

	// Gán giá trị màu sắc cho các đỉnh của hình lập phương	
	vertex_colors[0] = color4(0.0, 0.0, 0.0, 1.0); // black
	vertex_colors[1] = color4(1.0, 0.0, 0.0, 1.0); // red
	vertex_colors[2] = color4(1.0, 1.0, 0.0, 1.0); // yellow
	vertex_colors[3] = color4(0.0, 1.0, 0.0, 1.0); // green
	vertex_colors[4] = color4(0.0, 0.0, 1.0, 1.0); // blue
	vertex_colors[5] = color4(1.0, 0.0, 1.0, 1.0); // magenta
	vertex_colors[6] = color4(1.0, 0.5, 0.0, 1.0); // orange
	vertex_colors[7] = color4(0.0, 1.0, 1.0, 1.0); // cyan
}
int Index = 0;
void quad(int a, int b, int c, int d)  /*Tạo một mặt hình lập phương = 2 tam giác, gán màu cho mỗi đỉnh tương ứng trong mảng colors*/
{
	vec4 u = vertices[b] - vertices[a];
	vec4 v = vertices[c] - vertices[b];
	vec3 normal = normalize(cross(u, v));

	normals[Index] = normal; colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
	normals[Index] = normal; colors[Index] = vertex_colors[a]; points[Index] = vertices[b]; Index++;
	normals[Index] = normal; colors[Index] = vertex_colors[a]; points[Index] = vertices[c]; Index++;
	normals[Index] = normal; colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
	normals[Index] = normal; colors[Index] = vertex_colors[a]; points[Index] = vertices[c]; Index++;
	normals[Index] = normal; colors[Index] = vertex_colors[a]; points[Index] = vertices[d]; Index++;
}
void makeColorCube(void)  /* Sinh ra 12 tam giác: 36 đỉnh, 36 màu*/

{
	quad(1, 0, 3, 2);
	quad(2, 3, 7, 6);
	quad(3, 0, 4, 7);
	quad(6, 5, 1, 2);
	quad(4, 5, 6, 7);
	quad(5, 4, 0, 1);
}
void generateGeometry(void)
{
	initCube();
	makeColorCube();
}


void initGPUBuffers(void)
{
	// Tạo một VAO - vertex array object
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Tạo và khởi tạo một buffer object
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors) + sizeof(normals), NULL, GL_STATIC_DRAW);

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors), sizeof(normals), normals);


}

void shaderSetup(void)
{
	// Nạp các shader và sử dụng chương trình shader
	program = InitShader("vshader1.glsl", "fshader1.glsl");   // hàm InitShader khai báo trong Angel.h
	glUseProgram(program);

	// Khởi tạo thuộc tính vị trí đỉnh từ vertex shader
	GLuint loc_vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(loc_vPosition);
	glVertexAttribPointer(loc_vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	GLuint loc_vColor = glGetAttribLocation(program, "vColor");
	glEnableVertexAttribArray(loc_vColor);
	glVertexAttribPointer(loc_vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points)));

	GLuint loc_vNormal = glGetAttribLocation(program, "vNormal");
	glEnableVertexAttribArray(loc_vNormal);
	glVertexAttribPointer(loc_vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points) + sizeof(colors)));

	/* Khởi tạo các tham số chiếu sáng - tô bóng*/
	point4 light_position(0.0, 0.0, 1.0, 0.0);
	color4 light_ambient(0.2, 0.2, 0.2, 1.0);
	color4 light_diffuse(1.0, 1.0, 1.0, 1.0);
	color4 light_specular(1.0, 1.0, 1.0, 1.0);

	color4 material_ambient(1.0, 0.0, 1.0, 1.0);
	color4 material_diffuse(1.0, 0.8, 0.0, 1.0);
	color4 material_specular(1.0, 0.8, 0.0, 1.0);
	float material_shininess = 100.0;

	color4 ambient_product = light_ambient * material_ambient;
	color4 diffuse_product = light_diffuse * material_diffuse;
	color4 specular_product = light_specular * material_specular;

	glUniform4fv(glGetUniformLocation(program, "AmbientProduct"), 1, ambient_product);
	glUniform4fv(glGetUniformLocation(program, "DiffuseProduct"), 1, diffuse_product);
	glUniform4fv(glGetUniformLocation(program, "SpecularProduct"), 1, specular_product);
	glUniform4fv(glGetUniformLocation(program, "LightPosition"), 1, light_position);
	glUniform1f(glGetUniformLocation(program, "Shininess"), material_shininess);

	model_loc = glGetUniformLocation(program, "Model");
	projection_loc = glGetUniformLocation(program, "Projection");
	view_loc = glGetUniformLocation(program, "View");

	glEnable(GL_DEPTH_TEST);
	glClearColor(1.0, 1.0, 1.0, 1.0);        /* Thiết lập màu trắng là màu xóa màn hình*/
}
mat4 instance;
void customCube(float tx, float ty, float tz, float sx, float sy, float sz, float rx, float ry, float rz) {
	instance = Translate(tx, ty, tz) * RotateX(rx) * RotateY(ry) * RotateZ(rz) * Scale(sx, sy, sz);
	glUniformMatrix4fv(model_loc, 1, GL_TRUE, model * instance);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);
}
point4 light_position(0.0, 0.0, 1.0, 0.0);
color4 light_ambient(0.2, 0.2, 0.2, 1.0);
color4 light_diffuse(1.0, 1.0, 1.0, 1.0);
color4 light_specular(1.0, 1.0, 1.0, 1.0);

color4 material_ambient(1.0, 0.0, 1.0, 1.0);
color4 material_diffuse(0.0, 1.0, 0.0, 1.0);
color4 material_specular(1.0, 0.8, 0.0, 1.0);
float material_shininess = 100.0;
color4 ambient_product;
color4 diffuse_product;
color4 specular_product;
void setMau(float ad, float bd, float cd) {
	material_diffuse = vec4(ad, bd, cd, 1.0);
	ambient_product = light_ambient * material_ambient;
	diffuse_product = light_diffuse * material_diffuse;
	specular_product = light_specular * material_specular;
	glUniform4fv(glGetUniformLocation(program, "DiffuseProduct"), 1, diffuse_product);
}
// bắt đầu code từ đây

void matDat() {

	GLfloat x = 4, y = 0.001, z = 4;
	setMau(0.0, 0.0, 0.0);
	customCube(0, 0-y/2, 0, x, y, z, 0, 0, 0);
}
void drawLine() { // 주차선 그리는 함수
	GLfloat x = 1.2, y = 0.01, z = 0.01;
	setMau(1.0, 1.0, 1.0);
	customCube(1.2, y / 2, 0.3, x, y, z, 0, 0, 0);
	customCube(0.8, y / 2, 0.6, x+0.2, y, z, 0, 0, 0);
	customCube(1.2, y / 2, 0.9, x, y, z, 0, 0, 0);

	customCube(-0.4, y / 2, 0.3, x, y, z, 0, 0, 0);
	customCube(-0.9, y / 2, 0.6, x, y, z, 0, 0, 0);
	customCube(-0.4, y / 2, 0.9, x, y, z, 0, 0, 0);

	customCube(0.8, y / 2, 1.2, x + 0.2, y, z, 0, 0, 0);
	customCube(-0.9, y / 2, 1.2, x, y, z, 0, 0, 0);


}

void venDuong() {
	GLfloat x = 4, y = 0.001, z = 0.6;
	setMau(0.0, 1.0, 0.0);
	customCube(0, y/2, 1.7, x, y, z, 0, 0, 0);
}

void hangRao() {
	GLfloat x = 2, y = 0.3, z = 0.05;
	setMau(1.0, 0.0, 0.0);
	customCube(-x/2, y/2, 0, x, y, z, 0, 0, 0);
	customCube(x-((x-1)/2), y / 2, 0, x - 1, y, z, 0, 0, 0);
}
void drawCar()
{

	glTranslatef(0.0f, 0.8f, 0.0f);
	glEnable(GL_BLEND); //반투명 선언
	glBlendFunc(GL_ONE, GL_ZERO);

	// 자동차를 그리기 위한 선 따기. 각 알파벳은 설계도의 좌표점 이름이다.
	glBegin(GL_LINE_LOOP);
	glVertex3f(-1.12f, -.48f, 0.7f);//a
	glVertex3f(-0.86f, -.48f, 0.7f);//b
	glVertex3f(-.74f, -0.2f, 0.7f);//c
	glVertex3f(-.42f, -.2f, 0.7f);//d
	glVertex3f(-0.3f, -.48f, 0.7f);//e
	glVertex3f(.81, -0.48, 0.7);//f
	glVertex3f(.94, -0.2, 0.7);//g
	glVertex3f(1.24, -.2, 0.7);//h
	glVertex3f(1.38, -.48, 0.7);//i
	glVertex3f(1.52, -.44, 0.7);//j
	glVertex3f(1.52, .14, 0.7);//k
	glVertex3f(1.14, 0.22, 0.7);//l
	glVertex3f(0.76, .22, 0.7);//m
	glVertex3f(.52, 0.56, 0.7);//n
	glVertex3f(-0.1, 0.6, 0.7);//0
	glVertex3f(-1.02, 0.6, 0.7);//p
	glVertex3f(-1.2, 0.22, 0.7);//q
	glVertex3f(-1.2, -.28, 0.7);//r
	glEnd();

	glBegin(GL_LINE_LOOP);
	glVertex3f(-1.12f, -.48f, -0.7f);//a'
	glVertex3f(-0.86, -.48, -0.7);//b'
	glVertex3f(-.74, -0.2, -0.7);//c'
	glVertex3f(-.42, -.2, -0.7);//d'
	glVertex3f(-0.3, -.48, -0.7);//e'
	glVertex3f(.81, -0.48, -0.7);//f'
	glVertex3f(.94, -0.2, -0.7);//g'
	glVertex3f(1.24, -.2, -0.7);//h'
	glVertex3f(1.38, -.48, -0.7);//i'
	glVertex3f(1.52, -.44, -0.7);//j'
	glVertex3f(1.52, .14, -0.7);//k'
	glVertex3f(1.14, 0.22, -0.7);//l'
	glVertex3f(0.76, .22, -0.7);//m'
	glVertex3f(.52, 0.56, -0.7);//n'
	glVertex3f(-0.1, 0.6, -0.7);//o'
	glVertex3f(-1.02, 0.6, -0.7);//p'
	glVertex3f(-1.2, 0.22, -0.7);//q'
	glVertex3f(-1.2, -.28, -0.7);//r'
	glEnd();

	glBegin(GL_LINES);
	glVertex3f(-1.12, -.48, 0.7);//a
	glVertex3f(-1.12, -.48, -0.7);//a'
	glVertex3f(-0.86, -.48, 0.7);//b
	glVertex3f(-0.86, -.48, -0.7);//b'
	glVertex3f(-.74, -0.2, 0.7);//c
	glVertex3f(-.74, -0.2, -0.7);//c'
	glVertex3f(-.42, -.2, 0.7);//d
	glVertex3f(-.42, -.2, -0.7);//d'
	glVertex3f(-0.3, -.48, 0.7);//e
	glVertex3f(-0.3, -.48, -0.7);//e'
	glVertex3f(.81, -0.48, 0.7);//f
	glVertex3f(.81, -0.48, -0.7);//f'
	glVertex3f(.94, -0.2, 0.7);//g
	glVertex3f(.94, -0.2, -0.7);//g'
	glVertex3f(1.24, -.2, 0.7);//h
	glVertex3f(1.24, -.2, -0.7);//h'
	glVertex3f(1.38, -.48, 0.7);//i
	glVertex3f(1.38, -.48, -0.7);//i'
	glVertex3f(1.52, -.44, 0.7);//j
	glVertex3f(1.52, -.44, -0.7);//j'
	glVertex3f(1.52, .14, 0.7);//k
	glVertex3f(1.52, .14, -0.7);//k'
	glVertex3f(1.14, 0.22, 0.7);//l
	glVertex3f(1.14, 0.22, -0.7);//l'
	glVertex3f(0.76, .22, 0.7);//m
	glVertex3f(0.76, .22, -0.7);//m'
	glVertex3f(.52, 0.56, 0.7);//n
	glVertex3f(.52, 0.56, -0.7);//n'
	glVertex3f(-0.1, 0.6, 0.7);//0
	glVertex3f(-0.1, 0.6, -0.7);//o'
	glVertex3f(-1.02, 0.6, 0.7);//p
	glVertex3f(-1.02, 0.6, -0.7);//p'
	glVertex3f(-1.2, 0.22, 0.7);//q
	glVertex3f(-1.2, 0.22, -0.7);//q'
	glVertex3f(-1.2, -.28, 0.7);//r
	glVertex3f(-1.2, -.28, -0.7);//r'
	glEnd();


	// 자동차 천장 채우기
	glBegin(GL_POLYGON);
	glVertex3f(-0.1, 0.6, 0.7);//o
	glVertex3f(-0.1, 0.6, -0.7);//o'
	glVertex3f(-1.02, 0.6, -0.7);//p'
	glVertex3f(-1.02, 0.6, 0.7);//p
	glEnd();


	glBegin(GL_POLYGON);
	glVertex3f(-0.1, 0.6, 0.7);//o
	glVertex3f(-0.1, 0.6, -0.7);//o'
	glVertex3f(.52, 0.56, -0.7);//n'
	glVertex3f(.52, 0.56, 0.7);//n
	glEnd();

	//자동차 뒷면 채우기
	glBegin(GL_POLYGON);
	glVertex3f(-1.2, 0.22, 0.7);//q
	glVertex3f(-1.2, 0.22, -0.7);//q'
	glVertex3f(-1.2, -.28, -0.7);//r'
	glVertex3f(-1.2, -.28, 0.7);//r
	glEnd();



	glBegin(GL_POLYGON);
	glVertex3f(1.52, .14, 0.7);//k
	glVertex3f(1.14, 0.22, 0.7);//l
	glVertex3f(1.14, 0.22, -0.7);//l'
	glVertex3f(1.52, .14, -0.7);//k'
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3f(0.76, .22, 0.7);//m
	glVertex3f(0.76, .22, -0.7);//m'
	glVertex3f(1.14, 0.22, -0.7);//l'
	glVertex3f(1.14, 0.22, 0.7);//l
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3f(-1.12, -.48, 0.7);//a
	glVertex3f(-0.86, -.48, 0.7);//b
	glVertex3f(-.74, -0.2, 0.7);//c
	glVertex3f(-0.64, 0.22, 0.7);//cc
	glVertex3f(-1.08, 0.22, 0.7);//dd
	glVertex3f(-1.2, 0.22, 0.7);//q
	glVertex3f(-1.2, -.28, 0.7);//r
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3f(-.74, -0.2, 0.7);//c
	glVertex3f(-0.64, 0.22, 0.7);//cc
	glVertex3f(-0.5, 0.22, 0.7);//hh
	glVertex3f(-0.5, -0.2, 0.7);//pp
	glEnd();

	// 바퀴 이음새부분 채우기
	glBegin(GL_POLYGON);
	glVertex3f(0.0, 0.22, 0.7);//gg
	glVertex3f(1.14, 0.22, 0.7);//l
	glVertex3f(1.24, -.2, 0.7);//h
	glVertex3f(0.0, -0.2, 0.7);//oo
	glEnd();

	//왼쪽 뒷바퀴 부분 비운곳 채우기
	glBegin(GL_POLYGON);
	glVertex3f(-1.12, -.48, -0.7);//a'
	glVertex3f(-0.86, -.48, -0.7);//b'
	glVertex3f(-.74, -0.2, -0.7);//c'
	glVertex3f(-0.64, 0.22, -0.7);//cc'
	glVertex3f(-1.08, 0.22, -0.7);//dd'
	glVertex3f(-1.2, 0.22, -0.7);//q'
	glVertex3f(-1.2, -.28, -0.7);//r'
	glEnd();

	// 나머지부분 모두 채우기
	glBegin(GL_POLYGON);
	glVertex3f(-.74, -0.2, -0.7);//c'
	glVertex3f(-0.64, 0.22, -0.7);//cc'
	glVertex3f(-0.5, 0.22, -0.7);//hh'
	glVertex3f(-0.5, -0.2, -0.7);//pp'
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3f(0.0, 0.22, -0.7);//gg'
	glVertex3f(1.14, 0.22, -0.7);//l'
	glVertex3f(1.24, -.2, -0.7);//h'
	glVertex3f(0.0, -0.2, -0.7);//oo'
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3f(-1.2, 0.22, 0.7);//q
	glVertex3f(-1.08, 0.22, 0.7);//dd
	glVertex3f(-0.98, 0.5, 0.7);//aa
	glVertex3f(-1.02, 0.6, 0.7);//p
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3f(-1.02, 0.6, 0.7);//p
	glVertex3f(-0.98, 0.5, 0.7);//aa
	glVertex3f(0.44, 0.5, 0.7);//jj
	glVertex3f(.52, 0.56, 0.7);//n
	glVertex3f(-0.1, 0.6, 0.7);//0
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3f(-0.64, 0.5, 0.7);//bb
	glVertex3f(-0.64, 0.22, 0.7);//cc
	glVertex3f(-0.5, 0.22, 0.7);//hh
	glVertex3f(-0.5, 0.5, 0.7);//ee
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3f(0.0, 0.5, 0.7);//ff
	glVertex3f(0.0, 0.22, 0.7);//gg
	glVertex3f(0.12, 0.22, 0.7);//ll
	glVertex3f(0.12, 0.5, 0.7);//ii
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3f(.52, 0.56, 0.7);//n
	glVertex3f(0.44, 0.5, 0.7);//jj
	glVertex3f(0.62, 0.22, 0.7);//kk
	glVertex3f(0.76, .22, 0.7);//m
	glEnd();

	// 자동차 밑부분
	glBegin(GL_POLYGON);
	glVertex3f(-.42, -.2, 0.7);//d
	glVertex3f(.94, -0.2, 0.7);//g
	glVertex3f(.81, -0.48, 0.7);//f
	glVertex3f(-0.3, -.48, 0.7);//e
	glEnd();

	// 자동차 밑부분
	glBegin(GL_POLYGON);
	glVertex3f(1.14, 0.22, 0.7);//l
	glVertex3f(1.52, .14, 0.7);//k
	glVertex3f(1.52, -.44, 0.7);//j
	glVertex3f(1.38, -.48, 0.7);//i
	glVertex3f(1.24, -.2, 0.7);//h
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3f(-1.2, 0.22, -0.7);//q'
	glVertex3f(-1.08, 0.22, -0.7);//dd'
	glVertex3f(-0.98, 0.5, -0.7);//aa'
	glVertex3f(-1.02, 0.6, -0.7);//p'
	glEnd();

	// 천장과 유리가 맞닿은곳 채우기
	glBegin(GL_POLYGON);
	glVertex3f(-1.02, 0.6, -0.7);//p'
	glVertex3f(-0.98, 0.5, -0.7);//aa'
	glVertex3f(0.44, 0.5, -0.7);//jj'
	glVertex3f(.52, 0.56, -0.7);//n'
	glVertex3f(-0.1, 0.6, -0.7);//o'
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3f(-0.64, 0.5, -0.7);//bb'
	glVertex3f(-0.64, 0.22, -0.7);//cc'
	glVertex3f(-0.5, 0.22, -0.7);//hh'
	glVertex3f(-0.5, 0.5, -0.7);//ee'
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3f(0.0, 0.5, -0.7);//ff'
	glVertex3f(0.0, 0.22, -0.7);//gg'
	glVertex3f(0.12, 0.22, -0.7);//ll'
	glVertex3f(0.12, 0.5, -0.7);//ii'
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3f(.52, 0.56, -0.7);//n'
	glVertex3f(0.44, 0.5, -0.7);//jj'
	glVertex3f(0.62, 0.22, -0.7);//kk'
	glVertex3f(0.76, .22, -0.7);//m'
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3f(-.42, -.2, -0.7);//d'
	glVertex3f(.94, -0.2, -0.7);//g'
	glVertex3f(.81, -0.48, -0.7);//f'
	glVertex3f(-0.3, -.48, -0.7);//e'
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3f(1.14, 0.22, -0.7);//l'
	glVertex3f(1.52, .14, -0.7f);//k'
	glVertex3f(1.52, -.44, -0.7);//j'
	glVertex3f(1.38, -.48, -0.7);//i'
	glVertex3f(1.24, -.2, -0.7);//h'
	glEnd();


	// 오른쪽 뒷자석 차문
	glBegin(GL_POLYGON);
	glVertex3f(-0.5, 0.22, 0.7);//hh
	glVertex3f(0.0, 0.22, 0.7);//gg
	glVertex3f(0.0, -0.2, 0.7);//oo
	glVertex3f(-0.5, -0.2, 0.7);//pp
	glEnd();

	// 왼쪽 뒷자석 차문
	glBegin(GL_POLYGON);
	glVertex3f(-0.5, 0.22, -0.7);//hh'
	glVertex3f(0.0, 0.22, -0.7);//gg'
	glVertex3f(0.0, -0.2, -0.7);//oo'
	glVertex3f(-0.5, -0.2, -0.7);//pp'
	glEnd();


	glBegin(GL_POLYGON);
	glVertex3f(0.12, 0.22, 0.7);//ll
	glVertex3f(0.62, 0.22, 0.7);//kk
	glVertex3f(0.62, -0.2, 0.7);//mm
	glVertex3f(0.12, -0.2, 0.7);//nn
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3f(0.12, 0.22, -0.7);//ll'
	glVertex3f(0.62, 0.22, -0.7);//kk'
	glVertex3f(0.62, -0.2, -0.7);//mm'
	glVertex3f(0.12, -0.2, -0.7);//nn'
	glEnd();


	// 후방에서 볼때 앞에 비어있는 곳 채우기
	glBegin(GL_POLYGON);
	glVertex3f(1.52, .14, 0.7);//k
	glVertex3f(1.52, .14, -0.7);//k'
	glVertex3f(1.52, -.44, -0.7);//j'
	glVertex3f(1.52, -.44, 0.7);//j
	glEnd();

	// 타이어그리기
	glTranslatef(-.58, -.52, 0.7);// 첫번째 타이어 translate
	glColor3f(0.1, 0.1, 0.1);// 타이어 색
	glutSolidTorus(0.12f, .14f, 10, 25);
	glTranslatef(1.68, 0.0, 0.0);// 두번째 타이어 translate
	glutSolidTorus(0.12f, .14f, 10, 25);

	glTranslatef(0.0, 0.0, -1.4);// 세번째 타이어 translate
	glutSolidTorus(0.12f, .14f, 10, 25);
	glTranslatef(-1.68, 0.0, 0.0);// 네번째 타이어 translate
	glutSolidTorus(0.12f, .14f, 10, 25);
	glTranslatef(.58, .52, 0.7);// 원본 타이어 translate
	glRotatef(90.0, 0.0, 1.0, 0.0);
	glTranslatef(0.0, 0.0, -1.40);

	glutSolidTorus(0.2f, .2f, 10, 25); // 차 뒤에 달린 타이어. 장식용.

	glTranslatef(0.0, 0.0, 1.40);
	glRotatef(270.0, 0.0, 1.0, 0.0);


	//glColor3f(1.0, 1.0, 1.0);

	// 자동차 창문 코드
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);// 유리는 반투명

	// 전방유리
	glBegin(GL_POLYGON);
	glColor4f(0.0, 0.0, 0.0, 0.7);   //색 : 반투명 검은색
	glVertex3f(0.562, .5, .6);//AAA
	glVertex3f(.562, .5, -.6);//AAA'
	glVertex3f(.76, .22, -.6);//MMM'
	glVertex3f(.76, .22, .6);//MMM
	glEnd();

	// 뒷유리
	glBegin(GL_POLYGON);
	glVertex3f(-1.068, 0.5, 0.6);//pp
	glVertex3f(-1.068, 0.5, -0.6);//pp'
	glVertex3f(-1.2, 0.22, -0.6);//qq'
	glVertex3f(-1.2, 0.22, 0.6);//qq
	glEnd();

	// 오른쪽 뒷자석 유리
	glBegin(GL_POLYGON);
	glVertex3f(-0.98, 0.5, 0.7);//aa
	glVertex3f(-0.64, 0.5, 0.7);//bb
	glVertex3f(-0.64, 0.22, 0.7);//cc
	glVertex3f(-1.08, 0.22, 0.7);//dd
	glEnd();

	// 왼쪽 뒷자석 유리
	glBegin(GL_POLYGON);
	glVertex3f(-0.98, 0.5, -0.7);//aa
	glVertex3f(-0.64, 0.5, -0.7);//bb
	glVertex3f(-0.64, 0.22, -0.7);//cc
	glVertex3f(-1.08, 0.22, -0.7);//dd
	glEnd();


	// 오른쪽 중간 유리
	glBegin(GL_POLYGON);
	glVertex3f(-0.5, 0.5, 0.7);
	glVertex3f(0.0, 0.5, 0.7);
	glVertex3f(0.0, 0.22, 0.7);
	glVertex3f(-0.5, 0.22, 0.7);
	glEnd();

	// 왼쪽 중간 유리
	glBegin(GL_POLYGON);
	glVertex3f(-0.5, 0.5, -0.7);
	glVertex3f(0.0, 0.5, -0.7);
	glVertex3f(0.0, 0.22, -0.7);
	glVertex3f(-0.5, 0.22, -0.7);
	glEnd();

	//오른쪽 앞좌석 유리
	glBegin(GL_POLYGON);
	glVertex3f(0.12, 0.5, 0.7);//ii
	glVertex3f(0.44, 0.5, 0.7);//jj
	glVertex3f(0.62, 0.22, 0.7);//kk
	glVertex3f(0.12, 0.22, 0.7);//ll
	glEnd();


	//왼쪽 앞좌석 유리

	glBegin(GL_POLYGON);
	glVertex3f(0.12, 0.5, -0.7);//ii'
	glVertex3f(0.44, 0.5, -0.7);//jj'
	glVertex3f(0.62, 0.22, -0.7);//kk'
	glVertex3f(0.12, 0.22, -0.7);//ll'
	glEnd();



	glColor3f(0.0, 0.0, 1.0);
}

void hienThiCar() {
	glPushMatrix(); // 

	instance = Translate(0, 0, 0.03);
	glColor3f(1.0, 0.0, 0.0);
	glCallList(carDisplayList);
	glPopMatrix();
}
GLuint DisplayList() { // 리스트를 사용하여 속도향상. 인터넷에서 찾음
	GLuint carDL; //함수이름의 약자를 변수명으로 선언.

	// list 형성
	carDL = glGenLists(1);

	// 리스트 선언. 시작.
	glNewList(carDL, GL_COMPILE);

	// 자동차 그리는 함수 호출.
	drawCar();

	// list 끝
	glEndList();

	return(carDL);
}
void ve() {
	matDat();
	hangRao();
	drawLine();
	venDuong();
	hienThiCar();
}
void display(void)
{

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.7, 0.85, 1.0, 1.0);

	const vec3 viewer_pos(0.0, 2.0, 0.1);   /*Trùng với eye của camera*/
	glUniformMatrix4fv(model_loc, 1, GL_TRUE, model);
	ve();
	vec4 eye(xx, yy, zz, 1);
	vec4 at(0, 0, 0, 1);
	vec4 up(0, 1, 0, 1);

	view = LookAt(eye, at, up);
	glUniformMatrix4fv(view_loc, 1, GL_TRUE, view);

	projection = Frustum(-1, 1, -1, 1, 1, 4);
	glUniformMatrix4fv(projection_loc, 1, GL_TRUE, projection);
	glutSwapBuffers();
}
void reshape(int width, int height)
{
	
	glViewport(0, 0, width, height);
}

void keyboard(unsigned char key, int x, int y)
{
	// keyboard handler

	switch (key) {
	case 033:			// 033 is Escape key octal value
		exit(1);		// quit program
		break;
	case 'x':
		model *= RotateX(dr);
		break;
	case 'X':
		model *= RotateX(-dr);
		break;
	case 'y':
		model *= RotateY(dr);
		break;
	case 'Y':
		model *= RotateY(-dr);
		break;
	case 'z':
		model *= RotateZ(dr);
		break;
	case 'Z':
		model *= RotateZ(-dr);
		break;
	case 'a':

	case 't': 
		xx = 0;
		yy = 1;
		zz = 0.3;
		break;
		// chọn số 1 cho quạt
	}
	glutPostRedisplay();
}
void spinCube(void)
{
	glutPostRedisplay();
}
int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(1000, 500);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("A Cube is rotated by keyboard and shaded");


	glewInit();

	generateGeometry();
	initGPUBuffers();
	shaderSetup();

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(reshape);
	glutIdleFunc(spinCube);

	glutMainLoop();
	return 0;
}
