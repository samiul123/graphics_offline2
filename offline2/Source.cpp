#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<iostream>
#include<windows.h>
#include<GL/glut.h>
#include<vector>


#define pi (2*acos(0.0))

using namespace std;
struct point2d
{
	double x, y, z;
};

vector<struct point2d> allPointsVector;

struct point2d cp[200], dp[200], curve[300], scp[200], dcp[200], pos, onCurve[200];
double euclidDist[200];
int cpidx, num_points, dpidx, scpidx, dcpidx, allCurveIdx, moveIdx, onCurveIdx;
int right_click = 0, isDrawn = 0, toggleGeometry = 1, updateMode = 0, leftClickCount = 0, leftClickedTwice = 0, a_clicked = 0, left_clicked_start = 0;
double min;
int location_dcp, location_in_cp, location_in_dp, location_cp, location_on_curve;;
int iOnCurve, iOutCurve;
double basis_arr[4][4] = { {2, -2, 1, 1}, {-3, 3, -2, -1}, {0, 0, 1, 0}, {1, 0, 0, 0} };

void hermite(double del, struct point2d p1, struct point2d p4, struct point2d r1, struct point2d r4) {
	
	struct point2d geometry_arr[4][1] = { {p1}, {p4}, {r1}, {r4} };
	struct point2d coff[4][1] = {0};
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 1; ++j) {
			for (int k = 0; k < 4; ++k){
				coff[i][j].x += basis_arr[i][k] * geometry_arr[k][j].x;
				coff[i][j].y += basis_arr[i][k] * geometry_arr[k][j].y;
			}
		}
	}
		
	double diff1x = coff[0][0].x * pow(del, 3) + coff[1][0].x * pow(del, 2) + coff[2][0].x * del;
	double diff1y = coff[0][0].y * pow(del, 3) + coff[1][0].y * pow(del, 2) + coff[2][0].y * del;

	double diff2x = 6 * coff[0][0].x * pow(del, 3) + 2 * coff[1][0].x * pow(del, 2);
	double diff2y = 6 * coff[0][0].y * pow(del, 3) + 2 * coff[1][0].y * pow(del, 2);

	double diff3x = 6 * coff[0][0].x * pow(del, 3);
	double diff3y = 6 * coff[0][0].y * pow(del, 3);

	double currentX = p1.x;
	double currentY = p1.y;

	curve[0].x = currentX;
	curve[0].y = currentY;
	allPointsVector.push_back(curve[0]);
	//cout << "all_curve_idx: " << allCurveIdx << endl;
	for (int i = 1; i < 299; i++) {
		currentX += diff1x;
		currentY += diff1y;

		//increment differences
		diff1x += diff2x;
		diff1y += diff2y;

		diff2x += diff3x;
		diff2y += diff3y;

		curve[i].x = currentX;
		curve[i].y = currentY;
		
		allPointsVector.push_back(curve[i]);
	}

	curve[299].x = p4.x;
	curve[299].y = p4.y;
	allPointsVector.push_back(curve[299]);
	//all_curves[all_curves_idx] = curve;
}


void drawSquare(double a, struct point2d v)
{
	//glColor3f(1, 1, 0);
	glBegin(GL_QUADS);
	{
		glVertex3f(v.x + a * sqrt(2) * cos(45 * .0175), v.y + a * sqrt(2)*sin(45 * .0175), 0);
		glVertex3f(v.x - a * sqrt(2) * cos(45 * .0175), v.y + a * sqrt(2)*sin(45 * .0175), 0);
		glVertex3f(v.x - a * sqrt(2) * cos(45 * .0175), v.y - a * sqrt(2)*sin(45 * .0175), 0);
		glVertex3f(v.x + a * sqrt(2) * cos(45 * .0175), v.y - a * sqrt(2)*sin(45 * .0175), 0);
	}
	glEnd();
}

void drawLine(GLfloat p1_x, GLfloat p1_y, GLfloat p2_x, GLfloat p2_y) {
	glColor3f(1, 1, 1);
	glBegin(GL_LINES); {
		glVertex2f(p1_x, p1_y);
		glVertex2f(p2_x, p2_y);
	}
	glEnd();
}

