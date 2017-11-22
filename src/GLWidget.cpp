/*
* Copyright (C) 2016
* Computer Graphics Group, The Institute of Computer Graphics and Algorithms, TU Wien
* Written by Tobias Klein <tklein@cg.tuwien.ac.at>
* All rights reserved.
*/

#include "GLWidget.h"

#include <qopenglwidget.h>
#include <QMouseEvent>
#include <QDir>
#ifdef __linux__
#include <GL/glut.h>
#elif _WIN32
#include <gl/GLU.h>
#else
#error platform not supported
#endif
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "glsw.h"
#include "MainWindow.h"

const float msPerFrame = 50.0f;

#define GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX 0x9048
#define GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX 0x9049

typedef struct {
	GLuint modelViewMatrix;
	GLuint projMatrix;
	GLuint nearPlane;
	GLuint texture_AmbOccl;
	GLuint texture_ShadowMap;
	GLuint contourEnabled;
	GLuint ambientOcclusionEnabled;
	GLuint contourConstant;
	GLuint contourWidth;
	GLuint contourDepthFactor;
	GLuint ambientFactor;
	GLuint ambientIntensity;
	GLuint diffuseFactor;
	GLuint specularFactor;
	GLuint shadowModelViewMatrix;
	GLuint shadowProjMatrix;
	GLuint lightVec;
	GLuint shadowEnabled;
} ShaderUniformsMolecules;

static ShaderUniformsMolecules UniformsMolecules;



GLWidget::GLWidget(QWidget *parent, MainWindow *mainWindow)
	: QOpenGLWidget(parent)
{
	m_MainWindow = mainWindow;
	m_fileWatcher = new QFileSystemWatcher(this);
	connect(m_fileWatcher, SIGNAL(fileChanged(const QString &)), this, SLOT(fileChanged(const QString &)));


	// watch all shader of the shader folder 
	// every time a shader changes it will be recompiled on the fly
	QDir shaderDir(QCoreApplication::applicationDirPath() + "/../../src/shader/");
	QFileInfoList files = shaderDir.entryInfoList();
	qDebug() << "List of shaders:";
    foreach (QFileInfo file, files) {
		if (file.isFile()) {
			qDebug() << file.fileName();
			m_fileWatcher->addPath(file.absoluteFilePath());
		}
	}
	initglsw();

	renderMode = RenderMode::NONE;
	isImposerRendering = true;

    m_currentFrame = 0;
    m_nrAtoms = 0;

	ambientFactor = 0.05f;
	diffuseFactor = 0.5f;
	specularFactor = 0.3f;
}


GLWidget::~GLWidget()
{
    delete logger;
	glswShutdown();
}

void GLWidget::initglsw()
{
	glswInit();
	QString str = QCoreApplication::applicationDirPath() + "/../../src/shader/";
	QByteArray ba = str.toLatin1();
	const char *shader_path = ba.data();
	glswSetPath(shader_path, ".glsl");
	glswAddDirectiveToken("", "#version 330");
}

void GLWidget::cleanup()
{
	// makes the widget's rendering context the current OpenGL rendering context
	makeCurrent();
	//m_vao.destroy
	m_program_molecules = 0;
	doneCurrent();
}

void GLWidget::initializeGL()
{
	connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &GLWidget::cleanup);

	QWidget::setFocusPolicy(Qt::FocusPolicy::ClickFocus);

	initializeOpenGLFunctions();
	glClearColor(0.862f, 0.929f, 0.949f, 1.0f);
	//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

    // print glError messages
    logger = new QOpenGLDebugLogger(this);
    logger->initialize();
    connect(logger, &QOpenGLDebugLogger::messageLogged, this, &GLWidget::printDebugMsg);
    logger->startLogging();

	if (!m_vao_molecules.create()) {
		qDebug() << "error creating vao";
	}

	m_program_molecules = new QOpenGLShaderProgram();
	m_vertexShader = new QOpenGLShader(QOpenGLShader::Vertex);
	m_geomShader = new QOpenGLShader(QOpenGLShader::Geometry);
	m_fragmentShader = new QOpenGLShader(QOpenGLShader::Fragment);


    // GL_NVX_gpu_memory_info is an extension by NVIDIA
    // that provides applications visibility into GPU
    // hardware memory utilization
    GLint total_mem_kb = 0;
    GLint cur_avail_mem_kb = 0;
    glGetIntegerv(GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX, &total_mem_kb);
    glGetIntegerv(GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX, &cur_avail_mem_kb);

    float cur_avail_mem_mb = float(cur_avail_mem_kb) / 1024.0f;
    float total_mem_mb = float(total_mem_kb) / 1024.0f;
    m_MainWindow->displayTotalGPUMemory(total_mem_mb);
    m_MainWindow->displayUsedGPUMemory(0);

    // start scene update and paint timer
	connect(&mPaintTimer, SIGNAL(timeout()), this, SLOT(update()));
    mPaintTimer.start(16); // draw one frame each interval of 0.016 seconds, 1/0.016 is about 60 fps
    m_previousTimeFPS = 0;
	m_fpsTimer.start();

}




