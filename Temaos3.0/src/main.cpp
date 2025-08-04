#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <GyverOLED.h>
#include <GyverButton.h>
#include <GyverTimer.h>
#include <Wire.h>

// Настройки пинов
#define UP_BTN_PIN 19
#define DOWN_BTN_PIN 17
#define RIGHT_BTN_PIN 18
#define LEFT_BTN_PIN 22
#define SELECT_BTN_PIN 27
#define EXIT_BTN_PIN 14

// Инициализация дисплея и кнопок
GyverOLED<SSD1306_128x64, OLED_BUFFER> oled;
GButton upBtn(UP_BTN_PIN);
GButton downBtn(DOWN_BTN_PIN);
GButton rightBtn(RIGHT_BTN_PIN);
GButton leftBtn(LEFT_BTN_PIN);
GButton selectBtn(SELECT_BTN_PIN);
GButton exitBtn(EXIT_BTN_PIN);

// Увеличен интервал таймера до 100 мс для более плавной игры и лучшего отклика кнопок
GTimer gameTimer(MS, 100);
WebServer server(80);

// Переменные состояния
enum SystemState {
  BOOT,
  MAIN_MENU,
  SETTINGS,
  SYSTEM_INFO,
  MINI_APPS,
  APPS,
  GAMES,
  READER,
  STOPWATCH,
  GAME_ARKANOID,
  GAME_TETRIS,
  GAME_DINO,
  GAME_SNAKE
};

SystemState currentState = BOOT;
int menuIndex = 0;
int maxMenuItems = 0;
bool wifiAPMode = false;

// Структуры для игр и приложений
struct DinoGame {
  int dinoY = 47;
  float dinoSpeed = 0;
  int obstacleX = 128;
  int score = 0;
  bool gameOver = false;
  bool jumping = false;
};

struct SnakeGame {
  int snakeX[100];
  int snakeY[100];
  int snakeLength = 4;
  int foodX, foodY;
  int dirX = 1, dirY = 0;
  int score = 0;
  bool gameOver = false;
};

struct TetrisGame {
  byte field[20][10];
  int currentPiece[4][2];
  int pieceX = 4, pieceY = 0;
  int score = 0;
  bool gameOver = false;
};

struct ArkanoidGame {
  int paddleX = 50;
  int ballX = 64, ballY = 32;
  float ballVelX = 1.0, ballVelY = 1.0;
  bool bricks[5][10];
  int score = 0;
  bool gameOver = false;
};

// !!! ИСПРАВЛЕНИЕ: Добавлена недостающая структура StopwatchApp
struct StopwatchApp {
  unsigned long startTime = 0;
  unsigned long elapsedTime = 0;
  bool running = false;
};

DinoGame dino;
SnakeGame snake;
TetrisGame tetris;
StopwatchApp stopwatch;
ArkanoidGame arkanoid;

// Битмапы для игр
const uint8_t DinoStandL_bmp[] PROGMEM = {
  0xC0, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0xC0, 0xFE, 0xFF, 0xFD, 0xBF, 0xAF, 0x2F, 0x2F, 0x0E,
  0x03, 0x07, 0x1E, 0x1E, 0xFF, 0xBF, 0x1F, 0x3F, 0x7F, 0x4F, 0x07, 0x00, 0x01, 0x00, 0x00, 0x00,
};

const uint8_t DinoStandR_bmp[] PROGMEM = {
  0xC0, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0xC0, 0xFE, 0xFF, 0xFD, 0xBF, 0xAF, 0x2F, 0x2F, 0x0E,
  0x03, 0x07, 0x1E, 0x1E, 0x7F, 0x5F, 0x1F, 0x3F, 0xFF, 0x8F, 0x07, 0x00, 0x01, 0x00, 0x00, 0x00,
};

const uint8_t CactusSmall_bmp[] PROGMEM = {
  0x00, 0x00, 0x00, 0xE0, 0xC0, 0x00, 0xF8, 0xFC, 0xFC, 0xF8, 0x80, 0xFC, 0xFE, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x03, 0x07, 0x06, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
};

