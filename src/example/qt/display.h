#pragma once

/// ultrasound image display
class UltrasoundImage : public QGraphicsView
{
    Q_OBJECT
public:
    explicit UltrasoundImage(QWidget*);

    void loadImage(const void* img, int w, int h, int bpp, int sz);
    void setDepth(double d) { depth_ = d; }
    void checkRoi();

protected:
    virtual void drawForeground(QPainter*, const QRectF&) override;
    virtual void drawBackground(QPainter*, const QRectF&) override;
    virtual void mouseReleaseEvent(QMouseEvent*) override;

    virtual void resizeEvent(QResizeEvent*) override;
    virtual int heightForWidth(int w) const override;
    virtual QSize sizeHint() const override;

private:
    double depth_;  ///< depth display value
    QPolygonF roi_;
    QImage image_;  ///< the image buffer
    QMutex lock_;   ///< locking mechanism
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
    QMutex lock_;               ///< locking mechanism
};
