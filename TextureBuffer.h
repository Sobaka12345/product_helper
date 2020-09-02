#ifndef FRAMEHELPER_H
#define FRAMEHELPER_H

#include <QVideoFrame>
#include <QAbstractVideoBuffer>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLFramebufferObject>

#ifndef QT_NO_OPENGL
class TextureBuffer : public QAbstractVideoBuffer
{
public:
    TextureBuffer(uint id) : QAbstractVideoBuffer(GLTextureHandle), m_id(id) { }
    MapMode mapMode() const { return NotMapped; }
    uchar *map(MapMode, int *, int *) { return 0; }
    void unmap() { }
    QVariant handle() const { return QVariant::fromValue<uint>(m_id); }

private:
    GLuint m_id;
};
#endif // QT_NO_OPENGL

QVideoFrame frameFromTexture(uint textureId, const QSize &size, QVideoFrame::PixelFormat format)
{
#ifndef QT_NO_OPENGL
    return QVideoFrame(new TextureBuffer(textureId), size, format);
#else
    return QVideoFrame();
#endif // QT_NO_OPENGL
};

#endif // FRAMEHELPER_H
