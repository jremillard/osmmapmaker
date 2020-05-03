#pragma once

#include <QPixmap>

class TileCache : QObject
{
	Q_OBJECT

public:
	TileCache();
	~TileCache();


	bool GetTile(double zoom, int tileRow, int tileCol, uint renderVersion, QPixmap *image);

signals:
	void newImageAvailable();


private:

};




