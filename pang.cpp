#include <stdio.h>
#include <stdlib.h>

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#  include <OpenGL/glu.h>
#  include <GLUT/glut.h>
#else
#  include <GL/gl.h>
#  include <GL/glu.h>
#  include <GL/glut.h>
#  include <GL/freeglut.h>
#endif
#include <cmath>
#include<vector>
#include<string>


#define CIRCLE_RADIUS 0.15
#define PI 3.14159265f
//image loading code from
// https://stackoverflow.com/questions/8249282/set-background-image-of-an-opengl-window


// got rid of y to reset in keyboard

using namespace std;

//default screen height and width
int w = 500;
int h = 500;

int w1 = 0;
int h1 = 0;

int w1hb = 251; //top left x of character hitbox
int h1hb = 91;  //top left y of character hitbox
int w2hb = 200; //bottom left x of character hitbox
int h2hb = 40;  //bottom left y of character hitbox

float Health = 100.0; //the max hp of Emma
float xPosGun = 0; //the position of the character
float yPosGun = 0;
float xVal;
float initialY = 0.0; // default height for start of gun animation
GLuint *textures = new GLuint[2]; // for loading textures

bool shooting = false;
bool complete = false;
bool emptyList = false;

// default gun position values
float gunLeft = 2000.0;
float gunRight = -2000.0;
float gunTop = -400000.0;
bool shoot;
int hY;

//BUBBLE VARS
GLdouble xLeft, xRight, yBottom, yTop;
GLfloat xPosMax, xPosMin, yPosMax, yPosMin;
GLfloat xSpeed = 0.02f;
GLfloat ySpeed = 0.007f;

// Score and clock variables
int score = 0;
int timeLeft = 60;
// used to see how many times timer fcn has been called so i know when to change the timeLeft variable
int iterations = 0;

int level = 1;//the current level
bool timeBonus = false;//determine if the time bonus will be updated
int tfin = 0;//this value stores the time bonus so it does not change
int totalScore = 0; //the total score that is calculated when the level ends
bool GameOver = false; //the idea is to use this to determine what happens in the result screen.
//for example, we could use this value to determine wheter to draw a 'retry' button or to make a 'next level' button.s

void orthogonalStart()
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(-w1 / 2, w1 / 2, -h1 / 2, h1 / 2);
    glMatrixMode(GL_MODELVIEW);
}