// Forward declarations (function prototypes)
void showBootScreen();
void setupWiFiAP();
void updateButtons();
void handleMainMenu();
void handleSettings();
void handleSystemInfo();
void handleMiniApps();
void handleApps();
void handleGames();
void handleReader();
void handleStopwatch();
void handleDinoGame();
void handleSnakeGame();
void handleTetrisGame();
void handleArkanoidGame();
void handleRoot();
void handleFileCreate();
void handleFileUpload();
void drawMenu(const char* title, const char* items[], int itemCount);
void showMessage(const char* message);
void initDino();
void initSnake();
void initTetris();
void initArkanoid();

void setup() {
  Serial.begin(115200);

  upBtn.setType(HIGH_PULL);
  downBtn.setType(HIGH_PULL);
  rightBtn.setType(HIGH_PULL);
  leftBtn.setType(HIGH_PULL);
  selectBtn.setType(HIGH_PULL);
  exitBtn.setType(HIGH_PULL);

  Wire.begin(21, 23); // SDA=21, SCL=23

  oled.init();
  oled.clear();

  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
  }

  showBootScreen();
  // Добавлена задержка на 2 секунды после загрузки
  delay(2000);

  setupWiFiAP();

  currentState = MAIN_MENU;
  menuIndex = 0;
}

void loop() {
  updateButtons();

  server.handleClient();

  switch (currentState) {
    case BOOT:
      break;
    case MAIN_MENU:
      handleMainMenu();
      break;
    case SETTINGS:
      handleSettings();
      break;
    case SYSTEM_INFO:
      handleSystemInfo();
      break;
    case MINI_APPS:
      handleMiniApps();
      break;
    case APPS:
      handleApps();
      break;
    case GAMES:
      handleGames();
      break;
    case READER:
      handleReader();
      break;
    case STOPWATCH:
      handleStopwatch();
      break;
    case GAME_ARKANOID:
      handleArkanoidGame();
      break;
    case GAME_DINO:
      handleDinoGame();
      break;
    case GAME_SNAKE:
      handleSnakeGame();
      break;
    case GAME_TETRIS:
      handleTetrisGame();
      break;
  }
}

void showBootScreen() {
  oled.clear();

  oled.setCursor(0, 0);
  oled.setScale(1);
  oled.print("By Lilux12");

  oled.setCursor(95, 0);
  oled.print("v3.0");

  oled.setCursor(6, 3);
  oled.setScale(2);
  oled.print("Tema OS");

  oled.setScale(1);
  oled.rect(0, 55, 127, 58, OLED_FILL);

  oled.update();
}

void setupWiFiAP() {
  WiFi.softAP("TemaOs", "Temaos123");

  server.on("/", handleRoot);
  server.on("/upload", HTTP_POST, handleFileUpload);
  server.on("/create", HTTP_POST, handleFileCreate);

  server.on("/delete", HTTP_POST, []() {
    if (server.hasArg("filename")) {
      String filename = server.arg("filename");
      if (SPIFFS.exists("/" + filename)) {
        if (SPIFFS.remove("/" + filename)) {
          server.send(200, "text/plain", "File deleted successfully");
        } else {
          server.send(500, "text/plain", "Failed to delete file");
        }
      } else {
        server.send(404, "text/plain", "File not found");
      }
    } else {
      server.send(400, "text/plain", "Missing filename");
    }
  });

  server.begin();
  wifiAPMode = true;
}

void handleRoot() {
  String html = R"(
    <!DOCTYPE html>
    <html>
    <head>
        <title>TemaOS File Manager</title>
        <meta charset="UTF-8">
        <style>
          body { font-family: Arial, sans-serif; }
          .file-list li { margin: 5px 0; }
          .file-list button { background: none; border: none; color: red; text-decoration: underline; cursor: pointer; padding: 0; font-size: inherit; }
        </style>
    </head>
    <body>
        <h1>TemaOS File Manager</h1>
        <h2>Создать новый файл</h2>
        <form action="/create" method="post">
            <p>Название файла: <input type="text" name="filename" placeholder="example.txt"></p>
            <p>Содержимое:</p>
            <textarea name="content" rows="10" cols="50" placeholder="Введите текст..."></textarea><br>
            <input type="submit" value="Создать файл">
        </form>

        <h2>Список файлов</h2>
        <ul class="file-list">
  )";

  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  while (file) {
    html += "<li>" + String(file.name()) + " (" + String(file.size()) + " bytes) ";
    html += "<form action='/delete' method='post' style='display:inline;'>";
    html += "<input type='hidden' name='filename' value='" + String(file.name()) + "'>";
    html += "<button type='submit'>[удалить]</button>";
    html += "</form></li>";
    file = root.openNextFile();
  }

  html += R"(
        </ul>
    </body>
    </html>
  )";

  server.send(200, "text/html", html);
}

