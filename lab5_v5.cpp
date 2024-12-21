#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <random>
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>

// Константы
const int WIDTH = 800;
const int HEIGHT = 600;
const float EPSILON = 0.0001f;

// Настраиваемые параметры
struct RenderSettings {
    int maxDepth = 3;     // Уменьшаем глубину рекурсии
    int samples = 4;      // Уменьшаем количество сэмплов
    int antialiasing = 2; // Уменьшаем антиалиасинг
    bool needsUpdate = true;
    bool preview_mode = false;
    // Временные переменные для режима предпросмотра
    int temp_samples = 4;
    int temp_antialiasing = 2;
} settings;

// Структуры для работы с векторами и цветом
struct Vec3 {
    float x, y, z;
    
    Vec3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
    
    Vec3 operator+(const Vec3& v) const { return Vec3(x + v.x, y + v.y, z + v.z); }
    Vec3 operator-(const Vec3& v) const { return Vec3(x - v.x, y - v.y, z - v.z); }
    Vec3 operator*(float f) const { return Vec3(x * f, y * f, z * f); }
    Vec3 operator*(const Vec3& v) const { return Vec3(x * v.x, y * v.y, z * v.z); }
    Vec3 operator-() const { return Vec3(-x, -y, -z); }
    
    float dot(const Vec3& v) const { return x * v.x + y * v.y + z * v.z; }
    Vec3 cross(const Vec3& v) const { 
        return Vec3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x); 
    }
    
    float length() const { return std::sqrt(dot(*this)); }
    Vec3 normalize() const { return *this * (1.0f / length()); }
};

// Луч
struct Ray {
    Vec3 origin;
    Vec3 direction;
    
    Ray(const Vec3& o, const Vec3& d) : origin(o), direction(d.normalize()) {}
};

// Материал
struct Material {
    Vec3 color;
    float diffuse;
    float specular;
    float reflection;
    
    Material(const Vec3& c = Vec3(1, 1, 1), float d = 0.7f, float s = 0.3f, float r = 0.5f)
        : color(c), diffuse(d), specular(s), reflection(r) {}
};

// Базовый класс для объектов сцены
class Object {
public:
    Material material;
    
    virtual bool intersect(const Ray& ray, float& t) const = 0;
    virtual Vec3 getNormal(const Vec3& point) const = 0;
    virtual ~Object() = default;
};

// Сфера
class Sphere : public Object {
    Vec3 center;
    float radius;
    
public:
    Sphere(const Vec3& c, float r, const Material& m) : center(c), radius(r) {
        material = m;
    }
    
    bool intersect(const Ray& ray, float& t) const override {
        Vec3 oc = ray.origin - center;
        float a = ray.direction.dot(ray.direction);
        float b = 2.0f * oc.dot(ray.direction);
        float c = oc.dot(oc) - radius * radius;
        float discriminant = b * b - 4 * a * c;
        
        if (discriminant < 0) return false;
        
        float t0 = (-b - std::sqrt(discriminant)) / (2.0f * a);
        float t1 = (-b + std::sqrt(discriminant)) / (2.0f * a);
        
        if (t0 > EPSILON) {
            t = t0;
            return true;
        }
        if (t1 > EPSILON) {
            t = t1;
            return true;
        }
        return false;
    }
    
    Vec3 getNormal(const Vec3& point) const override {
        return (point - center).normalize();
    }
};

// Куб
class Cube : public Object {
    Vec3 min, max;
    
public:
    Cube(const Vec3& min_point, const Vec3& max_point, const Material& m) 
        : min(min_point), max(max_point) {
        material = m;
    }
    
    bool intersect(const Ray& ray, float& t) const override {
        Vec3 tMin = (min - ray.origin) * Vec3(1.0f / ray.direction.x, 
                                             1.0f / ray.direction.y, 
                                             1.0f / ray.direction.z);
        Vec3 tMax = (max - ray.origin) * Vec3(1.0f / ray.direction.x, 
                                             1.0f / ray.direction.y, 
                                             1.0f / ray.direction.z);
        
        Vec3 t1 = Vec3(std::min(tMin.x, tMax.x),
                       std::min(tMin.y, tMax.y),
                       std::min(tMin.z, tMax.z));
        Vec3 t2 = Vec3(std::max(tMin.x, tMax.x),
                       std::max(tMin.y, tMax.y),
                       std::max(tMin.z, tMax.z));
        
        float tNear = std::max(std::max(t1.x, t1.y), t1.z);
        float tFar = std::min(std::min(t2.x, t2.y), t2.z);
        
        if (tNear > tFar || tFar < EPSILON) return false;
        
        t = tNear > EPSILON ? tNear : tFar;
        return true;
    }
    
