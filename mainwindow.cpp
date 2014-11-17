/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mainwindow.h"
#include "chip.h"

#include <QtGui>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
	//FIRST HANDLE FILE MENU
	fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->setObjectName("fileMenu");

	loadImageAction = new QAction(tr("Load Images For Subtraction"), this);
	loadImageAction->setObjectName("loadImageAction");
	loadImageAction->setStatusTip(tr("Load an images for subtraction"));
	loadImageAction->setShortcut(tr("Ctrl+O"));
	connect(loadImageAction, SIGNAL(triggered()), this, SLOT(askLoadImage()));
	fileMenu->addAction(loadImageAction);

    h1Splitter = new QSplitter;

    QSplitter *vSplitter = new QSplitter;
    vSplitter->setOrientation(Qt::Vertical);
    vSplitter->addWidget(h1Splitter);

    view = new View("Central View");
    h1Splitter->addWidget(view);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(vSplitter);

	QWidget *widget = new QWidget();
	widget->setLayout(layout);
	setCentralWidget(widget);

    setWindowTitle(tr("Channels Editor"));

	imageManager = NULL;
}

MainWindow::~MainWindow()
{
	writeSettings();
	if( !lowResImages.empty() )
		for( itk::SizeValueType i=0; i<lowResImages.size(); ++i )
			free( lowResImages.at(i) );
	lowResImages.clear();
	if( !medResTileImagesAll.empty() )
	{
		for( itk::SizeValueType i=0; i<medResTileImagesAll.size(); ++i )
		{
			for( itk::SizeValueType j=0; j<medResTileImagesAll.at(i).size(); ++j )
				medResTileImagesAll.at(i).at(j)->UnRegister();
			medResTileImagesAll.at(i).clear();
		}
		medResTileImagesAll.clear();
	}
	if( imageManager!=NULL )
		delete imageManager;
}