void handleFileCreate() {
  if (server.hasArg("filename") && server.hasArg("content")) {
    String filename = "/" + server.arg("filename");
    String content = server.arg("content");

    File file = SPIFFS.open(filename, "w");
    if (file) {
      file.print(content);
      file.close();
      server.send(200, "text/html", "<h1>Файл создан успешно!</h1><a href='/'>Назад</a>");
    } else {
      server.send(500, "text/html", "<h1>Ошибка создания файла</h1><a href='/'>Назад</a>");
    }
  } else {
    server.send(400, "text/html", "<h1>Неверные параметры</h1><a href='/'>Назад</a>");
  }
}

void handleFileUpload() {
  server.send(200, "text/html", "<h1>Upload handler not fully implemented.</h1><a href='/'>Назад</a>");
}

void updateButtons() {
  upBtn.tick();
  downBtn.tick();
  rightBtn.tick();
  leftBtn.tick();
  selectBtn.tick();
  exitBtn.tick();
}

void handleMainMenu() {
  const char* mainMenuItems[] = {"Выключение", "Перезагрузка", "Мини приложения", "Настройки"};
  maxMenuItems = 4;

  drawMenu("Меню", mainMenuItems, maxMenuItems);

  if (upBtn.isClick()) {
    menuIndex = (menuIndex - 1 + maxMenuItems) % maxMenuItems;
  }

  if (downBtn.isClick()) {
    menuIndex = (menuIndex + 1) % maxMenuItems;
  }

  if (selectBtn.isClick()) {
    switch (menuIndex) {
      case 0: // Выключение
        oled.clear();
        oled.setCursor(2, 3);
        oled.setScale(2);
        oled.print("Выключение...");
        oled.update();
        delay(1000); // Краткая задержка перед выключением, чтобы сообщение успело показаться
        ESP.deepSleep(0);
        break;
      case 1: // Перезагрузка
        oled.clear();
        oled.setCursor(2, 3);
        oled.setScale(2);
        oled.print("Перезагрузка...");
        oled.update();
        delay(1000); // Краткая задержка перед перезагрузкой
        ESP.restart();
        break;
      case 2: // Мини приложения
        currentState = MINI_APPS;
        menuIndex = 0;
        break;
      case 3: // Настройки
        currentState = SETTINGS;
        menuIndex = 0;
        break;
    }
  }
}

void handleSettings() {
  const char* settingsItems[] = {"Калибровка", "О системе", "Назад"};
  maxMenuItems = 3;

  drawMenu("Настройки", settingsItems, maxMenuItems);

  if (upBtn.isClick()) {
    menuIndex = (menuIndex - 1 + maxMenuItems) % maxMenuItems;
  }

  if (downBtn.isClick()) {
    menuIndex = (menuIndex + 1) % maxMenuItems;
  }

  if (selectBtn.isClick()) {
    switch (menuIndex) {
      case 0: // Калибровка
        showMessage("Калибровка...");
        delay(2000);
        break;
      case 1: // О системе
        currentState = SYSTEM_INFO;
        break;
      case 2: // Назад
        currentState = MAIN_MENU;
        menuIndex = 0;
        break;
    }
  }

  if (exitBtn.isClick()) {
    currentState = MAIN_MENU;
    menuIndex = 0;
  }
}

void handleSystemInfo() {
  oled.clear();
  oled.setCursor(0, 0);
  oled.setScale(1);
  oled.print("О системе");
  oled.line(0, 10, 127, 10);

  oled.setCursor(0, 2);
  oled.print("TemaOS v3.0");
  oled.setCursor(0, 3);
  oled.print("By Lilux12");
  oled.setCursor(0, 4);
  oled.print("ESP32 Platform");
  oled.setCursor(0, 5);
  oled.print("RAM: ");
  oled.print(ESP.getFreeHeap());
  oled.print(" bytes");

  oled.setCursor(0, 7);
  oled.print("EXIT: назад");
  oled.update();

  if (exitBtn.isClick()) {
    currentState = SETTINGS;
    menuIndex = 1;
  }
}

