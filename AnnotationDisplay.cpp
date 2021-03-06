#include "AnnotationDisplay.h"
#include "glwidget.h"
#include "SkittleUtil.h"
#include <sstream>
#include <algorithm>

/** **********************
  This class is a Graph class that is designed to visualize annotation files.
  This is the most unusual of the graphs except for Cylinder.  It makes a slender bar
  on the far right of the screen with colored bars that correspond to each of the entries
  in the annotation.

  Annotations are marked by a beginning an end, and a text string describing what the element is.
  AnnotationDisplay uses a helper class called track_entry that stores this information.
  The beginning and end coordinates are matched up with the nucleotide index in all other views.
  A main issue that the AnnotationDisplay needs to overcome is that annotations can overlap.
  In cases of overlap, it adds another parallel lane and places additional annotation side by side.
  The logic for parallel lanes is in displayTrack().
  **********************/
AnnotationDisplay::AnnotationDisplay(UiVariables* gui, GLWidget* gl, string gtfFileName)
    :AbstractGraph(gui, gl)
{
    max_width = 1;
    hidden = false;

    actionLabel = trimPathFromFilename(gtfFileName).substr(0,20);
    actionTooltip = string("Genome annotation locations");
    actionData = actionLabel;

    fileName = gtfFileName;
}

AnnotationDisplay::~AnnotationDisplay()
{
    glDeleteLists(display_object, 1);
}

bool trackCompare(const track_entry& a, const track_entry& b)
{
    return a.start < b.start;
}

void AnnotationDisplay::newTrack(vector<track_entry> track)
{
    gtfTrack.clear();
    gtfTrack = vector<track_entry>(track);
    sort(gtfTrack.begin(), gtfTrack.end(), trackCompare);
    hidden = false;
    emit displayChanged();
}

void AnnotationDisplay::calculateOutputPixels()
{
    glDeleteLists(display_object, 1);
    display_object = render();
}

void AnnotationDisplay::display()
{
    if( !upToDate )
    {
        calculateOutputPixels();
    }
    glCallList(display_object);
}

GLuint AnnotationDisplay::render()
{
    qDebug() << "AnnotationDisplay::render(): " << ++frameCount;
    GLuint Annotation_Display_list = glGenLists(1);
    glNewList(Annotation_Display_list, GL_COMPILE);
    glPushMatrix();

    vector<vector<track_entry> > layout = calculateTrackLayout( gtfTrack );
    displayLayout(layout);

    glPopMatrix();
    glEndList();
    upToDate = true;
    return Annotation_Display_list;
}

void AnnotationDisplay::displayLayout(vector<vector<track_entry> > layout)
{
    vector<color> pixels;
    for(int y = 0; y < (int)layout.size(); ++y)
    {
        vector<track_entry>& lineEntries = layout[y];
        for(int x = 0; x < max_width / 2; ++x)
        {
            color c = color(200,200,200);
            if(x < (int)lineEntries.size())
            {
                if(lineEntries[x].isBlank())
                    c = color(200,200,200);
                else
                    c = lineEntries[x].col;
            }
            pixels.push_back(c);
            pixels.push_back(c);
        }
    }
    if(textureBuffer != NULL)
        delete textureBuffer;
    textureBuffer = new TextureCanvas(pixels, max_width);
    glPushMatrix();
    glScaled(1,-1,1);
    textureBuffer->display();
    glPopMatrix();
}