void orthogonalEnd()
{
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

//Bubble class that creates bubbles and gets/sets values
class Bubble
{
  public:
    GLfloat ballRadius;
    GLfloat xPos;
    GLfloat yPos;
    GLfloat xMvm; //the direction and
    GLfloat yMvm; //the movement of this particular bubble
    GLfloat red;
    GLfloat green;
    GLfloat blue;
    GLfloat age;
    bool left;

    Bubble(float x, float y, float xm, float ym, float r, float g, float b, float size, bool l)
    {
        glPointSize(1.0);
        xPos = x;
        yPos = y;
        xMvm = xm;
        yMvm = ym;
        red = r;
        green = g;
        blue = b;
        ballRadius = size;
        age = 0.0;
        left = l;
    }

    //get red value
    GLfloat getRed()
    {
        return red;
    }
    //get blue value
    GLfloat getBlue()
    {
        return blue;
    }

    //get green value
    GLfloat getGreen()
    {
        return green;
    }
    //get x position
    GLfloat getXPos()
    {
        return xPos;
    }
    //get y position
    GLfloat getYPos()
    {
        return yPos;
    }
    //set x position
    void setXPos(GLfloat num)
    {
        xPos = num;
    }
    //set y position
    void setYPos(GLfloat num)
    {
        yPos = num;
    }
    //get ball radius
    GLfloat getRad()
    {
        return ballRadius;
    }
    //get age value
    GLfloat getAge()
    {
        return age;
    }
    //set age value
    void setAge(GLfloat num)
    {
        age = num;
    }
    //get y movement speed
    GLfloat getyMvm()
    {
        return yMvm;
    }
    //set y movement speed
    void setyMvm(GLfloat num)
    {
        yMvm = num;
    }
    //get x movement speed
    GLfloat getxMvm()
    {
        return xMvm;
    }
    //set x movement speed
    void setxMvm(GLfloat num)
    {
        xMvm = num;
    }
    //reshape to allow for balls to bounce of walls given their radius
    void reshape()
    {
        xPosMin = xLeft + ballRadius;
        xPosMax = xRight - ballRadius;
        yPosMin = yBottom + ballRadius;
        yPosMax = yTop - ballRadius;
    }

    //drawing method
    void draw()
    {
        glBegin(GL_TRIANGLE_FAN);
        glColor3f(0.0f, 0.0f, 1.0f);
        int numSegments = 100;
        GLfloat angle;
        for (int i = 0; i <= numSegments; i++)
        {
            angle = i * 2.0f * PI / numSegments;
            glVertex2f(cos(angle) * ballRadius, sin(angle) * ballRadius);
        }
        glEnd();
    }
    //return if initially going left or not
    //This function works to decide which bubble goes left after two generated after a split
    bool goLeft()
    {
        return left;
    }

    //returns min x and y positions
    vector<float> getMinPos()
    {        
            vector <float> minVec;
            minVec.push_back((xPos - ballRadius)*225);
            minVec.push_back((yPos - ballRadius)*225);
        return minVec;
    }

    //returns max x and y positions
    vector<float> getMaxPos()
    {
        vector <float> maxVec;
        maxVec.push_back((xPos + ballRadius)*225);
        maxVec.push_back((yPos + ballRadius)*225);
        return maxVec;
    }
};

vector<Bubble> bubbleList;


//used to draw text on the window
void drawText(const char *text, int length, float x, float y){
    glMatrixMode(GL_PROJECTION);
    double *matrix = new double[16];
    glGetDoublev(GL_PROJECTION_MATRIX,matrix);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glPushMatrix();
        glLoadIdentity();
        glRasterPos2f(x,y); //specifies raster position for pixel operations
        for(int i = 0;i<length;i++) //goes through the length of string and writes the according character using bitmap
        {
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,(int)text[i]);
        }
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixd(matrix);
    glMatrixMode(GL_MODELVIEW);
}