void GLWidget::initMoleculeRenderMode(std::vector<std::vector<Atom> > *animation)
{
	// makes the widget's rendering context the current OpenGL rendering context
	makeCurrent();

	m_animation = animation;
	renderMode = RenderMode::NETCDF;

    // TODO: uncomment after shader is correctly loaded
    m_program_molecules->bind();

	loadMoleculeShader();

	QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao_molecules);
	QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    // TODO: bind shader positions etc.
	qDebug() << m_program_molecules->attributeLocation("atomPos");
	qDebug() << m_program_molecules->uniformLocation("view");
	qDebug() << m_program_molecules->uniformLocation("proj");

    // TODO: uncomment after shader is correctly loaded
   m_program_molecules->release();

	allocateGPUBuffer(0);
}

void GLWidget::allocateGPUBuffer(int frameNr)
{
	// makes the widget's rendering context the current OpenGL rendering context
	makeCurrent();

    // load atoms for current frame
    m_nrAtoms = (*m_animation)[frameNr].size();
    m_pos.clear();
	m_radii.clear();
	m_colors.clear();
    m_ambOcc.clear();

    for (size_t i = 0; i < m_nrAtoms; i++) {
		Atom atom = (*m_animation)[frameNr][i];
		m_pos.push_back(atom.position);
		m_radii.push_back(atom.radius);
		m_colors.push_back(atom.color);
	}

    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao_molecules); // destructor unbinds (i.e. when out of scope)
	QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

	if (!m_program_molecules->bind()) {
		qDebug() << "Error binding shader in allocateGPUBuffer";
	}

    // TODO: allocate data to buffers (positions, radii, colors) and bind buffers to shaders
	// POSITION
	if (!m_vbo_pos.create()) {
		qDebug() << "Error creating vbo_pos";
	}
	m_vbo_pos.setUsagePattern(QOpenGLBuffer::StaticDraw);
	if (!m_vbo_pos.bind()) {
		qDebug() << "Error binding vbo_pos";
	}

	float * flat_array_pos = &m_pos[0].x;
	m_vbo_pos.allocate(flat_array_pos, 3*m_nrAtoms * sizeof(float));

	m_program_molecules->setAttributeBuffer("atomPos", GL_FLOAT, 0, 3);
	m_program_molecules->enableAttributeArray("atomPos");

	m_vbo_pos.release();

	// COLOR
	if (!m_vbo_colors.create()) {
		qDebug() << "Error creating vbo_pos";
	}
	m_vbo_colors.setUsagePattern(QOpenGLBuffer::StaticDraw);
	if (!m_vbo_colors.bind()) {
		qDebug() << "Error binding vbo_pos";
	}

	float * flat_array_color = &m_colors[0].x;
	m_vbo_colors.allocate(flat_array_color, 3 * m_nrAtoms * sizeof(float));


	m_program_molecules->setAttributeBuffer("inputColor", GL_FLOAT, 0, 3);
	m_program_molecules->enableAttributeArray("inputColor");

	m_vbo_colors.release();

	// RADIUS
	if (!m_vbo_radii.create()) {
		qDebug() << "Error creating vbo_pos";
	}
	m_vbo_radii.setUsagePattern(QOpenGLBuffer::StaticDraw);
	if (!m_vbo_radii.bind()) {
		qDebug() << "Error binding vbo_pos";
	}

	float * flat_array_radii = &m_radii[0];
	m_vbo_radii.allocate(flat_array_radii, m_nrAtoms * sizeof(float));

	m_program_molecules->setAttributeBuffer("inputRadius", GL_FLOAT, 0, 1);
	m_program_molecules->enableAttributeArray("inputRadius");

	m_vbo_radii.release();



	m_program_molecules->release();

    // display memory usage
    glGetIntegerv(GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX, &total_mem_kb);
    glGetIntegerv(GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX, &cur_avail_mem_kb);
    m_MainWindow->displayUsedGPUMemory(float(total_mem_kb - cur_avail_mem_kb) / 1024.0f);
}