void drawCircle(GLfloat x, GLfloat y, GLfloat radius, int segments) {
	if (updateMode && leftClickCount == 1) {
		int i;
		GLfloat twicePi = 2.0f * pi;
		glBegin(GL_TRIANGLE_FAN);
		for (i = 0; i <= segments; i++) {
			glVertex2f(
				x + (radius * cos(i *  twicePi / segments)),
				y + (radius* sin(i * twicePi / segments))
			);
		}
		glEnd();
	}
	
}

void drawMovingCircle(GLfloat x, GLfloat y, GLfloat radius, int segments) {
	if (a_clicked && isDrawn) {
		int i;
		GLfloat twicePi = 2.0f * pi;
		glBegin(GL_TRIANGLE_FAN);
		for (i = 0; i <= segments; i++) {
			glVertex2f(
				x + (radius * cos(i *  twicePi / segments)),
				y + (radius* sin(i * twicePi / segments))
			);
		}
		glEnd();
	}

}


double euclid_dist(double x1, double y1, double x2, double y2) {
	return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

void drawTriangle(GLfloat p1X, GLfloat p1Y, GLfloat cX, GLfloat cY) {

	double fraction = euclid_dist(p1X, p1Y, cX, cY) / 700;
	struct point2d center_to_p_dir = { p1X - cX, p1Y - cY };
	struct point2d temp_p = { p1X - fraction * center_to_p_dir.x, p1Y - fraction * center_to_p_dir.y };
	struct point2d p2 = { temp_p.x + 0.05 * (cY - p1Y), temp_p.y + 0.05 * (p1X - cX) };
	struct point2d p3 = { temp_p.x - 0.05 * (cY - p1Y), temp_p.y - 0.05 * (p1X - cX) };
	glColor3f(1, 0, 0);
	glBegin(GL_TRIANGLES);
	{
		glVertex2f(p1X, p1Y);
		glVertex2f(p2.x, p2.y);
		glVertex2f(p3.x, p3.y);
	}
	glEnd();
}

void keyboardListener(unsigned char key, int x, int y) {
	switch (key) {

	case 'g':
		toggleGeometry = 1 - toggleGeometry;
		break;
	case 'u':
		if (isDrawn) {
			updateMode = 1 - updateMode;
			leftClickCount = 0;
			a_clicked = 0;
		}
		
		//moveIdx = 0;
		break;
	case 'a':
		if (isDrawn && !updateMode) {
			a_clicked = 1 - a_clicked;
			moveIdx = 0;
		}
		
		//leftClickCount = 0;
		break;
	default:

		break;
	}
}

void specialKeyListener(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_DOWN:		//down arrow key
		break;
	case GLUT_KEY_UP:		// up arrow key
		break;

	case GLUT_KEY_RIGHT:
		break;
	case GLUT_KEY_LEFT:
		break;

	case GLUT_KEY_PAGE_UP:
		break;
	case GLUT_KEY_PAGE_DOWN:
		break;

	case GLUT_KEY_INSERT:
		break;

	case GLUT_KEY_HOME:
		break;
	case GLUT_KEY_END:
		break;

	default:
		break;
	}
}



