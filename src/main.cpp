#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <GyverOLED.h>
#include <GyverButton.h>
#include <GyverTimer.h>
#include <Wire.h>
#include <math.h>

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

GTimer gameTimer(MS, 20);
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
  STOPWATCH,
  WIFI_SCANNER,
  TIMER_APP,
  FILE_MANAGER,
  DRAW_APP,
  TEMP_CONVERTER,
  COUNTER,
  TEXT_EDITOR,
  GAME_PONG,
  GAME_ASTEROIDS,
  GAME_FLAPPY_BIRD,
  GAME_TETRIS,
  GAME_DINO,
  GAME_SNAKE,
  GAME_ARKANOID,
  GAME_DICE,
  MULTIPLICATION_TABLE,
  READER_APP
};

SystemState currentState = BOOT;
SystemState previousState = BOOT;

// --- Состояние меню ---
struct MenuState {
    int index = 0;
    int page = 0;
    int maxItems = 0;
    int maxPages = 1;
};

File uploadFile;
File readerFile; // Глобальный объект файла для читалки

MenuState mainMenuState;
MenuState settingsMenuState;
MenuState miniAppsMenuState;
MenuState appsMenuState;
MenuState gamesMenuState;

bool wifiAPMode = false;

// --- Структуры для игр и приложений ---

// --- Структура для Динозавра ---
struct DinoGame {
  int dinoY = 47; float dinoSpeed = 0; int obstacleX = 128; int score = 0;
  bool gameOver = false; bool jumping = false; bool crouching = false; bool legFlag = true;
  bool birdFlag = true; int8_t enemyType = 0; uint8_t gameSpeed = 6;
  unsigned long lastScoreUpdate = 0; unsigned long lastEnemyUpdate = 0;
  unsigned long lastLegUpdate = 0; unsigned long lastBirdUpdate = 0; unsigned long lastDinoUpdate = 0;
};

// --- Структура для Змейки ---
struct SnakeGame {
  static const int MAX_LENGTH = 80; int snakeX[MAX_LENGTH]; int snakeY[MAX_LENGTH];
  int snakeLength = 4; int foodX, foodY; int dirX = 1, dirY = 0; int score = 0;
  bool gameOver = false; int segmentSize = 4; unsigned long lastMoveTime = 0; int moveDelay = 150;
};

// --- Структура для Тетриса ---
struct TetrisGame {
  static const int FIELD_WIDTH = 10; static const int FIELD_HEIGHT = 16;
  byte field[FIELD_HEIGHT][FIELD_WIDTH] = {0}; byte currentPieceType; byte nextPieceType;
  int currentPiece[4][2]; int pieceX = FIELD_WIDTH / 2 - 1; int pieceY = 0;
  int score = 0; bool gameOver = false; unsigned long lastDropTime = 0; int dropDelay = 500;
};

// --- Структура для Арканоида ---
struct ArkanoidGame {
  int paddleX = 54; int ballX = 64, ballY = 55; float ballVelX = 1.0, ballVelY = 1.0;
  bool bricks[5][10]; int score = 0; bool gameOver = false; int paddleWidth = 24; int ballSize = 2;
};

// --- Структура для Секундомера ---
struct StopwatchApp {
  unsigned long startTime = 0; unsigned long elapsedTime = 0; bool running = false;
};

// --- Структура для Счетчика ---
struct CounterApp { int count = 0; };

// --- Структура для Таймера ---
struct TimerAppState {
    unsigned long setTime = 0; unsigned long startTime = 0;
    bool running = false; bool alarmTriggered = false;
};

// --- Структура для Рисовалки ---
struct DrawAppState { int cursorX = 64; int cursorY = 32; };

// --- Структура для Конвертера температуры ---
struct TempConverterState {
    float celsius = 0.0; float fahrenheit = 32.0; bool convertingCtoF = true;
};

// --- Структура для Текстового редактора ---
struct TextEditorState {
    String filename = ""; String content = ""; int cursorPos = 0; bool isNewFile = true;
};

// --- Структура для Понга ---
struct PongGame {
    float paddle1Y = 24; float paddle2Y = 24; float ballX = 64, ballY = 32;
    float ballVelX = 1.5, ballVelY = 1.0; int score1 = 0, score2 = 0;
    bool gameOver = false; int paddleHeight = 16; int ballSize = 3;
};

// --- Структура для Астероидов ---
struct AsteroidsGame {
    float shipX = 64, shipY = 50; float shipVelX = 0, shipVelY = 0; float shipAngle = 0;
    bool thrusting = false; bool gameOver = false; int score = 0;
    static const int MAX_ASTEROIDS = 10; static const int MAX_BULLETS = 5;
    struct Asteroid { float x, y, velX, velY; int size; bool active; };
    struct Bullet { float x, y, velX, velY; bool active; };
    Asteroid asteroids[MAX_ASTEROIDS]; Bullet bullets[MAX_BULLETS];
};

// --- Структура для Flappy Bird ---
struct FlappyBirdGame {
    float birdY = 32; float birdVel = 0; static const int MAX_PIPES = 5;
    struct Pipe { int x; int gapY; bool passed; };
    Pipe pipes[MAX_PIPES]; int score = 0; bool gameOver = false;
    unsigned long lastPipeTime = 0; int pipeSpacing = 50;
};

// --- Структура для Сканера WiFi ---
struct WifiScannerState {
    int networkCount = 0; int currentPage = 0; int totalPages = 1; int networksPerPage = 4;
};

// --- Структура для Таблицы умножения ---
struct MultiplicationTableApp { int multiplier1 = 1; int multiplier2 = 1; };

// --- Структура для Читалки ---
struct ReaderAppState {
    int cursor = 0;
    int filesCount = 0;
    static const int MAX_PAGE_HISTORY = 150;
    long pageHistory[MAX_PAGE_HISTORY] = {0};
    int currentHistoryIndex = -1;
    int totalPages = 0;
    bool inFileReader = false; // Флаг, что мы внутри просмотра файла
};

DinoGame dino;
SnakeGame snake;
TetrisGame tetris;
StopwatchApp stopwatch;
ArkanoidGame arkanoid;
CounterApp counterApp;
TimerAppState timerApp;
DrawAppState drawApp;
TempConverterState tempConverter;
TextEditorState textEditor;
PongGame pong;
AsteroidsGame asteroids;
FlappyBirdGame flappyBird;
WifiScannerState wifiScanner;
MultiplicationTableApp multiplicationTable;
ReaderAppState readerApp;

// --- Битмапы для Динозавра ---
const uint8_t DinoStandL_bmp[] PROGMEM = { 0xC0,0x00,0x00,0x00,0x00,0x80,0x80,0xC0,0xFE,0xFF,0xFD,0xBF,0xAF,0x2F,0x2F,0x0E,0x03,0x07,0x1E,0x1E,0xFF,0xBF,0x1F,0x3F,0x7F,0x4F,0x07,0x00,0x01,0x00,0x00,0x00, };
const uint8_t DinoStandR_bmp[] PROGMEM = { 0xC0,0x00,0x00,0x00,0x00,0x80,0x80,0xC0,0xFE,0xFF,0xFD,0xBF,0xAF,0x2F,0x2F,0x0E,0x03,0x07,0x1E,0x1E,0x7F,0x5F,0x1F,0x3F,0xFF,0x8F,0x07,0x00,0x01,0x00,0x00,0x00, };
const uint8_t DinoStandDie_bmp[] PROGMEM = { 0xC0,0x00,0x00,0x00,0x00,0x80,0x80,0xC0,0xFE,0xF1,0xF5,0xB1,0xBF,0x2F,0x2F,0x0E,0x03,0x07,0x1E,0x1E,0xFF,0xBF,0x1F,0x3F,0xFF,0x8F,0x07,0x00,0x01,0x00,0x00,0x00, };
const uint8_t DinoCroachL_bmp[] PROGMEM = { 0x03,0x06,0x6C,0x5C,0x1C,0xFE,0xBE,0x1E,0x7E,0x5E,0x0E,0x1C,0x3E,0x2A,0x2E,0x0E, };
const uint8_t DinoCroachR_bmp[] PROGMEM = { 0x03,0x06,0xEC,0x9C,0x1C,0x7E,0x5E,0x1E,0x7E,0x5E,0x0E,0x1C,0x3E,0x2A,0x2E,0x0E, };
const uint8_t CactusSmall_bmp[] PROGMEM = { 0x00,0x00,0x00,0xE0,0xC0,0x00,0xF8,0xFC,0xFC,0xF8,0x80,0xFC,0xFE,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x07,0x06,0xFF,0xFF,0xFF,0xFF,0x01,0x01,0x00,0x00,0x00,0x00, };
const uint8_t CactusBig_bmp[] PROGMEM = { 0xF0,0x00,0xFC,0xFE,0xFE,0xC0,0x7C,0x00,0xF0,0x00,0xF8,0xFC,0x60,0x3E,0x00,0x80,0x00,0xF8,0x80,0xF8,0xFC,0xF8,0x30,0x1F,0x03,0x07,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x01,0x03,0xFF,0xFF,0x00,0x1F,0x30,0xFF,0x60,0x3C,0x01,0xFF,0xFF,0xFF,0x00,0x00, };
const uint8_t BirdL_bmp[] PROGMEM = { 0x30,0x38,0x3C,0x3C,0x3F,0x3F,0x7F,0x7C,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xE0,0xE0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0xFF,0x7F,0x1F,0x0F,0x0F,0x0F,0x07,0x07,0x07,0x07,0x07,0x03,0x03,0x03,0x00, };
const uint8_t BirdR_bmp[] PROGMEM = { 0x00,0x80,0xC0,0xE0,0xF0,0xF0,0xF0,0xC0,0x0F,0xFE,0xF8,0xF8,0xF0,0xE0,0xC0,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x03,0x03,0x03,0x03,0x03,0x03,0x07,0x0E,0x1F,0x7F,0x7F,0xFF,0xFF,0xFF,0xFF,0xFC,0xFC,0xF8,0xF8,0x78,0x68,0x68,0x68, };

// --- Фигуры для Тетриса ---
const byte PIECES[7][4][2] = {
  {{0, 1}, {1, 1}, {2, 1}, {3, 1}}, {{1, 0}, {1, 1}, {1, 2}, {2, 2}}, {{2, 0}, {2, 1}, {2, 2}, {1, 2}},
  {{1, 1}, {2, 1}, {2, 0}, {3, 0}}, {{1, 0}, {2, 0}, {2, 1}, {3, 1}}, {{1, 1}, {0, 1}, {2, 1}, {1, 2}},
  {{1, 1}, {2, 1}, {1, 2}, {2, 2}}
};