bool GLWidget::loadMoleculeShader()
{
    bool success = false;

    // TODO: This has been commented out since shader is merely a placeholder

	const char *vs = glswGetShader("molecules.Vertex");
	success = m_vertexShader->compileSourceCode(vs);

	const char *gs = glswGetShader("molecules.Geometry");
	success = m_geomShader->compileSourceCode(gs);

	const char *fs = glswGetShader("molecules.Fragment");
	success = m_fragmentShader->compileSourceCode(fs);

	m_program_molecules->addShader(m_vertexShader);
	m_program_molecules->addShader(m_geomShader);
	m_program_molecules->addShader(m_fragmentShader);

	bool result = m_program_molecules->link();

	if (!result)
		qDebug() << "Could not link shader program:" << m_program_molecules->log();

    return success;
}


void GLWidget::paintGL()
{
	calculateFPS();
	switch (renderMode) {
        case(RenderMode::NONE):
            break; // do nothing
        case(RenderMode::PDB):
            break; // optional
        case(RenderMode::NETCDF):
            drawMolecules();
            break;
        default:
            break;

	}
}


void GLWidget::drawMolecules()
{
    QOpenGLFunctions *glf = QOpenGLContext::currentContext()->functions();

	// animate frames
	if (m_isPlaying) {
		qint64 elapsed = m_AnimationTimer.elapsed() - m_lastTime;

		elapsed -= msPerFrame;
		while (elapsed > 0) {
			m_currentFrame++;
			m_lastTime = m_AnimationTimer.elapsed();
			elapsed -= msPerFrame;

		}
		if (m_currentFrame >= (*m_animation).size()) {
			m_currentFrame = (*m_animation).size() - 1;
			m_isPlaying = false;
		}

		m_MainWindow->setAnimationFrameGUI(m_currentFrame);
		allocateGPUBuffer(m_currentFrame);
	}

	if (isImposerRendering) {

        // TODO: implement

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glClearDepth(1.0f);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		GLfloat light_position[] = { 0.0f, 0.0f, 100.0f };

        // bind vertex array object and shader program
		QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao_molecules); // destructor unbinds (i.e. when out of scope)
		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

		m_program_molecules->bind();

		GLfloat m_viewport[4];
		glGetFloatv(GL_VIEWPORT, m_viewport);

        // set shader uniforms
		int viewMatrixId = m_program_molecules->uniformLocation("view");
		glUniformMatrix4fv(viewMatrixId, 1, GL_FALSE, glm::value_ptr(m_camera.getViewMatrix()));

		int projMatrixId = m_program_molecules->uniformLocation("proj");
		glUniformMatrix4fv(projMatrixId, 1, GL_FALSE, glm::value_ptr(m_camera.getProjectionMatrix()));

		int lightPosId = m_program_molecules->uniformLocation("lightPos");
		glUniform1fv(lightPosId, 1, light_position);

		int nearId = m_program_molecules->uniformLocation("near");
		glUniform1f(nearId, m_camera.getNearPlane());

		int farId = m_program_molecules->uniformLocation("far");
		glUniform1f(farId, m_camera.getFarPlane());

		int ambientId = m_program_molecules->uniformLocation("ambient");
		glUniform1f(ambientId, ambientFactor);

		int diffuseId = m_program_molecules->uniformLocation("diffuse");
		glUniform1f(diffuseId, diffuseFactor);

		int specularId = m_program_molecules->uniformLocation("specular");
		glUniform1f(specularId, specularFactor);

		int screenWidthId = m_program_molecules->uniformLocation("screenWidth");
		glUniform1f(screenWidthId, m_viewport[2]);

		int screenHeightId = m_program_molecules->uniformLocation("screenHeight");
		glUniform1f(screenHeightId, m_viewport[3]);


		// draw call
		glDrawArrays(GL_POINTS, 0, m_nrAtoms);

		m_program_molecules->release();



	}
	else {

        // simplistic implementation using OpenGL fixed function pipeline

        size_t m_nrAtoms = (*m_animation)[m_currentFrame].size();

        // setup light source and material

		GLfloat light_ambient[] = { ambientFactor,ambientFactor,ambientFactor, 1.0f };
		GLfloat light_diffuse[] = { diffuseFactor, diffuseFactor, diffuseFactor, 1.0f };
		GLfloat light_specular[] = { specularFactor, specularFactor, specularFactor, 1.0f };

        GLfloat light_position[] = { 0.0f, 0.0f, 100.0f, 1.0f };
		
        glLoadIdentity(); // replace the current transformation matrix of the GL state machine with the identity matrix
		glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glClearDepth(1.0f);

        glEnable(GL_LIGHTING);
		glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
		glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
		glEnable(GL_LIGHT0);

		glEnable(GL_COLOR_MATERIAL);
		glColorMaterial(GL_FRONT, GL_AMBIENT);
		glColorMaterial(GL_FRONT, GL_DIFFUSE);
		glColorMaterial(GL_FRONT, GL_SPECULAR);
		glMaterialf(GL_FRONT, GL_SHININESS, 128.0f);

        // transformations

		glLoadIdentity();
		GLenum er = glGetError();
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(glm::value_ptr(m_camera.getProjectionMatrix()));
		glMatrixMode(GL_MODELVIEW);
		glMultMatrixf(glm::value_ptr(m_camera.getViewMatrix()));

		glLightfv(GL_LIGHT0, GL_POSITION, light_position);

        // draw actual spheres (not imposters)

		er = glGetError();
		//for (size_t i = 0; i < 1; i++) {
        for (size_t i = 0; i < m_nrAtoms; i++) { //
			Atom atom = (*m_animation)[m_currentFrame][i];
			glPushMatrix();
            GLUquadric *quadric; // object to draw quadrics (surfaces described by second degree equation, e.g. ellipsoids like spheres)
            quadric = gluNewQuadric();
			//set color and position
			glColor4f(atom.color.r, atom.color.g, atom.color.b, 1);
			glTranslatef(atom.position.x, atom.position.y, atom.position.z);
			er = glGetError();
            gluSphere(quadric, atom.radius, 40, 40); // 40 vertical (polar angle) and horizontal (azimuthal angle) samples of the quadric function
			er = glGetError();
			glPopMatrix();
            gluDeleteQuadric(quadric);
		}
	}


}

