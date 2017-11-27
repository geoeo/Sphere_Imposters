/*
* Copyright (C) 2016
* Computer Graphics Group, The Institute of Computer Graphics and Algorithms, TU Wien
* Written by Tobias Klein <tklein@cg.tuwien.ac.at>
* All rights reserved.
*/

#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLDebugLogger>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QGLShader>
#include <QOpenGLShaderProgram>
#include <QFileSystemWatcher>
#include <QElapsedTimer>
#include <QTimer>

#include "Camera.h"
#include "PdbLoader.h"

class MainWindow;

class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
	Q_OBJECT

public:
	GLWidget(QWidget *parent, MainWindow *mainWindow);
	~GLWidget();

	void initMoleculeRenderMode(std::vector<std::vector<Atom> > *animation);

	void playAnimation();
	void pauseAnimation();
	bool isPlaying();
	void setAnimationFrame(int frameNr);

	float ambientFactor;
	float diffuseFactor;
	float specularFactor;
	bool isImposerRendering;

	inline QImage getImage()
	{
		return this->grabFramebuffer();
	}

	enum RenderMode
	{
		NONE,
        PDB,   // RCSB Protein Data Bank files
        NETCDF // Network Common Data Form files
	} renderMode;

public slots:
	void cleanup();

signals:

protected:

	void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
	void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

	void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
	void keyReleaseEvent(QKeyEvent *event) Q_DECL_OVERRIDE;

	void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;


protected slots:

	void paintGL() Q_DECL_OVERRIDE;
	void initializeGL() Q_DECL_OVERRIDE;
	void resizeGL(int w, int h) Q_DECL_OVERRIDE;
	void fileChanged(const QString &path);

private:

	void drawMolecules();

	bool loadMoleculeShader();

	void initglsw();

	void allocateGPUBuffer(int frameNr);

	void calculateFPS();

	Camera m_camera;

    size_t m_nrAtoms;
		
    // CPU atom data
    std::vector<std::vector<Atom> > *m_animation; // one atom vector for each frame
	std::vector<glm::vec3> m_pos;
	std::vector<float> m_radii;
	std::vector<glm::vec3> m_colors;
	std::vector<glm::vec3> m_ambOcc;
	
    // GPU atom data and shaders
	QOpenGLShaderProgram *m_program_molecules;
    QOpenGLVertexArrayObject m_vao_molecules; // a VAO (vertex array object) remembers states of buffer objects, allowing to easily bind/unbind different buffer states for rendering different objects in a scene.
	QOpenGLShader *m_vertexShader;
	QOpenGLShader *m_geomShader;
	QOpenGLShader *m_fragmentShader;

	QOpenGLBuffer m_vbo_pos;
	QOpenGLBuffer m_vbo_radii;
	QOpenGLBuffer m_vbo_colors;
	QOpenGLBuffer m_vbo_ambOcc;

	// ------------------------------
	
    QPoint m_lastPos; // last mouse position (to determine movement delta)

    int m_projMatrixLoc; // location of attribute in shader
	int m_mvMatrixLoc;
	
	QFileSystemWatcher *m_fileWatcher;

	int m_currentFrame;
	bool m_isPlaying;
	qint64 m_lastTime;
	QElapsedTimer m_AnimationTimer;

	// vars to measure fps
	size_t m_frameCount;
	size_t m_fps;
	qint64 m_previousTimeFPS;
	QElapsedTimer m_fpsTimer;

    // memory usage
    GLint total_mem_kb = 0;
    GLint cur_avail_mem_kb = 0;
	
	// triggers the rendering events
	QTimer mPaintTimer;

    QOpenGLDebugLogger *logger;
    void printDebugMsg(const QOpenGLDebugMessage &msg) { qDebug() << qPrintable(msg.message()); }

	MainWindow *m_MainWindow;
};

#endif