#define IMAGEDISPLAYCHIP 1
void MainWindow::populateScene()
{
    scene = new QGraphicsScene;
	// Populate scene
#ifdef IMAGEDISPLAYCHIP
	itk::SizeValueType xSizeMax = imageManager->GetmaxSizeX();
	itk::SizeValueType ySizeMax = imageManager->GetmaxSizeY();
	itk::SizeValueType xSizeMid = imageManager->GetmidSizeX();
	itk::SizeValueType ySizeMid = imageManager->GetmidSizeY();
	itk::SizeValueType xSizeLow = imageManager->GetlowSizeX();
	itk::SizeValueType ySizeLow = imageManager->GetlowSizeY();
	double deltaXY = std::ceil(imageManager->GetLowScaleFactor());
	double deltaXYMid = std::ceil(((double)xSizeMid)/((double)xSizeLow));
	itk::SizeValueType xSizeHalf = std::floor(((double)xSizeMid/2.0)+0.5);
	itk::SizeValueType ySizeHalf = std::floor(((double)ySizeMid/2.0)+0.5);
	std::vector< UCConstIterWIndex > LowResIters = imageManager->GetLowResDownSampIterators();

	for( itk::SizeValueType i = 0; i<LowResIters.size(); ++i )
		LowResIters.at(i).GoToBegin();
	while( !LowResIters.at(0).IsAtEnd() )
	{
		int val = LowResIters.at(0).Get();
		val = val<0 ? 0:val;
		itk::IndexValueType xx = LowResIters.at(0).GetIndex()[0];
		itk::IndexValueType yy = LowResIters.at(0).GetIndex()[1];
		if( !(yy%100)&&!(xx%500) ) 
			std::cout<<xx<<" "<<yy<<std::endl<<std::flush;
		qreal posX = -1.0*xSizeHalf + deltaXYMid*xx;
		qreal posY = -1.0*ySizeHalf + deltaXYMid*yy;
		QColor color(val,val,val,255);

		/*Get the mid and max resolution images for the current tile*/
		std::vector< UCImageType::Pointer > MidResTileImages;
		MidResTileImages.resize( imageManager->GetOnFlags().size() );
		this->CopyTileImages( MidResTileImages, true, xx, yy,
			xSizeMid, ySizeMid, deltaXYMid );
		medResTileImagesAll.push_back( MidResTileImages );
		if(xx==500 && yy==500) imageManager->WriteITKImage( MidResTileImages.at(0), "asd1.tif" );
/***		std::vector< InputImageType::Pointer > MaxResTileImages;
		MaxResTileImages.resize( LowResIters.size() );
		this->CopyTileImages( MaxResTileImages, false, xx, yy,
			xSizeMax, ySizeMax, deltaXY );***/
		std::vector< unsigned char* > MidResTileImagesBuffers;
		for( itk::SizeValueType i = 1; i<MidResTileImages.size(); ++i )
			MidResTileImagesBuffers.push_back( MidResTileImages.at(i)->GetBufferPointer() );
		Chip *item = new Chip(color, xx, yy,
			((double)MidResTileImages.at(0)->GetLargestPossibleRegion().GetSize()[0]),
			((double)MidResTileImages.at(0)->GetLargestPossibleRegion().GetSize()[1]),
			MidResTileImagesBuffers );//, MaxResTileImages);
		item->setPos(QPointF(posX, posY));
		scene->addItem(item);
		for( itk::SizeValueType i = 0; i<LowResIters.size(); ++i )
			++LowResIters.at(i);
	}
	//Copy the low res 16-bit image into an array for quick access
	lowResImages.resize( imageManager->GetOnFlags().size() );
	std::vector< USConstIter > LowResItersUS = imageManager->GetLowResIterators();
	for( itk::SizeValueType i=0; i<LowResItersUS.size(); ++i )
	{
		InputImageType::PixelType *lowResImage = (InputImageType::PixelType*)
			malloc( xSizeLow*ySizeLow*sizeof(InputImageType::PixelType) );
		lowResImages.push_back( lowResImage );
		LowResItersUS.at(i).GoToBegin();
		itk::SizeValueType count = 0;
		for( ; !LowResItersUS.at(i).IsAtEnd() && count<(xSizeLow*ySizeLow); ++LowResItersUS.at(i) )
			lowResImage[count++] = LowResItersUS.at(i).Get();
	}

	
//#pragma omp parallel for


	
#else
	QImage image(":/qt4logo.png");
	qreal sizeX = 110.0, sizeY = 70.0;
	itk::SizeValueType xx = 0;
    itk::SizeValueType nitems = 0;
	for (itk::IndexValueType i = -11000; i < 11000; i += 110) {
        ++xx;
        itk::SizeValueType yy = 0;
        for (itk::IndexValueType j = -7000; j < 7000; j += 70) {
            ++yy;
            qreal x = (i + 11000) / 22000.0;
            qreal y = (j + 7000) / 14000.0;
			QColor color(image.pixel(itk::SizeValueType(image.width() * x), itk::SizeValueType(image.height() * y)));
            QGraphicsItem *item = new Chip(color, xx, yy, sizeX, sizeY);
            item->setPos(QPointF(i, j));
            scene->addItem(item);
            ++nitems;
        }
    }
	std::cout<<"Number of items:"<<nitems<<std::endl;
#endif
}

void MainWindow::readSettings() 
{
	QSettings settings;
	lastPath = settings.value("lastPath", ".").toString();
}

void MainWindow::writeSettings()
{
	QSettings settings;
	settings.setValue("lastPath", lastPath);
}

