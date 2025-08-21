# 🎮 Tema OS - Игровая консоль для ESP32

<div align="center">

![Tema OS Logo](https://img.shields.io/badge/Tema%20OS-v3.5-brightgreen?style=for-the-badge&logo=arduino)

[![ESP32](https://img.shields.io/badge/ESP32-Compatible-red?style=flat-square&logo=espressif)](https://www.espressif.com/)
[![PlatformIO](https://img.shields.io/badge/PlatformIO-Ready-blue?style=flat-square&logo=platformio)](https://platformio.org/)
[![License](https://img.shields.io/badge/License-MIT-yellow?style=flat-square)](LICENSE)
[![Stars](https://img.shields.io/github/stars/Lilux122/Temaos3.0?style=flat-square&color=orange)](https://github.com/Lilux122/Temaos3.0/stargazers)
[![Forks](https://img.shields.io/github/forks/Lilux122/Temaos3.0?style=flat-square&color=purple)](https://github.com/Lilux122/Temaos3.0/network/members)

**Прошивка для портативной игровой консоли на базе ESP32 с OLED-дисплеем**

[📱 Попробовать онлайн](https://catdevcode.github.io/CatOs_webflasher/) • [📖 Документация](https://github.com/Lilux122/Temaos3.0/wiki) • [🐛 Сообщить об ошибке](https://github.com/Lilux122/Temaos3.0/issues)

</div>

---

## ✨ Особенности

<table>
<tr>
<td width="50%">

### 🎮 Встроенные игры
- 🧩 **Тетрис** - Классическая головоломка
- 🐍 **Змейка** - Ретро аркада  
- 🐦 **Flappy Bird** - Популярная мобильная игра
- 🏓 **Арканоид** - Разбивайте блоки
- 🦕 **Дино** - Игра из Chrome
- 🚀 **Астероиды** - Космический шутер
- 🎲 **Кубик** - Простая логическая игра

</td>
<td width="50%">

### 🛠️ Системные функции
- 📶 **WiFi поддержка** (STA и AP режимы)
- 📁 **Файловый менеджер** для LittleFS
- ⚙️ **Сервисное меню** с калибровкой

</td>
</tr>
</table>

---

## 🔧 Компоненты

<div align="center">

| Компонент | Описание | Количество |
|-----------|----------|------------|
| 🧠 **ESP32** | Микроконтроллер | 1 шт |
| 📺 **OLED 128x64** | Дисплей (I2C, 4 пина) | 1 шт |
| 🎮 **Кнопки** | Управление | 6 шт |

</div>

---

## 🚀 Быстрый старт


### 🛠️ Сборка из исходников

<details>
<summary><b>📋 Подробная инструкция</b></summary>

#### 1️⃣ Установите PlatformIO
```bash
pip install platformio
```

#### 2️⃣ Клонируйте репозиторий
```bash
git clone https://github.com/Lilux122/Temaos3.0.git
cd Temaos3.0
```

#### 3️⃣ Соберите проект
```bash
pio run
```

#### 4️⃣ Загрузите на ESP32
```bash
pio run --target upload
```

</details>

---

## 📋 Схема подключения

<div align="center">

[![Схема Wokwi](https://img.shields.io/badge/📐%20Посмотреть%20схему-на%20Wokwi-blue?style=for-the-badge&logo=wokwi)](https://wokwi.com/projects/436710693132425217)

<img src="https://ltdfoto.ru/images/2025/08/03/clipboard-img.png" alt="Схема подключения" width="600">

*Простая схема для самостоятельной сборки*

</div>

---

## 📚 Зависимости

<div align="center">

| Библиотека | Версия | Описание |
|------------|---------|----------|
| [**GyverOled**](https://github.com/GyverLibs/GyverOLED/) | ![Version](https://img.shields.io/github/v/release/GyverLibs/GyverOLED?style=flat-square) | Управление OLED дисплеем |
| [**ArduinoJson**](https://github.com/bblanchon/ArduinoJson) | ![Version](https://img.shields.io/github/v/release/bblanchon/ArduinoJson?style=flat-square) | JSON парсер |
| [**AsyncTCP**](https://github.com/me-no-dev/AsyncTCP) | ![Version](https://img.shields.io/github/v/release/me-no-dev/AsyncTCP?style=flat-square) | Асинхронная TCP библиотека |

</div>


## 🤝 Участие в проекте

<div align="center">

**Проект открыт для вклада! 🚀**

[![Contribute](https://img.shields.io/badge/🛠️%20Contribute-Welcome-success?style=for-the-badge)](https://github.com/Lilux122/Temaos3.0/pulls)
[![Issues](https://img.shields.io/badge/🐛%20Issues-Welcome-red?style=for-the-badge)](https://github.com/Lilux122/Temaos3.0/issues)

</div>

### Как помочь:
1. 🍴 Сделайте Fork репозитория
2. 🌿 Создайте ветку для новой функции
3. 💻 Внесите изменения
4. ✅ Создайте Pull Request

---

## 📄 Лицензия

Этот проект распространяется под лицензией MIT. Подробности в файле [LICENSE](LICENSE).

---

## 🙏 Благодарности

<div align="center">

### Особая благодарность:

[![AlexGyver](https://img.shields.io/badge/💜%20Алекс%20Гайвер-За%20библиотеки-purple?style=for-the-badge)](https://github.com/GyverLibs/)

[![MicroReader](https://img.shields.io/badge/🔧%20MicroReader-За%20функции-blue?style=for-the-badge)](https://github.com/Nich1con/microReader/)

[![Catos](https://img.shields.io/badge/🔧%20Сatos-За%20функции-blue?style=for-the-badge)](https://github.com/CatDevCode/CatOs.git)


[![Catoslite](https://img.shields.io/badge/🔧%20Сatoslite-За%20функции-blue?style=for-the-badge)](https://github.com/CatDevCode/CatOs_Lite.git)

</div>

---

<div align="center">

### 💖 Сделано с любовью

[![Tema OS](https://img.shields.io/badge/Tema%20OS-2025-red?style=for-the-badge&logo=heart)](https://github.com/Lilux122/Temaos3.0)

**⭐ Если проект понравился - поставьте звездочку!**

---

*Связанный проект: [Cat OS](https://github.com/CatDevCode/CatOs.git)*

</div>