// Объявления функций
void showBootScreen();
void handleReaderApp();
void initReaderApp();
void handleMainMenu();
void handleSettings();
void handleSystemInfo();
void handleMiniApps();
void handleApps();
void handleGames();
void handleStopwatch();
void handleWifiScanner();
void handleTimerApp();
void handleFileManager();
void handleDrawApp();
void handleTempConverter();
void handleCounter();
void handleTextEditor();
void handleDinoGame();
void handleSnakeGame();
void handleTetrisGame();
void handleArkanoidGame();
void handlePongGame();
void handleAsteroidsGame();
void handleFlappyBirdGame();
void handleDiceGame();
void handleMultiplicationTable();
void handleRoot();
void handleFileCreate();
void handleFileUpload();
void drawMenu(const char* title, const char* items[], int itemCount, int currentPage, int totalPages);
void showMessage(const char* message);
void tetrisNewPiece();
bool tetrisCheckCollision(int x, int y);


// ---- Функции инициализации игр и приложений ----
void initDinoGame() {
    dino = DinoGame();
    dino.enemyType = random(0, 3);
    dino.lastScoreUpdate = millis();
}
void initSnakeGame() {
    snake = SnakeGame();
    for (int i = 0; i < snake.snakeLength; i++) { snake.snakeX[i] = 64 - i * 4; snake.snakeY[i] = 32; }
    snake.foodX = random(0, 32) * 4; snake.foodY = random(3, 16) * 4;
}
void initTetrisGame() {
    tetris = TetrisGame();
    tetris.nextPieceType = random(7);
    for (int y = 0; y < tetris.FIELD_HEIGHT; y++) { for (int x = 0; x < tetris.FIELD_WIDTH; x++) tetris.field[y][x] = 0; }
    tetrisNewPiece();
}
void initArkanoidGame() {
    arkanoid = ArkanoidGame();
    for (int y = 0; y < 5; y++) { for (int x = 0; x < 10; x++) arkanoid.bricks[y][x] = true; }
}
void initPongGame() {
    pong = PongGame();
    pong.ballVelY = random(-10, 11) / 10.0;
}
void initAsteroidsGame() {
    asteroids = AsteroidsGame();
    for (int i = 0; i < asteroids.MAX_ASTEROIDS; i++) asteroids.asteroids[i].active = false;
    for (int i = 0; i < asteroids.MAX_BULLETS; i++) asteroids.bullets[i].active = false;
    for (int i = 0; i < 3; i++) {
        asteroids.asteroids[i].active = true; asteroids.asteroids[i].x = random(0, 128);
        asteroids.asteroids[i].y = random(0, 20); asteroids.asteroids[i].velX = random(-10, 11) / 10.0;
        asteroids.asteroids[i].velY = random(1, 10) / 10.0; asteroids.asteroids[i].size = 2;
    }
}
void initFlappyBirdGame() {
    flappyBird = FlappyBirdGame();
    for (int i = 0; i < flappyBird.MAX_PIPES; i++) {
        flappyBird.pipes[i].x = 128 + i * flappyBird.pipeSpacing;
        flappyBird.pipes[i].gapY = random(20, 44);
        flappyBird.pipes[i].passed = false;
    }
}
void initStopwatch() { stopwatch = StopwatchApp(); }
void initTimerApp() { timerApp = TimerAppState(); }
void initDrawApp() { drawApp = DrawAppState(); oled.clear(); }
void initTempConverter() { tempConverter = TempConverterState(); }
void initCounter() { counterApp = CounterApp(); }
void initTextEditor() { textEditor = TextEditorState(); }
void initMultiplicationTable() { multiplicationTable = MultiplicationTableApp(); }


// --- Функции для Тетриса ---
void tetrisRotatePiece() {
    int tempPiece[4][2];
    for (int i = 0; i < 4; i++) { tempPiece[i][0] = tetris.currentPiece[i][0]; tempPiece[i][1] = tetris.currentPiece[i][1]; }
    int pivotX = tempPiece[1][0]; int pivotY = tempPiece[1][1];
    for (int i = 0; i < 4; i++) {
        int newX = pivotX - (tempPiece[i][1] - pivotY); int newY = pivotY + (tempPiece[i][0] - pivotX);
        tempPiece[i][0] = newX; tempPiece[i][1] = newY;
    }
    int tempX = tetris.pieceX; int tempY = tetris.pieceY; bool collision = false;
    for (int i = 0; i < 4; i++) {
        int px = tempPiece[i][0] + tempX; int py = tempPiece[i][1] + tempY;
        if (px < 0 || px >= tetris.FIELD_WIDTH || py >= tetris.FIELD_HEIGHT || (py >= 0 && tetris.field[py][px])) { collision = true; break; }
    }
    if (!collision) { for (int i = 0; i < 4; i++) { tetris.currentPiece[i][0] = tempPiece[i][0]; tetris.currentPiece[i][1] = tempPiece[i][1]; } }
}
bool tetrisCheckCollision(int x, int y) {
    for (int i = 0; i < 4; i++) {
        int px = tetris.currentPiece[i][0] + x; int py = tetris.currentPiece[i][1] + y;
        if (px < 0 || px >= tetris.FIELD_WIDTH || py >= tetris.FIELD_HEIGHT) return true;
        if (py >= 0 && tetris.field[py][px]) return true;
    }
    return false;
}
void tetrisPlacePiece() {
    for (int i = 0; i < 4; i++) {
        int px = tetris.currentPiece[i][0] + tetris.pieceX; int py = tetris.currentPiece[i][1] + tetris.pieceY;
        if (py >= 0) { tetris.field[py][px] = tetris.currentPieceType + 1; }
    }
}
void tetrisClearLines() {
    for (int y = tetris.FIELD_HEIGHT - 1; y >= 0; y--) {
        bool lineFull = true;
        for (int x = 0; x < tetris.FIELD_WIDTH; x++) { if (tetris.field[y][x] == 0) { lineFull = false; break; } }
        if (lineFull) {
            for (int yy = y; yy > 0; yy--) { for (int x = 0; x < tetris.FIELD_WIDTH; x++) { tetris.field[yy][x] = tetris.field[yy-1][x]; } }
            for (int x = 0; x < tetris.FIELD_WIDTH; x++) { tetris.field[0][x] = 0; }
            tetris.score += 100; y++;
        }
    }
}
void tetrisNewPiece() {
    tetris.currentPieceType = tetris.nextPieceType; tetris.nextPieceType = random(7);
    for (int i = 0; i < 4; i++) { tetris.currentPiece[i][0] = PIECES[tetris.currentPieceType][i][0]; tetris.currentPiece[i][1] = PIECES[tetris.currentPieceType][i][1]; }
    tetris.pieceX = tetris.FIELD_WIDTH / 2 - 2; tetris.pieceY = 0;
    if (tetrisCheckCollision(tetris.pieceX, tetris.pieceY)) { tetris.gameOver = true; }
}

// --- Функции управления состоянием меню ---
void resetMenuState(MenuState& state) { state.index = 0; state.page = 0; state.maxItems = 0; state.maxPages = 1; }
int calculateMenuPages(int itemCount, int itemsPerPage) { return (itemCount + itemsPerPage - 1) / itemsPerPage; }
void handleMenuNavigation(MenuState& state, int itemCount, int itemsPerPage) {
  state.maxPages = calculateMenuPages(itemCount, itemsPerPage);
  if (upBtn.isClick()) {
    state.index--;
    if (state.index < 0) {
      if (state.page > 0) { state.page--; state.index = itemsPerPage - 1; } 
      else {
        state.page = state.maxPages - 1;
        int itemsOnLastPage = itemCount - (state.page * itemsPerPage);
        state.index = itemsOnLastPage - 1;
        if (state.index < 0) state.index = 0;
      }
    }
  }
  if (downBtn.isClick()) {
    state.index++;
    int itemsOnPage = itemsPerPage;
    if (state.page == state.maxPages - 1) { itemsOnPage = itemCount - (state.page * itemsPerPage); }
    if (state.index >= itemsOnPage) {
      if (state.page < state.maxPages - 1) { state.page++; state.index = 0; } 
      else { state.page = 0; state.index = 0; }
    }
  }
  if (leftBtn.isClick() && state.maxPages > 1) { state.page = (state.page - 1 + state.maxPages) % state.maxPages; state.index = 0; }
  if (rightBtn.isClick() && state.maxPages > 1) { state.page = (state.page + 1) % state.maxPages; state.index = 0; }
}

void setup() {
  Serial.begin(115200);
  randomSeed(analogRead(0));
  upBtn.setType(HIGH_PULL); downBtn.setType(HIGH_PULL); rightBtn.setType(HIGH_PULL);
  leftBtn.setType(HIGH_PULL); selectBtn.setType(HIGH_PULL); exitBtn.setType(HIGH_PULL);
  Wire.begin(21, 23);
  oled.init();
  oled.clear();
  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS Mount Failed");
    showMessage("LittleFS Ошибка!");
    delay(2000);
  }

  mainMenuState.maxItems = 4;
  settingsMenuState.maxItems = 3;
  miniAppsMenuState.maxItems = 3;
  appsMenuState.maxItems = 11;
  appsMenuState.maxPages = 3;
  gamesMenuState.maxItems = 9;
  gamesMenuState.maxPages = 2;

  showBootScreen();
  delay(2000);
  WiFi.softAP("TemaOs", "Temaos123");
  server.on("/", handleRoot);
  server.on("/upload", HTTP_POST, []() { server.send(200, "text/plain", "OK"); }, handleFileUpload);
  server.on("/create", HTTP_POST, handleFileCreate);
  server.on("/delete", HTTP_POST, []() {
    if (server.hasArg("filename")) {
      String filename = server.arg("filename");
      if (LittleFS.exists("/" + filename)) {
        if (LittleFS.remove("/" + filename)) { server.send(200, "text/plain", "File deleted"); } 
        else { server.send(500, "text/plain", "Failed to delete"); }
      } else { server.send(404, "text/plain", "File not found"); }
    } else { server.send(400, "text/plain", "Missing filename"); }
  });
  server.on("/list", HTTP_GET, [](){
      String json = "[";
      File root = LittleFS.open("/");
      File file = root.openNextFile();
      bool first = true;
      while(file){
        if(!first){ json += ","; }
        json += "{\"name\":\"" + String(file.name()).substring(1) + "\",\"size\":" + file.size() + "}";
        file = root.openNextFile();
        first = false;
      }
      json += "]";
      server.send(200, "application/json", json);
    });
  server.begin();
  wifiAPMode = true;
  currentState = MAIN_MENU;
  resetMenuState(mainMenuState);
}

void loop() {
  upBtn.tick(); downBtn.tick(); rightBtn.tick(); leftBtn.tick(); selectBtn.tick(); exitBtn.tick();
  server.handleClient();
  switch (currentState) {
    case BOOT: break;
    case MAIN_MENU: handleMainMenu(); break;
    case SETTINGS: handleSettings(); break;
    case SYSTEM_INFO: handleSystemInfo(); break;
    case MINI_APPS: handleMiniApps(); break;
    case APPS: handleApps(); break;
    case GAMES: handleGames(); break;
    case STOPWATCH: handleStopwatch(); break;
    case WIFI_SCANNER: handleWifiScanner(); break;
    case TIMER_APP: handleTimerApp(); break;
    case FILE_MANAGER: handleFileManager(); break;
    case DRAW_APP: handleDrawApp(); break;
    case TEMP_CONVERTER: handleTempConverter(); break;
    case COUNTER: handleCounter(); break;
    case TEXT_EDITOR: handleTextEditor(); break;
    case GAME_PONG: handlePongGame(); break;
    case GAME_ASTEROIDS: handleAsteroidsGame(); break;
    case GAME_FLAPPY_BIRD: handleFlappyBirdGame(); break;
    case GAME_TETRIS: handleTetrisGame(); break;
    case GAME_DINO: handleDinoGame(); break;
    case GAME_SNAKE: handleSnakeGame(); break;
    case GAME_ARKANOID: handleArkanoidGame(); break;
    case GAME_DICE: handleDiceGame(); break;
    case MULTIPLICATION_TABLE: handleMultiplicationTable(); break;
    case READER_APP: handleReaderApp(); break;
  }
}

