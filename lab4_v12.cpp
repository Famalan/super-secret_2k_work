#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>
#ifdef __APPLE__
    #include <OpenGL/glu.h>
#else
    #include <GL/glu.h>
#endif
#include <cmath>
#include <iostream>
#include <vector>

// Параметры вращения
float angleX = 0.0f;
float angleY = 0.0f;

// Параметры прожектора
struct SpotLight {
    float position[4] = {0.0f, 5.0f, 0.0f, 1.0f};
    float direction[3] = {0.0f, -1.0f, 0.0f};
    float ambient[4] = {0.2f, 0.2f, 0.2f, 1.0f};
    float diffuse[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    float specular[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    float cutoff = 45.0f;
    float exponent = 2.0f;
    
    // Коэффициенты затухания
    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;
} spotlight;

// Характеристики материала
GLfloat material_ambient[] = {0.2f, 0.2f, 0.2f, 1.0f};
GLfloat material_diffuse[] = {0.8f, 0.8f, 0.8f, 1.0f};
GLfloat material_specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat material_shininess[] = {50.0f};

// Параметры цилиндра
const int SLICES = 30;
const int STACKS = 30;
const float RADIUS = 1.0f;
const float HEIGHT = 3.0f;

// Добавим константы для размеров окна
const unsigned int WINDOW_WIDTH = 800;
const unsigned int WINDOW_HEIGHT = 600;
const std::string WINDOW_TITLE = "Cylinder with Spotlight"; // Меняем на английский заголовок

void drawCylinder() {
    std::vector<float> vertices;
    std::vector<float> normals;
    
    // Генерация вершин и нормалей для боковой поверхности
    for (int i = 0; i <= SLICES; ++i) {
        float theta = (2.0f * M_PI * i) / SLICES;
        float x = RADIUS * cos(theta);
        float z = RADIUS * sin(theta);
        
        // Нормаль для текущей точки
        float nx = cos(theta);
        float nz = sin(theta);
        
        vertices.push_back(x);
        vertices.push_back(HEIGHT/2);
        vertices.push_back(z);
        normals.push_back(nx);
        normals.push_back(0.0f);
        normals.push_back(nz);
        
        vertices.push_back(x);
        vertices.push_back(-HEIGHT/2);
        vertices.push_back(z);
        normals.push_back(nx);
        normals.push_back(0.0f);
        normals.push_back(nz);
    }
    
    glBegin(GL_TRIANGLE_STRIP);
    for (size_t i = 0; i < vertices.size()/3; ++i) {
        glNormal3f(normals[i*3], normals[i*3+1], normals[i*3+2]);
        glVertex3f(vertices[i*3], vertices[i*3+1], vertices[i*3+2]);
    }
    glEnd();
    
    // Отрисовка оснований
    for (int side = 0; side <= 1; ++side) {
        float y = (side == 0) ? -HEIGHT/2 : HEIGHT/2;
        float ny = (side == 0) ? -1.0f : 1.0f;
        
        glBegin(GL_TRIANGLE_FAN);
        glNormal3f(0.0f, ny, 0.0f);
        glVertex3f(0.0f, y, 0.0f);
        
        for (int i = 0; i <= SLICES; ++i) {
            float theta = (side == 0) ? -2.0f * M_PI * i / SLICES : 2.0f * M_PI * i / SLICES;
            float x = RADIUS * cos(theta);
            float z = RADIUS * sin(theta);
            glVertex3f(x, y, z);
        }
        glEnd();
    }
}

void init() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    
    // Настройка прожектора
    glLightfv(GL_LIGHT0, GL_POSITION, spotlight.position);
    glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, spotlight.direction);
    glLightfv(GL_LIGHT0, GL_AMBIENT, spotlight.ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, spotlight.diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, spotlight.specular);
    glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, spotlight.cutoff);
    glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, spotlight.exponent);
    
    // Настройка затухания света
    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, spotlight.constant);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, spotlight.linear);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, spotlight.quadratic);
    
    // Настройка материала
    glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, material_shininess);
}

