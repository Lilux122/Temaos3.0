# Tema OS - игровая консоль для ESP32
| *> Tema OS <* | [Tema OS](https://github.com/Lilux122/Temaos3.0-.git) |
| --- | --- |
Прошивка для портативной игровой консоли на базе ESP32 с OLED-дисплеем. 
## Особенности
- 🎮 Встроенные игры: Тетрис, Змейка, Flappy Bird, Арканоид, Ардуино дино, Астероид(пока только тетрис змейка арканоид и ардуино дино)
- ⚙️ Системные настройки через веб-интерфейс
- 📶 Поддержка WiFi (STA и AP режимы)
- 📖 Файловый менеджер для LittleFS
- 🛠️ Сервисное меню с калибровкой
## Компоненты
- Микроконтроллер ESP32
- OLED дисплей 128x64 (I2C, 4 pins)
- 6 кнопок управления
- Литий-ионный аккумулятор
## [МОЖНО ПРОШИТЬ ESP32 НА МОЁМ САЙТЕ](https://catdevcode.github.io/CatOs_webflasher/)
## Простой для DIY
1. [Схема подключения](https://wokwi.com/projects/436710693132425217)
![scheme](https://ltdfoto.ru/images/2025/08/03/clipboard-img.png)
2. Схема питания
![scheme_bat](https://github.com/CatDevCode/CatOs/blob/main/assets/bat_lite.png)
> [!TIP]
> Резисторы на 100 kOm
## Создание изображений и загрузка
1. Запустите [imageProcessor.exe](https://github.com/AlexGyver/imageProcessor) (установите java)
![IMG1](https://github.com/CatDevCode/CatOs/blob/main/assets/img1.png)
2. Откройте изображение
![IMG2](https://github.com/CatDevCode/CatOs/blob/main/assets/img2.png)
3. Настройте размер и порог изображения для получения лучшего результата
![IMG3](https://github.com/CatDevCode/CatOs/blob/main/assets/img3.png)
4. Сделайте инверсию цвета (белый цвет будет отображаться на экране). И убедитесь что Result height и Result width стоят также как на изображении
![IMG4](https://github.com/CatDevCode/CatOs/blob/main/assets/img4.png)
5. Сохраните файл нажав SAVE, в папке image-processor появится файл .h . Также можно переименовать этот файл.
![IMG5](https://github.com/CatDevCode/CatOs/blob/main/assets/img5.png)
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