void GLWidget::calculateFPS()
{
	m_frameCount++;

    // calculate time passed since last frame
    qint64 currentTime = m_fpsTimer.elapsed(); // in milliseconds
    qint64 timeInterval = currentTime - m_previousTimeFPS;

    // when timeInterval reaches one second
    if (timeInterval > ((qint64)1000)) {
		// calculate the number of frames per second
		m_fps = m_frameCount / (timeInterval / 1000.0f);

		m_previousTimeFPS = currentTime;
		m_frameCount = 0;
	}

	m_MainWindow->displayFPS(m_fps);
}



void GLWidget::resizeGL(int w, int h)
{
	m_camera.setAspect(float(w) / h);
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
	m_lastPos = event->pos();
}

void GLWidget::wheelEvent(QWheelEvent *event)
{
	m_camera.zoom(event->delta() / 30);
	update();
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
	int dx = event->x() - m_lastPos.x();
	int dy = event->y() - m_lastPos.y();

    // rotate camera
	if (event->buttons() & Qt::LeftButton) {
		m_camera.rotateAzimuth(dx / 100.0f);
		m_camera.rotatePolar(dy / 100.0f);
	}

	if (event->buttons() & Qt::RightButton) {
		m_camera.rotateAzimuth(dx / 100.0f);
		m_camera.rotatePolar(dy / 100.0f);
	}
	m_lastPos = event->pos();
	update();
}

void GLWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
        case Qt::Key_Space:
        {
            // TODO: play/pause animation
            break;
        }
        default:
        {
            event->ignore();
            break;
        }
    }
}

void GLWidget::keyReleaseEvent(QKeyEvent *event)
{

}

void GLWidget::fileChanged(const QString &path)
{
	// reboot glsw, otherwise it will use the old cached shader
	glswShutdown();
	initglsw();

	loadMoleculeShader();
	update();
}

void GLWidget::playAnimation()
{
	m_AnimationTimer.start();
	m_lastTime = 0;
	m_isPlaying = true;
}

void GLWidget::pauseAnimation()
{
	m_isPlaying = false;
}

void GLWidget::setAnimationFrame(int frameNr)
{
	m_currentFrame = frameNr;
	allocateGPUBuffer(frameNr);
}
