#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>
#include <chrono>

// Коды для алгоритма Коэна–Сазерленда
const int INSIDE = 0;  // 0000
const int LEFT   = 1;  // 0001
const int RIGHT  = 2;  // 0010
const int BOTTOM = 4;  // 0100
const int TOP    = 8;  // 1000

//------------------------------------------------------------------------------
// Класс ClippingWindow — окно для отсечения
//------------------------------------------------------------------------------
class ClippingWindow {
private:
    sf::RectangleShape rectangle;
    bool isDragging;         // Флаг, указывающий, перетаскивается ли сейчас окно
    sf::Vector2f dragOffset; // Вектор смещения при «захвате» окна мышью
    float scale;             // Текущий масштаб окна
    sf::Vector2f originalSize; // Исходные размеры окна

public:
    ClippingWindow(float x, float y, float width, float height) 
        : isDragging(false)
        , scale(1.0f)
        , originalSize(width, height)
    {
        rectangle.setPosition(x, y);
        rectangle.setSize(sf::Vector2f(width, height));
        rectangle.setFillColor(sf::Color::Transparent);
        rectangle.setOutlineColor(sf::Color::White);
        rectangle.setOutlineThickness(2.0f);
    }

    // Возвращает код положения точки (x, y) относительно окна отсечения
    // согласно битовым маскам алгоритма Коэна–Сазерленда.
    int getPointCode(float x, float y) const {
        int code = INSIDE;
        sf::Vector2f pos = rectangle.getPosition();
        sf::Vector2f size = rectangle.getSize();

        // Проверяем положение точки по оси X
        if (x < pos.x)
            code |= LEFT;
        else if (x > pos.x + size.x)
            code |= RIGHT;

        // Проверяем положение точки по оси Y
        if (y < pos.y)
            code |= TOP;           // В SFML ось Y направлена вниз, поэтому "меньше" — это "выше"
        else if (y > pos.y + size.y)
            code |= BOTTOM;

        return code;
    }

    // Обработка событий (мышь, клавиатура)
    void handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
        // Нажатие кнопки мыши
        if (event.type == sf::Event::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                sf::Vector2f worldPos = window.mapPixelToCoords(mousePos);
                
                // Если щёлкнули по прямоугольнику — начинаем перетаскивание
                if (rectangle.getGlobalBounds().contains(worldPos)) {
                    isDragging = true;
                    dragOffset = worldPos - rectangle.getPosition();
                }
            }
        }
        // Отпускание кнопки мыши
        else if (event.type == sf::Event::MouseButtonReleased) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                isDragging = false;
            }
        }
        // Обработка нажатий на клавиши (масштаб окна)
        else if (event.type == sf::Event::KeyPressed) {
            // Увеличение размера: клавиша '+'
            if (event.key.code == sf::Keyboard::Equal) {  
                scale *= 1.1f;
                rectangle.setSize(sf::Vector2f(originalSize.x * scale, 
                                               originalSize.y * scale));
                std::cout << "Увеличение. Масштаб: " << scale << std::endl;
            }
            // Уменьшение размера: клавиша '-'
            else if (event.key.code == sf::Keyboard::Dash) {  
                scale *= 0.9f;
                rectangle.setSize(sf::Vector2f(originalSize.x * scale, 
                                               originalSize.y * scale));
                std::cout << "Уменьшение. Масштаб: " << scale << std::endl;
            }
        }
    }

    // Обновляет позицию окна при перетаскивании
    void update(const sf::RenderWindow& window) {
        if (isDragging) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            sf::Vector2f worldPos = window.mapPixelToCoords(mousePos);
            rectangle.setPosition(worldPos - dragOffset);
        }
    }

    // Рисует окно на экране
    void draw(sf::RenderWindow& window) {
        window.draw(rectangle);
    }

    // Возвращаем границы окна (в глобальных координатах)
    sf::FloatRect getBounds() const {
        return rectangle.getGlobalBounds();
    }
};

//------------------------------------------------------------------------------
// Класс Line — отрезок, который мы будем пытаться отсечь
//------------------------------------------------------------------------------
class Line {
private:
    // Исходные (неизменные) вершины
    sf::Vector2f originalP1;
    sf::Vector2f originalP2;

public:
    Line(float x1, float y1, float x2, float y2) 
        : originalP1(x1, y1)
        , originalP2(x2, y2)
    {
    }