void showBootScreen() {
  oled.clear(); oled.setCursor(0, 0); oled.setScale(1); oled.print("By Lilux12");
  oled.setCursor(95, 0); oled.print("v3.6R"); oled.setCursor(6, 3); oled.setScale(2);
  oled.print("Tema OS"); oled.setScale(1); oled.rect(0, 55, 127, 58, OLED_FILL); oled.update();
}

void handleFileCreate() {
  if (server.hasArg("filename") && server.hasArg("content")) {
    String filename = "/" + server.arg("filename");
    String content = server.arg("content");
    if (LittleFS.exists(filename)) { LittleFS.remove(filename); }
    File file = LittleFS.open(filename, "w");
    if (!file) { server.send(500, "text/plain", "Ошибка: не удалось создать файл."); return; }
    size_t bytesWritten = file.print(content);
    file.close();
    if (bytesWritten > 0) { server.send(200, "text/plain", "Файл '" + server.arg("filename") + "' успешно создан!"); } 
    else { server.send(500, "text/plain", "Ошибка: не удалось записать данные."); }
  } else { server.send(400, "text/plain", "Ошибка: отсутствуют имя файла или его содержимое."); }
}

void handleRoot() {
  String html = R"rawliteral(
  <!DOCTYPE html><html><head><meta charset='utf-8'><title>TemaOS File Manager</title>
  <style>
    body { font-family: sans-serif; background: #f0f0f0; color: #333; }
    .container { max-width: 800px; margin: 20px auto; padding: 20px; background: #fff; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
    h1, h2 { color: #1a1a1a; }
    input, textarea, select, button { width: 100%; padding: 10px; margin: 5px 0 15px; border-radius: 4px; border: 1px solid #ccc; box-sizing: border-box; }
    textarea { height: 150px; resize: vertical; }
    button { background: #007bff; color: white; border: none; cursor: pointer; font-size: 16px; }
    button:hover { background: #0056b3; }
    #fileList { list-style-type: none; padding: 0; }
    #fileList li { display: flex; justify-content: space-between; align-items: center; padding: 8px; border-bottom: 1px solid #eee; }
    .delete-btn { background: #dc3545; color: white; border: none; padding: 5px 10px; border-radius: 4px; cursor: pointer; }
    .delete-btn:hover { background: #c82333; }
  </style>
  </head><body><div class='container'>
  <h1>TemaOS File Manager</h1>
  <h2>Создать/Загрузить файл</h2>
  <form id="createForm" action="/create" method="post">
    <label for="filename">Имя файла (например, test.txt или image.tos):</label>
    <input type="text" id="filename" name="filename" required>
    <label for="content">Содержимое (для .tos вставьте массив байтов):</label>
    <textarea id="content" name="content" required></textarea>
    <button type="submit">Создать Файл</button>
  </form>
  <h2>Существующие файлы</h2>
  <ul id="fileList"></ul>
  </div>
  <script>
    function fetchFiles() {
      fetch('/list').then(response => response.json()).then(files => {
        const fileList = document.getElementById('fileList');
        fileList.innerHTML = '';
        files.forEach(file => {
          const li = document.createElement('li');
          li.textContent = file.name + ' (' + file.size + ' bytes)';
          const deleteBtn = document.createElement('button');
          deleteBtn.textContent = 'Удалить';
          deleteBtn.className = 'delete-btn';
          deleteBtn.onclick = () => deleteFile(file.name);
          li.appendChild(deleteBtn);
          fileList.appendChild(li);
        });
      });
    }
    function deleteFile(filename) {
      if (confirm('Вы уверены, что хотите удалить ' + filename + '?')) {
        const formData = new FormData();
        formData.append('filename', filename);
        fetch('/delete', { method: 'POST', body: formData })
          .then(response => response.text())
          .then(result => {
            alert(result);
            fetchFiles();
          });
      }
    }
    document.addEventListener('DOMContentLoaded', fetchFiles);
  </script>
  </body></html>
  )rawliteral";
  server.send(200, "text/html", html);
}

void handleFileUpload() {
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = "/" + upload.filename;
    uploadFile = LittleFS.open(filename, "w");
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (uploadFile) uploadFile.write(upload.buf, upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    if (uploadFile) uploadFile.close();
  }
}

// --- Функции навигации и меню ---
void handleMainMenu() {
  const char* mainMenuItems[] = {"Выключение", "Перезагрузка", "Мини приложения", "Настройки"};
  mainMenuState.maxItems = 4;
  handleMenuNavigation(mainMenuState, mainMenuState.maxItems, 4);
  drawMenu("Меню", mainMenuItems, mainMenuState.maxItems, mainMenuState.page, mainMenuState.maxPages);
  if (selectBtn.isClick()) {
    switch (mainMenuState.page * 4 + mainMenuState.index) {
      case 0: oled.clear(); oled.print("Выключение..."); oled.update(); delay(1000); ESP.deepSleep(0); break;
      case 1: oled.clear(); oled.print("Перезагрузка..."); oled.update(); delay(1000); ESP.restart(); break;
      case 2: previousState = currentState; currentState = MINI_APPS; resetMenuState(miniAppsMenuState); break;
      case 3: previousState = currentState; currentState = SETTINGS; resetMenuState(settingsMenuState); break;
    }
  }
}

void handleSettings() {
  const char* settingsItems[] = {"Калибровка", "О системе", "Назад"};
  settingsMenuState.maxItems = 3;
  handleMenuNavigation(settingsMenuState, settingsMenuState.maxItems, 3);
  drawMenu("Настройки", settingsItems, settingsMenuState.maxItems, settingsMenuState.page, settingsMenuState.maxPages);
  if (selectBtn.isClick()) {
    switch (settingsMenuState.page * 3 + settingsMenuState.index) {
      case 0: showMessage("Калибровка..."); delay(2000); break;
      case 1: previousState = currentState; currentState = SYSTEM_INFO; break;
      case 2: currentState = MAIN_MENU; resetMenuState(mainMenuState); break;
    }
  }
  if (exitBtn.isClick()) { currentState = MAIN_MENU; resetMenuState(mainMenuState); }
}

void handleSystemInfo() {
  oled.clear(); oled.setCursor(0, 0); oled.print("О системе"); oled.line(0, 10, 127, 10);
  oled.setCursor(0, 2); oled.print("TemaOS v3.6R"); oled.setCursor(0, 3); oled.print("By Lilux12");
  oled.setCursor(0, 4); oled.print("ESP32 Platform"); oled.setCursor(0, 5); oled.print("RAM: "); oled.print(ESP.getFreeHeap());
  oled.setCursor(0, 7); oled.print("EXIT: назад"); oled.update();
  if (exitBtn.isClick()) { currentState = SETTINGS; resetMenuState(settingsMenuState); }
}

void handleMiniApps() {
  const char* miniAppsItems[] = {"Игры", "Приложения", "Назад"};
  miniAppsMenuState.maxItems = 3;
  handleMenuNavigation(miniAppsMenuState, miniAppsMenuState.maxItems, 3);
  drawMenu("Мини приложения", miniAppsItems, miniAppsMenuState.maxItems, miniAppsMenuState.page, miniAppsMenuState.maxPages);
  if (selectBtn.isClick()) {
    switch (miniAppsMenuState.page * 3 + miniAppsMenuState.index) {
      case 0: previousState = currentState; currentState = GAMES; resetMenuState(gamesMenuState); break;
      case 1: previousState = currentState; currentState = APPS; resetMenuState(appsMenuState); break;
      case 2: currentState = MAIN_MENU; resetMenuState(mainMenuState); break;
    }
  }
  if (exitBtn.isClick()) { currentState = MAIN_MENU; resetMenuState(mainMenuState); }
}

void handleApps() {
  const char* appsItems[] = { "Секундомер", "Сканер WiFi", "Таймер", "Файловый менеджер", "Рисовалка", "Конвертер темп", "Счетчик", "Текстовый редактор", "Таблица умножения", "Читалка", "Назад" };
  appsMenuState.maxItems = 11;
  int itemsPerPage = 5;
  handleMenuNavigation(appsMenuState, appsMenuState.maxItems, itemsPerPage);
  drawMenu("Приложения", appsItems, appsMenuState.maxItems, appsMenuState.page, appsMenuState.maxPages);
  if (selectBtn.isClick()) {
    previousState = currentState;
    switch (appsMenuState.page * itemsPerPage + appsMenuState.index) {
      case 0: initStopwatch(); currentState = STOPWATCH; break;
      case 1: currentState = WIFI_SCANNER; break;
      case 2: initTimerApp(); currentState = TIMER_APP; break;
      case 3: currentState = FILE_MANAGER; break;
      case 4: initDrawApp(); currentState = DRAW_APP; break;
      case 5: initTempConverter(); currentState = TEMP_CONVERTER; break;
      case 6: initCounter(); currentState = COUNTER; break;
      case 7: initTextEditor(); currentState = TEXT_EDITOR; break;
      case 8: initMultiplicationTable(); currentState = MULTIPLICATION_TABLE; break;
      case 9: initReaderApp(); currentState = READER_APP; break;
      case 10: currentState = MINI_APPS; resetMenuState(miniAppsMenuState); break;
    }
  }
  if (exitBtn.isClick()) { currentState = MINI_APPS; resetMenuState(miniAppsMenuState); }
}

void handleGames() {
  const char* gamesItems[] = { "Тетрис", "Змейка", "Flappy Bird", "Арканоид", "Ардуино дино", "Астероид", "Понг", "Кубик", "Назад" };
  gamesMenuState.maxItems = 9;
  handleMenuNavigation(gamesMenuState, gamesMenuState.maxItems, 5);
  drawMenu("Игры", gamesItems, gamesMenuState.maxItems, gamesMenuState.page, gamesMenuState.maxPages);
  if (selectBtn.isClick()) {
    previousState = currentState;
    switch (gamesMenuState.page * 5 + gamesMenuState.index) {
      case 0: initTetrisGame(); currentState = GAME_TETRIS; break;
      case 1: initSnakeGame(); currentState = GAME_SNAKE; break;
      case 2: initFlappyBirdGame(); currentState = GAME_FLAPPY_BIRD; break;
      case 3: initArkanoidGame(); currentState = GAME_ARKANOID; break;
      case 4: initDinoGame(); currentState = GAME_DINO; break;
      case 5: initAsteroidsGame(); currentState = GAME_ASTEROIDS; break;
      case 6: initPongGame(); currentState = GAME_PONG; break;
      case 7: currentState = GAME_DICE; break;
      case 8: currentState = MINI_APPS; resetMenuState(miniAppsMenuState); break;
    }
  }
  if (exitBtn.isClick()) { currentState = MINI_APPS; resetMenuState(miniAppsMenuState); }
}

// --- Приложения ---
void handleStopwatch() {
  if (exitBtn.isClick()) { currentState = previousState; return; }
  oled.clear();
  oled.setCursor(0, 0); oled.setScale(1); oled.print("Секундомер"); oled.line(0, 10, 127, 10);
  if (selectBtn.isClick()) {
    if (!stopwatch.running) {
      stopwatch.startTime = millis() - stopwatch.elapsedTime;
      stopwatch.running = true;
    } else {
      stopwatch.running = false;
    }
  }
  if (upBtn.isClick()) {
    stopwatch.startTime = 0;
    stopwatch.elapsedTime = 0;
    if(stopwatch.running) stopwatch.startTime = millis();
  }
  if (stopwatch.running) {
    stopwatch.elapsedTime = millis() - stopwatch.startTime;
  }
  int minutes = (stopwatch.elapsedTime / 60000) % 60;
  int seconds = (stopwatch.elapsedTime / 1000) % 60;
  int milliseconds = (stopwatch.elapsedTime % 1000) / 10;
  oled.setCursor(2, 3);
  oled.setScale(2);
  char timeStr[20];
  sprintf(timeStr, "%02d:%02d.%02d", minutes, seconds, milliseconds);
  oled.print(timeStr);
  oled.setScale(1);
  oled.setCursor(0, 6); oled.print("SELECT: старт/стоп");
  oled.setCursor(0, 7); oled.print("UP: сброс EXIT: выход");
  oled.update();
}

void handleWifiScanner() {
  if (exitBtn.isClick()) { WiFi.scanDelete(); currentState = previousState; return; }
  oled.clear();
  oled.setCursor(0, 0); oled.setScale(1); oled.print("Сканер WiFi"); oled.line(0, 10, 127, 10);
  int n = WiFi.scanComplete();
  if (n == WIFI_SCAN_RUNNING) {
      oled.setCursor(0, 2); oled.print("Сканирование...");
  } else if (n == WIFI_SCAN_FAILED) {
      oled.setCursor(0, 2); oled.print("Ошибка сканирования");
      oled.setCursor(0, 3); oled.print("SELECT: повторить");
  } else {
      wifiScanner.networkCount = n;
      wifiScanner.totalPages = calculateMenuPages(wifiScanner.networkCount, wifiScanner.networksPerPage);
      if (wifiScanner.networkCount == 0) {
          oled.setCursor(0, 2); oled.print("Нет сетей");
      } else {
          oled.setCursor(0, 2); oled.print("Найдено: "); oled.print(wifiScanner.networkCount);
          int startIndex = wifiScanner.currentPage * wifiScanner.networksPerPage;
          int endIndex = min(startIndex + wifiScanner.networksPerPage, wifiScanner.networkCount);
          for (int i = startIndex; i < endIndex; i++) {
              oled.setCursor(0, 3 + (i - startIndex));
              String ssid = WiFi.SSID(i);
              if (ssid.length() > 15) ssid = ssid.substring(0, 15) + "..";
              oled.print(ssid.c_str());
              oled.setCursor(90, 3 + (i - startIndex));
              oled.print(WiFi.RSSI(i));
          }
          if (wifiScanner.totalPages > 1) {
              oled.setCursor(100, 0); oled.print("("); oled.print(wifiScanner.currentPage + 1); oled.print("/"); oled.print(wifiScanner.totalPages); oled.print(")");
          }
      }
  }
  oled.setCursor(0, 7); oled.print("SELECT: обновить");
  oled.setCursor(90, 7); oled.print("EXIT");
  oled.update();
  if (wifiScanner.totalPages > 1) {
      if (leftBtn.isClick()) wifiScanner.currentPage = (wifiScanner.currentPage - 1 + wifiScanner.totalPages) % wifiScanner.totalPages;
      if (rightBtn.isClick()) wifiScanner.currentPage = (wifiScanner.currentPage + 1) % wifiScanner.totalPages;
  }
  if (selectBtn.isClick()) { WiFi.scanNetworks(true); }
}

void handleTimerApp() {
  if (exitBtn.isClick()) { currentState = previousState; return; }
  oled.clear();
  oled.setCursor(0, 0); oled.setScale(1); oled.print("Таймер"); oled.line(0, 10, 127, 10);
  if (timerApp.alarmTriggered) {
      oled.setCursor(20, 3); oled.setScale(2); oled.print("ВРЕМЯ!");
      oled.setScale(1); oled.setCursor(0, 6); oled.print("SELECT: сброс");
  } else {
      unsigned long remainingTime = 0;
      if (timerApp.running) {
          unsigned long elapsed = millis() - timerApp.startTime;
          if (elapsed >= timerApp.setTime * 1000) {
              timerApp.alarmTriggered = true;
              remainingTime = 0;
          } else {
             remainingTime = (timerApp.setTime * 1000) - elapsed;
          }
      } else {
          remainingTime = timerApp.setTime * 1000;
      }
      int minutes = (remainingTime / 60000) % 60;
      int seconds = (remainingTime / 1000) % 60;
      oled.setCursor(20, 3); oled.setScale(2);
      char timeStr[10];
      sprintf(timeStr, "%02d:%02d", minutes, seconds);
      oled.print(timeStr);
      oled.setScale(1);
      oled.setCursor(0, 6);
      if (timerApp.running) oled.print("SELECT: пауза");
      else oled.print("SELECT: старт");
      oled.setCursor(0, 7); oled.print("UP/DOWN: +/-1 мин");
  }
  oled.setCursor(90, 7); oled.print("EXIT");
  oled.update();
  if (!timerApp.alarmTriggered) {
      if (selectBtn.isClick()) {
          if (timerApp.running) {
              timerApp.running = false;
              timerApp.setTime = ((timerApp.setTime * 1000) - (millis() - timerApp.startTime)) / 1000;
          } else {
              if (timerApp.setTime > 0) {
                  timerApp.startTime = millis();
                  timerApp.running = true;
              }
          }
      }
      if (upBtn.isClick()) timerApp.setTime = min((unsigned long)(timerApp.setTime + 60), 3599UL);
      if (downBtn.isClick()) {
          if(timerApp.setTime >= 60) timerApp.setTime -= 60; else timerApp.setTime = 0;
      }
  } else {
      if (selectBtn.isClick()) {
          timerApp.alarmTriggered = false;
          timerApp.setTime = 0;
          timerApp.running = false;
      }
  }
}

void handleFileManager() {
  if (exitBtn.isClick()) { currentState = previousState; return; }
  oled.clear();
  oled.setCursor(0, 0); oled.setScale(1); oled.print("Файловый менеджер"); oled.line(0, 10, 127, 10);
  oled.setCursor(0, 2); oled.print("Откройте в браузере:");
  oled.setCursor(0, 3); oled.print(WiFi.softAPIP().toString().c_str());
  oled.setCursor(0, 5); oled.print("Файлы на ESP32:");
  File root = LittleFS.open("/");
  File file = root.openNextFile();
  int y = 6;
  while (file && y < 8) {
      oled.setCursor(0, y);
      String filename = String(file.name());
      int slashIndex = filename.lastIndexOf('/');
      String displayName = (slashIndex != -1) ? filename.substring(slashIndex + 1) : filename;
      if (displayName.length() > 15) displayName = displayName.substring(0, 15) + "..";
      oled.print(displayName.c_str());
      file = root.openNextFile();
      y++;
  }
  oled.setCursor(90, 7); oled.print("EXIT");
  oled.update();
}

void handleDrawApp() {
    if (exitBtn.isClick()) { currentState = previousState; return; }
    if (exitBtn.isHold()) { oled.clear(); }
    int prevX = drawApp.cursorX;
    int prevY = drawApp.cursorY;
    if (upBtn.isHold()) drawApp.cursorY = max(11, drawApp.cursorY - 1);
    if (downBtn.isHold()) drawApp.cursorY = min(63, drawApp.cursorY + 1);
    if (leftBtn.isHold()) drawApp.cursorX = max(0, drawApp.cursorX - 1);
    if (rightBtn.isHold()) drawApp.cursorX = min(127, drawApp.cursorX + 1);
    if (selectBtn.isHold()) {
        oled.line(prevX, prevY, drawApp.cursorX, drawApp.cursorY);
    }
    oled.update();
    oled.rect(0, 0, 127, 10, OLED_CLEAR);
    oled.setCursor(0, 0); oled.setScale(1); oled.print("Рисовалка");
    char coords[10];
    sprintf(coords, "X:%d Y:%d", drawApp.cursorX, drawApp.cursorY);
    oled.setCursor(80, 0); oled.print(coords);
    oled.line(0, 9, 127, 9);
    if (!selectBtn.isHold()) {
       if (millis() % 600 < 300) {
           oled.dot(drawApp.cursorX, drawApp.cursorY);
       }
    } else {
        oled.dot(drawApp.cursorX, drawApp.cursorY);
    }
    oled.update();
}

void handleTempConverter() {
  if (exitBtn.isClick()) { currentState = previousState; return; }
  oled.clear();
  oled.setCursor(0, 0); oled.setScale(1); oled.print("Конвертер темп."); oled.line(0, 10, 127, 10);
  oled.setCursor(0, 2); oled.print("Цельсий: "); oled.print(tempConverter.celsius, 1);
  oled.setCursor(0, 3); oled.print("Фаренгейт: "); oled.print(tempConverter.fahrenheit, 1);
  oled.setCursor(0, 5);
  if (tempConverter.convertingCtoF) oled.print("Изменяйте Цельсий");
  else oled.print("Изменяйте Фаренгейт");
  oled.setCursor(0, 6); oled.print("UP/DOWN: +/-1");
  oled.setCursor(0, 7); oled.print("SELECT: сменить");
  oled.setCursor(90, 7); oled.print("EXIT");
  oled.update();
  if (tempConverter.convertingCtoF) {
      if (upBtn.isClick()) tempConverter.celsius += 1.0;
      if (downBtn.isClick()) tempConverter.celsius -= 1.0;
      tempConverter.fahrenheit = (tempConverter.celsius * 9.0/5.0) + 32.0;
  } else {
      if (upBtn.isClick()) tempConverter.fahrenheit += 1.0;
      if (downBtn.isClick()) tempConverter.fahrenheit -= 1.0;
      tempConverter.celsius = (tempConverter.fahrenheit - 32.0) * 5.0/9.0;
  }
  if (selectBtn.isClick()) { tempConverter.convertingCtoF = !tempConverter.convertingCtoF; }
}

void handleCounter() {
  if (exitBtn.isClick()) { currentState = previousState; return; }
  oled.clear();
  oled.setCursor(0, 0); oled.setScale(1); oled.print("Счетчик"); oled.line(0, 10, 127, 10);
  oled.setCursor(0, 3); oled.setScale(3); oled.print(counterApp.count);
  oled.setScale(1);
  oled.setCursor(0, 7); oled.print("UP: +1, DOWN: -1");
  oled.setCursor(90, 7); oled.print("EXIT");
  oled.update();
  if (upBtn.isClick()) counterApp.count++;
  if (downBtn.isClick()) counterApp.count--;
}

void handleTextEditor() {
  if (exitBtn.isClick()) { currentState = previousState; return; }
  oled.clear();
  oled.setCursor(0, 0); oled.setScale(1); oled.print("Текстовый редактор"); oled.line(0, 10, 127, 10);
  oled.setCursor(0, 2); oled.print("Файл: ");
  if (textEditor.filename.length() > 0) oled.print(textEditor.filename.c_str());
  else oled.print("новый");
  oled.setCursor(0, 3);
  oled.print(textEditor.content.substring(0, 21));
  oled.setCursor(0, 7); oled.print("SELECT: сохранить");
  oled.setCursor(90, 7); oled.print("EXIT");
  oled.update();
  if (selectBtn.isClick()) { showMessage("Файл сохранен!"); delay(1000); }
}

void handleMultiplicationTable() {
    if (exitBtn.isClick()) { currentState = previousState; return; }
    if (selectBtn.isClick()) { 
        multiplicationTable.multiplier1 = 1;
        multiplicationTable.multiplier2 = 1;
    }
    oled.clear();
    oled.setCursor(0, 0); oled.setScale(1); oled.print("Таблица умножения"); oled.line(0, 10, 127, 10);
    if (upBtn.isClick()) multiplicationTable.multiplier1 = min(multiplicationTable.multiplier1 + 1, 10);
    if (downBtn.isClick()) multiplicationTable.multiplier1 = max(multiplicationTable.multiplier1 - 1, 1);
    if (rightBtn.isClick()) multiplicationTable.multiplier2 = min(multiplicationTable.multiplier2 + 1, 10);
    if (leftBtn.isClick()) multiplicationTable.multiplier2 = max(multiplicationTable.multiplier2 - 1, 1);
    char buffer[20];
    int result = multiplicationTable.multiplier1 * multiplicationTable.multiplier2;
    sprintf(buffer, "%d x %d = %d", multiplicationTable.multiplier1, multiplicationTable.multiplier2, result);
    int textWidth = strlen(buffer) * 12;
    int startX = (128 - textWidth) / 2;
    if (startX < 0) startX = 0;
    oled.setCursor(startX, 3);
    oled.setScale(2);
    oled.print(buffer);
    oled.setScale(1);
    oled.setCursor(0, 6); oled.print("UP/DN: 1-й, L/R: 2-й");
    oled.setCursor(0, 7); oled.print("SELECT: сброс");
    oled.setCursor(90, 7); oled.print("EXIT");
    oled.update();
}


// --- Игры ---
void handleDinoGame() {
  if (exitBtn.isClick()) { currentState = previousState; return; }
  if (dino.gameOver) {
    oled.clear();
    oled.setCursor(3, 2); oled.setScale(2); oled.print("GAME OVER");
    oled.setScale(1); oled.setCursor(2, 4); oled.print("Счет: "); oled.print(dino.score);
    oled.setCursor(0, 6); oled.print("SELECT: заново");
    oled.setCursor(0, 7); oled.print("EXIT: выход");
    oled.update();
    if (selectBtn.isClick()) { 
        initDinoGame();
    }
    return; 
  }
  if (upBtn.isClick() && dino.dinoY >= 47 && !dino.jumping) {
    dino.dinoSpeed = -2.8;
    dino.jumping = true;
    dino.crouching = false; 
  }
  if (downBtn.isHold()) {
    dino.crouching = true;
    if (dino.dinoY < 47) dino.dinoSpeed = 3.2;
  } else {
    dino.crouching = false;
  }
  unsigned long currentMillis = millis();
  if (currentMillis - dino.lastScoreUpdate >= 100) {
    dino.lastScoreUpdate = currentMillis;
    dino.score++;
    if (dino.score < 500) {
      dino.gameSpeed = constrain(map(dino.score, 0, 500, 6, 2), 2, 6);
    } else {
      dino.gameSpeed = 2;
    }
  }
  dino.obstacleX--;
  if (dino.obstacleX < -24) {
    dino.obstacleX = 128;
    dino.enemyType = random(0, 3);
  }
  if (currentMillis - dino.lastLegUpdate >= 130) {
    dino.lastLegUpdate = currentMillis;
    dino.legFlag = !dino.legFlag;
  }
  if (currentMillis - dino.lastBirdUpdate >= 200 && dino.enemyType == 2) {
    dino.lastBirdUpdate = currentMillis;
    dino.birdFlag = !dino.birdFlag;
  }
  if (dino.jumping || dino.crouching) {
    dino.dinoY += dino.dinoSpeed;
    dino.dinoSpeed += 0.17;
    if (dino.dinoY >= 47) {
      dino.dinoY = 47;
      dino.dinoSpeed = 0;
      dino.jumping = false;
    }
  }
  int dinoLeft = 0;
  int dinoRight = dino.crouching ? 16 : 16;
  int dinoTop = dino.dinoY;
  int dinoBottom = dino.crouching ? dino.dinoY + 8 : dino.dinoY + 16;
  int obstacleLeft = dino.obstacleX;
  int obstacleRight = dino.obstacleX + (dino.enemyType == 1 ? 24 : 16);
  int obstacleTop = (dino.enemyType == 2) ? 35 : 48;
  int obstacleBottom = (dino.enemyType == 2) ? 35 + 16 : 48 + 16;
  if ((dinoLeft < obstacleRight) && (dinoRight > obstacleLeft) && (dinoTop < obstacleBottom) && (dinoBottom > obstacleTop)) {
    dino.gameOver = true;
  }
  oled.clear();
  oled.setCursor(0, 0); oled.setScale(1); oled.print("Счет: "); oled.print(dino.score);
  oled.line(0, 63, 127, 63);
  if (dino.obstacleX >= -24 && dino.obstacleX < 128) {
    switch (dino.enemyType) {
      case 0: oled.drawBitmap(dino.obstacleX, 48, CactusSmall_bmp, 16, 16); break;
      case 1: oled.drawBitmap(dino.obstacleX, 48, CactusBig_bmp, 24, 16); break;
      case 2: oled.drawBitmap(dino.obstacleX, 35, dino.birdFlag ? BirdL_bmp : BirdR_bmp, 24, 16); break;
    }
  }
  if (dino.gameOver) oled.drawBitmap(0, dino.dinoY, DinoStandDie_bmp, 16, 16);
  else if (dino.crouching) oled.drawBitmap(0, 56, dino.legFlag ? DinoCroachL_bmp : DinoCroachR_bmp, 16, 8);
  else oled.drawBitmap(0, dino.dinoY, dino.legFlag ? DinoStandL_bmp : DinoStandR_bmp, 16, 16);
  oled.update();
}

void handleSnakeGame() {
  if (exitBtn.isClick()) { currentState = previousState; return; }
  if (snake.gameOver) {
    oled.clear();
    oled.setCursor(3, 2); oled.setScale(2); oled.print("GAME OVER");
    oled.setScale(1); oled.setCursor(2, 4); oled.print("Счет: "); oled.print(snake.score);
    oled.setCursor(0, 6); oled.print("SELECT: заново");
    oled.setCursor(0, 7); oled.print("EXIT: выход");
    oled.update();
    if (selectBtn.isClick()) {
        initSnakeGame();
    }
    return;
  }
  if (upBtn.isClick() && snake.dirY == 0) { snake.dirX = 0; snake.dirY = -1; }
  else if (downBtn.isClick() && snake.dirY == 0) { snake.dirX = 0; snake.dirY = 1; }
  else if (leftBtn.isClick() && snake.dirX == 0) { snake.dirX = -1; snake.dirY = 0; }
  else if (rightBtn.isClick() && snake.dirX == 0) { snake.dirX = 1; snake.dirY = 0; }
  if (millis() - snake.lastMoveTime > snake.moveDelay) {
    snake.lastMoveTime = millis();
    int newX = snake.snakeX[0] + snake.dirX * snake.segmentSize;
    int newY = snake.snakeY[0] + snake.dirY * snake.segmentSize;
    if (newX < 0 || newX >= 128 || newY < 12 || newY >= 64) { snake.gameOver = true; return; }
    for (int i = 0; i < snake.snakeLength; i++) {
      if (newX == snake.snakeX[i] && newY == snake.snakeY[i]) { snake.gameOver = true; return; }
    }
    for (int i = snake.snakeLength - 1; i > 0; i--) {
      snake.snakeX[i] = snake.snakeX[i - 1];
      snake.snakeY[i] = snake.snakeY[i - 1];
    }
    snake.snakeX[0] = newX;
    snake.snakeY[0] = newY;
    if (newX == snake.foodX && newY == snake.foodY) {
      if (snake.snakeLength < SnakeGame::MAX_LENGTH - 1) snake.snakeLength++;
      snake.score += 10;
      snake.moveDelay = max(80, snake.moveDelay - 3); 
      snake.foodX = random(0, 32) * 4; snake.foodY = random(3, 16) * 4;
    }
  }
  oled.clear();
  oled.setCursor(0, 0); oled.setScale(1); oled.print("Змейка Счет: "); oled.print(snake.score);
  oled.line(0, 11, 127, 11);
  for (int i = 0; i < snake.snakeLength; i++) {
    oled.rect(snake.snakeX[i], snake.snakeY[i], snake.snakeX[i] + snake.segmentSize - 1, snake.snakeY[i] + snake.segmentSize - 1, OLED_FILL);
  }
  oled.rect(snake.foodX, snake.foodY, snake.foodX + snake.segmentSize - 1, snake.foodY + snake.segmentSize - 1, OLED_STROKE);
  oled.update();
}

void handleTetrisGame() {
  if (exitBtn.isClick()) { currentState = previousState; return; }
  if (tetris.gameOver) {
    oled.clear();
    oled.setCursor(3, 2); oled.setScale(2); oled.print("GAME OVER");
    oled.setScale(1); oled.setCursor(2, 4); oled.print("Счет: "); oled.print(tetris.score);
    oled.setCursor(0, 6); oled.print("SELECT: заново");
    oled.setCursor(0, 7); oled.print("EXIT: выход");
    oled.update();
    if (selectBtn.isClick()) {
        initTetrisGame();
    }
    return;
  }
  unsigned long currentTime = millis();
  if (leftBtn.isClick()) if (!tetrisCheckCollision(tetris.pieceX - 1, tetris.pieceY)) tetris.pieceX--;
  if (rightBtn.isClick()) if (!tetrisCheckCollision(tetris.pieceX + 1, tetris.pieceY)) tetris.pieceX++;
  if (downBtn.isHold()) tetris.dropDelay = 50; 
  else tetris.dropDelay = max(100, 500 - (tetris.score / 100) * 50); 
  if (upBtn.isClick()) tetrisRotatePiece();
  if (currentTime - tetris.lastDropTime > tetris.dropDelay) {
    tetris.lastDropTime = currentTime;
    if (!tetrisCheckCollision(tetris.pieceX, tetris.pieceY + 1)) {
      tetris.pieceY++;
    } else {
      tetrisPlacePiece();
      tetrisClearLines();
      tetrisNewPiece();
    }
  }
  oled.clear();
  oled.setCursor(0, 0); oled.setScale(1); oled.print("Тетрис "); oled.print(tetris.score);
  int blockSize = 3; int fieldLeft = 40; int fieldTop = 14;
  int fieldWidth = tetris.FIELD_WIDTH * blockSize; int fieldHeight = tetris.FIELD_HEIGHT * blockSize; 
  oled.rect(fieldLeft - 1, fieldTop - 1, fieldLeft + fieldWidth, fieldTop + fieldHeight, OLED_STROKE);
  for (int y = 0; y < tetris.FIELD_HEIGHT; y++) {
    for (int x = 0; x < tetris.FIELD_WIDTH; x++) {
      if (tetris.field[y][x]) oled.rect(fieldLeft + x * blockSize, fieldTop + y * blockSize, fieldLeft + x * blockSize + blockSize - 1, fieldTop + y * blockSize + blockSize - 1, OLED_FILL);
    }
  }
  for (int i = 0; i < 4; i++) {
    int px = tetris.currentPiece[i][0] + tetris.pieceX;
    int py = tetris.currentPiece[i][1] + tetris.pieceY;
    oled.rect(fieldLeft + px * blockSize, fieldTop + py * blockSize, fieldLeft + px * blockSize + blockSize - 1, fieldTop + py * blockSize + blockSize - 1, OLED_FILL);
  }
  int previewX = fieldLeft + fieldWidth + 10; int previewY = fieldTop + 10;
  oled.setCursor(previewX - 5, fieldTop); oled.print("След:");
  for (int i = 0; i < 4; i++) {
    int px = PIECES[tetris.nextPieceType][i][0];
    int py = PIECES[tetris.nextPieceType][i][1];
    oled.rect(previewX + px * blockSize, previewY + py * blockSize, previewX + px * blockSize + blockSize -1, previewY + py * blockSize + blockSize -1, OLED_FILL);
  }
  oled.update();
}

void handleArkanoidGame() {
  if (exitBtn.isClick()) { currentState = previousState; return; }
  if (arkanoid.gameOver) {
    oled.clear();
    oled.setCursor(3, 2); oled.setScale(2); oled.print("GAME OVER");
    oled.setScale(1); oled.setCursor(2, 4); oled.print("Счет: "); oled.print(arkanoid.score);
    oled.setCursor(0, 6); oled.print("SELECT: заново");
    oled.setCursor(0, 7); oled.print("EXIT: выход");
    oled.update();
    if (selectBtn.isClick()) {
        initArkanoidGame();
    }
    return; 
  }
  if (gameTimer.isReady()) {
    if (leftBtn.isHold()) arkanoid.paddleX -= 4;
    if (rightBtn.isHold()) arkanoid.paddleX += 4;
    if (arkanoid.paddleX < 0) arkanoid.paddleX = 0;
    if (arkanoid.paddleX > 128 - arkanoid.paddleWidth) arkanoid.paddleX = 128 - arkanoid.paddleWidth;
    arkanoid.ballX += arkanoid.ballVelX;
    arkanoid.ballY += arkanoid.ballVelY;
    if (arkanoid.ballX <= 0 || arkanoid.ballX >= 127 - arkanoid.ballSize) arkanoid.ballVelX = -arkanoid.ballVelX;
    if (arkanoid.ballY <= 12) arkanoid.ballVelY = -arkanoid.ballVelY;
    if (arkanoid.ballY + arkanoid.ballSize >= 62 && arkanoid.ballY <= 63 && arkanoid.ballX + arkanoid.ballSize >= arkanoid.paddleX && arkanoid.ballX <= arkanoid.paddleX + arkanoid.paddleWidth) {
        arkanoid.ballVelY = -abs(arkanoid.ballVelY);
    }
    int brickWidth = 10; int brickHeight = 4;
    int brickCol = (arkanoid.ballX - 2) / (brickWidth + 2);
    int brickRow = (arkanoid.ballY - 12) / (brickHeight + 1);
    if (brickCol >= 0 && brickCol < 10 && brickRow >= 0 && brickRow < 5) {
        if (arkanoid.bricks[brickRow][brickCol]) {
            arkanoid.bricks[brickRow][brickCol] = false;
            arkanoid.ballVelY = -arkanoid.ballVelY;
            arkanoid.score += 10;
        }
    }
    if (arkanoid.ballY >= 65) arkanoid.gameOver = true;
  }
  oled.clear();
  oled.setCursor(0, 0); oled.setScale(1); oled.print("Счет: "); oled.print(arkanoid.score);
  oled.line(0, 10, 127, 10);
  int brickWidth = 10; int brickHeight = 4;
  for (int y = 0; y < 5; y++) {
    for (int x = 0; x < 10; x++) {
      if (arkanoid.bricks[y][x]) oled.rect(x * (brickWidth + 2) + 2, y * (brickHeight + 1) + 12, x * (brickWidth + 2) + brickWidth, y * (brickHeight + 1) + 12 + brickHeight -1, OLED_FILL);
    }
  }
  oled.rect(arkanoid.paddleX, 62, arkanoid.paddleX + arkanoid.paddleWidth - 1, 63, OLED_FILL);
  oled.rect(arkanoid.ballX, arkanoid.ballY, arkanoid.ballX + arkanoid.ballSize - 1, arkanoid.ballY + arkanoid.ballSize - 1, OLED_FILL);
  oled.update();
}

void handlePongGame() {
    if (exitBtn.isClick()) { currentState = previousState; return; }
    if (pong.gameOver) {
        oled.clear();
        oled.setCursor(3, 2); oled.setScale(2);
        if (pong.score1 >= 5) { oled.print("ИГРОК 1"); oled.setCursor(15, 4); oled.print("ПОБЕДИЛ!"); }
        else { oled.print("КОМПЬЮТЕР"); oled.setCursor(15,4); oled.print("ПОБЕДИЛ!"); }
        oled.setScale(1); oled.setCursor(0, 7); oled.print("SELECT: заново EXIT");
        oled.update();
        if (selectBtn.isClick()) {
            initPongGame();
        }
        return; 
    }
    if (upBtn.isHold()) pong.paddle1Y = max(12.0f, pong.paddle1Y - 2);
    if (downBtn.isHold()) pong.paddle1Y = min((float)(64 - pong.paddleHeight), pong.paddle1Y + 2);
    float targetY = pong.ballY - (pong.paddleHeight / 2.0f);
    float dy = targetY - pong.paddle2Y;
    pong.paddle2Y += dy * 0.1; 
    pong.paddle2Y = constrain(pong.paddle2Y, 12.0f, 64.0f - pong.paddleHeight);
    if (gameTimer.isReady()) {
        pong.ballX += pong.ballVelX;
        pong.ballY += pong.ballVelY;
        if (pong.ballY <= 12 || pong.ballY >= 63) pong.ballVelY = -pong.ballVelY;
        if (pong.ballX <= 3 && pong.ballX >= 1 && pong.ballY >= pong.paddle1Y && pong.ballY <= pong.paddle1Y + pong.paddleHeight) {
            pong.ballVelX = abs(pong.ballVelX);
            float relativeIntersectY = (pong.paddle1Y + (pong.paddleHeight / 2.0)) - pong.ballY;
            pong.ballVelY = -(relativeIntersectY / (pong.paddleHeight / 2.0)) * 2;
        }
        if (pong.ballX >= 124 && pong.ballX <= 126 && pong.ballY >= pong.paddle2Y && pong.ballY <= pong.paddle2Y + pong.paddleHeight) {
            pong.ballVelX = -abs(pong.ballVelX);
            float relativeIntersectY = (pong.paddle2Y + (pong.paddleHeight / 2.0)) - pong.ballY;
            pong.ballVelY = -(relativeIntersectY / (pong.paddleHeight / 2.0)) * 2;
        }
        if (pong.ballX < 0) {
            pong.score2++;
            if (pong.score2 >= 5) pong.gameOver = true;
            else { pong.ballX = 64; pong.ballY = 32; pong.ballVelX = -1.5; pong.ballVelY = random(-10, 11) / 10.0; }
        }
        if (pong.ballX > 127) {
            pong.score1++;
            if (pong.score1 >= 5) pong.gameOver = true;
            else { pong.ballX = 64; pong.ballY = 32; pong.ballVelX = 1.5; pong.ballVelY = random(-10, 11) / 10.0; }
        }
    }
    oled.clear();
    oled.setCursor(0, 0); oled.setScale(1); oled.print(pong.score1);
    oled.setCursor(120, 0); oled.print(pong.score2);
    oled.line(0, 11, 127, 11); oled.line(0, 63, 127, 63); oled.line(64, 12, 64, 63, OLED_STROKE);
    oled.rect(1, pong.paddle1Y, 2, pong.paddle1Y + pong.paddleHeight - 1, OLED_FILL);
    oled.rect(125, pong.paddle2Y, 126, pong.paddle2Y + pong.paddleHeight - 1, OLED_FILL);
    oled.rect(pong.ballX, pong.ballY, pong.ballX + pong.ballSize - 1, pong.ballY + pong.ballSize - 1, OLED_FILL);
    oled.update();
}

void handleAsteroidsGame() {
    if (exitBtn.isClick()) { currentState = previousState; return; }
    if (asteroids.gameOver) {
        oled.clear();
        oled.setCursor(3, 2); oled.setScale(2); oled.print("GAME OVER");
        oled.setScale(1); oled.setCursor(2, 4); oled.print("Счет: "); oled.print(asteroids.score);
        oled.setCursor(0, 6); oled.print("SELECT: заново");
        oled.setCursor(0, 7); oled.print("EXIT: выход");
        oled.update();
        if (selectBtn.isClick()) {
            initAsteroidsGame();
        }
        return; 
    }
    if (leftBtn.isHold()) asteroids.shipAngle -= 0.1;
    if (rightBtn.isHold()) asteroids.shipAngle += 0.1;
    if (upBtn.isHold()) {
        asteroids.thrusting = true;
        asteroids.shipVelX += sin(asteroids.shipAngle) * 0.1;
        asteroids.shipVelY -= cos(asteroids.shipAngle) * 0.1;
    } else {
        asteroids.thrusting = false;
    }
    if (selectBtn.isClick()) {
        for (int i = 0; i < asteroids.MAX_BULLETS; i++) {
            if (!asteroids.bullets[i].active) {
                asteroids.bullets[i].active = true; asteroids.bullets[i].x = asteroids.shipX;
                asteroids.bullets[i].y = asteroids.shipY; asteroids.bullets[i].velX = sin(asteroids.shipAngle) * 3;
                asteroids.bullets[i].velY = -cos(asteroids.shipAngle) * 3;
                break;
            }
        }
    }
    if (gameTimer.isReady()) {
        asteroids.shipX += asteroids.shipVelX; asteroids.shipY += asteroids.shipVelY;
        asteroids.shipVelX *= 0.98; asteroids.shipVelY *= 0.98;
        if (asteroids.shipX < 0) asteroids.shipX = 127; if (asteroids.shipX > 127) asteroids.shipX = 0;
        if (asteroids.shipY < 12) asteroids.shipY = 63; if (asteroids.shipY > 63) asteroids.shipY = 12;
        for (int i = 0; i < asteroids.MAX_ASTEROIDS; i++) {
            if (asteroids.asteroids[i].active) {
                asteroids.asteroids[i].x += asteroids.asteroids[i].velX;
                asteroids.asteroids[i].y += asteroids.asteroids[i].velY;
                if (asteroids.asteroids[i].x < -10) asteroids.asteroids[i].x = 138;
                if (asteroids.asteroids[i].x > 138) asteroids.asteroids[i].x = -10;
                if (asteroids.asteroids[i].y < 2) asteroids.asteroids[i].y = 73;
                if (asteroids.asteroids[i].y > 73) asteroids.asteroids[i].y = 2;
            }
        }
        for (int i = 0; i < asteroids.MAX_BULLETS; i++) {
            if (asteroids.bullets[i].active) {
                asteroids.bullets[i].x += asteroids.bullets[i].velX;
                asteroids.bullets[i].y += asteroids.bullets[i].velY;
                if (asteroids.bullets[i].x < 0 || asteroids.bullets[i].x > 127 || asteroids.bullets[i].y < 12 || asteroids.bullets[i].y > 63) {
                    asteroids.bullets[i].active = false;
                }
            }
        }
        for (int i = 0; i < asteroids.MAX_BULLETS; i++) {
            if (asteroids.bullets[i].active) {
                for (int j = 0; j < asteroids.MAX_ASTEROIDS; j++) {
                    if (asteroids.asteroids[j].active) {
                        float dx = asteroids.bullets[i].x - asteroids.asteroids[j].x;
                        float dy = asteroids.bullets[i].y - asteroids.asteroids[j].y;
                        float distance = sqrt(dx*dx + dy*dy);
                        float asteroidRadius = (asteroids.asteroids[j].size + 1) * 4;
                        if (distance < asteroidRadius) {
                            asteroids.bullets[i].active = false;
                            asteroids.score += (3 - asteroids.asteroids[j].size) * 10;
                            if (asteroids.asteroids[j].size > 0) {
                                for (int k = 0; k < 2; k++) {
                                    for (int l = 0; l < asteroids.MAX_ASTEROIDS; l++) {
                                        if (!asteroids.asteroids[l].active) {
                                            asteroids.asteroids[l].active = true;
                                            asteroids.asteroids[l].x = asteroids.asteroids[j].x;
                                            asteroids.asteroids[l].y = asteroids.asteroids[j].y;
                                            asteroids.asteroids[l].velX = asteroids.asteroids[j].velX + random(-10, 11) / 10.0;
                                            asteroids.asteroids[l].velY = asteroids.asteroids[j].velY + random(-10, 11) / 10.0;
                                            asteroids.asteroids[l].size = asteroids.asteroids[j].size - 1;
                                            break;
                                        }
                                    }
                                }
                            }
                            asteroids.asteroids[j].active = false;
                            break;
                        }
                    }
                }
            }
        }
        for (int i = 0; i < asteroids.MAX_ASTEROIDS; i++) {
            if (asteroids.asteroids[i].active) {
                float dx = asteroids.shipX - asteroids.asteroids[i].x;
                float dy = asteroids.shipY - asteroids.asteroids[i].y;
                float distance = sqrt(dx*dx + dy*dy);
                float asteroidRadius = (asteroids.asteroids[i].size + 1) * 4;
                if (distance < asteroidRadius + 2) asteroids.gameOver = true;
            }
        }
    }
    oled.clear();
    oled.setCursor(0, 0); oled.setScale(1); oled.print("Счет: "); oled.print(asteroids.score);
    oled.line(0, 11, 127, 11);
    if (!asteroids.gameOver) {
        int x1 = asteroids.shipX + 4 * sin(asteroids.shipAngle); int y1 = asteroids.shipY - 4 * cos(asteroids.shipAngle);
        int x2 = asteroids.shipX + 3 * sin(asteroids.shipAngle + 2.5); int y2 = asteroids.shipY - 3 * cos(asteroids.shipAngle + 2.5);
        int x3 = asteroids.shipX + 3 * sin(asteroids.shipAngle - 2.5); int y3 = asteroids.shipY - 3 * cos(asteroids.shipAngle - 2.5);
        oled.line(x1, y1, x2, y2); oled.line(x2, y2, x3, y3); oled.line(x3, y3, x1, y1);
        if (asteroids.thrusting) {
            int flameX = asteroids.shipX - 4 * sin(asteroids.shipAngle); int flameY = asteroids.shipY + 4 * cos(asteroids.shipAngle);
            oled.line(asteroids.shipX, asteroids.shipY, flameX, flameY);
        }
    }
    for (int i = 0; i < asteroids.MAX_ASTEROIDS; i++) {
        if (asteroids.asteroids[i].active) {
            float radius = (asteroids.asteroids[i].size + 1) * 4;
            oled.circle(asteroids.asteroids[i].x, asteroids.asteroids[i].y, radius, OLED_STROKE);
        }
    }
    for (int i = 0; i < asteroids.MAX_BULLETS; i++) {
        if (asteroids.bullets[i].active) oled.dot(asteroids.bullets[i].x, asteroids.bullets[i].y);
    }
    oled.update();
}

void handleFlappyBirdGame() {
    if (exitBtn.isClick()) { currentState = previousState; return; }
    if (flappyBird.gameOver) {
        oled.clear();
        oled.setCursor(3, 2); oled.setScale(2); oled.print("GAME OVER");
        oled.setScale(1); oled.setCursor(2, 4); oled.print("Счет: "); oled.print(flappyBird.score);
        oled.setCursor(0, 6); oled.print("SELECT: заново");
        oled.setCursor(0, 7); oled.print("EXIT: выход");
        oled.update();
        if (selectBtn.isClick()) {
            initFlappyBirdGame();
        }
        return; 
    }
    if (selectBtn.isClick() || upBtn.isClick()) flappyBird.birdVel = -2.5;
    if (gameTimer.isReady()) {
        flappyBird.birdVel += 0.2;
        flappyBird.birdY += flappyBird.birdVel;
        if (flappyBird.birdY < 12 || flappyBird.birdY > 60) flappyBird.gameOver = true;
        for (int i = 0; i < flappyBird.MAX_PIPES; i++) {
            flappyBird.pipes[i].x -= 2;
            if (flappyBird.pipes[i].x < -20) {
                // Find the rightmost pipe to calculate the new position
                int max_x = 0;
                for(int j=0; j<flappyBird.MAX_PIPES; ++j) {
                    if (flappyBird.pipes[j].x > max_x) max_x = flappyBird.pipes[j].x;
                }
                flappyBird.pipes[i].x = max_x + flappyBird.pipeSpacing;
                flappyBird.pipes[i].gapY = random(20, 44);
                flappyBird.pipes[i].passed = false;
            }
            if (!flappyBird.pipes[i].passed && flappyBird.pipes[i].x < 20) {
                flappyBird.pipes[i].passed = true;
                flappyBird.score++;
            }
            if (flappyBird.pipes[i].x < 24 && flappyBird.pipes[i].x > 16) { // Bird is at x=20, width 4
                if (flappyBird.birdY < flappyBird.pipes[i].gapY || flappyBird.birdY + 4 > flappyBird.pipes[i].gapY + 20) {
                    flappyBird.gameOver = true;
                }
            }
        }
    }
    oled.clear();
    oled.setCursor(0, 0); oled.setScale(1); oled.print("Счет: "); oled.print(flappyBird.score);
    oled.line(0, 11, 127, 11); oled.line(0, 63, 127, 63);
    if (!flappyBird.gameOver) oled.rect(20, flappyBird.birdY, 23, flappyBird.birdY + 3, OLED_FILL);
    for (int i = 0; i < flappyBird.MAX_PIPES; i++) {
        if (flappyBird.pipes[i].x > -20 && flappyBird.pipes[i].x < 128) {
            oled.rect(flappyBird.pipes[i].x, 12, flappyBird.pipes[i].x + 19, flappyBird.pipes[i].gapY, OLED_FILL);
            oled.rect(flappyBird.pipes[i].x, flappyBird.pipes[i].gapY + 20, flappyBird.pipes[i].x + 19, 63, OLED_FILL);
        }
    }
    oled.update();
}

void drawDiceFace(int value, int x, int y, int size) {
    oled.rect(x, y, x + size - 1, y + size - 1, OLED_STROKE);
    int dotSize = 2; int offset = size / 4; 
    int cx = x + size / 2 - dotSize / 2; int cy = y + size / 2 - dotSize / 2;
    int tl_x = x + offset; int tl_y = y + offset; 
    int tr_x = x + size - offset - dotSize; int tr_y = y + offset; 
    int bl_x = x + offset; int bl_y = y + size - offset - dotSize; 
    int br_x = x + size - offset - dotSize; int br_y = y + size - offset - dotSize; 
    int ml_x = x + offset; int ml_y = cy; 
    int mr_x = x + size - offset - dotSize; int mr_y = cy; 
    switch (value) {
        case 1: oled.rect(cx, cy, cx + dotSize, cy + dotSize, OLED_FILL); break;
        case 2: oled.rect(tl_x, tl_y, tl_x + dotSize, tl_y + dotSize, OLED_FILL); oled.rect(br_x, br_y, br_x + dotSize, br_y + dotSize, OLED_FILL); break;
        case 3: oled.rect(tl_x, tl_y, tl_x + dotSize, tl_y + dotSize, OLED_FILL); oled.rect(cx, cy, cx + dotSize, cy + dotSize, OLED_FILL); oled.rect(br_x, br_y, br_x + dotSize, br_y + dotSize, OLED_FILL); break;
        case 4: oled.rect(tl_x, tl_y, tl_x + dotSize, tl_y + dotSize, OLED_FILL); oled.rect(tr_x, tr_y, tr_x + dotSize, tr_y + dotSize, OLED_FILL); oled.rect(bl_x, bl_y, bl_x + dotSize, bl_y + dotSize, OLED_FILL); oled.rect(br_x, br_y, br_x + dotSize, br_y + dotSize, OLED_FILL); break;
        case 5: oled.rect(tl_x, tl_y, tl_x + dotSize, tl_y + dotSize, OLED_FILL); oled.rect(tr_x, tr_y, tr_x + dotSize, tr_y + dotSize, OLED_FILL); oled.rect(bl_x, bl_y, bl_x + dotSize, bl_y + dotSize, OLED_FILL); oled.rect(br_x, br_y, br_x + dotSize, br_y + dotSize, OLED_FILL); oled.rect(cx, cy, cx + dotSize, cy + dotSize, OLED_FILL); break;
        case 6: oled.rect(tl_x, tl_y, tl_x + dotSize, tl_y + dotSize, OLED_FILL); oled.rect(tr_x, tr_y, tr_x + dotSize, tr_y + dotSize, OLED_FILL); oled.rect(ml_x, ml_y, ml_x + dotSize, ml_y + dotSize, OLED_FILL); oled.rect(mr_x, mr_y, mr_x + dotSize, mr_y + dotSize, OLED_FILL); oled.rect(bl_x, bl_y, bl_x + dotSize, bl_y + dotSize, OLED_FILL); oled.rect(br_x, br_y, br_x + dotSize, br_y + dotSize, OLED_FILL); break;
    }
}

void handleDiceGame() {
  static int diceValue = 0;
  static bool rolled = false;
  if (exitBtn.isClick()) { currentState = previousState; rolled = false; return; }
  oled.clear();
  oled.setCursor(0, 0); oled.setScale(1); oled.print("Кубик"); oled.line(0, 10, 127, 10);
  if (selectBtn.isClick()) {
    diceValue = random(1, 7);
    rolled = true;
  }
  if (rolled) {
    drawDiceFace(diceValue, 48, 25, 32); 
  } else {
    oled.setCursor(10, 3); oled.setScale(1); oled.print("Нажмите SELECT");
    oled.setCursor(10, 4); oled.print("для броска.");
  }
  oled.setCursor(0, 7); oled.setScale(1); oled.print("SELECT: бросить");
  oled.update();
}

// --- Функционал читалки ---

int getReaderFilesCount() {
  File root = LittleFS.open("/");
  int count = 0;
  File file = root.openNextFile();
  while (file) {
    String filename = file.name();
    if (filename.endsWith(".txt") || filename.endsWith(".tos")) {
      count++;
    }
    file.close();
    file = root.openNextFile();
  }
  root.close();
  return count;
}

String getReaderFilenameByIndex(int idx) {
  File root = LittleFS.open("/");
  int i = 0;
  File file = root.openNextFile();
  while (file) {
    String filename = file.name();
    if (filename.endsWith(".txt") || filename.endsWith(".tos")) {
      if (i == idx) {
        file.close();
        root.close();
        return filename;
      }
      i++;
    }
    file.close();
    file = root.openNextFile();
  }
  root.close();
  return "";
}

void updateReaderCursor() {
  oled.clear(0, 12, 127, 63); 
  for (uint8_t i = 0; i < 6 && i < readerApp.filesCount; i++) {
    int fileIndex = (readerApp.cursor / 6) * 6 + i;
    if (fileIndex < readerApp.filesCount) {
        oled.setCursor(10, 2 + i);
        String filename = getReaderFilenameByIndex(fileIndex);
        if (filename.startsWith("/")) filename = filename.substring(1);
        oled.print(filename);
    }
  }
  oled.setCursor(0, 2 + (readerApp.cursor % 6));
  oled.print(">");
  oled.update();
}

bool drawReaderFileMenu() {
  oled.clear();
  oled.home();
  oled.print("Читалка: "); oled.print(readerApp.filesCount); oled.print(" файлов");
  oled.line(0, 10, 127, 10);
  if (readerApp.filesCount == 0) {
    oled.setCursor(10, 4);
    oled.print("Файлов нет :(");
    oled.update();
    return false;
  }
  updateReaderCursor();
  return true;
}

bool parseTosFile(uint8_t *img, File &file) {
  int imgLen = 0;
  memset(img, 0, 1024);
  while (file.available()) { if (file.read() == '{') break; }
  while (file.available() && imgLen < 1024) {
    char c = file.read();
    if (c == '}') break;
    if (c == '0' && file.peek() == 'x') {
      file.read();
      char hex[3] = {0};
      hex[0] = file.read();
      hex[1] = file.read();
      img[imgLen++] = strtoul(hex, NULL, 16);
    }
    yield();
  }
  return (imgLen > 0);
}

// ИЗМЕНЕНО: Эта функция заменена на версию с ручным переносом слов из catoslite.cpp для надежности
void drawTextPage(bool storeHistory = true) {
  if (storeHistory) {
    if (readerApp.currentHistoryIndex < readerApp.MAX_PAGE_HISTORY - 1) {
        readerApp.currentHistoryIndex++;
        readerApp.pageHistory[readerApp.currentHistoryIndex] = readerFile.position();
        readerApp.totalPages = readerApp.currentHistoryIndex;
    }
  }
  oled.clear();
  oled.home();
  oled.setScale(1); // Используем стандартный шрифт 8x6 пикселей

  const uint8_t maxCharsPerLine = 21; // 128px / 6px ширина символа
  uint8_t currentLine = 0;

  // Верхняя строка для информации
  oled.setCursor(0, 0);
  oled.print(readerFile.name());
  
  while (readerFile.available() && currentLine < 7) { // 7 строк для текста + 1 для заголовка
    String line = readerFile.readStringUntil('\n');
    line.trim();

    while (line.length() > 0 && currentLine < 7) {
      if (line.length() <= maxCharsPerLine) {
        oled.setCursor(0, currentLine + 1);
        oled.print(line);
        currentLine++;
        line = "";
      } else {
        int breakPoint = -1;
        // Ищем последний пробел в пределах длины строки
        for (int i = maxCharsPerLine; i >= 0; i--) {
          if (line.charAt(i) == ' ') {
            breakPoint = i;
            break;
          }
        }

        String partToPrint;
        if (breakPoint != -1) {
          // Разрываем по пробелу
          partToPrint = line.substring(0, breakPoint);
          line = line.substring(breakPoint + 1);
          line.trim(); 
        } else {
          // Если пробела нет, режем слово
          partToPrint = line.substring(0, maxCharsPerLine);
          line = line.substring(maxCharsPerLine);
        }
        
        oled.setCursor(0, currentLine + 1);
        oled.print(partToPrint);
        currentLine++;
      }
    }
  }
  oled.update();
}

void viewTosFile(String filename) {
    String fullPath = "/" + filename;
    File file = LittleFS.open(fullPath.c_str(), "r");
    if (!file) {
        showMessage("Ошибка файла!");
        delay(1000);
        readerApp.inFileReader = false;
        drawReaderFileMenu();
        return;
    }
    uint8_t *img = new uint8_t[1024];
    if (!parseTosFile(img, file)) {
        delete[] img; file.close(); 
        showMessage("Ошибка .tos");
        delay(1000);
        readerApp.inFileReader = false;
        drawReaderFileMenu();
        return;
    }
    file.close();
    oled.clear();
    oled.drawBitmap(0, 0, img, 128, 64);
    oled.update();
    delete[] img;
}

void initReaderApp() {
  readerApp.cursor = 0;
  readerApp.filesCount = getReaderFilesCount();
  readerApp.inFileReader = false;
  if (!drawReaderFileMenu() && readerApp.filesCount == 0) {
    delay(2000);
    currentState = previousState;
  }
}

void handleReaderApp() {
  if (readerApp.inFileReader) {
    // --- РЕЖИМ ПРОСМОТРА ФАЙЛА ---
    if (exitBtn.isClick()) {
      readerFile.close();
      readerApp.inFileReader = false;
      drawReaderFileMenu();
      return;
    }
    // Для .tos файлов выход по SELECT тоже
    if (selectBtn.isClick() && readerFile.name() && String(readerFile.name()).endsWith(".tos")) {
       readerFile.close();
       readerApp.inFileReader = false;
       drawReaderFileMenu();
       return;
    }

    if (readerFile.name() && String(readerFile.name()).endsWith(".txt")) {
        if (upBtn.isClick() || upBtn.isHold()) {
            if (readerApp.currentHistoryIndex > 0) {
                readerApp.currentHistoryIndex--;
                readerFile.seek(readerApp.pageHistory[readerApp.currentHistoryIndex]);
                drawTextPage(false);
            }
        }
        if (downBtn.isClick() || downBtn.isHold()) {
            if (readerFile.available()) {
                drawTextPage(true);
            }
        }
    }
  } else {
    // --- РЕЖИМ ВЫБОРА ФАЙЛА ---
    if (exitBtn.isClick()) {
        currentState = previousState;
        return;
    }
    static uint32_t timer = 0;
    if (upBtn.isClick() || (upBtn.isHold() && millis() - timer > 150)) {
        if (readerApp.cursor > 0) readerApp.cursor--;
        timer = millis();
        updateReaderCursor();
    }
    if (downBtn.isClick() || (downBtn.isHold() && millis() - timer > 150)) {
        if (readerApp.cursor < readerApp.filesCount - 1) readerApp.cursor++;
        timer = millis();
        updateReaderCursor();
    }
    if (selectBtn.isClick()) {
        String filename = getReaderFilenameByIndex(readerApp.cursor);
        if (filename != "") {
            readerApp.inFileReader = true;
            String fullPath = "/" + filename;
            if (filename.endsWith(".txt")) {
                readerFile = LittleFS.open(fullPath.c_str(), "r");
                if (!readerFile) {
                    readerApp.inFileReader = false;
                    showMessage("Ошибка файла!");
                    delay(1000);
                    drawReaderFileMenu();
                    return;
                }
                memset(readerApp.pageHistory, 0, sizeof(readerApp.pageHistory));
                readerApp.currentHistoryIndex = -1;
                readerApp.totalPages = 0;
                drawTextPage();
            } else if (filename.endsWith(".tos")) {
                 // Для .tos мы просто отображаем и ждем выхода
                 viewTosFile(filename);
            }
        }
    }
  }
}

// --- Конец функционала читалки ---

void drawMenu(const char* title, const char* items[], int itemCount, int currentPage, int totalPages) {
  oled.clear();
  oled.setCursor(0, 0); oled.setScale(1); oled.print(title); oled.line(0, 10, 127, 10);
  int itemsPerPage = (strcmp(title, "Игры") == 0 || strcmp(title, "Приложения") == 0) ? 5 : 4;
  int startIndex = currentPage * itemsPerPage;
  int endIndex = min(startIndex + itemsPerPage, itemCount);
  for (int i = startIndex; i < endIndex; i++) {
    int displayIndex = i - startIndex;
    oled.setCursor(10, 2 + displayIndex); oled.print(items[i]);
    int currentIndex = 0;
    if (strcmp(title, "Меню") == 0) currentIndex = mainMenuState.index;
    else if (strcmp(title, "Настройки") == 0) currentIndex = settingsMenuState.index;
    else if (strcmp(title, "Мини приложения") == 0) currentIndex = miniAppsMenuState.index;
    else if (strcmp(title, "Приложения") == 0) currentIndex = appsMenuState.index;
    else if (strcmp(title, "Игры") == 0) currentIndex = gamesMenuState.index;
    if (displayIndex == currentIndex) { oled.setCursor(0, 2 + displayIndex); oled.print(">"); }
  }
  if (totalPages > 1) { oled.setCursor(100, 0); oled.print("("); oled.print(currentPage + 1); oled.print("/"); oled.print(totalPages); oled.print(")"); }
  oled.update();
}

void showMessage(const char* message) {
  oled.clear(); oled.setCursor(2, 3); oled.setScale(2); oled.print(message); oled.update();
}
