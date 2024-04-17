#include "Render.h"

#include <sstream>
#include <iostream>

#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "GUItextRectangle.h"
using std::vector;
bool textureMode = true;
bool lightMode = true;

//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;

	
	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//считает позицию камеры, исходя из углов поворота, вызывается движком
	void SetUpCamera()
	{
		//отвечает за поворот камеры мышкой
		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist*cos(fi2)*cos(fi1),
			camDist*cos(fi2)*sin(fi1),
			camDist*sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		//функция настройки камеры
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(1, 1, 3);
	}

	
	//рисует сферу и линии под источником света, вызывается движком
	void  DrawLightGhismo()
	{
		glDisable(GL_LIGHTING);

		
		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale*0.08;
		s.Show();
		
		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//линия от источника света до окружности
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окруность
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale*1.5;
			c.Show();
		}

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света




//старые координаты мыши
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}

	
	//двигаем свет по плоскости, в точку где мышь
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k*r.direction.X() + r.origin.X();
		y = k*r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02*dy);
	}

	
}

void mouseWheelEvent(OpenGL *ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;

}

void keyDownEvent(OpenGL *ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}
}

void keyUpEvent(OpenGL *ogl, int key)
{
	
}



GLuint texId;

//выполняется перед первым рендером
void initRender(OpenGL *ogl)
{
	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);
	

	//массив трехбайтных элементов  (R G B)
	RGBTRIPLE *texarray;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char *texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);

	
	
	//генерируем ИД для текстуры
	glGenTextures(1, &texId);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//отчистка памяти
	free(texCharArray);
	free(texarray);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH); 


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	camera.fi1 = -1.3;
	camera.fi2 = 0.8;
}
double N_Vector_X(double A[], double B[], double C[], double height) {




	//Счиатем А и В по входным точкам
	double Vector_AB[3] = { B[0] - A[0], B[1] - A[1], B[2] - A[2] };
	double Vector_AC[3] = { C[0] - A[0], C[1] - A[1], C[2] - A[2] };

	//Создаем вектор N
	double N_X = Vector_AB[1] * Vector_AC[2] - Vector_AC[1] * Vector_AB[2];
	double N_Y = -Vector_AB[0] * Vector_AC[2] + Vector_AC[0] * Vector_AB[2];
	double N_Z = Vector_AB[0] * Vector_AC[1] - Vector_AC[0] * Vector_AB[1];

	double N_Vector[] = { N_X,N_Y,N_Z };
	double Abs_Vector = sqrt(N_X * N_X + N_Y * N_Y + N_Z * N_Z);

	N_Vector[0] = N_X / Abs_Vector;
	N_Vector[1] = N_Y / Abs_Vector;
	N_Vector[2] = N_Z / Abs_Vector;
	return N_Vector[0];
}

double N_Vector_Y(double A[], double B[], double C[], double height) {




	//Счиатем А и В по входным точкам
	double Vector_AB[3] = { B[0] - A[0], B[1] - A[1], B[2] - A[2] };
	double Vector_AC[3] = { C[0] - A[0], C[1] - A[1], C[2] - A[2] };

	//Создаем вектор N
	double N_X = Vector_AB[1] * Vector_AC[2] - Vector_AC[1] * Vector_AB[2];
	double N_Y = -Vector_AB[0] * Vector_AC[2] + Vector_AC[0] * Vector_AB[2];
	double N_Z = Vector_AB[0] * Vector_AC[1] - Vector_AC[0] * Vector_AB[1];

	double N_Vector[] = { N_X,N_Y,N_Z };
	double Abs_Vector = sqrt(N_X * N_X + N_Y * N_Y + N_Z * N_Z);

	N_Vector[0] = N_X / Abs_Vector;
	N_Vector[1] = N_Y / Abs_Vector;
	N_Vector[2] = N_Z / Abs_Vector;
	return N_Vector[1];
}