    // Алгоритм Коэна–Сазерленда для отсечения. Возвращает:
    //   - true,  если отрезок после отсечения видим (полностью или частично);
    //   - false, если полностью за границами окна.
    //
    // Параметры x1, y1, x2, y2 могут изменяться (укорачиваться).
    bool cohenSutherlandClip(const ClippingWindow& window,
                             float& x1, float& y1, float& x2, float& y2) const
    {
        // Получаем коды концов отрезка
        int code1 = window.getPointCode(x1, y1);
        int code2 = window.getPointCode(x2, y2);

        bool accept = false;

        while (true) {
            // (1) Проверка: оба конца внутри (code1 | code2 == 0)
            if ((code1 | code2) == 0) {
                // Отрезок целиком внутри окна
                accept = true;
                break;
            }
            // (2) Проверка: оба конца с одной «внешней» стороны (code1 & code2 != 0)
            else if (code1 & code2) {
                // Отрезок точно вне окна
                break;
            }
            else {
                // (3) Частично внутри, частично снаружи: отсечение
                int codeOut = (code1 != 0) ? code1 : code2;

                sf::FloatRect bounds = window.getBounds();
                float x = 0.f, y = 0.f;
                
                // Вычисляем пересечение с одной из границ
                if (codeOut & TOP) {
                    // y = bounds.top
                    // Поскольку bounds.top = pos.y верхней стороны окна
                    //   (x2 - x1) * (bounds.top - y1) / (y2 - y1)
                    float newY = bounds.top;
                    float newX = x1 + (x2 - x1) * (newY - y1) / (y2 - y1);
                    x = newX;
                    y = newY;
                }
                else if (codeOut & BOTTOM) {
                    float newY = bounds.top + bounds.height;
                    float newX = x1 + (x2 - x1) * (newY - y1) / (y2 - y1);
                    x = newX;
                    y = newY;
                }
                else if (codeOut & RIGHT) {
                    float newX = bounds.left + bounds.width;
                    float newY = y1 + (y2 - y1) * (newX - x1) / (x2 - x1);
                    x = newX;
                    y = newY;
                }
                else if (codeOut & LEFT) {
                    float newX = bounds.left;
                    float newY = y1 + (y2 - y1) * (newX - x1) / (x2 - x1);
                    x = newX;
                    y = newY;
                }

                // Перезаписываем координаты соответствующей вершины,
                // пересчитываем её код
                if (codeOut == code1) {
                    x1 = x;
                    y1 = y;
                    code1 = window.getPointCode(x1, y1);
                }
                else {
                    x2 = x;
                    y2 = y;
                    code2 = window.getPointCode(x2, y2);
                }
            }
        } // while (true)

        return accept;
    }

    // Рисуем линию: если часть отрезка видна внутри окна, рисуем её красным;
    // в противном случае — рисуем исходный отрезок (или ничего) зелёным.
    void draw(sf::RenderWindow& window, const ClippingWindow& clipWindow) {
        // Сначала берём исходные координаты (чтобы не затирать их окончательно)
        float x1 = originalP1.x;
        float y1 = originalP1.y;
        float x2 = originalP2.x;
        float y2 = originalP2.y;

        // Пытаемся отсечь
        bool isVisible = cohenSutherlandClip(clipWindow, x1, y1, x2, y2);

        if (isVisible) {
            // Если видима хотя бы часть — рисуем "обрезанный" отрезок красным
            sf::Vertex line[2];
            line[0] = sf::Vertex(sf::Vector2f(x1, y1), sf::Color::Red);
            line[1] = sf::Vertex(sf::Vector2f(x2, y2), sf::Color::Red);
            window.draw(line, 2, sf::Lines);
        } else {
            // Иначе можно либо не рисовать отрезок вовсе,
            // либо нарисовать исходный целиком зелёным:
            sf::Vertex line[2];
            line[0] = sf::Vertex(originalP1, sf::Color::Green);
            line[1] = sf::Vertex(originalP2, sf::Color::Green);
            window.draw(line, 2, sf::Lines);
        }
    }
};

//------------------------------------------------------------------------------
// Основная функция
//------------------------------------------------------------------------------
int main() {
    std::cout << "Запуск программы..." << std::endl;
    
    sf::ContextSettings settings;
    settings.antialiasingLevel = 4;
    
    sf::RenderWindow window(sf::VideoMode(800, 600), 
                            "Cohen-Sutherland Line Clipping",
                            sf::Style::Default, 
                            settings);
    
    if (!window.isOpen()) {
        std::cerr << "Ошибка создания окна!" << std::endl;
        return -1;
    }
    
    window.setFramerateLimit(60);
    
    // Создаём окно отсечения
    ClippingWindow clipWindow(200, 150, 400, 300);

    // Создаём несколько тестовых отрезков
    std::vector<Line> lines;
    lines.push_back(Line(100.f, 100.f, 700.f, 500.f));
    lines.push_back(Line(100.f, 500.f, 700.f, 100.f));
    lines.push_back(Line(400.f,  50.f, 400.f, 550.f));
    lines.push_back(Line( 50.f, 300.f, 750.f, 300.f));
    lines.push_back(Line(150.f, 150.f, 650.f, 450.f));

    auto frameCount = 0u;
    auto startTime = std::chrono::steady_clock::now();

    // Инструкции по управлению
    std::cout << "Управление:" << std::endl;
    std::cout << " - Перетаскивание окна: левая кнопка мыши" << std::endl;
    std::cout << " - Увеличение окна: клавиша '+'" << std::endl;
    std::cout << " - Уменьшение окна: клавиша '-'" << std::endl;
    std::cout << " - Выход: Escape" << std::endl;

    // Основной цикл приложения
    while (window.isOpen()) {
        // Подсчёт FPS (примерно раз в секунду)
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count();
        if (elapsedTime >= 1) {
            std::cout << "FPS: " << frameCount << std::endl;
            frameCount = 0;
            startTime = currentTime;
        }

        // Обработка событий
        sf::Event event;
        while (window.pollEvent(event)) {
            // Закрытие окна или нажатие Escape
            if (event.type == sf::Event::Closed || 
               (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape))
            {
                window.close();
            }
            clipWindow.handleEvent(event, window);
        }

        // Обновляем окно отсечения (на случай перетаскивания)
        clipWindow.update(window);

        // Очищаем экран
        window.clear(sf::Color::Black);

        // Сначала рисуем само окно отсечения
        clipWindow.draw(window);
        
        // Затем рисуем все линии с учётом отсечения
        for (auto& line : lines) {
            line.draw(window, clipWindow);
        }

        // Выводим итоговое изображение на экран
        window.display();
        ++frameCount;

        // Небольшая пауза (примерно 60 FPS)
        sf::sleep(sf::milliseconds(16));
    }

    std::cout << "Программа завершена" << std::endl;
    return 0;
}
