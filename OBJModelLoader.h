#ifndef OBJMODELLOADER_H
#define OBJMODELLOADER_H

#include <QVector>
#include <QVector2D>
#include <QVector3D>

//simple way to load *.obj models
class OBJModelLoader
{
public:
    OBJModelLoader(QString path);
    OBJModelLoader();

private:
    QString textureName;
    QVector<QVector3D> Vertices, ObjectVertices; //v
    QVector<QVector3D> VNormals, NormalVertices; //vn
    QVector<QVector2D> UVs, UVsCoordinates;      //vt

public:
    const QVector<QVector3D>& getVertices() const { return ObjectVertices; }

    const QVector<QVector3D>& getVNormals() const { return NormalVertices; }

    const QVector<QVector2D>& getUVs() const { return UVsCoordinates; }
};

#endif // OBJMODELLOADER_H