void handleMiniApps() {
  const char* miniAppsItems[] = {"Игры", "Приложения", "Назад"};
  maxMenuItems = 3;

  drawMenu("Мини приложения", miniAppsItems, maxMenuItems);

  if (upBtn.isClick()) {
    menuIndex = (menuIndex - 1 + maxMenuItems) % maxMenuItems;
  }

  if (downBtn.isClick()) {
    menuIndex = (menuIndex + 1) % maxMenuItems;
  }

  if (selectBtn.isClick()) {
    switch (menuIndex) {
      case 0: // Игры
        currentState = GAMES;
        menuIndex = 0;
        break;
      case 1: // Приложения
        currentState = APPS;
        menuIndex = 0;
        break;
      case 2: // Назад
        currentState = MAIN_MENU;
        menuIndex = 0;
        break;
    }
  }

  if (exitBtn.isClick()) {
    currentState = MAIN_MENU;
    menuIndex = 0;
  }
}

void handleApps() {
  const char* appsItems[] = {"Читалка", "Секундомер", "Назад"};
  maxMenuItems = 3;

  drawMenu("Приложения", appsItems, maxMenuItems);

  if (upBtn.isClick()) {
    menuIndex = (menuIndex - 1 + maxMenuItems) % maxMenuItems;
  }

  if (downBtn.isClick()) {
    menuIndex = (menuIndex + 1) % maxMenuItems;
  }

  if (selectBtn.isClick()) {
    switch (menuIndex) {
      case 0: // Читалка
        currentState = READER;
        break;
      case 1: // Секундомер
        currentState = STOPWATCH;
        stopwatch.startTime = 0;
        stopwatch.elapsedTime = 0;
        stopwatch.running = false;
        break;
      case 2: // Назад
        currentState = MINI_APPS;
        menuIndex = 0;
        break;
    }
  }

  if (exitBtn.isClick()) {
    currentState = MINI_APPS;
    menuIndex = 0;
  }
}

void handleGames() {
  const char* gamesItems[] = {"Арканоид", "Тетрис", "Гугл динозаврик", "Змейка", "Назад"};
  maxMenuItems = 5;

  drawMenu("Игры", gamesItems, maxMenuItems);

  if (upBtn.isClick()) {
    menuIndex = (menuIndex - 1 + maxMenuItems) % maxMenuItems;
  }

  if (downBtn.isClick()) {
    menuIndex = (menuIndex + 1) % maxMenuItems;
  }

  if (selectBtn.isClick()) {
    switch (menuIndex) {
      case 0: // Арканоид
        currentState = GAME_ARKANOID;
        initArkanoid();
        break;
      case 1: // Тетрис
        currentState = GAME_TETRIS;
        initTetris();
        break;
      case 2: // Гугл динозаврик
        currentState = GAME_DINO;
        initDino();
        break;
      case 3: // Змейка
        currentState = GAME_SNAKE;
        initSnake();
        break;
      case 4: // Назад
        currentState = MINI_APPS;
        menuIndex = 0;
        break;
    }
  }

  if (exitBtn.isClick()) {
    currentState = MINI_APPS;
    menuIndex = 0;
  }
}

void handleReader() {
  oled.clear();
  oled.setCursor(0, 0);
  oled.setScale(1);
  oled.print("Читалка");
  oled.line(0, 10, 127, 10);

  File root = SPIFFS.open("/");
  File file = root.openNextFile();

  if (!file) {
    oled.setCursor(3, 4);
    oled.setScale(2);
    oled.print("Нет файлов :(");
  } else {
    oled.setCursor(0, 2);
    oled.setScale(1);
    oled.print("Файлы:");

    int y = 3;
    while (file && y < 7) {
      oled.setCursor(0, y);
      oled.print(String(file.name()));
      file = root.openNextFile();
      y++;
    }
  }

  oled.update();

  if (exitBtn.isClick() || selectBtn.isClick()) {
    currentState = APPS;
    menuIndex = 0;
  }
}