double N_Vector_Z(double A[], double B[], double C[], double height) {




	//Счиатем А и В по входным точкам
	double Vector_AB[3] = { B[0] - A[0], B[1] - A[1], B[2] - A[2] };
	double Vector_AC[3] = { C[0] - A[0], C[1] - A[1], C[2] - A[2] };

	//Создаем вектор N
	double N_X = Vector_AB[1] * Vector_AC[2] - Vector_AC[1] * Vector_AB[2];
	double N_Y = -Vector_AB[0] * Vector_AC[2] + Vector_AC[0] * Vector_AB[2];
	double N_Z = Vector_AB[0] * Vector_AC[1] - Vector_AC[0] * Vector_AB[1];

	double N_Vector[] = { N_X,N_Y,N_Z };
	double Abs_Vector = sqrt(N_X * N_X + N_Y * N_Y + N_Z * N_Z);

	N_Vector[0] = N_X / Abs_Vector;
	N_Vector[1] = N_Y / Abs_Vector;
	N_Vector[2] = N_Z / Abs_Vector;
	return N_Vector[2];
}
void krug(float high) {
	glColor3d(0.6, 0.1, 0.1);
	float x, y;
	double D = sqrtf(45);
	double R = D / 2;
	//double center[] = { -2, 9.5, 0 };
	const double count = 200;
	double A = M_PI * 2 / count;
	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(0, 0, -1);
	for (double i = 0; i < count; i++) {
		x = sin(A * i) * R - 2;
		y = cos(A * i) * R + 9.5;
		if (y >= 0.5 * x + 10.5)
			glVertex3d(x, y, 0);
		else if (y >= 0.5 * x + 10.4) {
			glVertex3d(x, y, 0);
		}
		else if (y >= 0.5 * x + 10.3) {
			glVertex3d(x, y, 0);
		}
	}
	glEnd();
}
void krug1(float high) {
	glColor4f(0.6, 0.1, 0.1, 0.7);
	float x, y;
	double D = sqrtf(45);
	double R = D / 2;
	//double center[] = { -2, 9.5, 0 };
	const double count = 200;
	double A = M_PI * 2 / count;
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(0, 0, 1);
	for (double i = 0; i < count; i++) {
		x = sin(A * i) * R - 2;
		y = cos(A * i) * R + 9.5;
		if (y >= 0.5 * x + 10.5)
			glVertex3d(x, y, high);
		else if (y >= 0.5 * x + 10.4) {
			glVertex3d(x, y, high);
		}
		else if (y >= 0.5 * x + 10.3) {
			glVertex3d(x, y, high);
		}
	}
	glEnd();

	//Добавляю построение верха из вогнутости, чтобы сделать альфа наложение
	vector<double> xv;
	vector <double> yv;
	D = sqrtf(pow(4, 2) + pow(6, 2));
	R = D / 2;
	vector<double>::iterator it1, it2;
	for (double i = 0; i < count; i++) {
		x = sin(A * i) * R + 5;
		y = cos(A * i) * R;
		if (y >= 1.5 * x - 7.3 && y >= -x && y <= 3) {
			glVertex3d(x, y, 0);
			glVertex3d(x, y, high);
			xv.push_back(x);
			yv.push_back(y);

		}
	}
	it1 = xv.begin();
	it2 = yv.begin();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(0.6, 0.1, 0.1, 0.7);
	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(0, 0, 1);
	glVertex3d(0, 0, high);
	while (it1 != xv.end()) {
		glVertex3d(*it1, *it2, high);
		it1++;
		it2++;
	}
	glVertex3d(2, 3, high);
	glEnd();
}

void lepka(float high) {
	double g[] = { -5, 8, 0 };
	double h[] = { 1, 11, 0 };
	double g1[] = { -5, 8, high };
	double h1[] = { 1, 11, high };
	double n_x = N_Vector_X(g, g1, h, high);
	double n_y = N_Vector_Y(g, g1, h, high);
	double n_z = N_Vector_Z(g, g1, h, high);
	glColor3d(0, 1, 0);
	//glColor3d(0.6, 0.1, 0.1);
	//glBegin(GL_QUAD_STRIP);
	glBegin(GL_TRIANGLE_STRIP);
	//glBegin(GL_TRIANGLE_FAN);
	glNormal3f(n_x, n_y, n_z);
	float x, y;
	float D = sqrtf(45);
	float R = D / 2;
	const float count = 10;
	float A = M_PI * 2 / count;
	for (float i = 0; i < count; i += 0.01) {
		x = sin(A * i) * R - 2;
		y = cos(A * i) * R + 9.5;
		if (y > 0.5 * x + 10.5) {
			glVertex3d(x, y, 0);
			glVertex3d(x, y, high);
		}
	}
	glEnd();
}