vector< vector<track_entry> > AnnotationDisplay::calculateTrackLayout(const vector<track_entry>& annotationFile)
{
    max_width = 1;
    int line_start = ui->getStart(glWidget);
    int width = ui->getWidth();
    int line_stop = line_start + width;
    int temp_display_size = current_display_size();
    int nextInactiveAnnotation = 0;

    if(annotationFile.empty())
        return vector< vector<track_entry> >();
    /** activeEntries is the list annotations that are on the current display line
        this is a dynamically changing group with annotations coming in and out based on
        their start and stop positions.*/
    vector<vector<track_entry> > layout;
    vector<track_entry> activeEntries = vector<track_entry>();
    track_entry blank = track_entry(0,0, color(0,0,0));
    activeEntries.push_back(blank);

    for(int row = 0; row < temp_display_size / width; row++)//for each line on the screen
    {
        //check to see if any of the tracks already in activeEntries stop on this line
        for(int k = 0; k < (int)activeEntries.size(); ++k)
        {
            if( !activeEntries[k].isBlank() )
            {
                if( activeEntries[k].stop < line_start )
                {
                    if( activeEntries[k].col == color(200,200,200) )
                        activeEntries[k] = blank;//remove the entry
                    else
                        activeEntries[k].col = color(200,200,200);
                }
            }
        }
        //check to match start for a new track
        while(nextInactiveAnnotation < (int)annotationFile.size() && annotationFile[nextInactiveAnnotation].stop < line_start)//assumes tracks are in order
            nextInactiveAnnotation++;
        //keep adding annotations that start on this line
        while(nextInactiveAnnotation < (int)annotationFile.size()
              && rangeOverlap(annotationFile[nextInactiveAnnotation].start, annotationFile[nextInactiveAnnotation].stop, line_start, line_stop))
        {
            stackEntry(activeEntries, annotationFile[nextInactiveAnnotation++]);		//place new tracks in proper position
        }

        line_start += width;
        line_stop += width;
        if( (int)activeEntries.size()*2 > max_width)
            max_width = activeEntries.size()*2;
        layout.push_back(activeEntries);
    }
    return layout;
}

void AnnotationDisplay::stackEntry(vector<track_entry>& activeEntries, track_entry item)
{
    for(int k = 0; k < (int)activeEntries.size(); ++k)
    {
        if( activeEntries[k].isBlank() )
        {
            activeEntries[k] = item;
            return;
        }
    }//we reached the end without finding an open spot
    activeEntries.push_back(item);
}

string AnnotationDisplay::SELECT_MouseClick(point2D pt)
{
    string allStrings("");
    //range check
    if( pt.x <= width() && pt.x >= 0 )
    {
        int start = ui->getStart(glWidget) + pt.y * ui->getWidth() + pt.x;
        int stop = start + ui->getWidth();
        for(int i = 0; i < (int)gtfTrack.size(); ++i)
        {
            if(((gtfTrack[i].start >= start && gtfTrack[i].start <= stop)//start in range
                || (gtfTrack[i].stop >= start && gtfTrack[i].stop <= stop)//end in range
                || (gtfTrack[i].start < start && gtfTrack[i].stop > stop)) )//in the middle
            {
                allStrings.append(gtfTrack[i].toString());
                allStrings.append(1, '\n');
            }

        }
    }
    if( !allStrings.empty() )
        allStrings.insert(0,"-------------\n");
    return allStrings;
}

int AnnotationDisplay::width()
{
    return max_width;
}


string AnnotationDisplay::getFileName()
{
    return fileName;
}

int AnnotationDisplay::current_display_size()
{
    return ui->getSize();
}

int AnnotationDisplay::getNextAnnotationPosition()
{
    int i = 0;
    int lineStart = ui->getStart(glWidget) + ui->getWidth();//this is the start position at the _end_ of the line
    while( i < (int)gtfTrack.size() && gtfTrack[i].start < lineStart )//assumes tracks are in order
        i++;
    return gtfTrack[i].start;
}
int AnnotationDisplay::getPrevAnnotationPosition()
{
    int i = (int)gtfTrack.size()-1;
    int lineStart = ui->getStart(glWidget);//this is the start position
    while( i > 0 && gtfTrack[i].start >= lineStart )//assumes tracks are in order
        i--;
    return gtfTrack[i].start;
}

void AnnotationDisplay::setFileName(string gtfFileName)
{
    fileName = gtfFileName;
}

/******SLOTS*****/
void AnnotationDisplay::addEntry(track_entry entry)
{
    gtfTrack.push_back(entry);
    sort(gtfTrack.begin(), gtfTrack.end(), trackCompare);
    emit displayChanged();
}
/**/
