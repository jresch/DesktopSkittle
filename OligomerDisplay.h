#ifndef OLIG_DISP
#define OLIG_DISP

//#include <GL/glut.h>
#include <QtOpenGL/QGLWidget>
#include <math.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include <QGLWidget>
#include "BasicTypes.h"
#include "RepeatMap.h"
#include "UiVariables.h"
#include "AbstractGraph.h"

class QSpinBox;

using namespace std;

class OligomerDisplay : public AbstractGraph//: public RepeatMap//
{
    Q_OBJECT
    
public:
    OligomerDisplay(UiVariables* gui, GLWidget* gl);
    ~OligomerDisplay();
    QScrollArea* settingsUi();
    void checkVariables();
    void calculateOutputPixels();
    void display();
    //vector<color> calculateAverageSignature(int, int);
    //void isochores();
    void calculateHeatMap();
    vector<double> fillHalfMatrix(vector<double>& data);
    vector<double> rotateSquareMatrix(vector<double>& data);
    void selfCorrelationMap();
    void superCorrelationMap();
    vector<double> copyVectorRange(vector<double>& stuff, int index, int length);
    vector<color> colorNormalized(vector<double> heatData);
    color redBlueSpectrum(double i);
    void load_canvas();
    GLuint render();
    void freq_map();
    int oligToNumber(string a);
    string numberToOlig(int);
    int height();
    string SELECT_MouseClick(point2D pt);

    void display_freq();
    void calculate(vector<color>& img, int vote_size);
    double correlate(vector<double>& apples, vector<double>& oranges);
    void assignRanks(vector<point>& temp);
    double spearmanCorrelation(vector<double>& apples, vector<double>& oranges);
    int countsGraphWidth();
    int heatMapGraphWidth();
    int width();


public slots:	
    void changeMinDelta(double mD);
    void changeWordLength(int);
    void graphOneDisplay(int);
    void graphTwoDisplay(int);
    void graphThreeDisplay(int);

signals:
    void wordLengthChanged(int);

private:
    TextureCanvas* graphBuffer;
    TextureCanvas* avgBuffer;
    TextureCanvas* heatMapBuffer;
    TextureCanvas* correlationBuffer;
    TextureCanvas* superBuffer;
    vector< vector<double> > freq;
    vector<color> pixels;
    vector<int> boundaryIndices;
    vector<double> scores;
    vector<double> correlationScores;
    int wordLength;
    int widthMultiplier;
    int similarityGraphWidth;
    double minDeltaBoundary;
    QSpinBox* oligDial;
    double min_score;
    double max_score;
    double range;
    int F_width;
    int F_height;
    bool graphOneOn;
    bool graphTwoOn;
    bool graphThreeOn;
    QCheckBox *graphOne;
    QCheckBox *graphTwo;
    QCheckBox *graphThree;

};

#endif
