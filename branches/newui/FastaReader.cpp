#include <QtGui>
#include "FastaReader.h"
#include "BasicTypes.h"
#include "UiVariables.h"

#include <string>
#include <cctype>
#include <QDebug>
#include <QThread>
#include <QString>
#include <qtconcurrentrun.h>
#include <QApplication>

using namespace QtConcurrent;

FastaReader::FastaReader( GLWidget* gl)
{	
	glWidget = gl;
	sequence = string("AATCGATCGTACGCTACGATCGCTACGCAGCTAGGACGGATT");	
	bytesInFile = 0;
	blockSize = 5000000;
	progressBar = NULL;
}

bool FastaReader::initFile(string file)
{
	wordfile.clear();//reset the fail state to normal
	wordfile.open (file.c_str(), ifstream::in | ifstream::binary);
	if(wordfile.fail())
	{
		ErrorBox msg("Could not read the file.  Either Skittle doesn't have file permissions or the file does not exist.");
		return false;
	}
	int begin = wordfile.tellg();
	wordfile.seekg (0, ios::end);
	int end = wordfile.tellg();
	bytesInFile = end - begin;
	wordfile.seekg(0, ios::beg);//move pointer to the beginning of the file
	
	return true;
}

void FastaReader::readFile(QString fileName)
{
	string file = fileName.toStdString();
	glWidget->print(file);
	if( file.empty() )
		return;
	storeChrName(file);	
	sequence.clear();
	sequence = string(">"); //inserting this character at the beginning means the first nucleotide is at sequence[1]
	

	if(initFile(file))
	{
		buffer.push_back('>');
		loadingProgress();
		wordfile.close();
		int bufferSize = buffer.size();
		sequence.resize(bufferSize, 'N');
		
		if(progressBar)
		{
			delete progressBar;
			progressBar = NULL;
		}
		progressBar = new QProgressDialog("Capitalizing Sequence...", 0, 0, bytesInFile);//no ,"Cancel",
		progressBar->setMinimumDuration(0);
    	QObject::connect(this, SIGNAL(progressValueChanged(int)), progressBar, SLOT(setValue(int)));

		for(int i = 0; i < buffer.size(); ++i)
		{
			sequence[i] = upperCase(buffer[i]);
			if(i % blockSize == 0)
				emit progressValueChanged(i);
		}
		progressBar->close();
		if(progressBar)
		{
			delete progressBar;
			progressBar = NULL;
		}
		buffer.clear();
		buffer.resize(0);
		glWidget->print("Done reading file.");
		emit newFileRead( seq() );
	}
}

char FastaReader::upperCase(char& c)
{
	if( (int)c >= 97 && (int)c <= 122 )
		return (char)((int)c - 32);
	else
		return c;
}  

void FastaReader::loadingProgress()
{
	/*
#ifndef QT_NO_CONCURRENT
	//ProgressBar *dialog = new ProgressBar(this);
	// QThreadPool takes ownership and deletes 'hello' automatically
	//QThreadPool::globalInstance()->start(dialog);

	for(int i = 0; i < bytesInFile-1 && !wordfile.fail();)//&& !dialog->wasCanceled(); )//i+=blockSize)
	{
		i = readBlock3(i);//returns the last position read
	}


	//dialog->reset();//setValue(bytesInFile);//reset
	//dialog->close();
	//delete dialog;
	/*/
//#else	
	if(progressBar)
	{
		delete progressBar;
		progressBar = NULL;
	}
	progressBar = new QProgressDialog("Reading File...", 0, 0, bytesInFile);//, this);
	progressBar->setMinimumDuration(0);
    QObject::connect(this, SIGNAL(progressValueChanged(int)), progressBar, SLOT(setValue(int)));

    for(int i = 0; i < bytesInFile-1 && !wordfile.fail() && !progressBar->wasCanceled(); )//i+=blockSize)
    {
		i = readBlock3(i);//returns the last position read
	}
	progressBar->reset();//setValue(bytesInFile);//reset
	progressBar->close();/**/
//#endif
}

int FastaReader::readBlock(int &start)
{
	//wordfile.seekg(start, ios::beg);//start is in file bytes, not sequence letters (blasted newlines)
	int end = start + blockSize;//end is in file bytes
	string line;	
	if(start == 0)
		wordfile.ignore(500, '\n');//skip first line of the FASTA file
	while( wordfile >> line )
	{//TODO:This may have bad behaviour in a file with no new lines
		transform(line.begin(), line.end(), line.begin(), to_upper() );
		sequence.append(line);
		if(wordfile.tellg() > end)
		{
			break;
		}
	}
	
	emit progressValueChanged( start );
	return wordfile.tellg();
}

int FastaReader::readBlock2(int &start)
{
	//wordfile.seekg(start, ios::beg);//start is in file bytes, not sequence letters (blasted newlines)
	int end = start + blockSize;//end is in file bytes
	string line;	
	if(start == 0)
		wordfile.ignore(500, '\n');//skip first line of the FASTA file
	char curr;
	for( int i = start; i < end; ++i) 
	{
		curr = wordfile.get();
		if( curr != '\n')
			buffer.push_back(curr);
	}
	
	emit progressValueChanged( start );
	return wordfile.tellg();
}

int FastaReader::readBlock3(int &start)
{
	//wordfile.seekg(start, ios::beg);//start is in file bytes, not sequence letters (blasted newlines)
	int bytesLeft = bytesInFile - start;
	int readBytes = min(blockSize, bytesLeft);//end is in file bytes
	string line;	
	if(start == 0)
		wordfile.ignore(500, '\n');//skip first line of the FASTA file
		
	vector<char> temp;
	temp.resize(readBytes, '\n');
	wordfile.read(&(temp[0]), readBytes);
	
	char curr;
	int tempSize = temp.size();
	for( int i = 0; i < tempSize; ++i) 
	{
		curr = temp[i];
		if( !(curr == '\n' || curr == '\r') )//!isspace(curr))// != '\n')//check speed
			buffer.push_back(curr);
	}
	
	emit progressValueChanged( start );//*100 / bytesInFile
	return wordfile.tellg();
}

void FastaReader::storeChrName(string path)
{
	int startI = path.find_last_of('\\');
	if(startI == path.size())
		startI = path.find_last_of('/');
	int endI = path.find_last_of('.');
	int sizeI = endI - startI;
	string name = path.substr(startI+1, sizeI-1);
	glWidget->chromosomeName = name;
	emit fileNameChanged(name);
}

const string& FastaReader::seq()
{
	return sequence;
}