void handleStopwatch() {
  oled.clear();
  oled.setCursor(0, 0);
  oled.setScale(1);
  oled.print("Секундомер");
  oled.line(0, 10, 127, 10);

  if (selectBtn.isClick()) {
    if (!stopwatch.running) {
      stopwatch.startTime = millis();
      stopwatch.running = true;
    } else {
      stopwatch.elapsedTime += millis() - stopwatch.startTime;
      stopwatch.running = false;
    }
  }

  if (upBtn.isClick()) {
    stopwatch.startTime = 0;
    stopwatch.elapsedTime = 0;
    stopwatch.running = false;
  }

  unsigned long totalTime = stopwatch.elapsedTime;
  if (stopwatch.running) {
    totalTime += millis() - stopwatch.startTime;
  }

  int minutes = (totalTime / 60000) % 60;
  int seconds = (totalTime / 1000) % 60;
  int milliseconds = (totalTime % 1000) / 10;

  oled.setCursor(2, 3);
  oled.setScale(2);
  char timeStr[20];
  sprintf(timeStr, "%02d:%02d.%02d", minutes, seconds, milliseconds);
  oled.print(timeStr);

  oled.setScale(1);
  oled.setCursor(0, 6);
  oled.print("SELECT: старт/стоп");
  oled.setCursor(0, 7);
  oled.print("UP: сброс EXIT: выход");

  oled.update();

  if (exitBtn.isClick()) {
    currentState = APPS;
    menuIndex = 0;
  }
}

// Улучшена логика столкновений
void initDino() {
  dino.dinoY = 47;
  dino.dinoSpeed = 0;
  dino.obstacleX = 128;
  dino.score = 0;
  dino.gameOver = false;
  dino.jumping = false;
}

void handleDinoGame() {
  if (dino.gameOver) {
    oled.clear();
    oled.setCursor(3, 2);
    oled.setScale(2);
    oled.print("GAME OVER");
    oled.setScale(1);
    oled.setCursor(2, 4);
    oled.print("Счет: ");
    oled.print(dino.score);
    oled.setCursor(0, 6);
    oled.print("SELECT: заново");
    oled.setCursor(0, 7);
    oled.print("EXIT: выход");
    oled.update();

    if (selectBtn.isClick()) {
      initDino();
    }
    if (exitBtn.isClick()) {
      currentState = GAMES;
      menuIndex = 2;
    }
    return;
  }

  // Обновление физики
  if (selectBtn.isClick() && dino.dinoY >= 47) {
    dino.dinoSpeed = -2.8;
    dino.jumping = true;
  }

  if (dino.jumping) {
    dino.dinoY += dino.dinoSpeed;
    dino.dinoSpeed += 0.2;

    if (dino.dinoY >= 47) {
      dino.dinoY = 47;
      dino.dinoSpeed = 0;
      dino.jumping = false;
    }
  }

  dino.obstacleX -= 3;
  if (dino.obstacleX < -16) {
    dino.obstacleX = 128;
    dino.score++;
  }

  // Точная проверка столкновений
  // Динозавр: x: 0..15, y: dinoY..dinoY+15
  // Кактус: x: obstacleX..obstacleX+15, y: 48..63
  bool x_overlap = (0 < dino.obstacleX + 16) && (16 > dino.obstacleX);
  bool y_overlap = (dino.dinoY < 64) && (dino.dinoY + 16 > 48);

  if (x_overlap && y_overlap) {
    dino.gameOver = true;
  }

  oled.clear();
  oled.setCursor(0, 0);
  oled.setScale(1);
  oled.print("Счет: ");
  oled.print(dino.score);

  oled.line(0, 63, 127, 63);

  oled.drawBitmap(0, dino.dinoY, DinoStandL_bmp, 16, 16);

  if (dino.obstacleX >= 0 && dino.obstacleX < 128) {
    oled.drawBitmap(dino.obstacleX, 48, CactusSmall_bmp, 16, 16);
  }

  oled.update();

  if (exitBtn.isClick()) {
    currentState = GAMES;
    menuIndex = 2;
  }
}

void initSnake() {
  snake.snakeLength = 4;
  for (int i = 0; i < snake.snakeLength; i++) {
    snake.snakeX[i] = 64 - i * 4;
    snake.snakeY[i] = 32;
  }
  snake.foodX = random(4, 124);
  snake.foodY = random(16, 60);
  snake.dirX = 4;
  snake.dirY = 0;
  snake.score = 0;
  snake.gameOver = false;
}