void handleInput(sf::Window& window) {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
        }
        else if (event.type == sf::Event::KeyPressed) {
            switch (event.key.code) {
                case sf::Keyboard::Q:
                    spotlight.constant = std::max(0.0f, spotlight.constant - 0.1f);
                    break;
                case sf::Keyboard::W:
                    spotlight.constant += 0.1f;
                    break;
                case sf::Keyboard::A:
                    spotlight.linear = std::max(0.0f, spotlight.linear - 0.01f);
                    break;
                case sf::Keyboard::S:
                    spotlight.linear += 0.01f;
                    break;
                case sf::Keyboard::Z:
                    spotlight.quadratic = std::max(0.0f, spotlight.quadratic - 0.001f);
                    break;
                case sf::Keyboard::X:
                    spotlight.quadratic += 0.001f;
                    break;
                case sf::Keyboard::Escape:
                    window.close();
                    break;
                default:
                    break;
            }
            
            // Обновление параметров затухания
            glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, spotlight.constant);
            glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, spotlight.linear);
            glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, spotlight.quadratic);
        }
    }
}

void update() {
    angleX += 0.5f;
    angleY += 0.5f;
    
    if (angleX > 360.0f) angleX -= 360.0f;
    if (angleY > 360.0f) angleY -= 360.0f;
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    // Настройка камеры
    #ifdef __APPLE__
        // Для macOS используем gluLookAt
        gluLookAt(0.0f, 0.0f, 10.0f,
                  0.0f, 0.0f, 0.0f,
                  0.0f, 1.0f, 0.0f);
    #else
        // Для других платформ можно использовать альтернативный подход
        glTranslatef(0.0f, 0.0f, -10.0f);
    #endif
    
    // Обновление позиции прожектора
    glLightfv(GL_LIGHT0, GL_POSITION, spotlight.position);
    
    // Вращение сцены
    glRotatef(angleX, 1.0f, 0.0f, 0.0f);
    glRotatef(angleY, 0.0f, 1.0f, 0.0f);
    
    drawCylinder();
}

int main() {
    // Настройки SFML
    sf::ContextSettings settings;
    settings.depthBits = 24;
    settings.stencilBits = 8;
    settings.antialiasingLevel = 4;
    settings.majorVersion = 2;
    settings.minorVersion = 1;
    
    // Создание окна с английским заголовком
    sf::Window window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), 
                     WINDOW_TITLE,
                     sf::Style::Default, 
                     settings);
                     
    window.setVerticalSyncEnabled(true);
    
    // Вывод информации об управлении в консоль
    std::cout << "\nУправление программой:" << std::endl;
    std::cout << "------------------------" << std::endl;
    std::cout << "Коэффициенты затухания света:" << std::endl;
    std::cout << "Q/W - уменьшить/увеличить постоянный коэффициент затухания" << std::endl;
    std::cout << "A/S - уменьшить/увеличить линейный коэффициент затухания" << std::endl;
    std::cout << "Z/X - уменьшить/увеличить квадратичный коэффициент затухания" << std::endl;
    std::cout << "ESC - выход из программы" << std::endl;
    std::cout << "------------------------" << std::endl;
    
    // Проверяем, что окно создалось успешно
    if (!window.isOpen()) {
        std::cerr << "Failed to create window!" << std::endl;
        return -1;
    }
    
    try {
        // Инициализация OpenGL
        init();
        
        // Настройка viewport
        glViewport(0, 0, window.getSize().x, window.getSize().y);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        
        #ifdef __APPLE__
            gluPerspective(45.0f, (float)window.getSize().x/window.getSize().y, 0.1f, 100.0f);
        #else
            float aspect = (float)window.getSize().x/window.getSize().y;
            float fH = tan(45.0f * 3.14159f / 360.0f) * 0.1f;
            float fW = fH * aspect;
            glFrustum(-fW, fW, -fH, fH, 0.1f, 100.0f);
        #endif
        
        glMatrixMode(GL_MODELVIEW);
        
        // Главный цикл
        while (window.isOpen()) {
            handleInput(window);
            update();
            render();
            window.display();
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
    
    return 0;
}
