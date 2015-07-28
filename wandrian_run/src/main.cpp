/*
 * main.cpp
 *
 *  Created on: Jun 25, 2015
 *      Author: sontd
 */

#include <GL/glut.h>
#include <stdio.h>
#include <iostream>
#include <limits>
#include <set>
#include <ctime>
#include <cstdlib>
#include <boost/next_prior.hpp>
#include "../include/common/polygon.hpp"
#include "../include/common/segment.hpp"

using namespace common::shapes;

Polygon *polygon;

std::map<Point*, std::set<Point*, PointComp>, PointComp> graph;
std::set<Point*> upper_vertices;
std::set<Point*> lower_vertices;

/**
 * Linked libraries to compile: -lglut -lGL (g++)
 */

void display() {
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-6, 6, -6, 6, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glScalef(0.5, 0.5, 0);

	// TODO draw primitives here

	glPointSize(4);
	glColor3ub(255, 255, 0);
	glBegin(GL_POINTS);
	glVertex2i(0, 0);
	glEnd();

	glPointSize(1);
	glColor3ub(255, 255, 255);
	glBegin(GL_POINTS);
	for (int i = -10; i <= 10; i++) {
		for (int j = -10; j <= 10; j++) {
			if (i != 0 || j != 0)
				glVertex2i(i, j);
		}
	}
	glEnd();

	glColor3ub(255, 255, 255);
	for (std::map<Point*, std::set<Point*, PointComp> >::iterator current =
			graph.begin(); current != graph.end(); current++) {
		std::cout << "\033[1;31m" << (*current).first->x << " "
				<< (*current).first->y << "\033[1;0m: ";
//		glPointSize(4);
//		glColor3ub(255, 255, 255);
//		glBegin(GL_POINTS);
//		glVertex2d((*current).first->x, (*current).first->y);
//		glEnd();
		for (std::set<Point*>::iterator adjacent = (*current).second.begin();
				adjacent != (*current).second.end(); adjacent++) {
//			glPointSize(4);
//			glColor3ub(255, 255, 255);
//			glBegin(GL_POINTS);
//			glVertex2d((*adjacent)->x, (*adjacent)->y);
//			glEnd();
			glBegin(GL_LINE_STRIP);
			glVertex2d((*current).first->x, (*current).first->y);
			glVertex2d((*adjacent)->x, (*adjacent)->y);
			glEnd();
			std::cout << (*adjacent)->x << " " << (*adjacent)->y << ", ";
		}
		std::cout << "\n";
	}
	std::cout << "\n";

//	for (std::map<Point*, std::set<Point*, PointComp> >::iterator current =
//			graph.begin(); current != graph.end(); current++) {
//		glPointSize(4);
//		glColor3ub(255, 0, 255);
//		glBegin(GL_POINTS);
//		glVertex2d((*current).first->x, (*current).first->y);
//		glEnd();
//
//		for (std::set<Point*>::iterator adjacent = (*current).second.begin();
//				adjacent != (*current).second.end(); adjacent++) {
//			glColor3ub(0, 255, 255);
//			glBegin(GL_POINTS);
//			glVertex2d((*adjacent)->x, (*adjacent)->y);
//			glEnd();
//		}
//		glutSwapBuffers();
//		getchar();
//		glPointSize(4);
//		glColor3ub(255, 255, 255);
//		glBegin(GL_POINTS);
//		glVertex2d((*current).first->x, (*current).first->y);
//		glEnd();
//		for (std::set<Point*>::iterator adjacent = (*current).second.begin();
//				adjacent != (*current).second.end(); adjacent++) {
//			glColor3ub(255, 255, 255);
//			glBegin(GL_POINTS);
//			glVertex2d((*adjacent)->x, (*adjacent)->y);
//			glEnd();
//		}
//	}

	glColor3ub(0, 255, 0);
	std::cout << "\033[1;32m";
	for (std::set<Point*>::iterator current = upper_vertices.begin();
			boost::next(current) != upper_vertices.end(); current++) {
		std::cout << (*current)->x << " " << (*current)->y << ", ";
		glBegin(GL_LINE_STRIP);
		glVertex2d((*current)->x, (*current)->y);
		glVertex2d((*boost::next(current))->x, (*boost::next(current))->y);
		glEnd();
	}
	std::cout << "\033[1;0m\n";

	glColor3ub(0, 0, 255);
	std::cout << "\033[1;34m";
	for (std::set<Point*>::iterator current = lower_vertices.begin();
			boost::next(current) != lower_vertices.end(); current++) {
		std::cout << (*current)->x << " " << (*current)->y << ", ";
		glBegin(GL_LINE_STRIP);
		glVertex2d((*current)->x, (*current)->y);
		glVertex2d((*boost::next(current))->x, (*boost::next(current))->y);
		glEnd();
	}
	std::cout << "\033[1;0m\n";

	glutSwapBuffers();
}

int run(int argc, char **argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(600, 600);
	glutCreateWindow("GLUT");
	glutDisplayFunc(display);
	glutMainLoop();
	return 0;
}

int main(int argc, char **argv) {
	std::set<Point*> points;

	std::srand(std::time(0));
	int r = std::rand() % 45;
	for (int i = 0; i <= r; i++) {
		int x = std::rand() % 21 - 10;
		int y = std::rand() % 21 - 10;
		std::cout << x << " " << y << "\n";
		points.insert(new Point(x, y));
	}
	polygon = new Polygon(points);
	graph = polygon->get_graph();
	upper_vertices = polygon->upper_vertices();
	lower_vertices = polygon->lower_vertices();

	run(argc, argv);
	return 0;
}
