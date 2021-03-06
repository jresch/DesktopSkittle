#ifndef FREQ_DISP
#define FREQ_DISP

//#include <GL/glut.h>
#include <QtOpenGL/QGLWidget>
#include <math.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include <QGLWidget>
#include "BasicTypes.h"
#include "AbstractGraph.h"
#include "NucleotideDisplay.h"
#include "UiVariables.h"

using namespace std;

class RepeatMap : public AbstractGraph 
{
    Q_OBJECT
    
public:	
    RepeatMap(UiVariables* gui, GLWidget* gl);
    ~RepeatMap();
    QScrollArea* settingsUi();
    void display();
    void load_3mer_canvas(vector<float> scores);
    void link(NucleotideDisplay* nuc_display);
    void load_canvas();
    void calculateOutputPixels();
    GLuint render();
    void freq_map();
    vector<float> convolution_3mer();
    int height();
    string SELECT_MouseClick(point2D pt);
    int getRelativeIndexFromMouseClick(point2D pt);

    vector<point> bestMatches();
    void display_freq();
    void calculate(vector<color>& img, int vote_size);
    double correlate(vector<color>& img, int beginA, int beginB, int pixelsPerSample);
    int width();

public slots:
    void changeFStart(int val);
    void changeGraphWidth(int val);
    void toggle3merGraph(bool m);

signals:
    void fStartChanged(int);
    void graphWidthChanged(int);

protected:
    TextureCanvas* canvas_3mer;
    NucleotideDisplay* nuc;
    GLuint display_object;
    vector< vector<float> > freq;
    int barWidth;
    int spacerWidth;
    int F_width;
    int F_start;
    int F_height;

    int freq_map_count;
    int calculate_count;
    bool using3merGraph;
};

#endif
