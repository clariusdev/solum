#pragma once

#include <sdk/solum_def.h>

/// ultrasound image display
class UltrasoundImage : public QGraphicsView
{
    Q_OBJECT
public:
    explicit UltrasoundImage(bool overlay, QWidget*);

    void loadImage(const void* img, int w, int h, int bpp, CusImageFormat format, int sz);
    void setDepth(double d) { depth_ = d; }
    void checkActiveRegion();
    void checkRoi();
    void checkGate();

protected:
    virtual void drawForeground(QPainter*, const QRectF&) override;
    virtual void drawBackground(QPainter*, const QRectF&) override;
    virtual void mouseReleaseEvent(QMouseEvent*) override;

    virtual void resizeEvent(QResizeEvent*) override;
    virtual int heightForWidth(int w) const override;
    virtual QSize sizeHint() const override;

private:
    double depth_;          ///< depth display value
    bool overlay_;          ///< flag if this is an overlay display
    QPolygonF activeRoi_;   ///< active region for grayscale imaging
    QPolygonF modeRoi_;     ///< region of interest for doppler or elastography modes
    QVector<QLineF> gate_;  ///< gate lines to draw
    QImage image_;          ///< the image buffer
};

/// spectrum display
class Spectrum : public QGraphicsView
{
    Q_OBJECT
public:
    explicit Spectrum(QWidget*);

    void loadImage(const void* img, int l, int s, int bps);
    void reset();

protected:
    virtual void drawForeground(QPainter*, const QRectF&) override;
    virtual void drawBackground(QPainter*, const QRectF&) override;

    virtual void resizeEvent(QResizeEvent*) override;
    virtual int heightForWidth(int w) const override;
    virtual QSize sizeHint() const override;

private:
    QImage spectrum_;   ///< the spectrum buffer
    int offset_;        ///< offset for pushing into buffer
};

/// rf signal display
class RfSignal : public QGraphicsView
{
    Q_OBJECT
public:
    explicit RfSignal(QWidget*);

    void loadSignal(const void* rf, int l, int s, int ss);
    void setZoom(int zoom);

protected:
    virtual void drawForeground(QPainter*, const QRectF&) override;
    virtual void drawBackground(QPainter*, const QRectF&) override;

    virtual void resizeEvent(QResizeEvent* e) override;
    virtual int heightForWidth(int w) const override;
    virtual QSize sizeHint() const override;

private:
    QVector<int16_t> signal_;   ///< the rf signal
    qreal zoom_;                ///< zoom level
};

/// spectrum display
class Prescan : public QGraphicsView
{
    Q_OBJECT
public:
    explicit Prescan(QWidget*);

    void loadImage(const void* img, int w, int h, int bpp, CusImageFormat format, int sz);

protected:
    virtual void drawForeground(QPainter*, const QRectF&) override;
    virtual void drawBackground(QPainter*, const QRectF&) override;

    virtual void resizeEvent(QResizeEvent*) override;
    virtual int heightForWidth(int w) const override;
    virtual QSize sizeHint() const override;

private:
    QImage image_;  ///< the spectrum buffer
};