void mouseListener(int button, int state, int x, int y) {	//x, y is the x-y of the screen (2D)
	switch (button) {
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN) {		// 2 times?? in ONE click? -- solution is checking DOWN or UP
			//right_click = 0;
			if (!updateMode && !isDrawn) {
				//leftClickCount = 0;
				std::cout << x << " " << 600 - y << std::endl;
				cp[cpidx].x = (double)x;
				cp[cpidx].y = (double)(600 - y);
				if (cpidx % 2) {
					dcp[dcpidx].x = cp[cpidx].x;
					dcp[dcpidx].y = cp[cpidx].y;
					dp[dpidx].x = cp[cpidx].x - cp[cpidx - 1].x;
					dp[dpidx].y = cp[cpidx].y - cp[cpidx - 1].y;
					dpidx++;
					dcpidx++;
					num_points++;
					scp[scpidx] = cp[cpidx - 1];
					cout << "scp " << scp[scpidx].x << " " << scp[scpidx].y << endl;
					scpidx++;
				}
				
				cpidx++;
			}
			else if(updateMode) {
				cout << "UPDATE" << endl;
				//right_click = 0;
				leftClickCount++;
				
				if (leftClickCount == 1) {
					//right_click = 0;
					for (int i = 0; i < cpidx; i++) {
						euclidDist[i] = euclid_dist(x, 600 - y, cp[i].x, cp[i].y);
					}
					min = euclidDist[0];
					location_cp = 0;
					location_on_curve = 0;
					for (int c = 0; c < 200; c++) {
						if (euclidDist[c] < min && euclidDist[c]) {
							min = euclidDist[c];
							location_cp = c;

						}
					}
					cout << "min " << cp[location_cp].x << " " << cp[location_cp].y << endl;
					for (int i = 0; i < 200; i++) {
						if (scp[i].x == cp[location_cp].x && scp[i].y == cp[location_cp].y) {
							iOnCurve = 1;
							iOutCurve = 0;
							location_on_curve = i;
							cout << scp[location_on_curve].x << " " << scp[location_on_curve].y << endl;
							break;
						}
					}
					for (int i = 0; i < 200; i++) {
						if (dcp[i].x == cp[location_cp].x && dcp[i].y == cp[location_cp].y) {
							iOutCurve = 1;
							iOnCurve = 0;
							location_dcp = i;
							break;
						}
					}
					//cout << "to be updated " << cp[location_cp].x << " " << cp[location_cp].y << endl;
					//cout << dcp[location_dcp].x << " " << dcp[location_dcp].y << endl;
				}
				
				else if (leftClickCount == 2) {
					
					if (iOutCurve && !iOnCurve) {
						for (int i = 0; i < 200; i++) {
							if (dp[i].x == (dcp[location_dcp].x - cp[location_cp - 1].x) &&
								dp[i].y == (dcp[location_dcp].y - cp[location_cp - 1].y)) {
								location_in_dp = i;
								break;
							}
						}
						dcp[location_dcp].x = x;
						dcp[location_dcp].y = 600 - y;
						cp[location_cp].x = x;
						cp[location_cp].y = 600 - y;
						dp[location_in_dp].x = cp[location_cp].x - cp[location_cp - 1].x;
						dp[location_in_dp].y = cp[location_cp].y - cp[location_cp - 1].y;

					}
					else if (iOnCurve && !iOutCurve) {
						
						for (int i = 0; i < 200; i++) {
							if (dp[i].x == (cp[location_cp + 1].x - cp[location_cp].x) &&
								dp[i].y == (cp[location_cp + 1].y - cp[location_cp].y)) {
								location_in_dp = i;
								break;
							}
						}
						scp[location_on_curve].x = x;
						scp[location_on_curve].y = 600 - y;
						cp[location_cp].x = x;
						cp[location_cp].y = 600 - y;
						dp[location_in_dp].x = cp[location_cp + 1].x - cp[location_cp].x;
						dp[location_in_dp].y = cp[location_cp + 1].y - cp[location_cp].y;
						cout << "dp " << cp[location_cp + 1].x - cp[location_cp].x << " " << cp[location_cp + 1].y - cp[location_cp].y << endl;

					}
					
					allPointsVector.clear();
					leftClickedTwice = 1;
					leftClickCount = 0;
					updateMode = 0;
				}
			}
		}
		break;

	case GLUT_RIGHT_BUTTON:
		//........
		cout << num_points << endl;
		if (cpidx > 2 && (cpidx+1) % 2) {
			if (state == GLUT_DOWN) {
				right_click = 1;
				a_clicked = 0;
				updateMode = 0;
			}
		}
		break;

	case GLUT_MIDDLE_BUTTON:
		//........
		break;

	default:
		break;
	}
}