void handleSnakeGame() {
  if (snake.gameOver) {
    oled.clear();
    oled.setCursor(3, 2);
    oled.setScale(2);
    oled.print("GAME OVER");
    oled.setScale(1);
    oled.setCursor(2, 4);
    oled.print("Счет: ");
    oled.print(snake.score);
    oled.setCursor(0, 6);
    oled.print("SELECT: заново");
    oled.setCursor(0, 7);
    oled.print("EXIT: выход");
    oled.update();

    if (selectBtn.isClick()) {
      initSnake();
    }
    if (exitBtn.isClick()) {
      currentState = GAMES;
      menuIndex = 3;
    }
    return;
  }

  if (upBtn.isClick() && snake.dirY == 0) {
    snake.dirX = 0;
    snake.dirY = -4;
  }
  if (downBtn.isClick() && snake.dirY == 0) {
    snake.dirX = 0;
    snake.dirY = 4;
  }
  if (leftBtn.isClick() && snake.dirX == 0) {
    snake.dirX = -4;
    snake.dirY = 0;
  }
  if (rightBtn.isClick() && snake.dirX == 0) {
    snake.dirX = 4;
    snake.dirY = 0;
  }

  if (gameTimer.isReady()) {
    int newX = snake.snakeX[0] + snake.dirX;
    int newY = snake.snakeY[0] + snake.dirY;

    if (newX < 0 || newX >= 128 || newY < 16 || newY >= 64) {
      snake.gameOver = true;
      return;
    }

    for (int i = 1; i < snake.snakeLength; i++) {
      if (newX == snake.snakeX[i] && newY == snake.snakeY[i]) {
        snake.gameOver = true;
        return;
      }
    }

    bool ateFood = false;
    if (abs(newX - snake.foodX) < 4 && abs(newY - snake.foodY) < 4) {
      snake.snakeLength++;
      snake.score++;
      snake.foodX = random(4, 124);
      snake.foodY = random(16, 60);
      ateFood = true;
    }

    if (!ateFood) {
      for (int i = snake.snakeLength - 1; i > 0; i--) {
        snake.snakeX[i] = snake.snakeX[i - 1];
        snake.snakeY[i] = snake.snakeY[i - 1];
      }
    }

    snake.snakeX[0] = newX;
    snake.snakeY[0] = newY;
  }

  oled.clear();
  oled.setCursor(0, 0);
  oled.setScale(1);
  oled.print("Счет: ");
  oled.print(snake.score);
  oled.line(0, 12, 127, 12);

  for (int i = 0; i < snake.snakeLength; i++) {
    oled.rect(snake.snakeX[i], snake.snakeY[i], snake.snakeX[i] + 3, snake.snakeY[i] + 3, OLED_FILL);
  }

  oled.rect(snake.foodX, snake.foodY, snake.foodX + 3, snake.foodY + 3, OLED_FILL);

  oled.update();

  if (exitBtn.isClick()) {
    currentState = GAMES;
    menuIndex = 3;
  }
}

void initTetris() {
  for (int y = 0; y < 20; y++) {
    for (int x = 0; x < 10; x++) {
      tetris.field[y][x] = 0;
    }
  }
  tetris.score = 0;
  tetris.gameOver = false;
}

void handleTetrisGame() {
  oled.clear();
  oled.setCursor(3, 2);
  oled.setScale(2);
  oled.print("TETRIS");
  oled.setScale(1);
  oled.setCursor(2, 4);
  oled.print("Скоро будет готов!");
  oled.setCursor(0, 6);
  oled.print("EXIT: выход");
  oled.update();

  if (exitBtn.isClick()) {
    currentState = GAMES;
    menuIndex = 1;
  }
}

// Реализация Арканоида
void initArkanoid() {
  arkanoid.paddleX = 50;
  arkanoid.ballX = 64;
  arkanoid.ballY = 32;
  arkanoid.ballVelX = 1.0;
  arkanoid.ballVelY = 1.0;
  arkanoid.score = 0;
  arkanoid.gameOver = false;
  // Инициализация кирпичей
  for (int y = 0; y < 5; y++) {
    for (int x = 0; x < 10; x++) {
      arkanoid.bricks[y][x] = true;
    }
  }
}

