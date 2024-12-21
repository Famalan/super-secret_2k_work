#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>
#include <cmath>

// Параметры вращения куба
float angleX = 0.0f;
float angleY = 0.0f;

// Позиции источников света
GLfloat light0_position[] = { 5.0f, 5.0f, 5.0f, 1.0f };
GLfloat light1_position[] = { -5.0f, -5.0f, 5.0f, 1.0f };

// Характеристики материала
GLfloat material_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
GLfloat material_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
GLfloat material_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat material_shininess[] = { 50.0f };

void init() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);

    // Настройка первого источника света
    GLfloat light0_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat light0_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat light0_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    
    glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);

    // Настройка второго источника света
    GLfloat light1_ambient[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    GLfloat light1_diffuse[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    GLfloat light1_specular[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    
    glLightfv(GL_LIGHT1, GL_POSITION, light1_position);
    glLightfv(GL_LIGHT1, GL_AMBIENT, light1_ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light1_specular);

    // Настройка материала
    glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, material_shininess);
}

void drawCube() {
    glBegin(GL_QUADS);
    
    // Передняя грань
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-1.0f, -1.0f, 1.0f);
    glVertex3f(1.0f, -1.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);

    // Задняя грань
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f(-1.0f, 1.0f, -1.0f);
    glVertex3f(1.0f, 1.0f, -1.0f);
    glVertex3f(1.0f, -1.0f, -1.0f);

    // Верхняя грань
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-1.0f, 1.0f, -1.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, -1.0f);

    // Нижняя грань
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f(1.0f, -1.0f, -1.0f);
    glVertex3f(1.0f, -1.0f, 1.0f);
    glVertex3f(-1.0f, -1.0f, 1.0f);

    // Правая грань
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(1.0f, -1.0f, -1.0f);
    glVertex3f(1.0f, 1.0f, -1.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);
    glVertex3f(1.0f, -1.0f, 1.0f);

    // Левая грань
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f(-1.0f, -1.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, -1.0f);

    glEnd();
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Настройка камеры
    glTranslatef(0.0f, 0.0f, -5.0f);

    // Вращение куба
    glRotatef(angleX, 1.0f, 0.0f, 0.0f);
    glRotatef(angleY, 0.0f, 1.0f, 0.0f);

    drawCube();
}

void update() {
    angleX += 1.0f;
    angleY += 1.0f;

    if (angleX > 360.0f) angleX -= 360.0f;
    if (angleY > 360.0f) angleY -= 360.0f;
}

void handleEvents(sf::Window& window) {
    sf::Event event;
    while (window.pollEvent(event)) {
        switch (event.type) {
            case sf::Event::Closed:
                window.close();
                break;
                
            case sf::Event::Resized: {
                glViewport(0, 0, event.size.width, event.size.height);
                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();
                float aspect = static_cast<float>(event.size.width) / event.size.height;
                float fH = tan(45.0f * 3.14159f / 360.0f) * 0.1f;
                float fW = fH * aspect;
                glFrustum(-fW, fW, -fH, fH, 0.1f, 100.0f);
                glMatrixMode(GL_MODELVIEW);
                break;
            }
                
            case sf::Event::KeyPressed:
                if (event.key.code == sf::Keyboard::Escape)
                    window.close();
                break;
                
            default:
                break;
        }
    }
}

int main() {
    // Настройки SFML
    sf::ContextSettings settings;
    settings.depthBits = 24;
    settings.stencilBits = 8;
    settings.antialiasingLevel = 4;
    settings.majorVersion = 2;
    settings.minorVersion = 1;
    
    // Создание окна
    sf::Window window(sf::VideoMode(800, 600), "3D Cube with Lighting", 
                     sf::Style::Default, settings);
    window.setVerticalSyncEnabled(true);

    // Инициализация OpenGL
    init();

    // Настройка начально��о viewport и проекции
    glViewport(0, 0, window.getSize().x, window.getSize().y);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = static_cast<float>(window.getSize().x) / window.getSize().y;
    float fH = tan(45.0f * 3.14159f / 360.0f) * 0.1f;
    float fW = fH * aspect;
    glFrustum(-fW, fW, -fH, fH, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);

    // Главный цикл
    sf::Clock clock;
    while (window.isOpen()) {
        handleEvents(window);

        // Обновление каждые 16 мс (примерно 60 FPS)
        if (clock.getElapsedTime().asMilliseconds() >= 16) {
            update();
            clock.restart();
        }

        render();
        window.display();
    }

    return 0;
}