void display() {

	//clear the display
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0, 0, 0, 0);	//color black
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/********************
	/ set-up camera here
	********************/
	//load the correct matrix -- MODEL-VIEW matrix
	glMatrixMode(GL_MODELVIEW);

	//initialize the matrix
	glLoadIdentity();

	//now give three info
	//1. where is the camera (viewer)?
	//2. where is the camera looking?
	//3. Which point2d is the camera's UP point2d?

	//gluLookAt(100,100,100,	0,0,0,	0,0,1);
	//gluLookAt(150*cos(cameraAngle), 150*sin(cameraAngle), cameraHeight,		0,0,0,		0,0,1);
	gluLookAt(0, 0, 0, 0, 0, -1, 0, 1, 0);


	//again select MODEL-VIEW
	glMatrixMode(GL_MODELVIEW);


	/****************************
	/ Add your objects from here
	****************************/
	//add objects

	glPushMatrix(); {
		//if (a_clicked) {
		drawMovingCircle(pos.x, pos.y, 6, 20);
		//}
	}
	glPopMatrix();
	
	int i, j;

	if (toggleGeometry) {
		for (i = 0; i < cpidx; i++)
		{
			glPushMatrix();
			{
				if (i % 2) {
					glColor3f(1, 1, 0);
				}
				else {
					glColor3f(0, 1, 0);
				}
				struct point2d p = { cp[i].x, cp[i].y, 0 };
				drawSquare(3, p);
			}
			glPopMatrix();
			if (i % 2) {
				glPushMatrix(); {
					drawTriangle(cp[i].x, cp[i].y, cp[i - 1].x, cp[i - 1].y);
					drawLine(cp[i].x, cp[i].y, cp[i - 1].x, cp[i - 1].y);
				}
				glPopMatrix();
			}

		}
	}
	
	if ((right_click || leftClickedTwice)) {
		
		//moveIdx = 0;
		for (int i = 0; i < num_points; i++) {
			
			if (i == num_points - 1) {
				hermite((double)1 / (double)300, scp[i], scp[0], dp[i], dp[0]);
			}
			else {
				hermite((double)1 / (double)300, scp[i], scp[i + 1], dp[i], dp[i + 1]);
			}
			

			for (int i = 0; i < 300; i++) {
				//drawLine(curve[i].x, curve[i].y, curve[i + 1].x, curve[i + 1].y);
				glColor3f(1, 1, 1);
				glBegin(GL_POINTS);
				glVertex3f(curve[i].x, curve[i].y, curve[i].z);
				glEnd();
			}

		}
		isDrawn = 1;
		//a_clicked = 0;
		
	}
	glPushMatrix();
	{
		glColor3ub(0, 255, 255);
		//drawCircle(dcp[location_dcp].x, dcp[location_dcp].y, 10, 20);
		drawCircle(cp[location_cp].x, cp[location_cp].y, 10, 20);
		//}
		
	}
	glPopMatrix();

	
	//ADD this line in the end --- if you use double buffer (i.e. GL_DOUBLE)
	glutSwapBuffers();
}


void animate() {
	if (a_clicked) {
		vector<struct point2d>::iterator it = allPointsVector.begin();
		pos = *(it + moveIdx);
		moveIdx++;
	}
	
	//}
	
	//codes for any changes in Models, Camera
	glutPostRedisplay();
}

void init() {
	//codes for initialization
	//cout << allPointsVector.size() << endl;
	cpidx = 0;
	dpidx = 0;
	scpidx = 0;
	dcpidx = 0;
	num_points = 0;
	allCurveIdx = 0;
	moveIdx = 0;
	onCurveIdx = 0;
	location_cp = 0;
	iOnCurve = 0;
	iOutCurve = 0;
	location_on_curve = 0;
	//clear the screen
	glClearColor(0, 0, 0, 0);

	/************************
	/ set-up projection here
	************************/
	//load the PROJECTION matrix
	glMatrixMode(GL_PROJECTION);

	//initialize the matrix
	glLoadIdentity();

	//give PERSPECTIVE parameters
	gluOrtho2D(0, 800, 0, 600);
	//gluPerspective(80,	1,	1,	1000.0);
	//field of view in the Y (vertically)
	//aspect ratio that determines the field of view in the X point2d (horizontally)
	//near distance
	//far distance
}

int main(int argc, char **argv) {
	glutInit(&argc, argv);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(0, 0);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);	//Depth, Double buffer, RGB color

	glutCreateWindow("My OpenGL Program");

	init();

	glEnable(GL_DEPTH_TEST);	//enable Depth Testing

	glutDisplayFunc(display);	//display callback function
	glutIdleFunc(animate);		//what you want to do in the idle time (when no drawing is occuring)

	glutKeyboardFunc(keyboardListener);
	glutSpecialFunc(specialKeyListener);
	glutMouseFunc(mouseListener);

	glutMainLoop();		//The main loop of OpenGL

	return 0;
}