void vpuklost(float high) {
	glColor3d(0, 0, 1);
	double a2[] = { 1.98,-1.97, 0 };
	double a3[] = { 1.98,-1.97, high };
	double b2[] = { 3, 3, 0 };
	double b3[] = { 3, 3, high };
	double n_x = N_Vector_X(a2, a3, b2, high);
	double n_y = N_Vector_Y(a2, a3, b2, high);
	double n_z = N_Vector_Z(a2, a3, b2, high);
	glBegin(GL_QUAD_STRIP);
	glNormal3f(-n_x, n_y, n_z);
	double x, y;
	vector<double> xv;
	vector <double> yv;
	double D = sqrtf(pow(4, 2) + pow(6, 2));
	double R = D / 2;
	const double count = 200;
	double A = M_PI * 2 / count;
	for (double i = 0; i < count; i++) {
		x = sin(A * i) * R + 5;
		y = cos(A * i) * R;
		if (y >= 1.5 * x - 7.3 && y >= -x && y <= 3) {
			glVertex3d(x, y, 0);
			glVertex3d(x, y, high);
			xv.push_back(x);
			yv.push_back(y);

		}
	}
	glEnd();
	glColor3d(0.6, 0.1, 0.1);
	vector<double>::iterator it1, it2;
	it1 = xv.begin();
	it2 = yv.begin();
	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(0, 0, -1);
	glVertex3d(0, 0, 0);
	while (it1 != xv.end()) {
		glVertex3d(*it1, *it2, 0);
		it1++;
		it2++;
	}
	glVertex3d(2, 3, 0);
	glEnd();
	/*it1 = xv.begin();
	it2 = yv.begin();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(0.6, 0.1, 0.1, 0.7);
	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(0, 0, 1);
	glVertex3d(0, 0, high);
	while (it1 != xv.end()) {
		glVertex3d(*it1, *it2, high);
		it1++;
		it2++;
	}
	glVertex3d(2, 3, high);
	glEnd();*/
}
void figure(float high) {

	double a[] = { 3, -3, 0 };
	double a2[] = { 1.98,-1.97, 0 };
	double a3[] = { 1.98,-1.97, high };
	double b[] = { 7, 3, 0 };
	double b2[] = { 3, 3, 0 };
	double b3[] = { 3, 3, high };
	double c[] = { 0, 0, 0 };
	double d[] = { 2, 3, 0 };
	double e[] = { -7, -3, 0 };
	double f[] = { -1, 2, 0 };
	double g[] = { -5, 8, 0 };
	double h[] = { 1, 11, 0 };
	glBegin(GL_TRIANGLES);

	glNormal3f(0, 0, -1);
	glColor3d(0.6, 0.1, 0.1);
	/*glVertex3dv(a);
	glVertex3dv(b);
	glVertex3dv(c);*/


	/*glVertex3dv(b);
	glVertex3dv(d);
	glVertex3dv(c);*/

	glVertex3dv(c);
	glVertex3dv(d);
	glVertex3dv(f);

	glVertex3dv(f);
	glVertex3dv(c);
	glVertex3dv(e);

	glVertex3dv(f);
	glVertex3dv(g);
	glVertex3dv(d);


	glVertex3dv(g);
	glVertex3dv(h);
	glVertex3dv(d);
	glEnd();

	double a1[] = { 3, -3, high };
	double b1[] = { 7, 3, high };
	double c1[] = { 0, 0, high };
	double d1[] = { 2, 3, high };
	double e1[] = { -7, -3, high };
	double f1[] = { -1, 2, high };
	double g1[] = { -5, 8, high };
	double h1[] = { 1, 11, high };
	
	double n_x = N_Vector_X(a2, a3, c, high);
	double n_y = N_Vector_Y(a2, a3, c, high);
	double n_z = N_Vector_Z(a2, a3, c, high);

	glBindTexture(GL_TEXTURE_2D, texId);
	glBegin(GL_QUADS);
	glNormal3f(n_x, n_y, n_z);
	glColor3d(0.2, 0.4, 0.3);
	glTexCoord2d(0, 0);
	glVertex3dv(a2);
	glTexCoord2d(0, 1);
	glVertex3dv(a3);
	glTexCoord2d(1, 1);
	glVertex3dv(c1);
	glTexCoord2d(1, 0);
	glVertex3dv(c);
	glEnd(); 

	n_x = N_Vector_X(c, c1, e, high);
	n_y = N_Vector_Y(c, c1, e, high);
	n_z = N_Vector_Z(c, c1, e, high);

	glBindTexture(GL_TEXTURE_2D, texId);
	glBegin(GL_QUADS);
	glNormal3f(n_x, n_y, n_z);
	glColor3d(0.2, 0.4, 0.3);
	glTexCoord2d(0, 0);
	glVertex3dv(c);
	glTexCoord2d(0, 1);
	glVertex3dv(c1);
	glTexCoord2d(1, 1);
	glVertex3dv(e1);
	glTexCoord2d(1, 0);
	glVertex3dv(e);
	glEnd();
	
	n_x = N_Vector_X(e, e1, f, high);
	n_y = N_Vector_Y(e, e1, f, high);
	n_z = N_Vector_Z(e, e1, f, high);

	glBindTexture(GL_TEXTURE_2D, texId);
	glBegin(GL_QUADS);
	glNormal3f(n_x, n_y, n_z);
	glColor3d(0.2, 0.4, 0.3);
	glTexCoord2d(0, 0);
	glVertex3dv(e);
	glTexCoord2d(0, 1);
	glVertex3dv(e1);
	glTexCoord2d(1, 1);
	glVertex3dv(f1);
	glTexCoord2d(1, 0);
	glVertex3dv(f);
	glEnd();

	n_x = N_Vector_X(f, f1, g, high);
	n_y = N_Vector_Y(f, f1, g, high);
	n_z = N_Vector_Z(f, f1, g, high);

	glBindTexture(GL_TEXTURE_2D, texId);
	glBegin(GL_QUADS);
	glNormal3f(n_x, n_y, n_z);
	glColor3d(0.2, 0.4, 0.3);
	glTexCoord2d(0, 0);
	glVertex3dv(f);
	glTexCoord2d(0, 1);
	glVertex3dv(f1);
	glTexCoord2d(1, 1);
	glVertex3dv(g1);
	glTexCoord2d(1, 0);
	glVertex3dv(g);
	glEnd();

	n_x = N_Vector_X(g, g1, h, high);
	n_y = N_Vector_Y(g, g1, h, high);
	n_z = N_Vector_Z(g, g1, h, high);

	/*glBindTexture(GL_TEXTURE_2D, texId);
	glBegin(GL_QUADS);
	glNormal3f(n_x, n_y, n_z);
	glColor3d(0.2, 0.4, 0.3);
	glTexCoord2d(0, 0);
	glVertex3dv(g);
	glTexCoord2d(0, 1);
	glVertex3dv(g1);
	glTexCoord2d(1, 1);
	glVertex3dv(h1);
	glTexCoord2d(1, 0);
	glVertex3dv(h);
	glEnd();*/

	n_x = N_Vector_X(h, h1, d, high);
	n_y = N_Vector_Y(h, h1, d, high);
	n_z = N_Vector_Z(h, h1, d, high);

	glBindTexture(GL_TEXTURE_2D, texId);
	glBegin(GL_QUADS);
	glNormal3f(n_x, n_y, n_z);
	glColor3d(0.2, 0.4, 0.3);
	glTexCoord2d(0, 0);
	glVertex3dv(h);
	glTexCoord2d(0, 1);
	glVertex3dv(h1);
	glTexCoord2d(1, 1);
	glVertex3dv(d1);
	glTexCoord2d(1, 0);
	glVertex3dv(d);
	glEnd();

	n_x = N_Vector_X(d, d1, b2, high);
	n_y = N_Vector_Y(d, d1, b2, high);
	n_z = N_Vector_Z(d, d1, b2, high);

	glBindTexture(GL_TEXTURE_2D, texId);
	glBegin(GL_QUADS);
	glNormal3f(n_x, n_y, n_z);
	glColor3d(0.2, 0.4, 0.3);
	glTexCoord2d(0, 0);
	glVertex3dv(d);
	glTexCoord2d(0, 1);
	glVertex3dv(d1);
	glTexCoord2d(1, 1);
	glVertex3dv(b3);
	glTexCoord2d(1, 0);
	glVertex3dv(b2);
	glEnd();


	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBegin(GL_TRIANGLES);
	glNormal3f(0, 0, 1);
	glColor4f(0.6f, 0.1f, 0.1f, 0.7f);
	/*glVertex3dv(a1);
	glVertex3dv(b1);
	glVertex3dv(c1);*/


	/*glVertex3dv(b1);
	glVertex3dv(d1);
	glVertex3dv(c1);*/

	glVertex3dv(c1);
	glVertex3dv(d1);
	glVertex3dv(f1);

	glVertex3dv(f1);
	glVertex3dv(c1);
	glVertex3dv(e1);

	glVertex3dv(f1);
	glVertex3dv(g1);
	glVertex3dv(d1);


	glVertex3dv(g1);
	glVertex3dv(h1);
	glVertex3dv(d1);
	glEnd();

}
void circle() {
	glBindTexture(GL_TEXTURE_2D, texId);
	glBegin(GL_POLYGON);
	for (double i = 0; i <= 2; i += 0.01)
	{
		double x = 9 * cos(i * 3.141593);
		double y = 9 * sin(i * 3.141593);

		double tx = cos(i * 3.141593) * 0.5 + 0.5;
		double ty = sin(i * 3.141593) * 0.5 + 0.5;

		glColor3d(0.5f, 0.5f, 0.5f);
		glNormal3d(0, 0, 1);
		glTexCoord2d(tx, ty);
		glVertex3d(x, y, 0);
	}
	glEnd();
}
void ShowFigure(float high = 4) {
	//circle(); 
	krug(high);
	lepka(high);
	vpuklost(high);
	figure(high);
	krug1(high);
}
void Render(OpenGL *ogl)
{



	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);


	//альфаналожение
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//настройка материала
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;


	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); \
		//размер блика
		glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//чтоб было красиво, без квадратиков (сглаживание освещения)
	glShadeModel(GL_SMOOTH);
	//===================================
	//ПРОГАТЬ ТУТ

	ShowFigure();


	//Начало рисования квадратика станкина
	/*double A[2] = { -4, -4 };
	double B[2] = { 4, -4 };
	double C[2] = { 4, 4 };
	double D[2] = { -4, 4 };

	glBindTexture(GL_TEXTURE_2D, texId);

	glColor3d(0.6, 0.6, 0.6);
	glBegin(GL_QUADS);

	glNormal3d(0, 0, 1);
	glTexCoord2d(0, 0);
	glVertex2dv(A);
	glTexCoord2d(1, 0);
	glVertex2dv(B);
	glTexCoord2d(1, 1);
	glVertex2dv(C);
	glTexCoord2d(0, 1);
	glVertex2dv(D);

	glEnd();*/
	//конец рисования квадратика станкина


   //Сообщение вверху экрана

	
	glMatrixMode(GL_PROJECTION);	//Делаем активной матрицу проекций. 
	                                //(всек матричные операции, будут ее видоизменять.)
	glPushMatrix();   //сохраняем текущую матрицу проецирования (которая описывает перспективную проекцию) в стек 				    
	glLoadIdentity();	  //Загружаем единичную матрицу
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //врубаем режим ортогональной проекции

	glMatrixMode(GL_MODELVIEW);		//переключаемся на модел-вью матрицу
	glPushMatrix();			  //сохраняем текущую матрицу в стек (положение камеры, фактически)
	glLoadIdentity();		  //сбрасываем ее в дефолт

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //классик моего авторства для удобной работы с рендером текста.
	rec.setSize(300, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);


	std::stringstream ss;
	ss << "T - вкл/выкл текстур" << std::endl;
	ss << "L - вкл/выкл освещение" << std::endl;
	ss << "F - Свет из камеры" << std::endl;
	ss << "G - двигать свет по горизонтали" << std::endl;
	ss << "G+ЛКМ двигать свет по вертекали" << std::endl;
	ss << "Коорд. света: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "Коорд. камеры: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "Параметры камеры: R="  << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;
	
	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //восстанавливаем матрицы проекции и модел-вью обратьно из стека.
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}