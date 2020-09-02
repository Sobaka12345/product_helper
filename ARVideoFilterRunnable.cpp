#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QFileInfo>
#include <QDir>
#include <QDirIterator>

#include "TextureBuffer.h"
#include "ARVideoFilterRunnable.h"

const static QHash<QChar ,QVector4D> NutriColors = {
    {'-',{0.294f , 0.490f, 0.553f, 1}}, // rgb(75,125,141)
    {'A',{0.011f , 0.505f, 0.254f, 1}}, // rgb(3,129,65)
    {'B',{0.521f , 0.733f, 0.184f, 1}}, // rgb(133,187,47)
    {'C',{0.996f , 0.796f, 0.008f, 1}}, // rgb(254,203,2)
    {'D',{0.933f , 0.505f, 0.0f, 1}},   // rgb(238,129,0)
    {'E',{0.901f , 0.243f, 0.066f, 1}}, // rgb(230,62,17)
};

const static QHash<QChar ,QVector4D> URColors = {
    {'-',{0.294f , 0.490f, 0.553f, 1}}, // rgb(75,125,141)
    {'5',{0.011f , 0.505f, 0.254f, 1}}, // rgb(3,129,65)
    {'4',{0.521f , 0.733f, 0.184f, 1}}, // rgb(133,187,47)
    {'3',{0.996f , 0.796f, 0.008f, 1}}, // rgb(254,203,2)
    {'2',{0.933f , 0.505f, 0.0f, 1}},   // rgb(238,129,0)
    {'1',{0.901f , 0.243f, 0.066f, 1}}, // rgb(230,62,17)
};

const static QHash<QChar ,QVector4D> NOVAColors = {
    {'0',{0.294f , 0.490f, 0.553f, 1}}, // rgb(75,125,141)
    {'1',{0.298f , 0.651f, 0.188f, 1}}, // rgb(76,166,48)
    {'2',{0.972f , 0.804f, 0.274f, 1}}, // rgb(248,205,70)
    {'3',{0.1f , 0.796f, 0.4f, 1}},     // rgb(255,102,0)
    {'4',{0.921f , 0.2f, 0.137f, 1}},   // rgb(235,51,35)
};

ARVideoFilterRunnable::ARVideoFilterRunnable(ARVideoFilter *filter) :
    m_filter(filter),
    m_launcher(false),
    m_launcherContour(true),
    m_board(":/models/Board.obj"),
    m_titles(":/models/Titles.obj"),
    m_circle(":/models/circle.obj"),
    m_symbols({
        {'A', OBJModelLoader(":/models/A.obj")},
        {'B', OBJModelLoader(":/models/B.obj")},
        {'C', OBJModelLoader(":/models/C.obj")},
        {'D', OBJModelLoader(":/models/D.obj")},
        {'1', OBJModelLoader(":/models/1.obj")},
        {'2', OBJModelLoader(":/models/2.obj")},
        {'3', OBJModelLoader(":/models/3.obj")},
        {'4', OBJModelLoader(":/models/4.obj")},
        {'5', OBJModelLoader(":/models/5.obj")},
        {'6', OBJModelLoader(":/models/6.obj")},
        {'7', OBJModelLoader(":/models/7.obj")},
        {'8', OBJModelLoader(":/models/8.obj")},
        {'9', OBJModelLoader(":/models/9.obj")},
        {'0', OBJModelLoader(":/models/0.obj")},
        {'.', OBJModelLoader(":/models/dot.obj")},
        {'-', OBJModelLoader(":/models/defis.obj")},
}),
    m_ARFBO(0),
    m_MVP(glm::identity<glm::mat4x4>())
{
    qDebug() << m_modelProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shader/model_vertex.vsh");

    qDebug() << m_modelProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shader/model_fragment.fsh");

    qDebug() << m_modelProgram.link();

    m_modelProgram.bindAttributeLocation("vertex", 0);
}

ARVideoFilterRunnable::~ARVideoFilterRunnable()
{
}

QImage ARVideoFilterRunnable::readImage(QVideoFrame *input)
{
    QImage image = QImage(input->width(), input->height(), QImage::Format_BGR30);
    GLuint textureId = input->handle().toUInt();
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    GLint prevFbo;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0);
    glReadPixels(0, 0, input->width(), input->height(), GL_RGBA, GL_UNSIGNED_BYTE, image.bits());
    glBindFramebuffer(GL_FRAMEBUFFER, prevFbo);
    glDeleteFramebuffers(1, &fbo);

    return image;
}