void MainWindow::askLoadImage()
{
	//Get the filename of the new image
	standardImageTypes = tr("Images (*.tif *.tiff *.pic *.png *.jpg *.lsm)\n");
	QString fileName = QFileDialog::getOpenFileName(this, "Choose Image to Subtract from", lastPath, standardImageTypes);
	std::string stdFileName = fileName.toStdString();
	//If no filename do nothing
	if(stdFileName == "")
		return;
	lastPath = fileName.left( fileName.lastIndexOf( "/" )+1 );
	std::vector<std::string> subtractImages;
	QMessageBox::StandardButton reply;
	do
	{
		QString fileNames = QFileDialog::getOpenFileName(this, "Choose Image to Subtract", lastPath, standardImageTypes);
		lastPath = fileNames.left( fileNames.lastIndexOf( "/" )+1 );
		//If no filename do nothing
		if(!(fileNames.toStdString() == ""))
		{
			subtractImages.push_back( fileNames.toStdString() );
			lastPath = fileNames.left( fileNames.lastIndexOf( "/" )+1 );
		}
		reply = QMessageBox::question(this, "Add Channels", "Use one more channel for subtraction?",
										QMessageBox::Yes|QMessageBox::No);
	}
	while(reply == QMessageBox::Yes);
	if( subtractImages.empty() ) return;
	std::vector<std::string>::iterator it = subtractImages.begin();
	subtractImages.insert( it , stdFileName );
	imageManager = new ImageManager( subtractImages );
	this->showMaximized();
	populateScene();
    view->view()->setScene(scene);
	return;
}

UCImageType::Pointer MainWindow::AllocateMemoryForImage(UCImageType::SizeType size)
{
	UCImageType::Pointer outImage = UCImageType::New();
	UCImageType::IndexType start;
	start[0] =   0;  // first index on X
	start[1] =   0;  // first index on Y
	UCImageType::PointType origin;
	origin[0] = 0;
	origin[1] = 0;
	outImage->SetOrigin( origin );
	UCImageType::RegionType region;
	region.SetSize( size );
	region.SetIndex( start );
	outImage->SetRegions( region );
	outImage->Allocate();
	outImage->Update();
	return outImage;
}

itk::SizeValueType MainWindow::GetTileImageSizeFromBounds( itk::SizeValueType index, 
	itk::SizeValueType max, double delta )
{
	itk::SizeValueType returnVal;
	if( (index+1)*delta <= max )
		returnVal = delta;
	else
		if( index*delta < max )
			returnVal = max-index*delta;
		else
			returnVal = 0;
	return returnVal;
}

void MainWindow::CopyTileImages( std::vector< UCImageType::Pointer > &TileImages,
		bool midFlag, itk::IndexValueType xx,
		itk::IndexValueType yy, itk::SizeValueType sizeX,
		itk::SizeValueType sizeY, double delta)
{
	UCImageType::IndexType index;
	index[0] = xx*delta;
	index[1] = yy*delta;
	UCImageType::SizeType size;
	size[0] = this->GetTileImageSizeFromBounds( xx, sizeX, delta );
	size[1] = this->GetTileImageSizeFromBounds( yy, sizeY, delta );
	UCImageType::RegionType region;
	region.SetSize( size );
	region.SetIndex( index );
	std::vector< UCConstIter > ResIters;
	if( midFlag )
		ResIters = imageManager->GetMidResIterators( region );
	else
		ResIters = imageManager->GetMaxResIterators( region );
	for( itk::SizeValueType i=0; i<ResIters.size(); ++i )
	{ 
		//Get the corresponding mid res image
		if( !size[0] || !size[1] )
			continue; /*Not worth the pain*/
		UCImageType::Pointer currentImage = this->AllocateMemoryForImage( size );
		ResIters.at(i).GoToBegin();
		UCIter iter( currentImage, currentImage->GetLargestPossibleRegion() );
		while( !ResIters.at(i).IsAtEnd() || !iter.IsAtEnd() )
		{
			iter.Set( ResIters.at(i).Get() );
			++iter; ++ResIters.at(i);
		}
		currentImage->Register();
		TileImages.at(i) = currentImage;
	}
	return;
}