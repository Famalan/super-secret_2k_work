#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>
#include <chrono>

// Коды для алгоритма Коэна-Сазерленда
const int INSIDE = 0;  // 0000
const int LEFT = 1;    // 0001
const int RIGHT = 2;   // 0010
const int BOTTOM = 4;  // 0100
const int TOP = 8;     // 1000

class ClippingWindow {
private:
    sf::RectangleShape rectangle;
    bool isDragging;
    sf::Vector2f dragOffset;
    float scale;
    sf::Vector2f originalSize;

public:
    ClippingWindow(float x, float y, float width, float height) 
        : isDragging(false), scale(1.0f), originalSize(width, height) {
        rectangle.setPosition(x, y);
        rectangle.setSize(sf::Vector2f(width, height));
        rectangle.setFillColor(sf::Color::Transparent);
        rectangle.setOutlineColor(sf::Color::White);
        rectangle.setOutlineThickness(2.0f);
    }

    // Получение кода точки для алгоритма Коэна-Сазерленда
    int getPointCode(float x, float y) const {
        int code = INSIDE;
        sf::Vector2f pos = rectangle.getPosition();
        sf::Vector2f size = rectangle.getSize();

        if (x < pos.x)
            code |= LEFT;
        else if (x > pos.x + size.x)
            code |= RIGHT;
        if (y < pos.y)
            code |= TOP;
        else if (y > pos.y + size.y)
            code |= BOTTOM;

        return code;
    }

    void handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
        if (event.type == sf::Event::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                sf::Vector2f worldPos = window.mapPixelToCoords(mousePos);
                
                if (rectangle.getGlobalBounds().contains(worldPos)) {
                    isDragging = true;
                    dragOffset = worldPos - rectangle.getPosition();
                }
            }
        }
        else if (event.type == sf::Event::MouseButtonReleased) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                isDragging = false;
            }
        }
        else if (event.type == sf::Event::KeyPressed) {
            // Увеличение размера: клавиша '+'
            if (event.key.code == sf::Keyboard::Equal) {  // '+' на той же клавише что и '='
                scale *= 1.1f;
                rectangle.setSize(sf::Vector2f(originalSize.x * scale, originalSize.y * scale));
                std::cout << "Увеличение. Масштаб: " << scale << std::endl;
            }
            // Уменьшение размера: клавиша '-'
            else if (event.key.code == sf::Keyboard::Dash) {  // '-'
                scale *= 0.9f;
                rectangle.setSize(sf::Vector2f(originalSize.x * scale, originalSize.y * scale));
                std::cout << "Уменьшение. Масштаб: " << scale << std::endl;
            }
        }
    }

    void update(const sf::RenderWindow& window) {
        if (isDragging) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            sf::Vector2f worldPos = window.mapPixelToCoords(mousePos);
            rectangle.setPosition(worldPos - dragOffset);
        }
    }

    void draw(sf::RenderWindow& window) {
        window.draw(rectangle);
    }

    sf::FloatRect getBounds() const {
        return rectangle.getGlobalBounds();
    }
};

class Line {
private:
    sf::Vertex vertices[2];

public:
    Line(float x1, float y1, float x2, float y2) {
        vertices[0].position = sf::Vector2f(x1, y1);
        vertices[1].position = sf::Vector2f(x2, y2);
        vertices[0].color = sf::Color::Green;
        vertices[1].color = sf::Color::Green;
    }

    bool clipLine(const ClippingWindow& window) {
        sf::FloatRect bounds = window.getBounds();
        float x1 = vertices[0].position.x;
        float y1 = vertices[0].position.y;
        float x2 = vertices[1].position.x;
        float y2 = vertices[1].position.y;

        int code1 = window.getPointCode(x1, y1);
        int code2 = window.getPointCode(x2, y2);
        
        // Если оба конца внутри окна
        if (!(code1 | code2)) {
            vertices[0].color = sf::Color::Red;
            vertices[1].color = sf::Color::Red;
            return true;
        }
        
        // Если линия полностью снаружи
        if (code1 & code2) {
            vertices[0].color = sf::Color::Green;
            vertices[1].color = sf::Color::Green;
            return false;
        }

        // Отсечение по каждой границе
        float x = 0, y = 0;
        int codeOut = code1 ? code1 : code2;

        // Находим точку пересечения с границей
        if (codeOut & TOP) {
            x = x1 + (x2 - x1) * (bounds.top - y1) / (y2 - y1);
            y = bounds.top;
        }
        else if (codeOut & BOTTOM) {
            x = x1 + (x2 - x1) * (bounds.top + bounds.height - y1) / (y2 - y1);
            y = bounds.top + bounds.height;
        }
        else if (codeOut & RIGHT) {
            y = y1 + (y2 - y1) * (bounds.left + bounds.width - x1) / (x2 - x1);
            x = bounds.left + bounds.width;
        }
        else if (codeOut & LEFT) {
            y = y1 + (y2 - y1) * (bounds.left - x1) / (x2 - x1);
            x = bounds.left;
        }

        // Обновляем конечную точку
        if (codeOut == code1) {
            x1 = x;
            y1 = y;
        }
        else {
            x2 = x;
            y2 = y;
        }

        // Обновляем координаты и цвет
        vertices[0].position = sf::Vector2f(x1, y1);
        vertices[1].position = sf::Vector2f(x2, y2);
        vertices[0].color = sf::Color::Red;
        vertices[1].color = sf::Color::Red;
        
        return true;
    }

    void draw(sf::RenderWindow& window) {
        window.draw(vertices, 2, sf::Lines);
    }
};

int main() {
    std::cout << "Запуск программы..." << std::endl;
    
    sf::ContextSettings settings;
    settings.antialiasingLevel = 4;
    
    sf::RenderWindow window(sf::VideoMode(800, 600), "Cohen-Sutherland Line Clipping", 
                           sf::Style::Default, settings);
    
    if (!window.isOpen()) {
        std::cerr << "Ошибка создания окна!" << std::endl;
        return -1;
    }
    
    window.setFramerateLimit(60);
    
    ClippingWindow clipWindow(200, 150, 400, 300);
    std::vector<Line> lines = {
        Line(100, 100, 700, 500),
        Line(100, 500, 700, 100),
        Line(400, 50, 400, 550),
        Line(50, 300, 750, 300),
        Line(150, 150, 650, 450)
    };

    auto frameCount = 0;
    auto startTime = std::chrono::steady_clock::now();

    std::cout << "Управление:" << std::endl;
    std::cout << "- Перетаскивание окна: левая кнопка мыши" << std::endl;
    std::cout << "- Увеличение окна: клавиша '+'" << std::endl;
    std::cout << "- Уменьшение окна: клавиша '-'" << std::endl;
    std::cout << "- Выход: Escape" << std::endl;

    while (window.isOpen()) {
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count();
        
        if (elapsedTime >= 1) {
            std::cout << "FPS: " << frameCount << std::endl;
            frameCount = 0;
            startTime = currentTime;
        }

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed || 
                (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)) {
                window.close();
            }
            clipWindow.handleEvent(event, window);
        }

        clipWindow.update(window);
        window.clear(sf::Color::Black);
        clipWindow.draw(window);
        
        for (auto& line : lines) {
            line.clipLine(clipWindow);
            line.draw(window);
        }

        window.display();
        frameCount++;
        
        sf::sleep(sf::milliseconds(16));
    }

    std::cout << "Программа завершена" << std::endl;
    return 0;
}