void ARVideoFilterRunnable::drawObject(const OBJModelLoader &obj)
{
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, obj.getVertices().data());
    glDrawArrays(GL_TRIANGLES, 0, obj.getVertices().size());
    glDisableVertexAttribArray(0);
}

#ifdef FAST_ARM

void ARVideoFilterRunnable::findContour(QImage &image)
{
    static int failures = 0;
    constexpr int MAX_FAILURES = 5;

    cv::Mat imgSrc(image.height(),
                   image.width(),
                   CV_8UC4,
                   image.bits(),
                   image.bytesPerLine());

    cv::Mat img, filtered;
    std::vector<cv::Vec4i> hierarchy;

    float k = 1;
    cv::rotate(img, img, cv::ROTATE_90_CLOCKWISE);

    //600 - оптимальная высота, при которой можно различить код
    k = 600.0f / imgSrc.cols;
    cv::resize(imgSrc, imgSrc, cv::Size(imgSrc.cols * k, imgSrc.rows * k), 0,0, cv::INTER_NEAREST);
    cv::cvtColor(imgSrc, img, cv::COLOR_BGR2GRAY);
    cv::Sobel(img, filtered, CV_16S, 0, 1, 3, 1, 0, cv::BORDER_DEFAULT);
    cv::convertScaleAbs(filtered, filtered);

    cv::threshold(filtered, filtered,148,255,cv::THRESH_BINARY);
    cv::Mat quad = getStructuringElement(cv::MORPH_RECT, cv::Size(1,8));
    cv::morphologyEx(filtered, filtered, cv::MORPH_CLOSE, quad);
    quad = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(4,8));
    cv::erode(filtered, filtered, quad, cv::Point(-1,-1),3);
    cv::dilate(filtered, filtered, quad, cv::Point(-1,-1),4);

    std::vector <std::vector<cv::Point>> contours;
    cv::findContours(filtered,contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    if(contours.size() <= 0)
    {
        failures++;
        if(failures < MAX_FAILURES)
            return;
        failures = 0;
        m_MVP = glm::identity<glm::mat4x4>();
        return;
    }

    double max = -1;
    int idx = -1;
    for (size_t i = 0; i < contours.size(); i++)
    {
        double area = cv::contourArea(contours[i]);
        if(area > max)
        {
            idx = i;
            max = area;
        }
    }

    cv::RotatedRect rect = minAreaRect(contours[idx]);

    float objPnts[] = {
        -0.5f, -1.0f, 0.0f,
        -0.5f, 1.0f, 0.0f,
        0.5f, 1.0f, 0.0f,
        0.5f, -1.0f, 0.0f,
    };

    cv::Point2f rectPoints[4];
    rect.points(rectPoints);
    float dy = qAbs(rectPoints[1].y - rectPoints[0].y);

    if(dy == 0)
    {
        failures++;
        if(failures < MAX_FAILURES)
            return;
        failures = 0;
        m_MVP = glm::identity<glm::mat4x4>();
        return;
    }

    float dx = dy * 0.5;
    cv::Point2f cameraObjPnts[4] = {
        cv::Point2f(rect.center.x - dx / 2, rect.center.y + dy / 2),
        cv::Point2f(rect.center.x - dx / 2, rect.center.y - dy / 2),
        cv::Point2f(rect.center.x + dx / 2, rect.center.y - dy / 2),
        cv::Point2f(rect.center.x + dx / 2, rect.center.y + dy / 2)
    };


    for(int i = 0; i < 4; i++){
        cameraObjPnts[i] = cv::Point2f(cameraObjPnts[i].x * (1 / k),
                                      (cameraObjPnts[i].y * (1 / k)));
    }
    std::vector<cv::Point2f> vectorCOP = {
        cameraObjPnts[0],
        cameraObjPnts[1],
        cameraObjPnts[2],
        cameraObjPnts[3]
    };

    const float zfar = 20.0f;
    const float znear = 0.1f;
    const float fx = image.width();         // Focal length in x axis
    const float fy = image.width();         // Focal length in y axis
    const float cx = fx / 2.0f;             // Primary point x
    const float cy = image.height() / 2.0f; // Primary point y

    //Матрица, описывающая камеру
    float camera [9] = {
        fx, 0.0f, cx,
        0.0f, fy, cy,
        0.0f, 0.0f, 1.0f
    };

    cv::Mat cameraMat(3, 3, CV_32F, camera);

    cv::Mat objPntsMat(4, 3, CV_32F, objPnts);
    std::vector<float> camDist = {
        0,0,0,0
    };

    cv::Mat transMat, rotMat;
    cv::solvePnP(objPntsMat, vectorCOP, cameraMat, camDist, rotMat, transMat);

    cv::Mat Rot(3,3,CV_32FC1);
    cv::Rodrigues(rotMat, Rot);
    cv::Mat_<double> para = cv::Mat_<double>::eye(4,4);
    Rot.convertTo(para(cv::Rect(0,0,3,3)),CV_64F);
    transMat.copyTo(para(cv::Rect(3,0,1,3)));

    cv::Mat cvToGl = cv::Mat::zeros(4, 4, CV_64F);
    cvToGl.at<double>(0, 0) =  1.0f;
    cvToGl.at<double>(1, 1) = -1.0f; // Invert the y axis
    cvToGl.at<double>(2, 2) = -1.0f; // invert the z axis
    cvToGl.at<double>(3, 3) =  1.0f;

    para = cvToGl * para;

    cv::Mat_<double> modelview_matrix;

    //Перевод матрицы в OpenGL формат
    cv::Mat(para.t()).copyTo(modelview_matrix);
    glm::mat4 openGLViewMatrix;
    for(int col = 0; col < modelview_matrix.cols; col++)
    {
        for(int row = 0; row < modelview_matrix.rows; row++)
        {
            openGLViewMatrix[col][row] = modelview_matrix.at<double>(col,row);
        }
    }

    glm::mat4x4 projectionMatrix;
    projectionMatrix[0][0] = 2.0f * fx / image.width();
    projectionMatrix[0][1] = 0.0f;
    projectionMatrix[0][2] = 0.0f;
    projectionMatrix[0][3] = 0.0f;

    projectionMatrix[1][0] = 0.0f;
    projectionMatrix[1][1] = -2.0f * fy / image.height();
    projectionMatrix[1][2] = 0.0f;
    projectionMatrix[1][3] = 0.0f;

    projectionMatrix[2][0] = 1.0 - 2.0f * cx / image.width();
    projectionMatrix[2][1] = 2.0f * cy / image.height() - 1.0f;
    projectionMatrix[2][2] = (zfar + znear) / (znear - zfar);
    projectionMatrix[2][3] = -1.0f;

    projectionMatrix[3][0] = 0.0f;
    projectionMatrix[3][1] = 0.0f;
    projectionMatrix[3][2] = 2.0f * zfar * znear / (znear - zfar);
    projectionMatrix[3][3] = 0.0f;

    m_MVP = projectionMatrix * openGLViewMatrix;
}

