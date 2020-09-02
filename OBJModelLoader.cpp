#include <QFile>
#include <QTextStream>
#include <QDebug>

#include "OBJModelLoader.h"

OBJModelLoader::OBJModelLoader(QString path)
{
    QVector3D temp3D, tempN3D;
    //QVector2D temp2D;
    QFile file(path);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream fileText(&file);
        while (!fileText.atEnd())
        {
            QString fileLine = fileText.readLine();
            if(fileLine.startsWith("vn "))
            {
                QStringList lineList = fileLine.split(" ");
                temp3D.setX(lineList[1].toFloat());
                temp3D.setY(lineList[2].toFloat());
                temp3D.setZ(lineList[3].toFloat());
                VNormals.push_back(temp3D);
            }
            /*else if(fileLine.startsWith("vt "))
            {
                QStringList lineList = fileLine.split(" ");
                temp2D.setX(lineList[1].toFloat());
                temp2D.setY(lineList[2].toFloat());
                UVs.push_back(temp2D);
            }*/
            else if(fileLine.startsWith("v "))
            {
                QStringList lineList = fileLine.split(" ");
                temp3D.setX(lineList[1].toFloat());
                temp3D.setY(lineList[2].toFloat());
                temp3D.setZ(lineList[3].toFloat());
                Vertices.push_back(temp3D);
            }
            else if(fileLine.startsWith("f "))
            {
                QStringList lineList = fileLine.split(" ");
                QStringList tmp = lineList[1].split("/");
                temp3D.setX(tmp[0].toInt());
                //temp2D.setX(tmp[1].toInt());
                tempN3D.setX(tmp[2].toInt());

                tmp = lineList[2].split("/");
                temp3D.setY(tmp[0].toInt());
                //temp2D.setY(tmp[1].toInt());
                tempN3D.setY(tmp[2].toInt());

                tmp = lineList[3].split("/");
                temp3D.setZ(tmp[0].toInt());
                tempN3D.setZ(tmp[2].toInt());

                ObjectVertices.push_back(Vertices[temp3D.x()-1]);
                ObjectVertices.push_back(Vertices[temp3D.y()-1]);
                ObjectVertices.push_back(Vertices[temp3D.z()-1]);
                NormalVertices.push_back(VNormals[tempN3D.x()-1]);
                NormalVertices.push_back(VNormals[tempN3D.y()-1]);
                NormalVertices.push_back(VNormals[tempN3D.z()-1]);
                //UVsCoordinates.push_back(UVs[temp2D.x()-1]);
                //UVsCoordinates.push_back(UVs[temp2D.y()-1]);
            }
        }
    }
    file.close();
}

OBJModelLoader::OBJModelLoader()
{

}
