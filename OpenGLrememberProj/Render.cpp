﻿#include "Render.h"

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

bool textureMode = true;
bool lightMode = true;

void DrawingFigure();
void DrawingCircleWalls(double* X, double* Y);
void FoundR(double* X, double* Y, double R[]);
double FoundCosAngle();

double A1[] = { 0,0,0.1 };
double B1[] = { 5,0,0.1 };
double C1[] = { 2,7,0.1 };
double D1[] = { 9,15,0.1 };
double E1[] = { -1,13,0.1 };
double F1[] = { -4,6,0.1 };

double A2[] = { 0,0,4 };
double B2[] = { 5,0,4 };
double C2[] = { 2,7,4 };
double D2[] = { 9,15,4 };
double E2[] = { -1,13,4 };
double F2[] = { -4,6,4 };

double R[] = { 0,0,0,0 };

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
	//Прогать тут  



	//-------------------------------------------------------------------------

	glPushMatrix();
	glTranslated(4.5, -4, 0);
	DrawingFigure();
	glPopMatrix();

	//-------------------------------------------------------------------------



	////Начало рисования квадратика станкина
	//double A[2] = { -4, -4 };
	//double B[2] = { 4, -4 };
	//double C[2] = { 4, 4 };
	//double D[2] = { -4, 4 };
	//
	//glBindTexture(GL_TEXTURE_2D, texId);
	//
	//glColor3d(0.6, 0.6, 0.6);
	//glBegin(GL_QUADS);
	//
	//glNormal3d(0, 0, 1);
	//glTexCoord2d(0, 0);
	//glVertex2dv(A);
	//glTexCoord2d(1, 0);
	//glVertex2dv(B);
	//glTexCoord2d(1, 1);
	//glVertex2dv(C);
	//glTexCoord2d(0, 1);
	//glVertex2dv(D);
	//
	//glEnd();
	////конец рисования квадратика станкина


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

void DrawingFigure()
{
#pragma region Base

	glBegin(GL_TRIANGLES);
	glColor3d(0.2, 0.2, 0.2);
	glVertex3dv(A1);
	glVertex3dv(B1);
	glVertex3dv(C1);

	glVertex3dv(C1);
	glVertex3dv(A1);
	glVertex3dv(F1);

	glVertex3dv(F1);
	glVertex3dv(C1);
	glVertex3dv(E1);

	glVertex3dv(E1);
	glVertex3dv(C1);
	glVertex3dv(D1);
	//---------------------------
	glColor3d(0.2, 0.2, 0.2);
	glVertex3dv(A2);
	glVertex3dv(B2);
	glVertex3dv(C2);

	glVertex3dv(C2);
	glVertex3dv(A2);
	glVertex3dv(F2);

	glVertex3dv(F2);
	glVertex3dv(C2);
	glVertex3dv(E2);

	glVertex3dv(E2);
	glVertex3dv(C2);
	glVertex3dv(D2);
	glEnd();
#pragma endregion

#pragma region Walls
	glBegin(GL_QUADS);

	glColor3d(0.8, 0, 0);
	glVertex3dv(A1);
	glVertex3dv(A2);
	glVertex3dv(B2);
	glVertex3dv(B1);

	glColor3d(0.7, 0, 0);
	glVertex3dv(C1);
	glVertex3dv(C2);
	glVertex3dv(B2);
	glVertex3dv(B1);

	glColor3d(0.6, 0, 0);
	glVertex3dv(C1);
	glVertex3dv(C2);
	glVertex3dv(D2);
	glVertex3dv(D1);

	glColor3d(0.5, 0, 0);
	glVertex3dv(D1);
	glVertex3dv(D2);
	glVertex3dv(E2);
	glVertex3dv(E1);

	glColor3d(0.4, 0, 0);
	glVertex3dv(F1);
	glVertex3dv(F2);
	glVertex3dv(A2);
	glVertex3dv(A1);
	glEnd();
#pragma endregion

#pragma region Semicircle
	double grad = 0.005;	//Øàã ìåæäó òî÷êàìè íà îêðóæíîñòè â ðàäèàíàõ
	double X[] = { 0,0,0.1 };
	double Y[] = { 0,0,0.1 };

	FoundR(F1, E1, R);//Íàõîäèì êîîðäèíàòû öåíòðà îêðóæíîñòè è ðàäèóñ

	//double vector[] = { 3, 7, 0 };
	//R[0] += R[0] * 3 * -0.00035;		//Íåáîëüøàÿ êîððåêòèðîâêà êîîðäèíàò öåíòðà îêðóæíîñòè
	//R[1] += R[1] * 7 * -0.00035;

	glPushMatrix();
	//glTranslated(0.01, 0.01, 0);
	double angle = FoundCosAngle();
	for (double i = 0.005; i < 3.145; i += 0.005)
	{
		X[0] = (R[0] + cos(angle + i - grad) * R[3]);//êîîðäèíàòà Õ ïåðâîé òî÷êè
		X[1] = (R[1] + sin(angle + i - grad) * R[3]);//êîîðäèíàòà Ó ïåðâîé òî÷êè

		Y[0] = (R[0] + cos(angle + i) * R[3]);//êîîðäèíàòà Õ âòîðîé òî÷êè
		Y[1] = (R[1] + sin(angle + i) * R[3]);//êîîðäèíàòà Ó âòîðîé òî÷êè

		glBegin(GL_TRIANGLES);
		glColor3d(0, 0.7, 0);
		glVertex3d(R[0], R[1], R[2]);
		glVertex3d(X[0], X[1], 0.1);
		glVertex3d(Y[0], Y[1], 0.1);

		glVertex3d(R[0], R[1], 4);
		glVertex3d(X[0], X[1], 4);
		glVertex3d(Y[0], Y[1], 4);

		glEnd();

		glBegin(GL_QUADS);
		glColor3d(0, 0, 0.6);
		glVertex3d(X[0], X[1], 0.1);
		glVertex3d(Y[0], Y[1], 0.1);
		glVertex3d(Y[0], Y[1], 4);
		glVertex3d(X[0], X[1], 4);
		glEnd();
	}

	glPopMatrix();
#pragma endregion

}

void DrawingCircleWalls(double* X, double* Y)
{
	glBegin(GL_QUADS);
	glVertex3d(*X, *++X, 0.1);
	glVertex3d(*Y, *++Y, 0.1);
	*--Y;
	glVertex3d(*Y, *++Y, 4);
	*--X;
	glVertex3d(*X, *++X, 4);
	glEnd();
}

void FoundR(double* X, double* Y, double R[])
{
	R[0] = (*X + *Y) / 2;
	X++;
	Y++;
	R[1] = (*X + *Y) / 2;
	R[2] = 0.1;				
	X--;
	Y--;
	R[3] = sqrt(pow(X[0] - Y[0], 2) + pow(X[1] - Y[1], 2) + 0) / 2;
}

double FoundCosAngle()
{
	double Gipoten = sqrt(1.5 * 1.5 + 3.5 * 3.5);
	double CosAngle = 1.5 / Gipoten;
	return acos(CosAngle);
}