void handleArkanoidGame() {
  if (arkanoid.gameOver) {
    oled.clear();
    oled.setCursor(3, 2);
    oled.setScale(2);
    oled.print("GAME OVER");
    oled.setScale(1);
    oled.setCursor(2, 4);
    oled.print("Счет: ");
    oled.print(arkanoid.score);
    oled.setCursor(0, 6);
    oled.print("SELECT: заново");
    oled.setCursor(0, 7);
    oled.print("EXIT: выход");
    oled.update();

    if (selectBtn.isClick()) {
      initArkanoid();
    }
    if (exitBtn.isClick()) {
      currentState = GAMES;
      menuIndex = 0;
    }
    return;
  }

  if (gameTimer.isReady()) {
    // Управление ракеткой
    if (leftBtn.isClick()) {
      arkanoid.paddleX -= 5;
    }
    if (rightBtn.isClick()) {
      arkanoid.paddleX += 5;
    }
    if (arkanoid.paddleX < 0) arkanoid.paddleX = 0;
    if (arkanoid.paddleX > 128 - 20) arkanoid.paddleX = 128 - 20;

    // Движение мяча
    arkanoid.ballX += arkanoid.ballVelX;
    arkanoid.ballY += arkanoid.ballVelY;

    // Столкновения со стенами
    if (arkanoid.ballX <= 0 || arkanoid.ballX >= 127) {
      arkanoid.ballVelX = -arkanoid.ballVelX;
    }
    if (arkanoid.ballY <= 0) {
      arkanoid.ballVelY = -arkanoid.ballVelY;
    }

    // Столкновение с ракеткой
    if (arkanoid.ballY >= 63 - 5 && arkanoid.ballY <= 63 &&
        arkanoid.ballX >= arkanoid.paddleX && arkanoid.ballX <= arkanoid.paddleX + 20) {
      arkanoid.ballVelY = -arkanoid.ballVelY;
    }

    // Столкновение с кирпичами
    int brickCol = (arkanoid.ballX - 1) / 12;
    int brickRow = (arkanoid.ballY - 10) / 4;
    if (brickCol >= 0 && brickCol < 10 && brickRow >= 0 && brickRow < 5) {
      if (arkanoid.bricks[brickRow][brickCol]) {
        arkanoid.bricks[brickRow][brickCol] = false;
        arkanoid.ballVelY = -arkanoid.ballVelY;
        arkanoid.score += 10;
      }
    }
    
    // Проверка на проигрыш
    if (arkanoid.ballY >= 64) {
      arkanoid.gameOver = true;
    }
  }

  // Отрисовка
  oled.clear();
  oled.setCursor(0, 0);
  oled.setScale(1);
  oled.print("Счет: ");
  oled.print(arkanoid.score);
  oled.line(0, 10, 127, 10);
  
  // Отрисовка кирпичей
  for (int y = 0; y < 5; y++) {
    for (int x = 0; x < 10; x++) {
      if (arkanoid.bricks[y][x]) {
        oled.rect(x * 12 + 2, y * 4 + 12, x * 12 + 10, y * 4 + 14, OLED_FILL);
      }
    }
  }

  // Отрисовка ракетки и мяча
  oled.rect(arkanoid.paddleX, 63, arkanoid.paddleX + 20, 63, OLED_FILL);
  oled.dot(arkanoid.ballX, arkanoid.ballY);

  oled.update();

  if (exitBtn.isClick()) {
    currentState = GAMES;
    menuIndex = 0;
  }
}


void drawMenu(const char* title, const char* items[], int itemCount) {
  oled.clear();

  oled.setCursor(0, 0);
  oled.setScale(1);
  oled.print(title);
  oled.line(0, 10, 127, 10);

  for (int i = 0; i < itemCount; i++) {
    oled.setCursor(10, 2 + i);
    oled.setScale(1);
    oled.print(items[i]);

    if (i == menuIndex) {
      oled.setCursor(0, 2 + i);
      oled.print(">");
    }
  }

  oled.update();
}

void showMessage(const char* message) {
  oled.clear();
  oled.setCursor(2, 3);
  oled.setScale(2);
  oled.print(message);
  oled.update();
}
