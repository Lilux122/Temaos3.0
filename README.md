# Tema OS - игровая консоль для ESP32
| *> Tema OS <* | [Cat OS](https://github.com/CatDevCode/CatOs.git) |
| --- | --- |
Прошивка для портативной игровой консоли на базе ESP32 с OLED-дисплеем. 
## Особенности
- 🎮 Встроенные игры: Тетрис, Змейка, Flappy Bird, Арканоид, Ардуино дино, Астероид, Кубик
- 📶 Поддержка WiFi (STA и AP режимы)
- 📖 Файловый менеджер для LittleFS
- 🛠️ Сервисное меню с калибровкой
## Компоненты
- Микроконтроллер ESP32
- OLED дисплей 128x64 (I2C, 4 pins)
- 6 кнопок управления
## [МОЖНО ПРОШИТЬ ESP32 НА МОЁМ САЙТЕ](https://catdevcode.github.io/CatOs_webflasher/)
## Простой для DIY
1. [Схема подключения](https://wokwi.com/projects/436710693132425217)
![scheme](https://ltdfoto.ru/images/2025/08/03/clipboard-img.png)

## Библиотеки
- [GyverOled](https://github.com/GyverLibs/GyverOLED/)
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
- [AsyncTCP](https://github.com/me-no-dev/AsyncTCP)
- PS. Библиотеки для игровой консоли
## Установка
1. Установите [PlatformIO](https://platformio.org/)
```bash
pip install platformio
```
2. Клонируйте репозиторий:
```bash
git clone https://github.com/Lilux122/Temaos3.0.git
```
3. Перейдите в папку с проектом:
```bash
cd Temaos3.0-
```
4. Сбилдите проект
```bash
pio run
```
5. Загрузите проект на ESP32
```bash
pio run --target upload 
```
## Кредиты
- Спасибо [Алексу Гайверу](https://github.com/GyverLibs/) за библиотеки ❤
- Спасибо проекту [MicroReader](https://github.com/Nich1con/microReader/) за некоторые функции.
## Проект открыт для Pull-реквестов
## Сделано с любовью ❤
