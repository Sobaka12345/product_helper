#ifndef BARCODEFINDER_H
#define BARCODEFINDER_H

#include <QThread>
#include <QImage>
#include <QDebug>
#include <QTime>

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "opencv2/opencv.hpp"
#include "QZXing/QZXing.h"


class BarcodeFinder : public QObject
{
    Q_OBJECT

public slots:
    void findContour(QImage image)
    {
        cv::Mat imgSrc(image.height(),
                       image.width(),
                       CV_8UC4,
                       image.bits(),
                       image.bytesPerLine());

        cv::Mat img, filtered;
        std::vector<cv::Vec4i> hierarchy;

        float k = 1;
        cv::rotate(img, img, cv::ROTATE_90_CLOCKWISE);

        float shrink = 0.6f;
        if(image.width() > 2000)
            k = 0.3f * shrink;
        else if(image.width() > 1500)
            k = 0.5f * shrink;
        else if(image.width() > 1000)
            k = 0.7f * shrink;

        cv::resize(imgSrc, imgSrc, cv::Size(imgSrc.cols * k, imgSrc.rows * k), 0,0, cv::INTER_NEAREST);
        cv::cvtColor(imgSrc, img, cv::COLOR_BGR2GRAY);
        cv::Sobel(img, filtered, CV_16S, 0, 1, 3, 1, 0, cv::BORDER_DEFAULT);
        cv::convertScaleAbs(filtered, filtered);

        cv::threshold(filtered, filtered,148,255,cv::THRESH_BINARY);
        cv::Mat quad = getStructuringElement(cv::MORPH_RECT, cv::Size(1,8));
        cv::morphologyEx(filtered, filtered, cv::MORPH_CLOSE, quad);
        quad = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(4,8));
        cv::erode(filtered, filtered, quad, cv::Point(-1,-1),3);
        cv::dilate(filtered, filtered, quad, cv::Point(-1,-1),6);

        std::vector <std::vector<cv::Point>> contours;
        cv::findContours(filtered,contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        if(contours.size() <= 0)
        {
            emit contourFinded(glm::identity<glm::mat4x4>());
            return;
        }

        double max = -1;
        int idx = -1;
        for (size_t i = 0; i < contours.size(); i++)
        {
            double area = contourArea(contours[i]);
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
            emit contourFinded(glm::identity<glm::mat4x4>());
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

        //CREATE CAMERA MATRIX
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
        cv::Mat(para.t()).copyTo(modelview_matrix); // transpose to col-major for OpenGL
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

        glm::mat4x4 MVP = projectionMatrix * openGLViewMatrix;

        emit contourFinded(MVP);
    }

    void scanCode(QImage image)
    {
        cv::Mat imgSrc(image.height(),
                       image.width(),
                       CV_8UC4,
                       image.bits(),
                       image.bytesPerLine());

        cv::Mat img, filtered;
        std::vector<cv::Vec4i> hierarchy;

        float k = 1;
        cv::rotate(img, img,cv::ROTATE_90_CLOCKWISE);
        if(image.width() > 2000)
            k = 0.3f;
        else if(image.width() > 1500)
            k = 0.5f;
        else if(image.width() > 1000)
            k = 0.7f;

        cv::resize(imgSrc, imgSrc, cv::Size(imgSrc.cols * k, imgSrc.rows * k));

        cv::cvtColor(imgSrc, img, cv::COLOR_BGR2GRAY);

        cv::Sobel(img, filtered, CV_16S, 0, 1, 3, 1, 0, cv::BORDER_DEFAULT);

        convertScaleAbs(filtered, filtered);

        cv::threshold(filtered, filtered,148,255,cv::THRESH_BINARY);
        cv::Mat quad = getStructuringElement(cv::MORPH_RECT, cv::Size(1,26));
        morphologyEx(filtered, filtered, cv::MORPH_CLOSE, quad);
        quad = getStructuringElement(cv::MORPH_RECT, cv::Size(13,26));
        erode(filtered, filtered, quad, cv::Point(-1,-1),3);
        dilate(filtered, filtered, quad, cv::Point(-1,-1),6);
        std::vector <std::vector<cv::Point>> contours;

        findContours(filtered,contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        if(contours.size() <= 0)
        {
            emit codeScanned("");
            return;
        }

        double max = -1;
        int idx = -1;
        for (size_t i = 0; i < contours.size(); i++)
        {
            double area = contourArea(contours[i]);
            if(area > max)
            {
                idx = i;
                max = area;
            }
        }

        cv::RotatedRect rect = minAreaRect(contours[idx]);

        cv::Mat M, rotated, cropped;

        float angle = rect.angle;
        cv::Size rect_size = cv::Size(rect.size.width, rect.size.height);

        if (rect.angle < -45.) {
            angle += 90.0;
            cv::swap(rect_size.width, rect_size.height);
        }

        M = getRotationMatrix2D(rect.center, angle, 1.0);

        cv::cvtColor(imgSrc, imgSrc, cv::COLOR_BGRA2BGR);
        cv::warpAffine(imgSrc, rotated, M, imgSrc.size(), cv::INTER_CUBIC);

        cv::getRectSubPix(rotated, rect_size, rect.center, cropped);
        cv::cvtColor(cropped,cropped, cv::COLOR_BGR2GRAY);

        cv::rotate(cropped, cropped, cv::ROTATE_90_CLOCKWISE);

        QZXing dec;
        dec.setDecoder(QZXing::DecoderFormat_EAN_13 | QZXing::DecoderFormat_UPC_A);
        dec.setTryHarder(false);
        QImage decImg(cropped.data, cropped.cols, cropped.rows,
                      cropped.step, QImage::Format_Grayscale8);

        QString res = dec.decodeImage(decImg);

        emit codeScanned(qMove(res));
    }

signals:
    void contourFinded(glm::mat4x4);
    void codeScanned(QString);
};



class ThreadLauncher: public QObject
{
    Q_OBJECT
    QThread m_thread;
    bool m_isFinding;
    QString m_code;
    glm::mat4x4 m_MVP;

public:
    ThreadLauncher(bool onlyContour = false):
        m_isFinding(false)
    {
        BarcodeFinder * finder = new BarcodeFinder;
        finder->moveToThread(&m_thread);
        connect(&m_thread, &QThread::finished, finder, &QObject::deleteLater);

        if(onlyContour)
        {
            connect(this, &ThreadLauncher::launch, finder, &BarcodeFinder::findContour);
            connect(finder, &BarcodeFinder::contourFinded, this, &ThreadLauncher::handleMVP);
        } else
        {
            connect(this, &ThreadLauncher::launch, finder, &BarcodeFinder::scanCode);
            connect(finder, &BarcodeFinder::codeScanned, this, &ThreadLauncher::handleCode);
        }

        m_thread.start(QThread::Priority::HighPriority);
    }

    void find(const QImage& image)
    {
        m_isFinding = true;
        emit launch(image);
    }

    ~ThreadLauncher()
    {
        m_thread.quit();
        m_thread.wait();
    }

    bool isFinding() const { return m_isFinding; }
    QString code() const { return m_code; }
    glm::mat4x4 MVP() const { return m_MVP; }

public slots:
    void handleMVP(glm::mat4x4 MVP)
    {
        m_MVP = MVP;
        m_isFinding = false;
    }
    void handleCode(QString code)
    {
        m_code = code;
        m_isFinding = false;
    }

signals:
    void launch(QImage image);
};

#endif // BARCODEFINDER_H