    Vec3 getNormal(const Vec3& point) const override {
        float dx1 = std::abs(point.x - min.x);
        float dx2 = std::abs(point.x - max.x);
        float dy1 = std::abs(point.y - min.y);
        float dy2 = std::abs(point.y - max.y);
        float dz1 = std::abs(point.z - min.z);
        float dz2 = std::abs(point.z - max.z);
        
        float minDist = std::min({dx1, dx2, dy1, dy2, dz1, dz2});
        
        if (minDist == dx1) return Vec3(-1, 0, 0);
        if (minDist == dx2) return Vec3(1, 0, 0);
        if (minDist == dy1) return Vec3(0, -1, 0);
        if (minDist == dy2) return Vec3(0, 1, 0);
        if (minDist == dz1) return Vec3(0, 0, -1);
        return Vec3(0, 0, 1);
    }
};

// Класс сцены
class Scene {
    std::vector<Object*> objects;
    std::vector<Vec3> lights;
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_real_distribution<float> dis;
    RenderSettings& settings;
    
public:
    Scene(RenderSettings& s) : gen(rd()), dis(0, 1), settings(s) {
        // Добавляем объекты в сцену
        objects.push_back(new Sphere(Vec3(0, 0, -5), 1, 
                         Material(Vec3(1, 0.2f, 0.2f), 0.7f, 0.3f, 0.5f)));
        objects.push_back(new Cube(Vec3(-2, -2, -7), Vec3(-1, -1, -6),
                         Material(Vec3(0.2f, 1, 0.2f), 0.7f, 0.3f, 0.5f)));
        
        // Добавляем источники света
        lights.push_back(Vec3(5, 5, 5));
        lights.push_back(Vec3(-5, 5, 5));
    }
    
    ~Scene() {
        for (auto obj : objects) {
            delete obj;
        }
    }
    
    Vec3 trace(const Ray& ray, int depth) {
        // Ранний выход для слабых лучей
        if (depth > 2 && dis(gen) > 0.5f) {
            return Vec3();
        }
        
        if (depth >= settings.maxDepth) return Vec3();
        
        float closest = std::numeric_limits<float>::infinity();
        const Object* hitObject = nullptr;
        float t;
        
        // Находим ближайшее пересечение
        for (const auto& obj : objects) {
            if (obj->intersect(ray, t) && t < closest) {
                closest = t;
                hitObject = obj;
            }
        }
        
        if (!hitObject) return Vec3();
        
        Vec3 hitPoint = ray.origin + ray.direction * closest;
        Vec3 normal = hitObject->getNormal(hitPoint);
        Vec3 color;
        
        // Прямое освещение
        for (const auto& light : lights) {
            Vec3 lightDir = (light - hitPoint).normalize();
            Ray shadowRay(hitPoint + normal * EPSILON, lightDir);
            bool inShadow = false;
            
            for (const auto& obj : objects) {
                if (obj->intersect(shadowRay, t)) {
                    inShadow = true;
                    break;
                }
            }
            
            if (!inShadow) {
                float diff = std::max(0.0f, normal.dot(lightDir));
                Vec3 reflection = (normal * (2.0f * normal.dot(lightDir)) - lightDir).normalize();
                float spec = std::pow(std::max(0.0f, reflection.dot(-ray.direction)), 20);
                
                color = color + hitObject->material.color * 
                        (hitObject->material.diffuse * diff + 
                         hitObject->material.specular * spec);
            }
        }
        
        // Глобальное освещение (Monte Carlo)
        if (depth < settings.maxDepth) {
            for (int i = 0; i < settings.samples; ++i) {
                Vec3 randomDir = getRandomHemisphereDirection(normal);
                Ray bounceRay(hitPoint + normal * EPSILON, randomDir);
                color = color + trace(bounceRay, depth + 1) * hitObject->material.reflection * 
                        (1.0f / settings.samples);
            }
        }
        
        return color;
    }
    
    Vec3 getRandomHemisphereDirection(const Vec3& normal) {
        float theta = 2 * M_PI * dis(gen);
        float phi = std::acos(2 * dis(gen) - 1);
        float x = std::sin(phi) * std::cos(theta);
        float y = std::sin(phi) * std::sin(theta);
        float z = std::cos(phi);
        Vec3 dir(x, y, z);
        
        if (dir.dot(normal) < 0) {
            dir = dir * -1;
        }
        
        return dir.normalize();
    }
};

