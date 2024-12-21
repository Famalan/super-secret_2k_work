#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>
#include <cmath>
#include <vector>

// Константы для камеры
const float PI = 3.14159265359f;
const float MIN_DISTANCE = 2.0f;
const float MAX_DISTANCE = 10.0f;
const float MIN_PITCH = -89.0f;
const float MAX_PITCH = 89.0f;

// Параметры камеры
struct Camera {
    float distance = 5.0f;    // Расстояние от камеры до центра сферы
    float yaw = 0.0f;        // Угол поворота вокруг оси Y (в градусах)
    float pitch = 0.0f;      // Угол наклона (в градусах)
    float lastX = 0.0f;      // Последняя X-координата мыши
    float lastY = 0.0f;      // Последняя Y-координата мыши
    bool firstMouse = true;  // Флаг первого движения мыши
};

// Функция для создания вершин сферы
std::vector<float> createSphereVertices(float radius, int segments) {
    std::vector<float> vertices;
    
    for(int i = 0; i <= segments; i++) {
        float lat0 = PI * (-0.5f + (float)(i - 1) / segments);
        float lat1 = PI * (-0.5f + (float)i / segments);
        float y0 = radius * sin(lat0);
        float y1 = radius * sin(lat1);
        float r0 = radius * cos(lat0);
        float r1 = radius * cos(lat1);

        for(int j = 0; j <= segments; j++) {
            float lng = 2 * PI * (float)j / segments;
            float x = cos(lng);
            float z = sin(lng);

            // Первый треугольник
            vertices.push_back(r0 * x);
            vertices.push_back(y0);
            vertices.push_back(r0 * z);

            vertices.push_back(r1 * x);
            vertices.push_back(y1);
            vertices.push_back(r1 * z);
        }
    }
    
    return vertices;
}

// Функция для отрисовки сферы
void drawSphere(const std::vector<float>& vertices) {
    glBegin(GL_LINE_STRIP);
    for(size_t i = 0; i < vertices.size(); i += 3) {
        glVertex3f(vertices[i], vertices[i + 1], vertices[i + 2]);
    }
    glEnd();
}

// Обработчик движения мыши
void handleMouseMove(Camera& camera, float xpos, float ypos) {
    if (camera.firstMouse) {
        camera.lastX = xpos;
        camera.lastY = ypos;
        camera.firstMouse = false;
        return;
    }

    float xoffset = xpos - camera.lastX;
    float yoffset = camera.lastY - ypos;
    camera.lastX = xpos;
    camera.lastY = ypos;

    const float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    camera.yaw += xoffset;
    camera.pitch += yoffset;

    // Ограничение угла наклона
    if (camera.pitch > MAX_PITCH) camera.pitch = MAX_PITCH;
    if (camera.pitch < MIN_PITCH) camera.pitch = MIN_PITCH;
}

int main() {
    // Создание окна SFML
    sf::ContextSettings settings;
    settings.depthBits = 24;
    settings.stencilBits = 8;
    settings.antialiasingLevel = 4;
    settings.majorVersion = 2;
    settings.minorVersion = 1;
    
    sf::Window window(sf::VideoMode(800, 600), "Orbital Camera", sf::Style::Default, settings);
    window.setVerticalSyncEnabled(true);

    // Инициализация OpenGL
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    
    // Создание вершин сферы
    std::vector<float> sphereVertices = createSphereVertices(1.0f, 32);
    Camera camera;

    // Настройка начального viewport и проекции
    glViewport(0, 0, window.getSize().x, window.getSize().y);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = static_cast<float>(window.getSize().x) / window.getSize().y;
    float fH = tan(45.0f * PI / 360.0f) * 0.1f;
    float fW = fH * aspect;
    glFrustum(-fW, fW, -fH, fH, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            switch (event.type) {
                case sf::Event::Closed:
                    window.close();
                    break;
                    
                case sf::Event::Resized:
                    glViewport(0, 0, event.size.width, event.size.height);
                    glMatrixMode(GL_PROJECTION);
                    glLoadIdentity();
                    aspect = static_cast<float>(event.size.width) / event.size.height;
                    fH = tan(45.0f * PI / 360.0f) * 0.1f;
                    fW = fH * aspect;
                    glFrustum(-fW, fW, -fH, fH, 0.1f, 100.0f);
                    glMatrixMode(GL_MODELVIEW);
                    break;
                    
                case sf::Event::MouseMoved:
                    handleMouseMove(camera, event.mouseMove.x, event.mouseMove.y);
                    break;
                    
                case sf::Event::MouseWheelScrolled:
                    camera.distance -= event.mouseWheelScroll.delta * 0.5f;
                    camera.distance = std::max(MIN_DISTANCE, std::min(MAX_DISTANCE, camera.distance));
                    break;
                    
                case sf::Event::KeyPressed:
                    if (event.key.code == sf::Keyboard::Escape)
                        window.close();
                    break;
                    
                default:
                    break;
            }
        }

        // Очистка буферов
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();

        // Вычисление позиции камеры
        float x = camera.distance * cos(camera.pitch * PI / 180.0f) * cos(camera.yaw * PI / 180.0f);
        float y = camera.distance * sin(camera.pitch * PI / 180.0f);
        float z = camera.distance * cos(camera.pitch * PI / 180.0f) * sin(camera.yaw * PI / 180.0f);

        // Установка камеры
        glLoadIdentity();
        glTranslatef(0.0f, 0.0f, -camera.distance);
        glRotatef(camera.pitch, 1.0f, 0.0f, 0.0f);
        glRotatef(camera.yaw, 0.0f, 1.0f, 0.0f);

        // Отрисовка сферы
        drawSphere(sphereVertices);

        // Отображение кадра
        window.display();
    }

    return 0;
}