void init(void)
{
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glOrtho(0.0, w, 0.0, h, 1.0, -1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

//resets all values back to default
void restart(){ 
    w1hb = 251; //top left x of character hitbox
    h1hb = 91;  //top left y of character hitbox
    w2hb = 200; //bottom left x of character hitbox
    h2hb = 40;  //bottom left y of character hitbox

    Health = 100.0; //the max hp of Emma
    xPosGun = 0; //the position of the character
    yPosGun = 0;
    initialY = 0.0;
    GLuint *textures = new GLuint[2];

    shooting = false;
    complete = false;
    emptyList = false;
    GameOver = false;

    timeLeft = 60;
    score = 0;

    gunLeft = 2000.0;
    gunRight = -2000.0;
    gunTop = -400000.0;
    // clearns bubble vector before pushing back a number of bubbles where the number of bubbles 
    // depends on the level, the higher the level, the more bubbles
    if (level == 1){
        bubbleList.clear(); //empties bubble
        bubbleList.push_back(Bubble(0, 0, 0.007, 0.01, 1, 0, 0, 0.2, false));}
    else if (level == 2){
        bubbleList.clear();
        bubbleList.push_back(Bubble(-0.3, 0, -0.007, 0.01, 1, 0, 0, 0.2, false));
        bubbleList.push_back(Bubble(0.3, 0, 0.007, 0.01, 1, 0, 0, 0.2, false));
    }
    else if (level == 3){
        bubbleList.clear();
        bubbleList.push_back(Bubble(-0.4, 0.2, -0.007, 0.011, 1, 0, 0, 0.1, false));
        bubbleList.push_back(Bubble(0.4, 0.2, 0.007, 0.011, 1, 0, 0, 0.1, false));
        bubbleList.push_back(Bubble(-0.7, 0.4, -0.007, 0.011, 1, 0, 0, 0.2, false));
        bubbleList.push_back(Bubble(0.7, 0.4, 0.007, 0.011, 1, 0, 0, 0.2, false));
    }
    else if (level == 4){
        bubbleList.clear();
        bubbleList.push_back(Bubble(-0.7, -0.1, 0.008, -0.012, 1, 0, 0, 0.05, false));
        bubbleList.push_back(Bubble(0.7, -0.1, -0.008, -0.012, 1, 0, 0, 0.05, false));
        bubbleList.push_back(Bubble(0.8, 0.8, 0.008, 0.012, 1, 0, 0, 0.05, false));
        bubbleList.push_back(Bubble(-0.8, 0.8, -0.008, 0.012, 1, 0, 0, 0.05, false));
        bubbleList.push_back(Bubble(-0.4, 0.1, 0.0075, 0.012, 1, 0, 0, 0.1, false));
        bubbleList.push_back(Bubble(0.4, 0.1, -0.0075, 0.012, 1, 0, 0, 0.1, false));
        bubbleList.push_back(Bubble(-0.4, 0.5, -0.007, 0.012, 1, 0, 0, 0.2, false));
        bubbleList.push_back(Bubble(0.4, 0.5, 0.007, 0.012, 1, 0, 0, 0.2, false));
    }
    else if (level == 5){
        bubbleList.clear();
        bubbleList.push_back(Bubble(-0.4, -0.3, 0.008, -0.012, 1, 0, 0, 0.05, false));
        bubbleList.push_back(Bubble(0.4, -0.3, -0.008, -0.012, 1, 0, 0, 0.05, false));
        bubbleList.push_back(Bubble(0.6, 0.5, 0.008, -0.012, 1, 0, 0, 0.05, false));
        bubbleList.push_back(Bubble(-0.6, 0.5, -0.008, -0.012, 1, 0, 0, 0.05, false));
        bubbleList.push_back(Bubble(-0.7, 0.1, 0.008, -0.012, 1, 0, 0, 0.1, false));
        bubbleList.push_back(Bubble(0.7, 0.1, -0.008, -0.012, 1, 0, 0, 0.1, false));
        bubbleList.push_back(Bubble(-0.7, -0.1, -0.008, -0.012, 1, 0, 0, 0.1, false));
        bubbleList.push_back(Bubble(0.7, -0.1, -0.008, -0.012, 1, 0, 0, 0.1, false));
        bubbleList.push_back(Bubble(-0.3, 0.6, -0.008, 0.012, 1, 0, 0, 0.2, false));
        bubbleList.push_back(Bubble(0.3, 0.6, 0.008, 0.012, 1, 0, 0, 0.2, false));
    }
    glutPostRedisplay();
    
}


void initGL()
{
    glClearColor(0.0, 0.0, 0.0, 1.0);
}

//split method to create balls after a hit
void Split(int bub)
{
    for (int i = 0; i < bubbleList.size(); i++)
    {
        //if i (the ball hit) is same as ball we find iterating over vector
        if (i == bub)
        {
            //if bubbles not at smallest size, create two more after hit
            if (bubbleList[i].getRad() > .05f)
            {
                GLfloat newX = bubbleList[i].getXPos();
                GLfloat newY = bubbleList[i].getYPos();
                GLfloat newr = bubbleList[i].getRed();
                GLfloat newg = bubbleList[i].getGreen();
                GLfloat newb = bubbleList[i].getBlue();
                GLfloat newsize;
                if (bubbleList[i].getRad() == 0.1f)
                {
                    score += 200;
                    newsize = 0.05;
                    //delete old bubble and creat two more and push onto vector
                    bubbleList.erase(bubbleList.begin() + i);
                    bubbleList.push_back(Bubble(newX+0.01, newY, 0.007, 0.01, newr, newg, newb, newsize, true));
                    bubbleList.push_back(Bubble(newX-0.01, newY, -0.007, 0.01, newr, newg, newb, newsize, false));
                    break;
                }
                else if (bubbleList[i].getRad() == 0.2f)
                {
                    score += 100;
                    newsize = 0.1;
                    //delete old bubble and create two more and push onto vector
                    bubbleList.erase(bubbleList.begin() + i);
                    bubbleList.push_back(Bubble(newX+0.1, newY, 0.007, 0.01, newr, newg, newb, newsize, true));
                    bubbleList.push_back(Bubble(newX-0.1, newY, -0.007, 0.01, newr, newg, newb, newsize, false));
                    break;
                }
            }
            //if bubble at smallest size, disappear
            else
            {
                score += 500;
                bubbleList.erase(bubbleList.begin() + i);
                if(bubbleList.size()==0){
                    emptyList = true;
                    timeBonus = true;
                }
            }
        }
    }
}


void mouse(int btn, int state, int x, int y)
{
    if (btn == GLUT_LEFT_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            // incrementing level after finishing a level
            // next button seen below results 
            if (215 <= x && x <= 285 && 260 <= y && y <= 290 && emptyList ){
                if (level != 5){
                    level += 1;
                }
                else{
                    level = 1;
                }
                restart();
            }

            if (215 <= x && x <= 285 && 300 <= y && y <= 330 && emptyList ){
                restart();
            }

            if (level == 1 && emptyList){
                //yeet
            }
        }
    }
}

// creates background
void background()
{
    glBindTexture(GL_TEXTURE_2D, textures[0]);

    orthogonalStart();

    // texture width/height
    const int iw = 500;
    const int ih = 500;

    glPushMatrix();
    glTranslatef(-iw / 2, -ih / 2, 0);
    glBegin(GL_QUADS);
        glTexCoord2i(0, 0);glVertex2i(0, 0);
        glTexCoord2i(1, 0);glVertex2i(iw, 0);
        glTexCoord2i(1, 1);glVertex2i(iw, ih);
        glTexCoord2i(0, 1);glVertex2i(0, ih);
    glEnd();
    glPopMatrix();

    orthogonalEnd();
}


void character(int w, int h)
{
    glBindTexture(GL_TEXTURE_2D, textures[1]);

    orthogonalStart();

    // texture width/height
    const int iw = 51;
    const int ih = 51;
    glPushMatrix();
    glBegin(GL_QUADS);
        glTexCoord2i(0, 0);glVertex2i(w, h);
        glTexCoord2i(1, 0);glVertex2i(w + iw, h);
        glTexCoord2i(1, 1);glVertex2i(w + iw, h + ih);
        glTexCoord2i(0, 1);glVertex2i(w, h + ih);
    glEnd();
    glPopMatrix();

    orthogonalEnd();
}

// used to display everything visible on window
void display(void)
{
    hY = glutGet(GLUT_WINDOW_HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glColor3f(1, 1, 1);
    background();
    // making sure that game is suppose to continue running
    if(timeLeft>0 && !emptyList)
    {
        // in case health reaches 0
        if (Health <= 0.0)
        {
            GameOver = true;
            bubbleList.clear();
            emptyList = true;
        }
        glPushMatrix();
        glTranslatef(0, 0, 1);
        if (shooting == true) //if spacebar has been pressed and gun is suppose to be shooting
        {
            //Drawing the gun
            orthogonalStart();
            glColor3f(0, 1, 0);
            glBegin(GL_POLYGON);
                glVertex2f(-2.0f + xVal, -158.0f + yPosGun);
                    glVertex2f(-2.0f + xVal, -158.0f);
                    glVertex2f(2.0f + xVal, -158.0f);
                    glVertex2f(2.0f + xVal, -158.0f + yPosGun);
                glEnd();
                glColor3f(1, 0, 0);
                glBegin(GL_POLYGON);
                    glVertex2f(-3.0f + xVal, -162.0f + yPosGun);
                    glVertex2f(xVal, -154.0f + yPosGun);
                    glVertex2f(3.0f + xVal, -162.0f + yPosGun);
                glEnd();
                orthogonalEnd();

                gunTop = yPosGun;
                yPosGun += 5; // moving the upper part of the harpoon up
                complete = true;
                if (yPosGun >= hY - 90) //if harpoon hits the top edge of window
                { 
                    shooting = false; //dont show gun
                    complete = false; 
                    xVal = -300; //set xVal (gun's current xCoord) to off the screen so ball's
                            // can't be broken by it once animation is done.
                    gunLeft = 30000;
                    gunRight = -30000;
                    gunTop = -500000;
                    yPosGun = initialY;
                }
            }
        glPopMatrix();

        glColor3f(1, 1, 1);
        glPushMatrix();
            glTranslatef(-50 + xPosGun, -210, 0);
            //if (Health > 0.0){
            character(0, 0);//}
        glPopMatrix();
        glDisable(GL_TEXTURE_2D);
        glPushMatrix();
            glTranslatef(0, 0, 1);
        glPopMatrix();
        

        glPushMatrix(); //loop through all bubbles and draw them
        for(int i=0; i<bubbleList.size(); i++) 
        {
            glPushMatrix(); 
                glColor3f(bubbleList[i].getRed(),bubbleList[i].getGreen(),bubbleList[i].getBlue());
                glTranslatef(bubbleList[i].getXPos(), bubbleList[i].getYPos(), 1);
            
                bubbleList[i].draw(); 

                bubbleList[i].setXPos(bubbleList[i].getXPos()+bubbleList[i].getxMvm());
                bubbleList[i].setYPos(bubbleList[i].getYPos()+bubbleList[i].getyMvm() );
                //if bubble hits the right edge of the window, bounce off
                if (bubbleList[i].getXPos() > xPosMax-bubbleList[i].getRad()+0.05/*1 * w1/h1*/ ) 
                {
                    bubbleList[i].setXPos(xPosMax-bubbleList[i].getRad()+0.05/*1*w1/h1*/);
                    //xSpeed = -xSpeed;
                    bubbleList[i].setxMvm(-1*bubbleList[i].getxMvm());
                } 
                //if bubble hits the left edge of the window, bounce off
                else if (bubbleList[i].getXPos() < xPosMin+bubbleList[i].getRad()-0.05/*-1 * w1/h1*/) 
                {//xPosMin
                    bubbleList[i].setXPos(xPosMin+bubbleList[i].getRad()-0.05/*-1 * w1/h1*/);//xPosMin
                    //xSpeed = -xSpeed;
                    bubbleList[i].setxMvm(-1*bubbleList[i].getxMvm());
                }

                //if bubble hits the top edge of the window, bounce off
                if (bubbleList[i].getYPos() > yPosMax-bubbleList[i].getRad()+0.05/*1 * h1/w1*/) 
                {
                    bubbleList[i].setYPos(yPosMax-bubbleList[i].getRad()+0.05/*1 * h1/w1*/);
                    //ySpeed = -ySpeed;
                    bubbleList[i].setyMvm(-1*bubbleList[i].getyMvm());
                } 

                //if bubble hits the bottom edge of the window, bounce off
                else if (bubbleList[i].getYPos() < yPosMin+bubbleList[i].getRad()+0.1/*-1 * h1/w1*/) 
                {
                    bubbleList[i].setYPos(yPosMin+bubbleList[i].getRad()+0.1/*-1 * h1/w1*/);
                    //ySpeed = -ySpeed;
                    bubbleList[i].setyMvm(-1*bubbleList[i].getyMvm());
                }
                    //hit detection for bubbles and the bullet
                    if((bubbleList[i].getMinPos().at(0) <= gunRight && bubbleList[i].getMaxPos().at(0) >= gunLeft
                        && bubbleList[i].getAge() > 17.0  && bubbleList[i].getMinPos().at(1) <= (-158.0f + yPosGun))) 
                    {
                        Split(i);
                    }	    
                bubbleList[i].setAge(bubbleList[i].getAge() + 1.0);
        
                //hit detection for bubble and character
                if( bubbleList[i].getMinPos().at(0) <= 1 + xPosGun && bubbleList[i].getMaxPos().at(0) >= -50 + xPosGun
                        && bubbleList[i].getMaxPos().at(1) >= h2hb -210 && bubbleList[i].getMinPos().at(1) <= h1hb-210)
                {
                    Health -= (3*bubbleList[i].getRad());
                }
            glPopMatrix(); 
        }
        glPopMatrix();

        // outputting score, time and health at the bottom of the window
        string points;
        points = "  Score ";
        //points += std::to_string(score);
        string scors = std::to_string(score);
        if (scors.length() == 3) 
        {
            points += "0";
        }
        if (scors.length() == 2) 
        {
            points += "00";
        }
        if (scors.length() == 1) 
        {
            points += "000";
        }

        points += std::to_string(score);
        glColor3f(1,0,0);
        //drawText(points.data(),points.size(),-1,-1);

        string timer;
        timer = "Time ";
        timer += std::to_string(timeLeft);
        //drawText(timer.data(), timer.size(),1,-1);
        string healthtxt;
        healthtxt = "Health ";
        healthtxt += std::to_string((int)Health);
        //drawText(healthtxt.data(),healthtxt.size(),0,-1);
        string text;
        text += points;
        text += "               ";
        text += healthtxt;
        text += "               ";
        text+= timer;
        drawText(text.data(),text.size(),-1,-1);
    } 
    else //the below code is for scenario that the player either dies, runs out of time or completes the level
    {
        glDisable(GL_TEXTURE_2D);
        glPushMatrix();
            glTranslatef(0, 0, 1);
        glPopMatrix();
        if (Health <= 0.0) 
        {
            Health = -30; //setting negative score if player loses all their health
        }
        else if (timeLeft == 0 && timeBonus) 
        {
            tfin = -40; //setting negative score if player runs out of time
            //timeBonus = false;
        }
        orthogonalStart();
        glColor3f(0,0,0);
        //drawing box which results will be contained in
        glBegin(GL_POLYGON);
            glVertex2f(-120,150);
            glVertex2f(-120,0);
            glVertex2f(100,0);
            glVertex2f(100,150);
        glEnd();
        glColor3f(1,0,0);

        //outputting results
        string cleare = "Level " + std::to_string(level) + " ";
        if (GameOver)
        {
            cleare += "Failed!";
        }
        else
        {
            cleare += "Complete!";
        }
        drawText(cleare.data(),cleare.size(),-0.4,0.5);
        string points;
        points = "Level Score: ";
        string scors = std::to_string(score);
        points += std::to_string(score);
        glColor3f(1,0,0);
        drawText(points.data(),points.size(),-0.4,0.4);
        string timer;
        timer = "Time Bonus: ";
        if (timeBonus)
        {
            tfin = timeLeft; timeBonus = false;}
            string scort = std::to_string(tfin*100);
            timer += scort;
            drawText(timer.data(), timer.size(),-0.4,0.3);
            string healthtxt;
            healthtxt = "Health Bonus: ";
            string scorh = std::to_string((int)Health*10);
            healthtxt += scorh;
            drawText(healthtxt.data(),healthtxt.size(),-0.4,0.2);
            string text;
            totalScore = score + ((int)Health*10) + ((int)tfin*100);
            text += "Total Score: ";
            text += std::to_string(totalScore);
            drawText(text.data(),text.size(),-0.4,0.1);

            // Creating buttons to restart or increase difficulty
           if (!GameOver){
                //creating next button that would increase difficulty
                glColor3f(0, 0, 0);
                glBegin(GL_POLYGON);
                glVertex2f(-35, -40);
                glVertex2f(-35, -10);
                glVertex2f(35, -10);
                glVertex2f(35, -40);
                glEnd();
                glColor3f(1, 0, 0);

                string texw = "next";
                drawText(texw.data(), texw.size(), -0.1, -0.125);glColor3f(0, 0, 0);
                
                //creating restart button that would restart game at same difficulty
                glBegin(GL_POLYGON);
                glVertex2f(-35, -80);
                glVertex2f(-35, -50);
                glVertex2f(35, -50);
                glVertex2f(35, -80);
                glEnd();
                glColor3f(1, 0, 0);

                string texr = "restart";
                drawText(texr.data(), texr.size(), -0.11, -0.3);
            }

            orthogonalEnd();
        }
    glutSwapBuffers();
}

// keyboard inputs for many functions
void keyboard(unsigned char key, int xIn, int yIn)
{
    switch (key)
    {
    case 'q':
    case 27: //27 is the esc key
        exit(0);
        break;

    case 'a':         // move rectangle left
        if (w2hb > 0) // checks if character hitbox is hitting left border or not
        {
            w1hb -= 5;
            w2hb -= 5;
            xPosGun -= 5;
            break;
        }
        else
        {
        }; // i.e do nothing
        break;

    case 'd':         // move rectangle right
        if (w1hb < w) // checks if character hitbox is hitting right border or not
        {
            w1hb += 5;
            w2hb += 5;
            xPosGun += 5;
            break;
        }
        else
        {
        }; // do nothing
        break;

    case 'r': // reset game at current level
        restart();
        break;

    case ' ': //shoots gun
        if (shooting == false)
        {
            shooting = true;
            xVal = xPosGun-25; //+ 225;
            gunLeft = xVal - 2.0f;
            gunRight = xVal + 2.0f;

        }
        break;
    }
}

GLuint LoadTexture(const char *filename, int width, int height, GLuint texture)
{
    GLuint *textures = new GLuint[2];
    unsigned char *data;
    FILE *file;

    //reads RAW file
    file = fopen(filename, "rb");
    if (file == NULL)
        return 0;
    data = (unsigned char *)malloc(width * height * 3);
    fread(data, width * height * 3, 1, file);
    fclose(file);

    glGenTextures(2, textures);
    glBindTexture(GL_TEXTURE_2D, texture);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    //even better quality, but this will do for now.
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    //to the edge of our shape.
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    //Generate the texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    free(data);     //free the texture
    return texture; //return whether it was successful
}

void reshape(int w, int h)
{
    w1 = w;
    h1 = h;
    GLfloat height = h;
    GLfloat width = w;
    if (height == 0) height = 1;
    float aspect = (float)width / height;

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (width <= height)
    {
        xLeft = -1.0;
        xRight = 1.0;
        yBottom = -1.0 / aspect;
        yTop = 1.0 / aspect;
    }
    else
    {
        xLeft = -1.0 * aspect;
        xRight = 1.0 * aspect;
        yBottom = -1.0;
        yTop = 1.0;
    }
    gluOrtho2D(xLeft, xRight, yBottom, yTop);
    for (int i = 0; i < bubbleList.size(); i++)
    {
        xPosMin = xLeft + 0.05f;
        xPosMax = xRight - 0.05f;
        yPosMin = yBottom + 0.05f;
        yPosMax = yTop - 0.05f;
    }
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}


void FPS(int val)
{
    iterations++;
    if(iterations % 60 == 0 && timeLeft > 0)
        timeLeft--;
    glutPostRedisplay();
    glutTimerFunc(17, FPS, 0); // 1sec = 1000, 60fps = 1000/60 = ~17
}

// Setting up what occurs when an item from menu has been selected
void menuProc(int value){
// values: 1-5 are to change level of the game while value 6 is to exit the game
	if (value == 1) {
		level = 1;
        restart();
	} else if (value == 2) {
		level = 2;
        restart();
	} else if (value == 3) {
		level = 3;
        restart();
	} else if (value == 4) {
		level = 4;
        restart();
    } else if (value == 5) {
		level = 5;
        restart(); 
    } else if (value == 6){
        exit(0);
    }
}

void instructions(void){
    printf("\n");
    printf("           OBJECTIVES    \n");
    printf("______________________________________________________\n");
    printf("Shoot the harpoon at balls to split them; each ball   \n");
    printf("splits into two smaller balls. Once the balls reach   \n");
    printf("their smallest size, they break!                      \n");
    printf("\n");
    printf("Avoid getting hit by balls as if they deplete your    \n");
    printf("healthbar it's game over. If you break all the bubbles\n");
    printf("in a level, you may move onto the next level.         \n");
    printf("______________________________________________________\n");
    printf("           CONTROLS    \n");
    printf("______________________________________________________\n");
    printf("a           : Move left                          \n");
    printf("d           : Move right                         \n");
    printf("______________________________________________________\n");
    printf("spacebar    : Shoots the gun                     \n");
    printf("r           : Restart the Game                   \n");
    printf("______________________________________________________\n");
    printf("Right Click : Select level of difficulty         \n");
    printf("q or esc    : Quit Program                       \n");
    printf("______________________________________________________\n");
}


// Creating menu which works by clicking the right click button
void createOurMenu(){
    // Submenus for changing the difficulty
	int subMenu_id = glutCreateMenu(menuProc);
    glutAddMenuEntry("1", 1);
	glutAddMenuEntry("2", 2);
	glutAddMenuEntry("3", 3);
	glutAddMenuEntry("4", 4);
	glutAddMenuEntry("5", 5);


    // Creating the main entries of the menu
	int main_id = glutCreateMenu(menuProc);
	glutAddSubMenu("Bubbles", subMenu_id);
	glutAddMenuEntry("Quit", 6);
	

	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void callBackInit()
{
    instructions();
    bubbleList.push_back(Bubble(0, 0, 0.007, 0.01, 1, 0, 0, 0.2, false));
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(0, FPS, 0);
    glutMouseFunc(mouse);
    createOurMenu();
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv); // starts GLUT
    
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(w, h);
    glutInitWindowPosition(300, 100);
    glutCreateWindow("Pang Game"); //creates the window

    callBackInit();
    LoadTexture("background.raw", 500, 500, textures[0]);
    LoadTexture("emma.raw", 51, 51, textures[1]);
    init();

    glutMainLoop();

    return (0);
}