int main() {
    std::cout << "Ray Tracing - Global Illumination\n";
    std::cout << "Управление:\n";
    std::cout << "↑/↓ - Изменение глубины рекурсии (количество отражений)\n";
    std::cout << "←/→ - Изменение количества сэмплов (качество освещения)\n";
    std::cout << "A/Z - Изменение уровня антиалиасинга\n";
    std::cout << "P - Переключение режима предпросмотра\n";
    std::cout << "ESC - Выход\n\n";
    
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Ray Tracing - Global Illumination");
    sf::Image image;
    image.create(WIDTH, HEIGHT);
    sf::Texture texture;
    sf::Sprite sprite;
    
    RenderSettings settings;
    Scene scene(settings);
    Vec3 camera(0, 0, 1);
    
    // Генератор случайных чисел
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0, 1);
    
    std::atomic<int> progress{0};
    const int total_pixels = WIDTH * HEIGHT;
    
    // Функция рендеринга
    auto renderScene = [&]() {
        const int num_threads = std::thread::hardware_concurrency();
        std::vector<std::thread> threads;
        std::mutex mtx;
        
        auto renderPart = [&](int start_y, int end_y) {
            for (int y = start_y; y < end_y; ++y) {
                for (int x = 0; x < WIDTH; ++x) {
                    Vec3 finalColor;
                    // Антиалиасинг через multiple sampling
                    for(int aa = 0; aa < settings.antialiasing; aa++) {
                        for(int ab = 0; ab < settings.antialiasing; ab++) {
                            float rx = dis(gen) / settings.antialiasing;
                            float ry = dis(gen) / settings.antialiasing;
                            float fx = (2.0f * (x + (aa + rx)/settings.antialiasing) - WIDTH) / HEIGHT;
                            float fy = (2.0f * (y + (ab + ry)/settings.antialiasing) - HEIGHT) / HEIGHT;
                            Vec3 direction(fx, -fy, -1);
                            
                            Ray ray(camera, direction.normalize());
                            finalColor = finalColor + scene.trace(ray, 0);
                        }
                    }
                    finalColor = finalColor * (1.0f / (settings.antialiasing * settings.antialiasing));

                    // Тональная компрессия (tone mapping) с улучшенной гамма-коррекцией
                    const float gamma = 2.2f;
                    finalColor = Vec3(
                        std::pow(std::min(1.0f, finalColor.x), 1.0f/gamma),
                        std::pow(std::min(1.0f, finalColor.y), 1.0f/gamma),
                        std::pow(std::min(1.0f, finalColor.z), 1.0f/gamma)
                    );

                    std::lock_guard<std::mutex> lock(mtx);
                    image.setPixel(x, y, 
                        sf::Color(
                            static_cast<sf::Uint8>(finalColor.x * 255),
                            static_cast<sf::Uint8>(finalColor.y * 255),
                            static_cast<sf::Uint8>(finalColor.z * 255)
                        )
                    );

                    progress++;
                    if (progress % (total_pixels / 100) == 0) {
                        std::cout << "\rПрогресс: " << (progress * 100 / total_pixels) << "%" << std::flush;
                    }
                }
            }
        };
        
        // Разделяем работу между потоками
        int rows_per_thread = HEIGHT / num_threads;
        for (int i = 0; i < num_threads; ++i) {
            int start_y = i * rows_per_thread;
            int end_y = (i == num_threads - 1) ? HEIGHT : (i + 1) * rows_per_thread;
            threads.emplace_back(renderPart, start_y, end_y);
        }
        
        // Ждем завершения всех потоков
        for (auto& thread : threads) {
            thread.join();
        }
        
        texture.loadFromImage(image);
        sprite.setTexture(texture);
        settings.needsUpdate = false;
    };
    
    // Начальный рендер
    renderScene();
    
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::KeyPressed) {
                switch (event.key.code) {
                    case sf::Keyboard::Escape:
                        window.close();
                        break;
                    case sf::Keyboard::Up:
                        settings.maxDepth = std::min(10, settings.maxDepth + 1);
                        settings.needsUpdate = true;
                        std::cout << "Глубина рекурсии: " << settings.maxDepth << std::endl;
                        break;
                    case sf::Keyboard::Down:
                        settings.maxDepth = std::max(1, settings.maxDepth - 1);
                        settings.needsUpdate = true;
                        std::cout << "Глубина рекурсии: " << settings.maxDepth << std::endl;
                        break;
                    case sf::Keyboard::Right:
                        settings.samples = std::min(16, settings.samples + 1);
                        settings.needsUpdate = true;
                        std::cout << "Количество сэмплов: " << settings.samples << std::endl;
                        break;
                    case sf::Keyboard::Left:
                        settings.samples = std::max(1, settings.samples - 1);
                        settings.needsUpdate = true;
                        std::cout << "Количество сэмплов: " << settings.samples << std::endl;
                        break;
                    case sf::Keyboard::A:
                        settings.antialiasing = std::min(8, settings.antialiasing + 1);
                        settings.needsUpdate = true;
                        std::cout << "Антиалиасинг: " << settings.antialiasing << "x" << std::endl;
                        break;
                    case sf::Keyboard::Z:
                        settings.antialiasing = std::max(1, settings.antialiasing - 1);
                        settings.needsUpdate = true;
                        std::cout << "Антиалиасинг: " << settings.antialiasing << "x" << std::endl;
                        break;
                    case sf::Keyboard::P:
                        settings.preview_mode = !settings.preview_mode;
                        if (settings.preview_mode) {
                            settings.temp_samples = settings.samples;
                            settings.temp_antialiasing = settings.antialiasing;
                            settings.samples = 2;
                            settings.antialiasing = 1;
                        } else {
                            settings.samples = settings.temp_samples;
                            settings.antialiasing = settings.temp_antialiasing;
                        }
                        settings.needsUpdate = true;
                        std::cout << (settings.preview_mode ? "Режим предпросмотра" : "Полное качество") << std::endl;
                        break;
                    default:
                        break;
                }
            }
        }
        
        if (settings.needsUpdate) {
            renderScene();
        }
        
        window.clear();
        window.draw(sprite);
        window.display();
    }
    
    return 0;
}