#endif

QVideoFrame ARVideoFilterRunnable::run(QVideoFrame *input, const QVideoSurfaceFormat &surfaceFormat, QVideoFilterRunnable::RunFlags flags)
{
    Q_UNUSED(flags)
    Q_UNUSED(surfaceFormat)

    if(input->handleType() != QAbstractVideoBuffer::GLTextureHandle)
    {
        return *input;
    }

    static bool show = false;
    static int failures = 0;
    constexpr int MAX_FAILURES = 6;

    const GLuint srcTex = input->handle().toUInt();
    const QSize size = input->size();

    QImage frame(readImage(input));

    if(!m_launcher.isFinding())
    {
        const QString result = m_launcher.code();

        if(result.isEmpty())
        {
            failures++;
            if(failures > MAX_FAILURES) {
                show = false;
                failures = 0;
            }
        } else show = true;

        if(!result.isEmpty() && !m_filter->isRequesting())
        {
            QJsonDocument prod = m_filter->result();

            if(!prod["product_info"].isNull() && prod["product_info"]["barcode"].toString()
                    != m_product["product_info"]["barcode"].toString())
            {
                m_product = qMove(prod);
                emit m_filter->insertIntoHistory(m_product);
                m_filter->setProduct(m_product);
                show = true;
            }

            if(result != m_lastCode)
            {
                emit m_filter->request(result);
                m_lastCode = result;
            }
        }

        m_launcher.find(frame);
    }

#ifdef FAST_ARM
    findContour(frame);
#endif

#ifdef SLOW_ARM
    if(!m_launcherContour.isFinding())
    {
        m_MVP = m_launcherContour.MVP();
        m_launcherContour.find(frame);
    }
#endif

    if(m_ARFBO == 0)
    {

        glGenFramebuffers(1, &m_ARFBO);
        glGenTextures(1, &m_ARTex);
        glGenRenderbuffers(1, &m_ARRBO);
        glBindFramebuffer(GL_FRAMEBUFFER, m_ARFBO);
        glViewport(0, 0, size.width(), size.height());

        glBindTexture(GL_TEXTURE_2D, m_ARTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.width(), size.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ARTex, 0);


        glGenRenderbuffers(1,&m_ARRBO);
        glBindRenderbuffer(GL_RENDERBUFFER, m_ARRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, size.width(), size.height());

        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            qDebug() << "Ploho";

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    GLuint fboId = 0;
    glGenFramebuffers(1, &fboId);
    glBindFramebuffer(GL_FRAMEBUFFER, fboId);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, srcTex, 0);

    glBindTexture(GL_TEXTURE_2D, m_ARTex);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, size.width(), size.height(), 0);
    glDeleteFramebuffers(1, &fboId);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glBindFramebuffer(GL_FRAMEBUFFER, m_ARFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    //:DDDDDDDDDDDDDDDdd
    if(m_MVP != glm::identity<glm::mat4x4>() && show)
    {
        QVector<QVector4D> colors;
        m_modelProgram.bind();
        m_modelProgram.setUniformValue("color", QVector4D(0.756, 0.768, 0.733, 0.750));
        glUniformMatrix4fv(m_modelProgram.uniformLocation("mvp_matrix"), 1, GL_FALSE, &m_MVP[0][0]);
        drawObject(m_board);

        m_modelProgram.setUniformValue("color", QVector4D(0.286, 0.208, 0.251, 1.0));
        drawObject(m_titles);

        QString NOVA = QString::number(m_product["product_info"]["OFF"]["nova_group"].toInt());
        if(NOVA[0] != '0')
            colors.append(NOVAColors[NOVA[0]]);
        m_modelProgram.setUniformValue("color", NOVAColors[NOVA[0]]);
        drawObject(m_symbols[NOVA[0] == '0' ? QChar('-') : NOVA[0]]);

        QString nutriscore = m_product["product_info"]["OFF"]["nutriscore_grade"].toString();
        if(nutriscore.isEmpty())
        {
            m_modelProgram.setUniformValue("color", NutriColors['-']);
            auto trans = glm::translate(m_MVP, glm::vec3(0.6f, 0.0f, 0.0f));
            glUniformMatrix4fv(m_modelProgram.uniformLocation("mvp_matrix"), 1, GL_FALSE, &trans[0][0]);
            drawObject(m_symbols[QChar('-')]);
        }
        else
        {
            auto score = nutriscore[0].toUpper();
            qDebug() << score;
            colors.append(NutriColors[score]);
            m_modelProgram.setUniformValue("color", colors.last());
            drawObject(m_symbols[score]);
        }

        auto trans = glm::translate(m_MVP, glm::vec3(0.0f, 1.23f, 0.0f)); //SHIFT
        glUniformMatrix4fv(m_modelProgram.uniformLocation("mvp_matrix"), 1, GL_FALSE, &trans[0][0]);
        QString ur = QString::number(m_product["user_rating"].toDouble());


        if(ur.length() > 1)
        {
            colors.append(URColors[ur[0]]);
            m_modelProgram.setUniformValue("color", colors.last());
            drawObject(m_symbols[ur[0]]);
            trans =  glm::translate(trans, glm::vec3(0.0f, 0.15f, 0.0f));
            glUniformMatrix4fv(m_modelProgram.uniformLocation("mvp_matrix"), 1, GL_FALSE, &trans[0][0]);
            drawObject(m_symbols[ur[1]]);
            trans =  glm::translate(trans, glm::vec3(0.0f, 0.12f, 0.0f));
            glUniformMatrix4fv(m_modelProgram.uniformLocation("mvp_matrix"), 1, GL_FALSE, &trans[0][0]);
            drawObject(m_symbols[ur[2]]);
        } else if(ur[0] == '0')
        { 
            m_modelProgram.setUniformValue("color", URColors[QChar('-')]);
            drawObject(m_symbols[QChar('-')]);
        } else
        {
            colors.append(URColors[ur[0]]);
            m_modelProgram.setUniformValue("color", colors.last());
            drawObject(m_symbols[ur[0]]);
        }

        glUniformMatrix4fv(m_modelProgram.uniformLocation("mvp_matrix"), 1, GL_FALSE, &m_MVP[0][0]);
        QVector4D circleColor(0.0f, 0.0f, 0.0f, 0.0f);
        for(auto &x : colors)
            circleColor += x;
        if(colors.size())
            m_modelProgram.setUniformValue("color", circleColor / colors.size());
        else
            m_modelProgram.setUniformValue("color", QVector4D(0.294f , 0.490f, 0.553f, 1));

        drawObject(m_circle);
    }

    glDisable(GL_CULL_FACE);

    return frameFromTexture(m_ARTex, size, input->pixelFormat());
}

